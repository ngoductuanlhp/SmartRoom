//--------------------------------------------------------------------
//                            INCLUDE LIBRARIES
//--------------------------------------------------------------------
#include <DHT.h>
#include <Wire.h> 
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h>
#include "myLCD.h"
#include "myRFID.h"

//--------------------------------------------------------------------
//                            DEFINE KEYWORDS
//--------------------------------------------------------------------

//DHT pin and type
#define dhtTYPE DHT11
#define dhtPIN 3 //RX

//SIM800L
#define RXSIM D0
#define TXSIM D1

//Button pin
#define buttonPin A0

//LED pins
#define ledPin1 D4 //D4
#define ledPin2 10 //SD3

//Wifi
#define ssid "TuanTu"
#define password "123456789"

//MQTT Broker
#define mqtt_server "m16.cloudmqtt.com" 
#define mqtt_topic_pub1 "temperature"
#define mqtt_topic_pub2 "humidity"
#define mqtt_topic_pub3 "hello"
#define mqtt_user "tuanngo"
#define mqtt_pwd "123456789"

//--------------------------------------------------------------------
//                            SETUP
//--------------------------------------------------------------------

//DHT
DHT dht(dhtPIN, dhtTYPE);

//LCD
myLCD lcd;

//RFID
myRFID rfid;

//Wifi and MQTT
const uint16_t mqtt_port =  12986; //Port of CloudMQTT
WiFiClient espClient;
PubSubClient client(espClient);
SoftwareSerial sim800(TXSIM,RXSIM);
//--------------------------------------------------------------------
//                            GLOBAL VARIABLES
//--------------------------------------------------------------------

//Button
bool button_active=false; //check whether button is pressed
int button_press=0; //check type of button pressed

unsigned long time_button; //time interval to read button signal
unsigned long time_turntoMenu; //time interval to turn back to home menu (>6s)
unsigned long time_buttonConsti; //time interval to check button 1 is pressed continuously
unsigned long time_toDeactive; //time interval to deactive system until the last press (>6min)
unsigned long time111;

//Wifi and MQTT
bool wifi_connect=false;
bool MQTT_connect=false;

//State variables for FSM
int state_button=0;
int state_system=0;
int state_MQTT=0;
int state_userManage=0;

//Time interval to do tasks
unsigned long time_readSensors;
unsigned long time_checkConnect;
unsigned long time_userManage;

//Sensor value
float temperature=0;
float humidity=0;

//data for LCD
int screen=0;
int cur=0;

//data for LCD
int screen1=0;
int cur1=0;

//--------------------------------------------------------------------
//                            IMPLEMENT FUNCTIONS
//--------------------------------------------------------------------

/*void LED_init(){
  bool state=false;
  int i=0;
  while(i<21){
    digitalWrite(ledPin1,state);
    digitalWrite(ledPin2,state);
    delay(200);
    state=!state;
    i++;
  }
}

void LED_activeSystem(){
  bool state=false;
  int i=0;
  while(i<9){
    digitalWrite(ledPin1,state);
    digitalWrite(ledPin2,state);
    delay(500);
    state=!state;
    i++;
  }
}*/

void connectMQTT(){
  switch(state_MQTT){
    case 0:
      digitalWrite(ledPin1,HIGH);
      digitalWrite(ledPin2,HIGH);
      if(WiFi.status() == WL_CONNECTED){
        state_MQTT=1;
      }
    break;
    case 1:
      digitalWrite(ledPin1,LOW);
      digitalWrite(ledPin2,HIGH);
      if(WiFi.status() != WL_CONNECTED){
        state_MQTT=0;
      }
      else if(client.connect("ESP8266Client",mqtt_user, mqtt_pwd))
          state_MQTT=2;
    break;
    case 2:
      digitalWrite(ledPin1,HIGH);
      digitalWrite(ledPin2,HIGH);
      if(!client.loop())
        state_MQTT=1;
    break;
  }
}

void readSensors()
{
  temperature = dht.readTemperature();  //read temperature
  humidity = dht.readHumidity();    //read humidty
  lcd.displayInfo(temperature,humidity);
}

void read_Button(){
  int val=analogRead(A0);
  switch(state_button){
    case 0:
      if(val<100){
        button_press=0;
        return;
      }
      if(val<550&&val>300){
         button_press=1;
         button_active=true;
         time_buttonConsti=millis();
      }
      else if(val>600&&val<800){
        button_active=true;
        button_press=2;
      }
      state_button=1;
    break;
    case 1:
      if(val<100||((val<550&&val>300)&&(millis()-time_buttonConsti>500)&&button_press==1)){
        state_button=0;
      }
    break;
  }
}

bool sendSMS(String str){
  sim800.print("AT+CMGF=1\r\n");
  delay(100);
  sim800.print("AT+CMGS=\"0933395305\"\r\n");
  delay(100);
  sim800.print(str+"\r\n");
  delay(100);
  sim800.write(26);
  delay(100);
  return true;
}

