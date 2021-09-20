#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include "ClosedCube_HDC1080.h"
#include <Adafruit_Sensor.h>
#include "Adafruit_TSL2591.h"


Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);
ClosedCube_HDC1080 hdc1080;

#define OLED_RESET -1
Adafruit_SSD1306 display(128, 64, &Wire, -1);

#define LDR 34
#define Moisture 35
#define ledRED 4
#define ledIR 5
#define startBtn 18
#define calBtn 19
int irMax, redMax = 0;
bool LUX, DISP = false;
bool first = true;
uint16_t ir, full, vis;
uint32_t lum;
float iir,ired, cci;

void initLUX(){
  if (tsl.begin())
    LUX = true;
  else{
    Serial.println("[LUX] OLED not Working. Not Found");
    LUX = false;
  }
  tsl.setGain(TSL2591_GAIN_MED);
  tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);
}

void intiTEMP_HUM(){
  hdc1080.begin(0x40);
}

void intiDisplay(){
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
    Serial.print("SSD1306 allocation failed");
    DISP = false;
  }
  else
    DISP = true;
}

void initGPIOS(){
  pinMode(ledRED, OUTPUT);
  pinMode(ledIR, OUTPUT);
  pinMode(startBtn, INPUT);
  pinMode(calBtn, INPUT);
  digitalWrite(ledRED,LOW);
  digitalWrite(ledIR,LOW);
}

void Cal(){
  digitalWrite(ledRED,HIGH);
  delay(500);
  lum = tsl.getFullLuminosity();
  ir = lum >> 16;
  full = lum & 0xFFFF;
  redMax = full - ir;
  digitalWrite(ledRED,LOW);
  digitalWrite(ledIR,HIGH);
  delay(500);
  lum = tsl.getFullLuminosity();
  irMax = lum >> 16;
  digitalWrite(ledIR,LOW);
  if(first)
    first = false;
  delay(2000);
}

void clearDisplay(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
}

void setup(void){
  Serial.begin(115200);
  intiDisplay();
  clearDisplay();
  display.setCursor(0,0);
  display.print("     Plants Health");
  display.setCursor(0,20);
  display.print("      Monitoring");
  display.setCursor(0,40);
  display.print("        System");
  display.display();
  initGPIOS();
  initLUX();
  intiTEMP_HUM();
  Cal();
  delay(5000);
}

void advancedRead(void){
  display.setCursor(0,10);
  display.print("     Measuring CCI");
  display.display();
  digitalWrite(ledRED, HIGH);
  delay(100);
  lum = tsl.getFullLuminosity();
  ir = lum >> 16;
  full = lum & 0xFFFF;
  vis = full - ir;
  digitalWrite(ledRED, LOW);
  digitalWrite(ledIR, HIGH);
  delay(100);
  lum = tsl.getFullLuminosity();
  ir = lum >> 16;
  digitalWrite(ledIR, LOW);
  iir = ((float)ir/irMax)*100;
  ired = ((float)vis/redMax)*100;

  cci = iir/ired;
  
  display.setCursor(0,34);
  display.print("      CCI: ");
  display.print(cci, 4);
  display.display();
  delay(2000);
  clearDisplay();
}

void loop(void) {
  if(DISP){
    clearDisplay();
    if(LUX && !digitalRead(calBtn)){
      while(!digitalRead(calBtn)){
        clearDisplay();
        if(!first){
          display.setCursor(0,10);
          display.print("     Calibrating");
          display.display();
        }
        Cal();
        if(!first){
          display.setCursor(0,30);
          display.print("  Calibration Done");
          display.display();
          delay(500);
          clearDisplay();
        }
      }
    }
    if(LUX && !digitalRead(startBtn)){
      clearDisplay();
      while(!digitalRead(startBtn))
        advancedRead();
        
    }
    else{
      display.setCursor(0,44);
      display.print("Last CCI: ");
      display.print(cci, 4);
    }
    display.setCursor(0,0);
    display.print("T: ");
    display.print(hdc1080.readTemperature());
    display.print((char)247);
    display.print("C H: ");
    display.print(hdc1080.readHumidity(),1);
    display.print("%");
    display.setCursor(0,22);
    display.print("LDR: ");
    display.print(analogRead(LDR));
    display.print(" Moist: ");
    int val = analogRead(Moisture);
    val = map(val,4095,1080,0,100);
    if(val>100)
      val = 100;
    display.print(val);
    display.print("%");
    display.display();
    delay(1000);
  }
}
