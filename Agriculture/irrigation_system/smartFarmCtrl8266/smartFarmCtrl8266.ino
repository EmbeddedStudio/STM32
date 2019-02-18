#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
const char *ssid     = "s008-flying";
const char *password = "dianxin132";

String water1Url = "http://117.21.178.99:8098/index.php/Admin/Devdata/ajax_set_control?id=1&str_v=";
String windUrl = "http://117.21.178.99:8098/index.php/Admin/Devdata/ajax_set_control?id=3&str_v=";
String temUrl = "http://117.21.178.99:8098/index.php/Admin/Devdata/ajax_set_control?id=2&str_v=";
String receiveUrl = "http://117.21.178.99:8098/index.php/Admin/Devdata/ajax_get_latestdata?sel_dev=2";
String cookie = "IOTADMIN=6ea35119r4rfvb04s8vggc4km1";

HTTPClient http;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

}
unsigned long lastUpdateTime = 0;
void loop() {
  // put your main code here, to run repeatedly:
  while (Serial.available()) {
    String s = Serial.readStringUntil('*');
    int pos = s.indexOf('_');
    String msgFirst = s.substring(0, pos); // eg:"wind"
    String msgSecond = s.substring(pos + 1, s.length()); // eg:"1"
    if (msgFirst == "wind") {
      request(msgSecond, windUrl);
    } else if (msgFirst == "water1") {
      request(msgSecond, water1Url);
    } else if (msgFirst == "tem") {
      request(msgSecond, temUrl);
    }
  }

  if (lastUpdateTime == 0 || millis() - lastUpdateTime > 3000) {
    lastUpdateTime = millis();
    receivceFromServer();
  }
}
void receivceFromServer() {
  http.begin(receiveUrl);
  http.addHeader("Cookie", cookie);
  int httpCode = http.GET();
  if (httpCode = HTTP_CODE_OK) {
    String payload = http.getString();
    //    payload = "{\"contents\":"+payload+"}";
    //    Serial.println(payload);
    int locations[50];
    int j = 0;

    for (int i = 0; i < payload.length(); i++) {
      if (payload[i] == '\"')
        locations[j++] = i;
    }
    String tem = payload.substring(locations[6] + 1, locations[7]);
    String hum = payload.substring(locations[14] + 1, locations[15]);
    String light = payload.substring(locations[38] + 1, locations[39]);
    //    Serial.println("tem:"+tem);
    //    Serial.println("hum:"+hum);
    //    Serial.println("light:"+light);
    int temDegree = workOutTemDegree(tem.toFloat());
    int humDegree = workOutHumDegree(hum.toFloat());
    int lightDegree = workOutLightDegree(light.toFloat());
    Serial.print("z0.val=" + String(temDegree));
    Serial.write(0xFF); Serial.write(0xFF); Serial.write(0xFF);
    Serial.print("z1.val=" + String(humDegree));
    Serial.write(0xFF); Serial.write(0xFF); Serial.write(0xFF);
    Serial.print("z2.val=" + String(lightDegree));
    Serial.write(0xFF); Serial.write(0xFF); Serial.write(0xFF);
    Serial.print("t1.txt=\"" + tem + "\"");
    Serial.write(0xFF); Serial.write(0xFF); Serial.write(0xFF);
    Serial.print("t2.txt=\"" + hum + "\"");
    Serial.write(0xFF); Serial.write(0xFF); Serial.write(0xFF);
    Serial.print("t3.txt=\"" + light + "\"");
    Serial.write(0xFF); Serial.write(0xFF); Serial.write(0xFF);
    
  } else {
    Serial.println("receive error:" + String(httpCode));

  }
  http.end();
}
void request(String v, String url) {
  http.begin(url + v);
  http.addHeader("Cookie", cookie);
  int httpCode = http.GET();
  http.end();
}

int workOutTemDegree(float v) {
  if (v > 0)
    return v * 4.5;
  else
    return 360 - v * 4.5;
}

int workOutHumDegree(float v) {
  if (v < 20)
    return 360 + (v - 20) * 2;
  else if (v > 20 && v < 80)
    return (v - 20) * 3;
  else
    return (v - 80) * 2 + 180;
}

int workOutLightDegree(float v) {
  if (v < 200)
    return 360 + (v - 200) * 0.2;
  else if (v > 200 && v < 800)
    return (v - 200) * 0.3;
  else
    return (v - 800) * 0.2 + 180;

}

