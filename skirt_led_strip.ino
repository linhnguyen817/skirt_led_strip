#include "FastLED.h"
#include <WiFi.h>     //Connect to WiFi Network
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define PIN 2
#define NUM_LEDS 30
#define NUM_COLOR_CODE_LENGTH 7
#define FETCH_PERIOD 60000

char network[] = "MIT GUEST"; //SSID for MIT wifi
//char network[] = "Joe's Hotspot"; // SSID for home wifi
//char pass[] = "in$ertpa$tdifference";

const char* api_url = "https://light-fiber-gui.herokuapp.com/led_design";
unsigned long last_fetch_time;

CRGB leds[NUM_LEDS];
char ledColors[NUM_LEDS][NUM_COLOR_CODE_LENGTH];

void setup() {
  Serial.begin(9600);
  FastLED.addLeds<WS2811, PIN, GRB>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  
  WiFi.begin(network); //attempt to connect to MIT wifi
//  WiFi.begin(network, pass); //attempt to connect to home wifi

  uint8_t count = 0;   //count used for Wifi check times
  Serial.print("Attempting to connect to ");
  Serial.println(network);
  while (WiFi.status() != WL_CONNECTED && count < 12)
  {
    delay(500);
    Serial.print(".");
    count++;
  }
  delay(2000);
  if (WiFi.isConnected())
  { //if we connected then print our IP, Mac, and SSID we"re on
    Serial.println("CONNECTED!");
    Serial.println(WiFi.localIP().toString() + " (" + WiFi.macAddress() + ") (" + WiFi.SSID() + ")");
    delay(500);
  }
  else
  { //if we failed to connect just Try again.
    Serial.println("Failed to Connect :/  Going to restart");
    Serial.println(WiFi.status());
    ESP.restart(); // restart the ESP (proper way)
  }
  get_led_design();
  last_fetch_time = millis();
}

void loop() {
  FastLED.show();

  // every 1 min, fetch LED design data
  if (millis() - last_fetch_time >= FETCH_PERIOD)
  {
    get_led_design();
    last_fetch_time = millis();
  }
}

void get_led_design()
{
  const String response = httpGETRequest(api_url);
  Serial.println(response);

  StaticJsonBuffer<2000> jsonBuffer;
  JsonObject& responseJson = jsonBuffer.parseObject(response);
  
  // Test if parsing succeeds.
  if (!responseJson.success()) {
    Serial.println("parseObject() failed");
    return;
  }

  // store fetched color data in ledColors
  for(int i = 0; i < NUM_LEDS; i++){
    for (int j = 1; j < NUM_COLOR_CODE_LENGTH; j++) {  // skip over '#' character
        const char* color = responseJson["ledColors"][i];
        const char test = color[j];
        ledColors[i][j-1] = test;
    }
  }

  // convert color string data into color code for LED strip
  for(int i = 0; i < NUM_LEDS; i++){
    String colorString = "0x";
    for (int j = 0; j < NUM_COLOR_CODE_LENGTH-1; j++){
      colorString += ledColors[i][j];
    }
    long colorCode = strtol(colorString.c_str(), NULL, 0);
    leds[i] = colorCode;
  }
}

String httpGETRequest(const char* serverName) {
  Serial.println("Fetching data...");
  HTTPClient http;

  // Your IP address with path or Domain name with URL path
  http.begin(serverName);

  // Send HTTP POST request
  int httpResponseCode = http.GET();

  String payload = "{}";

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}
