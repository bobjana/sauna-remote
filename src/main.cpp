  
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <SimpleTimer.h>

#include "DHTesp.h"

#ifdef ESP32
#pragma message(THIS EXAMPLE IS FOR ESP8266 ONLY!)
#error Select ESP8266 board.
#endif

SimpleTimer timer;
char auth[] = "ONSxsBUvzdBn5-SES-ZZfEKUuziaaPID";
char ssid[] = "Ziggo8655252";
char pass[] = "nyDgvy7H6smv";
float t;
float h;

DHTesp dht;

void sendUptime()
{
  float h = dht.getHumidity();
  float t = dht.getTemperature();
  Serial.println("Humidity and temperature\n\n");
  Serial.print("Current humidity = ");
  Serial.print(h);
  Serial.print("%  ");
  Serial.print("temperature = ");
  Serial.print(t); 
  Blynk.virtualWrite(V0, t);
  Blynk.virtualWrite(V1, h); 
}

void setup()
{
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);
  dht.setup(D4, DHTesp::DHT11);
  timer.setInterval(2000, sendUptime);
  Serial.println();
  Serial.println("Status\tHumidity (%)\tTemperature (C)\t(F)\tHeatIndex (C)\t(F)");
}

void loop()
{
  Blynk.run();
  timer.run();
}