bool sendMessage(){
  String str1;
  String str2;
  bool sentMQTT=false;
  bool sentSMS=false;
  if(screen==8){
    str1="Temperature:";
    str1+=(String)temperature;
    str2="Humidity:";
    str2+=(String)humidity;
    bool sent1=client.publish(mqtt_topic_pub1,str1.c_str());
    bool sent2=client.publish(mqtt_topic_pub2,str2.c_str());
    delay(500);
    sentMQTT=sent1&&sent2;
    sentSMS=sendSMS(str1+" | "+str2);
  }
  else{
    str1="Hello User";
    sentMQTT=client.publish(mqtt_topic_pub3,str1.c_str());
    delay(500);
    sentSMS=sendSMS(str1);
  }
  return sentSMS||sentMQTT;
}

void setup() {
  Serial.begin(115200);
  delay(500);
  sim800.begin(9600);
  delay(2000);
  WiFi.begin(ssid, password);
  delay(2000);
  client.setServer(mqtt_server, mqtt_port); 
  delay(1000);

  //INPUT/OUTPUT
  pinMode(buttonPin,INPUT);
  pinMode(ledPin1,OUTPUT);
  pinMode(ledPin2,OUTPUT);

  //setup output
  digitalWrite(ledPin1,HIGH);
  digitalWrite(ledPin2,HIGH);

}

void loop() {
  int check=rfid.checkRFID();
  switch(state_system){
    case 0:
      if(check==-1){
        lcd.print(0,0,"Input Card",500,true);
      }
      if(check==0){
        lcd.print(0,0,"Invalid Card",500,true);
      }
      if(check==1||check==2){
        lcd.print(0,0,"Authenticated",1000,true);
        state_system=1;
        time_button=millis();
        time_turntoMenu=millis(); 
        time_toDeactive=millis(); 
        time_readSensors=millis();
        time_checkConnect=millis();
        lcd.displayLCD(screen,cur);
      }
    break;
    case 1:
      if(check==1||check==2){
        lcd.print(0,0,"Turn off system",1000,true);
        state_system=0;
      }
      else{
        if(millis()-time_checkConnect>200){
          connectMQTT();
          time_checkConnect=millis();
        }   
        if(millis()-time_readSensors>1000){
          readSensors();
          time_readSensors=millis();
        }
        if(millis()-time_button>50){
          read_Button();
          time_button=millis();
        }
        if(button_active==true){
          time_turntoMenu=millis();
          time_toDeactive=millis();
          if(button_press==1){
            if(screen==0||screen==1||screen==4){
              cur++;
              if(cur==2){
                cur=0;
                screen= (screen==0) ? 1 :
                        (screen==1) ? 0 :
                        5;
              }
            }
            else if(screen==5){
              screen=4;
              cur=0;             
            }
          }
          else if(button_press==2){
            //update sreen
            if(screen==2||screen==7||screen==8){
              screen=0;
              cur=0;
            }
            else if(screen==0||screen==1){
              if(screen==1&&cur==1){
                lcd.print(0,0,"Turn off system",1000,true);
                state_system=0;
                screen=0;
                cur=0;
              }
              else{
                screen=2*screen+cur+2;
                cur=0;
              }
            }
            else if(screen==4){
              screen=screen+cur+3;
              cur=0;
            }
            else if(screen==5){
              screen=0;
              cur=0;
            }
            //do task
            if(screen==3){
              state_system=2;
              state_userManage=0;
              time_userManage=millis();
            }
            else if(screen==7||screen==8){
              bool checkSent=sendMessage();
              lcd.checkSendSMS(checkSent,screen);
            }                       
          }
          lcd.displayLCD(screen,cur);
          delay(500);   
          button_active=false;
        }
        if(millis()-time_turntoMenu>6000){
          screen=0;
          cur=0;
          lcd.displayLCD(screen,cur);
          time_turntoMenu=millis();
        }
        if(millis()-time_toDeactive>600000){
          screen=0;
          cur=0;
          state_system=0;
          lcd.print(0,0,"Turn off system",1000,true);
        }
      }
    break;
    case 2:
      if(millis()-time_userManage>20000){
        cur1=0;
        screen1=0;
        state_system=1;
      }
      else{
        switch(state_userManage){
          case 0:
            if(check==2){
              lcd.print(0,0,"Not master card",500,true);
              return;
            }
            if(check==1){
               lcd.print(0,0,"Authenticated",500,true);
               delay(1000);
               screen=0;
               cur=0;
               state_userManage=1;
               cur1=0;
               screen1=0;
               lcd.displayLCD_userManage(screen1,cur1);
            }         
          break;   
          case 1:
            if(millis()-time_button>50){
              read_Button();
              time_button=millis();
            }
            if(button_active==true){
              if(button_press==1){
                cur1++;
                cur1=cur1%2;
              }
              if(button_press==2){
                //update screen
                screen1=cur1+1;
                state_userManage=2;
              }
              lcd.displayLCD_userManage(screen1,cur1);
              button_active=false;
            }
          break;
          case 2:
            int c;
            if(screen1==1){
              c=rfid.addUIDcard();
              if(c==-1)
                return;
              lcd.checkAddUID(c);
            }
            else if(screen1==2){
              c=rfid.removeUIDcard();
              if(c==-1)
                return;
              lcd.checkRemoveUID(c);
            }
            cur1=0;
            screen1=0;
            state_system=1;
          break;
        }      
      }
    break;
  }
}
