
typedef void (*rfCallback0)();
typedef void (*rfCallback1)(String val);
#include <RCSwitch.h>

class RF_DEV {
  public:
    String name;
    String topiOn;
    String topicOff;
};

class RF {
  private:
    rfCallback1 cb0;
    rfCallback1 cb1;
    String oldValue;
    RCSwitch mySwitch = RCSwitch();
    int pin;
    String devs[4][4] = {{"Lamper Hems", "83029", "23028", ""},
      {"Lamper Stuen", "86101", "86100", ""},
      {"Lys Spisebord", "70741", "70740", ""},
      {"Ubrugt", "21589", "21588", ""}
    };
  public:
    String name;
    String topic;
    String value;
    void begin(String _name, String _topic, int tx_pin, int rx_pin) {
      name = _name;
      topic = _topic;
      mySwitch.enableTransmit(tx_pin);
      mySwitch.enableReceive(rx_pin);  // Receiver on interrupt 0 => that is pin #2
    }
    void onReceive(rfCallback1 _cb0) {
      cb0 = _cb0;
    }
    void onChange(rfCallback1 _cb1) {
      cb1 = _cb1;
    }
    void check(char* _topic, char* _payload) {  //  check for input
      String tpic = String((char*)_topic);
      String value = String((char*)_payload);
      if (topic == tpic) {
        if (cb0) cb0(value);
      }
    }
    void handle() { //  check for value changed
      String value;
      if (mySwitch.available()) {
        value = mySwitch.getReceivedValue();
        //Serial.println(mySwitch.getReceivedValue());
        //output(mySwitch.getReceivedValue(), mySwitch.getReceivedBitlength(), mySwitch.getReceivedDelay(), mySwitch.getReceivedRawdata(), mySwitch.getReceivedProtocol());
        mySwitch.resetAvailable();
        if (cb1) cb1(value);
        //Serial.println(value);
      }
      /*
      return
        get();
      if (value != oldValue) {
        if (cb1) {
          cb1();
          oldValue = value;
        }
      }
      */
    }
    void set(String val) {
      Serial.println(val);
      for (int i = 0; i < 3; i++)
        mySwitch.send(val.toInt(), 24);
      delay(20);
    }
    void get() {
      //Serial.println("Get Lys");
      value = (digitalRead(pin) ? "OFF" : "ON");
    }
    String html() {
      String ret = "<tr><th class='hdr' colspan='5'>Switch Devices:<th></tr>";
      ret += "<tr><th class='left'>Name:</th><th class='left'>Type:</th><th class='right' width='40px'>Value:</th><th class='left'></th></tr>\n";
      ret += "<tr>\n";
      ret += "<th class=\"left\">" + name + ":</th>\n";
      ret += "<th colspan=2 class=\"right\">Topic:</th>\n";
      ret += "<td><input id=\"" + name + "\" value=\"" + topic + "\"/></td>\n";
      ret += "<td><button onclick=\"document.location='/?" + name + "=' + getElementById('" + name + "').value + '&dev=rf433'\">Update</button></td>\n<";
      ret += "/tr>\n";
      return ret;
    }
};

