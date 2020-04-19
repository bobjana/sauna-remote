  
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

int red_light_pin= D6;
int green_light_pin = D5;
int blue_light_pin = D0;

DHTesp dht;

void RGB_color(int red_light_value, int green_light_value, int blue_light_value)
 {
  analogWrite(red_light_pin, red_light_value);
  analogWrite(green_light_pin, green_light_value);
  analogWrite(blue_light_pin, blue_light_value);
}

void sendUptime()
{
  RGB_color(255, 0, 0);

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

  pinMode(red_light_pin, OUTPUT);
  pinMode(green_light_pin, OUTPUT);
  pinMode(blue_light_pin, OUTPUT);

  Serial.println();
  Serial.println("Status\tHumidity (%)\tTemperature (C)\t(F)\tHeatIndex (C)\t(F)");
}

void loop()
{
  Blynk.run();
  timer.run();
}

