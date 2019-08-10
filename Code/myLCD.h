#ifndef  _mylcd_h_
#define _mylcd_h_ 
#include "Arduino.h"
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

#define SDA_PIN D2
#define SCL_PIN D3

class myLCD{
public:
  LiquidCrystal_I2C lcd;
  String data[9][2]={
{" 1. Sensor value"," 2. User manage "},
{" 3. Send SMS    "," 4. Log out     "},
{"Temperature:    ","Humidity:       "},
{"Insert Master   ","Card            "},
{" A. Hello User  "," B. Send data   "},
{" C. Back to menu","                "},
{"                ","                "},
{"Sending Hello   ","                "},
{"Sending data    ","                "}
};
  String data1[3][2]={
{" 1. Add UID     "," 2. Remove UID  "},
{" Insert UID     ","card to add     "},
{" Insert UID     ","card to remove  "},
};

//---------------------------------------------METHODS---------------------------------------------
  myLCD():lcd(0x27,16,2){
    Wire.begin(SDA_PIN,SCL_PIN);
    lcd.init(); 
    lcd.backlight();
  }

  void clear(){
    lcd.clear();
  }

  void print(int r, int c, String mess, unsigned long delay_time=0, bool clearScreen=false){
    if(clearScreen)
      lcd.clear();
    lcd.setCursor(c,r);
    lcd.print(mess);
    delay(delay_time);
  }
  
  void displayLCD(int screen,int cur){
    lcd.setCursor(0,0);
    lcd.print(data[screen][0]);
    lcd.setCursor(0,1);
    lcd.print(data[screen][1]);
    if(screen==0||screen==1||screen==4 || screen == 5){
      lcd.setCursor(0,cur);
      lcd.print(">");
    }
  }

  void displayLCD_userManage(int screen,int cur){
    lcd.setCursor(0,0);
    lcd.print(data1[screen][0]);
    lcd.setCursor(0,1);
    lcd.print(data1[screen][1]);
    if(screen==0){
      lcd.setCursor(0,cur);
      lcd.print(">");
    }
  }
  
  void displayInfo(int temperature,int humidity){
    data[2][0]="Temperature:";
    data[2][0]+=(String)temperature;
    data[2][0]+="  ";
    data[2][1]="Humidity:";
    data[2][1]+=(String)humidity;
    data[2][1]+="    ";
  }

  void checkAddUID(int check){
    lcd.clear();
    lcd.setCursor(0,0);
    if(check==3)
      lcd.print("Add success");
    else{
      lcd.print("Add failed");
      lcd.setCursor(0,1);
      if(check==0)
        lcd.print("Card existed");
      else if(check==1)
        lcd.print("Full member");
      else
        lcd.print("Master card");
    }
    delay(2000);
  }

  void checkRemoveUID(int check){
    lcd.clear();
    lcd.setCursor(0,0);
    if(check==1)
      lcd.print("Remove success");
    else{
      lcd.print("Remove failed");
      lcd.setCursor(0,1);
      if(check==0)
        lcd.print("Card not exist");
      else if(check==2)
        lcd.print("Master card");
    }
    delay(2000);
  }

  void checkSendSMS(bool checkSent,int screen){
    if(checkSent){
      data[screen][1]="Success";
    }
    else{
      data[screen][1]="Failed";
    }
  }
};
#endif
