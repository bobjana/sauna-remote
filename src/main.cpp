  
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <SimpleTimer.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHTesp.h"

#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
#define OLED_RESET     0 // Reset pin # (or -1 if sharing Arduino reset pin)

Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
SimpleTimer timer;
DHTesp dht;

char auth[] = "ONSxsBUvzdBn5-SES-ZZfEKUuziaaPID";
char ssid[] = "Ziggo8655252";
char pass[] = "nyDgvy7H6smv";

const long THRESHOLD_1 = 1;
const long THRESHOLD_2 = 4;
const long THRESHOLD_3 = 7;
const long DEFAULT_MAX_TEMP = 60;
const long DEFAULT_MAX_TIME = 45;
const long DEFAULT_WARMUP_TIME_IN_MINS = 5;
const long WARMUP_TEMP_THRESHOLD = 40;

const int H1_PIN= D0;
const int H2_PIN= D5;
const int H3_PIN= D6;
const int H4_PIN= D7;
const int TEMP_PIN= D4;
const int BTN_PIN= D8;

int maxTemp = DEFAULT_MAX_TEMP;
long totalTimeInSeconds;
long timeRemainingInSeconds;
long warmUptimeRemainingInSeconds;
String displayTimeRemaining;

float temp = -1;
float humidity = -1;
int statusOn = 1;
boolean warmedUp = false;
boolean maxReached = false;

void resetCounters(){
  totalTimeInSeconds = DEFAULT_MAX_TIME * 60;
  timeRemainingInSeconds = totalTimeInSeconds;
  warmUptimeRemainingInSeconds = 30; //WARMUP_TIME * 60;
  warmedUp = false;
}

void readTemperature()
{
  humidity = dht.getHumidity();
  temp = dht.getTemperature();

  Blynk.virtualWrite(V0, temp);
  Blynk.virtualWrite(V1, humidity); 
}

void switchHeaters(int h1, int h2, int h3, int h4)
 {
  digitalWrite(H1_PIN, h1);
  digitalWrite(H2_PIN, h2);
  digitalWrite(H3_PIN, h3);
  digitalWrite(H4_PIN, h4);
}

void adjustHeaters(){
  int buttonState = digitalRead(BTN_PIN);
  if (buttonState == HIGH) {
    int randTemp = random(50, 70);
    temp = randTemp;
    Serial.println("-----");
    Serial.print("TEMP: ");
    Serial.println(randTemp);
  }

  if (warmedUp && statusOn == 0){
    switchHeaters(0, 0, 0, 0);        
    return;
  }

  if (!maxReached){
      maxReached = temp >= maxTemp;
     switchHeaters(1, 1, 1, 1);        
  }

  if (maxReached && temp > maxTemp)
  {
    long overTemp = temp - maxTemp;
    Serial.print("OVER: ");
    Serial.println(overTemp);
    if (overTemp < THRESHOLD_2)
    {
      switchHeaters(1, 0, 1, 0);
    }
    else
    {
      switchHeaters(0, 0, 0, 0);
    }
  }
  else if (maxReached && temp < maxTemp)
  {
    long underTemp = maxTemp - temp;
    Serial.print("UNDER: ");
    Serial.println(underTemp);
    if (underTemp <= THRESHOLD_1){
      switchHeaters(1, 0, 0, 0);
    }
    else if (underTemp > THRESHOLD_1 && underTemp <= THRESHOLD_2){
      switchHeaters(1, 1, 0, 0);
    }
    else if (underTemp > THRESHOLD_2 && underTemp <= THRESHOLD_3){
      switchHeaters(1, 1, 1, 0);
    }
    else {
       switchHeaters(1, 1, 1, 1);
    }
  }
}

String getDisplayTime(int timeInSeconds){
    int mins = timeInSeconds / 60;
    int secs = timeInSeconds - (mins*60);
    return String(mins) + ":" + String(secs);
}

