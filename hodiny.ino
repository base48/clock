#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Adafruit_NeoPixel.h>
#include <ezTime.h>
#include <ArduinoJson.h>


#define LED_PIN    D6
#define LED_COUNT (14 * 4 + 2)
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

const char *ssid     = "Base48-2";
const char *password = "";

const long utcOffsetInSeconds = 3600 * 2;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

Timezone tz;
WiFiClient client;
HTTPClient http, httpb;


void setup(){
  Serial.begin(115200);

  WiFi.begin(ssid, password);

  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }

  waitForSync();
  tz.setLocation("Europe/Prague");
  strip.begin();
  //strip.show(); // Initialize all pixels to 'off'


}
const int digitOffset[] = {14 * 3 + 2, 14 * 2 + 2, 14, 0};
const int digits[][16] = {
  {2,3,4,5,6,7,8,9,10,11,12,13,-1},
  {6,7,8,9,-1},
  {0,1,2,3,4,5,8,9,10,11,-1},
  {0,1,4,5,6,7,8,9,10,11,-1},
  {0,1,6,7,8,9,12,13,-1},
  {0,1,4,5,6,7,10,11,12,13,-1},
  {0,1,2,3,4,5,6,7,10,11,12,13,-1},
  {6,7,8,9,10,11,-1},
  {0,1,2,3,4,5,6,7,8,9,10,11,12,13,-1},
  {0,1,4,5,6,7,8,9,10,11,12,13-1}
};

int intensity = 64;

void showNumber(int offset, int number) {
   int i = 0;
   while (digits[number][i] != -1) {
      strip.setPixelColor(digitOffset[offset] + digits[number][i], 0, intensity, 0);
      i++;
   }
}
void showDots() {
      strip.setPixelColor(14 * 2, 0, intensity, 0);
      strip.setPixelColor(14 * 2 + 1, 0, intensity, 0);
}

bool getIntensity(){
  String server = "http://mozajk:10000/";
  Serial.println(server);
  http.begin(client, server.c_str());
  int r = http.GET();
  if (r > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(r);
    String load = http.getString();
    Serial.println(load);
    const int jsonSize = JSON_OBJECT_SIZE(4);
    DynamicJsonDocument doc(jsonSize);
    deserializeJson(doc, load);
    int op = doc["illuminance"];
    Serial.println(op);
    if (op == 0) {
      http.end();
      return false;
    }
  } else {
    Serial.print("Errior code: ");
    Serial.println(r);
    return true;
  }
  http.end();
  return true;  
}

int lastS = 0;
int loopCount = 0;

void loop() {
  events();
  Serial.println(tz.dateTime());

  int n = tz.now();
  int h = hour(n);
  int m = minute(n);
  int s = second(n);

strip.clear();

if (s != lastS) {
   showDots();
}
lastS = s;
if (loopCount++ % 10 == 0) {
  if (getIntensity()) {
    intensity = 64;
  } else {
    intensity = 8;
  }
}
showNumber(0, h / 10);
showNumber(1, h % 10);
showNumber(2, m / 10);
showNumber(3, m % 10);
strip.show();

  delay(500);
}
