  
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

const int H1_PIN= D0;
const int H2_PIN= D5;
const int H3_PIN= D6;
const int H4_PIN= D7;
const int TEMP_PIN= D4;
const int BTN_PIN= D8;

long TEMP_MAX = 60;
int buttonState = 0;  
long randTemp = 0;
int totalTimeInMinutes = 10;
long startMillis = -1;
float temp = -1;
float humidity = -1;
boolean off = true;
boolean maxReached = false;


void readTemperature()
{
  humidity = dht.getHumidity();
  temp = dht.getTemperature();

  Blynk.virtualWrite(V0, temp);
  Blynk.virtualWrite(V1, humidity); 

  off = false;
}

void switchHeaters(int h1, int h2, int h3, int h4)
 {
  digitalWrite(H1_PIN, h1);
  digitalWrite(H2_PIN, h2);
  digitalWrite(H3_PIN, h3);
  digitalWrite(H4_PIN, h4);
}

void adjustHeaters(){
  buttonState = digitalRead(BTN_PIN);
  if (buttonState == HIGH) {
    randTemp = random(50, 70);
    Serial.println("-----");
    Serial.print("TEMP: ");
    Serial.println(randTemp);
  }

  if (!maxReached){
      maxReached = randTemp >= TEMP_MAX;
     switchHeaters(1, 1, 1, 1);        
  }

  if (maxReached && randTemp > TEMP_MAX)
  {
    long overTemp = randTemp - TEMP_MAX;
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
  else if (maxReached && randTemp < TEMP_MAX)
  {
    long underTemp = TEMP_MAX - randTemp;
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


void updateTimer(){

}

void refreshDisplay(){
  oled.clearDisplay();

  //initializing
  if (off){
    oled.setTextSize(1);
    oled.setTextColor(WHITE);
    oled.setCursor(20,25);
    oled.println("Intializing...");
    oled.display(); 
  }
  else{
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
    oled.setTextColor(INVERSE);        
    oled.setTextSize(3);             
    oled.setCursor(15,43);            
    oled.println("12:30");

    oled.display(); 
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
  refreshDisplay();

  timer.setInterval(2000, readTemperature);
  timer.setInterval(1000, refreshDisplay);
  // timer.setInterval(1000, updateTimer);
  // timer.setInterval(10000, adjustHeaters);

}

void loop()
{
  Blynk.run();
  timer.run();
}