void updateTimer(){
  if (!warmedUp){
    warmUptimeRemainingInSeconds--;
    displayTimeRemaining = getDisplayTime(warmUptimeRemainingInSeconds);
    Blynk.virtualWrite(V3, "Warming Up...");
    Blynk.virtualWrite(V6, getDisplayTime(totalTimeInSeconds - timeRemainingInSeconds));
  }  
  else if (statusOn == 1 && timeRemainingInSeconds > 0){
    timeRemainingInSeconds--;
    displayTimeRemaining = getDisplayTime(timeRemainingInSeconds);
    Blynk.virtualWrite(V3, getDisplayTime(timeRemainingInSeconds));
    Blynk.virtualWrite(V6, getDisplayTime(totalTimeInSeconds - timeRemainingInSeconds));
  }
}

void refreshDisplay(){
  oled.clearDisplay();

  if (!warmedUp){
    oled.setTextSize(1);
    oled.setTextColor(WHITE);
    oled.setCursor(20,5);
    oled.println("Warming up...");

    //timer   
    oled.setTextSize(3);             
    oled.setCursor(15,43);            
    oled.println(displayTimeRemaining);
  }
  else if (statusOn == 1){
    //temperature  
    oled.setTextColor(WHITE);        
    oled.setTextSize(3);             
    oled.setCursor(5,10);            
    oled.println((int)temp);
    oled.drawCircle(45, 12, 1, WHITE);
    oled.setTextSize(1);             
    oled.setCursor(46,12);            
    oled.println("C");

    //humidity
    oled.setCursor(80,10);            
    oled.setTextSize(3);
    oled.println((int)humidity);
    oled.setTextSize(1);             
    
    oled.setCursor(80,12);            
    oled.drawCircle(120, 12, 1, WHITE);
    oled.setTextSize(1);             
    oled.setCursor(120,12);            
    oled.println("/");
    oled.drawCircle(123, 18, 1, WHITE);

    //timer   
    oled.setTextSize(3);             
    oled.setCursor(15,43);            
    oled.println(displayTimeRemaining);
  }

  oled.display(); 
}

void warmUp(){
  if (!warmedUp){
    warmedUp = warmUptimeRemainingInSeconds <= 0 || temp >= WARMUP_TEMP_THRESHOLD;
    if (warmedUp){
      Serial.println("warmed up..");
      warmUptimeRemainingInSeconds = DEFAULT_WARMUP_TIME_IN_MINS * 60;
      Blynk.virtualWrite(V4, statusOn);
    }
  }
}

void shutDown(){
  if (timeRemainingInSeconds <= 0){
      statusOn = 0;
      Serial.println("shutdown...");
      Blynk.virtualWrite(V4, statusOn);
  }
}

void setup()
{
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);

  dht.setup(TEMP_PIN, DHTesp::DHT11);

  pinMode(H1_PIN, OUTPUT);
  pinMode(H2_PIN, OUTPUT);
  pinMode(H3_PIN, OUTPUT);
  pinMode(H4_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT);

  if(!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); 
  }

  resetCounters();
  refreshDisplay();

  timer.setInterval(2000, readTemperature);
  timer.setInterval(1000, refreshDisplay);
  timer.setInterval(1000, updateTimer);
  timer.setInterval(10000, adjustHeaters);
  timer.setInterval(500, warmUp);
  timer.setInterval(1000, shutDown);
}

void loop()
{
  Blynk.run();
  timer.run();
}

BLYNK_WRITE(V2)
{
  maxTemp = param.asInt(); 
}

BLYNK_WRITE(V4)
{
  statusOn = param.asInt(); 
  Serial.println("off: " + statusOn);
  if (statusOn == 0){
    timeRemainingInSeconds = 0;
  }
  else {
    resetCounters();
  }
}

BLYNK_WRITE(V5)
{
  timeRemainingInSeconds =  param.asInt() * 60;
  totalTimeInSeconds = timeRemainingInSeconds;
}

BLYNK_CONNECTED() {
  Blynk.virtualWrite(V5, DEFAULT_MAX_TIME);
  Blynk.virtualWrite(V2, maxTemp);
  Blynk.virtualWrite(V4, statusOn);
}

