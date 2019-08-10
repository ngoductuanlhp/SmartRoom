#ifndef  _myrfid_h_
#define _myrfid_h_
#include "Arduino.h"
#include <SPI.h>
#include <MFRC522.h>

//RFID pins
#define SS_PIN D8 //D8
#define RST_PIN D3 //D3

class myRFID{
private:
  MFRC522 mfrc522;
  String masterUID="89 68 C8 73";
  String arrUID[4]={
  "",
  "",
  "",
  ""
  };
//---------------------------------------------METHODS---------------------------------------------
public:
  myRFID():mfrc522(SS_PIN, RST_PIN){
    SPI.begin();       // Init SPI bus
    mfrc522.PCD_Init(); // Init MFRC522
  }
  String readUID(){
    String content= "";
    byte letter;
    for (byte i = 0; i < mfrc522.uid.size; i++) 
    {
       content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
       content.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    content.toUpperCase();
    return content.substring(1);
  }
  //check UID to activate
  int checkUID()
  {
    String uid=readUID();
    if(uid == masterUID){
      return 1;//master card
    }
    for(int i=0; i<4; i++){
      if(uid == arrUID[i]){
        return 2;//member card
      }
    }
    return 0;//no found
  }
  int checkRFID(){      
    if(!mfrc522.PICC_IsNewCardPresent())
      return -1;
    if(!mfrc522.PICC_ReadCardSerial())
      return -1;
    //If UID card is incorrect, delay 500ms and reloop
    int checkCard=checkUID();
    return checkCard;
  }
  int addUIDcard(){
    if(!mfrc522.PICC_IsNewCardPresent())
      return -1;
    if(!mfrc522.PICC_ReadCardSerial())
      return -1;
    String uid = readUID();
    int idx=-1;
    int exist=0;
    if(uid==masterUID)
      return 2; // master card
    for(int i=0; i<4; i++){
      if(arrUID[i]==uid){
        return 0; //exist
      }
      if(arrUID[i]==""&&exist==0){
        idx=i;
        exist=1;
      }
    }
    if(idx!=-1){
      arrUID[idx]+=uid;
      return 3; //success
    }
    return 1; //full
  }
  
  int removeUIDcard(){
    if(!mfrc522.PICC_IsNewCardPresent())
      return -1;
    if(!mfrc522.PICC_ReadCardSerial())
      return -1;
    String uid = readUID();
    if(uid==masterUID)
      return 2; // master
    for(int i=0;i<4;i++){
      if(arrUID[i]==uid){
        arrUID[i]="";
        return 1; // exist and delete
      }
    }
    return 0; // not exist
  }
};
#endif
