#include <ESP8266WiFi.h>
#include<Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Adafruit_HTU21DF.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

Adafruit_SSD1306 display(128, 64, & Wire, -1);
Adafruit_HTU21DF htu = Adafruit_HTU21DF();
WiFiClient client;

// replace with your channel’s thingspeak API key and your SSID and password
String apiKey = "SGEZ7C285SMF16KY";
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* server = "api.thingspeak.com";

void setup()
{
  Serial.begin(115200);
  Wire.begin();
  delay(10);
  WiFi.begin(ssid, password);

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  //Checking the BMP Sensor
  if (!htu.begin()) // For BMP 180.
  {
    Serial.println("BMP180 sensor not found");
    while (1) {}
  }

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
   }
  delay(2000);
  display.clearDisplay();
  display.setTextColor(WHITE);
}

void loop(){
    float t = htu.readTemperature();
    float h = htu.readHumidity();
    if (isnan(t) || isnan(h))
    {
      Serial.println("Failed to read from HTU21 sensor!");
      return;
    }

    if (client.connect(server, 80)) {
      String postStr = apiKey;
      postStr += "&field1=";
      postStr += String(t);
      postStr += "&field2=";
      postStr += String(h);
      postStr += "\r\n\r\n"; // Insert this to return to original

      client.print("POST /update HTTP/1.1\n");
      client.print("Host: api.thingspeak.com\n");
      client.print("Connection: close\n");
      client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
      client.print("Content-Type: application/x-www-form-urlencoded\n");
      client.print("Content-Length: ");
      client.print(postStr.length());
      client.print("\n\n");
      client.print(postStr);

      Serial.print("Temperature: ");
      Serial.print(t);
      Serial.print(" degrees Celsius, Humidity (%): ");
      Serial.print(h);
      Serial.println("Sending data to Thingspeak");

      //display on OLED LCD
      display.clearDisplay();

      // display temperature
      display.setTextSize(1);
      display.setCursor(0,0);
      display.print("Temperature: ");
      display.setTextSize(2);
      display.setCursor(0,10);
      display.print(t);
      display.print(" ");
      display.setTextSize(1);
      display.cp437(true);
      display.write(167);
      display.setTextSize(2);
      display.print("C");
      
      // display humidity
      display.setTextSize(1);
      display.setCursor(0, 35);
      display.print("Humidity: ");
      display.setTextSize(2);
      display.setCursor(0, 45);
      display.print(h);
      display.print(" %"); 
      
      display.display(); 
     
    }

    client.stop();

    Serial.println("Waiting 5 minutes");
    // thingspeak needs at least a 15 sec delay between updates
    // 20 seconds to be safe
    delay(20000);
}
