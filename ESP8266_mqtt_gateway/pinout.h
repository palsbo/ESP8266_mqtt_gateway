
typedef void (*pinOutCallback0)();
typedef void (*pinOutCallback1)(String val);

class PINOUT {
  private:
    pinOutCallback1 cb0;
    pinOutCallback0 cb1;
    String oldValue;
    int pin;
  public:
    String name;
    String topic;
    String value;
    void begin(String _name, String _topic, int _pin) {
      name = _name;
      pin = _pin;
      topic = _topic;
      pinMode(pin, OUTPUT);
      digitalWrite(pin, HIGH);
    }
    void onReceive(pinOutCallback1 _cb0) {
      cb0 = _cb0;
    }
    void onChange(pinOutCallback0 _cb1) {
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
      get();
      if (value != oldValue) {
        if (cb1) {
          cb1();
          oldValue = value;
        }
      }
    }
    void set(String val) {
      digitalWrite(pin, val == "ON" ? LOW : HIGH);
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
      ret += "<td><button onclick=\"document.location='/?" + name + "=' + getElementById('" + name + "').value + '&dev=pinout'\">Update</button></td>\n<";
      ret += "/tr>\n";
      return ret;
    }
};


