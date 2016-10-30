#include <OneWire.h>

#define owPin 2  // Connect your 1-wire device to pin 2


#define MAXOWDEV 10
#define OW_TIME (millis()-owTimer)
#define OW_RESET owTimer = millis()

OneWire  ds(owPin);

class DEV {
  public:
    byte addr[8];
    String fn;
    String chip;
    float value;
    String topic;
};

typedef void (*owcallback)(DEV * unit);

class OW {
  private:
    long owTimer;
    owcallback cb0;
    owcallback cb1;
  public:
    void onChange(owcallback x_cb) {
      cb0 = x_cb;
    };
    void onFound(owcallback x_cb) {
      cb1 = x_cb;
    }
    DEV unit[MAXOWDEV];
    int devCount = 0;
    void discover(void) {
      devCount = 0;
      while (ds.search(unit[devCount].addr)) {
        if ( OneWire::crc8( unit[devCount].addr, 7) != unit[devCount].addr[7]) return;
        String val = "0123456789ABCDEF";
        for ( int i = 0; i < 8; i++) {
          unit[devCount].fn += val[unit[devCount].addr[i] >> 4];
          unit[devCount].fn += val[unit[devCount].addr[i] & 0x0F];
        }
        switch (unit[devCount].addr[0]) {
          case 0x10: unit[devCount].chip = "DS18S20"; break;
          case 0x28: unit[devCount].chip = "DS18B20"; break;
          case 0x22:  unit[devCount].chip = "DS1822"; break;
          default: unit[devCount].chip = "Unknown"; return;
        }
        unit[devCount].topic = readFile(unit[devCount].fn, "");
        if (cb1) cb1(&unit[devCount]);
        devCount += 1;
      }
      ds.reset_search();
    }
    DEV * getByAddr(String addr) {
      for (int i = 0; i < devCount; i++) {
        if (unit[i].fn == addr) return &unit[i];
      }
      return NULL;
    }
    int getIndex(String fn) {
      for (int i = 0; i < devCount; i++) {
        if (unit[i].fn == fn) return i;
      }
      return -1;
    }
    void handle() {
      byte i;
      byte present = 0;
      byte type_s;
      byte data[12];
      byte addr[8];
      String chip;
      float celsius, fahrenheit;
      if (OW_TIME > 1000) {
        OW_RESET;
        for (int j = 0; j < devCount; j++) {
          memcpy(&addr, &unit[j].addr, 8);
          present = ds.reset();
          ds.select(addr);
          ds.write(0xBE);         // Read Scratchpad
          for ( i = 0; i < 9; i++) data[i] = ds.read();
          int16_t raw = (data[1] << 8) | data[0];
          switch (unit[j].addr[0]) {
            case 0x10:
              raw = raw << 3; // 9 bit resolution default
              if (data[7] == 0x10) {
                raw = (raw & 0xFFF0) + 12 - data[6];
              }
              break;
            case 0x22:
            case 0x28:
              byte cfg = (data[4] & 0x60);
              if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
              else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
              else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
              break;
          }
          float val = (float)raw / 16.0;
          if (val != unit[j].value) {
            unit[j].value = val;
            if (cb0) cb0(&unit[j]);
          }
          ds.reset();
          ds.select(addr);
          ds.write(0x44, 0);        // start conversion, with parasite power on at the end
        }
        OW_RESET;
      }
    }
    String html() {
      String list = "<tr><th class='hdr' colspan='5'>Onewire Devices:<th></tr>";
      list += "<tr><th class='left'Name:</th><th class='left'>Type:</th><th class='right' width='40px'>Value:</th><th class='left'></th></tr>\n";
      String ad;
      String to;
      String ch;
      String va;
      for (int i = 0; i < devCount; i++) {
        switch (unit[i].addr[0]) {
          case 0x10:
          case 0x22:
          case 0x28:
            ad = unit[i].fn;
            to = String(unit[i].topic);
            ch = unit[i].chip;
            va = String(unit[i].value);
            list += "<tr><td>" + ad + "</td>\n";
            list += "<td>" + ch + "</td>\n";
            list += "<td>" + va + "</td>\n";
            list += "<td><input type='text' id='" + ad + "' value='" + to + "'/></td>\n";
            list += "<td><button onclick=\"";
            list += "document.location='/?" + ad + "=' + getElementById('" + ad + "').value + '&dev=ow'";
            list += "\">update</button></td>\n";
            list += "</tr>\n";
            break;
          default:
            break;
        }
      }
      return list;
    }
};


