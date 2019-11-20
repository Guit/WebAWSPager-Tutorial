#include <Arduino.h>
#include <LiquidCrystal.h>
const int rs = 23, en = 22, d4 = 5, d5 = 18 , d6 = 19, d7 = 21;
LiquidCrystal lcd(rs,en,d4,d5,d6,d7);

void setup() {
   lcd.begin(16, 2);
    lcd.clear();
    lcd.print("Hello");
    lcd.setCursor(0,1); 
    lcd.print ("WORLD"); 
}

void loop() {
  // put your main code here, to run repeatedly:
}