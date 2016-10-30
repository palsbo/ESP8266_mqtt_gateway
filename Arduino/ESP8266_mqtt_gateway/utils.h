#include <FS.h>

String hex(byte h) {
  String res;
  char lit[] = "0123456789ABCDEF";
  res += lit[h >> 4];
  res += lit[h & 15];
  return res;
}

String float2string(float value, int width, int dec) {
  char buf[width+1];
  dtostrf(value, width, dec, buf);
  return String(buf);
}

String readFile(String fn, String defaultval) {
  String val = defaultval;
  File topicfile = SPIFFS.open(fn, "r");
  if (topicfile) {
    val = topicfile.readString();
    val.trim();
    topicfile.close();
  }
  return val;
}

int readFile(String fn, int defaultval){
  String val = (String)defaultval;
  val = readFile(fn, val);
  return val.toInt();
}

void readFile(String fn, char * result, int len) {
  File topicfile = SPIFFS.open(fn, "r");
  if (topicfile) {
    String line = topicfile.readString();
    line.trim();
    memcpy(result, &line[0], line.length());
    for (int i = line.length(); i < 20; i++) result[i] = 0;
    topicfile.close();
  }
}

void writeFile(String fn, String val) {
  File topicfile = SPIFFS.open(fn, "w");
  topicfile.println(val);
  topicfile.close();
}

