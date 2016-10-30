#include <Wire.h>

#define MAXI2CDEV 10
#define I2C_TIME (millis()-i2cTimer)
#define I2C_RESET i2cTimer = millis()
#define SRF_TIME (millis()-srfTimer)
#define SRF_RESET srfTimer = millis()

class TANK {  //  tank
  private:
    int lastRange;
    float radius = 32.5;
    float length = 150.0;
    float height = 137.0;
    int offset = 19;
    int volume;
    int range;
    int level;
    float min(float a, float b) {
      if (a < b) return a;
      return b;
    }
    float max(float a, float b) {
      if (a > b)return a;
      return b;
    }
    float area(float height, float radius) {
      float v = acos(1 - height / radius) * 2;  // i radianer
      float retval = radius * radius / 2 * (v - sin(v));
      return (retval);
    }
  public:
    int calc(int newRange) {
      int maxrange = (int)height + offset;
      if (newRange > maxrange) return 0;
      if ((newRange < lastRange * 1.2) && (newRange > lastRange * .8)) {
        range = (newRange);
      }
      lastRange = newRange;
      level = height + offset - range;
      float a =
        area(min(level, radius), radius) +
        max(min(level - radius, height - 2 * radius), 0) * 2 * radius +
        area(radius, radius) - area(min((height - level), radius), radius);
      return round(a * length / 1000);
    }
} tank;

class I2C_DEV {
  public:
    byte addr;
    String name;
    String fn;
    String type;
    String topic;
    float value;
};

typedef void (*i2ccallback)(I2C_DEV * unit);

class I2C {
  private:
    i2ccallback cb0;
    i2ccallback cb1;
    long srfTimer;
    long i2cTimer;
    bool mode = true;
    int reading = -999;
    void SRF02cmd() {
      Wire.beginTransmission(0x70); // transmit to device #112 (0x70)
      Wire.write(byte(0x00));      // sets register pointer to the command register (0x00)
      Wire.write(byte(0x51));      // command sensor to measure in "cn" (0x50)
      Wire.endTransmission();      // stop transmitting
    }
    int SRF02data() {
      int res;
      Wire.beginTransmission(112); // transmit to device #112
      Wire.write(byte(0x02));      // sets register pointer to echo #1 register (0x02)
      Wire.endTransmission();      // stop transmitting
      Wire.requestFrom(112, 2);    // request 2 bytes from slave device #112
      if (2 <= Wire.available()) { // if two bytes were received
        res = Wire.read();  // receive high byte (overwrites previous reading)
        res = res << 8;    // shift high byte to be high 8 bits
        res |= Wire.read(); // receive low byte as lower 8 bits
      }
      return res;
    }
    int getSRF02() {
      if ((millis() - srfTimer) > 100) {
        srfTimer = millis();
        if (mode) {
          SRF02cmd();
          if (reading < 0) {
            delay(70);
            mode = !mode;
            reading = SRF02data();
          }
        } else {
          reading = SRF02data();
        }
        mode = !mode;
      }
      return reading;   // return the reading
    }
  public:
    void onChange(i2ccallback x_cb) {
      cb0 = x_cb;
    };
    void onFound(i2ccallback x_cb) {
      cb1 = x_cb;
    }
  public:
    void begin() {
#if ESP01
      Wire.pins(0, 2);
#endif
      Wire.begin();
      Wire.setClockStretchLimit(1500);    // in µs
      discover();
    }
    I2C_DEV unit[MAXI2CDEV];
    int devCount = 0;
    void discover(void) {
      devCount = 0;
      byte err, addr;
      for (addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        err = Wire.endTransmission();
        if ((err == 0) || (err == 4)) {
          unit[devCount].addr = addr;
          unit[devCount].fn = "i2c_" + hex(addr);
          unit[devCount].topic = readFile(unit[devCount].fn, "ha/xx");
          unit[devCount].type = readFile(unit[devCount].fn + "_type", "");
          if (cb1) cb1(&unit[devCount]);
          devCount++;
          if (devCount >= MAXI2CDEV) return;
        }
      }
    }
    I2C_DEV * getByAddr(String addr) {
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
    void check() {
    }
    void handle() {
      if ((millis() - i2cTimer) > 500) {
        i2cTimer = millis();
        for (int i = 0; i < devCount; i++) {
          float val;
          bool valid = true;
          if (unit[i].type == "1") { //  SRF02
            val = getSRF02() * 1.0;
          } else if (unit[i].type == "2") { //  Tank
            val = tank.calc(getSRF02()) * 1.0;
            if (val > 1250) valid = false;
          } else return;
          if (valid) if (val != unit[i].value) {
              unit[i].value = val;
              if (cb0) cb0(&unit[i]);
            }
        }
      }
    }
    String html() {
      String options = "<option>Undefined</option><option>SRF02</option><option>Tank</option><option>Tank</option><option>HIH-6130/6131</option><opton>HTU21D</option>";
      String list = "<tr><th class='hdr' colspan='5'>I2C Devices:</th></tr>";
      list += "<tr><th class='left'Name:</th><th class='left'>Type:</th><th class='right' width='40px'>Value:</th><th class='left'></th></tr>\n";
      for (int i = 0; i < devCount; i++) {
        I2C_DEV x = unit[i];
        list += "<tr><th class='left'>" + x.fn + "</td>\n";
        list += "<td><select id=\"" + x.fn + "_type\" onchange=\"document.location='?" + x.fn + "_type=' + this.selectedIndex + '&dev=i2c'\">";
        list += (options + "</select>");
        list += "<script>document.getElementById(\"" + x.fn + "_type\").selectedIndex = \"" + x.type + "\";</script></td>";
        list += "<td class='right'>" + float2string(x.value, 4, 0) + "</td>\n";
        list += "<td><input type='text' id='" + x.fn + "' value='" + x.topic + "'/></td>\n";
        list += "<td><button onclick=\"";
        list += "document.location='/?" + x.fn + "=' + getElementById('" + x.fn + "').value + '&dev=i2c'";
        list += "\">update</button></td>\n";
        list += "</tr>\n";
      }
      return list;
    }
};

