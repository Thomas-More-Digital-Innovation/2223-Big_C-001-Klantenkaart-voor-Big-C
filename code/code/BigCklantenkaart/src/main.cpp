#include <Arduino.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include "WiFi.h"
#include "time.h"
//wifi
const char* ssid = "embedded";
const char* password =  "IoTembedded";
//datum en tijd
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;
//lcd
LiquidCrystal_I2C lcd(0x27, 16, 2);

#define SS_PIN  32  // ESP32 pin GIOP5 
#define RST_PIN 14 // ESP32 pin GIOP27 
#define BUZZER_PIN 15
MFRC522 rfid(SS_PIN, RST_PIN);
String last_uid = "";
String uid = "";

void beep(){
  digitalWrite(BUZZER_PIN,LOW);
  delay(500);
  digitalWrite(BUZZER_PIN,HIGH);
}

String time_now(){
  String current_time = "";
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
  }
  else{
    char timeHour[3];
    strftime(timeHour,3, "%H", &timeinfo);
    Serial.println(timeHour);
    char timeMinute[3];
    strftime(timeMinute,3, "%M", &timeinfo);
    Serial.println(timeMinute);
    char timeSecond[3];
    strftime(timeSecond,3, "%S", &timeinfo);
    Serial.println(timeSecond);
    Serial.println();
    current_time = String(timeHour) +":"+ String(timeMinute) +":"+ String(timeSecond);
  }
  return current_time;  
}

void ldc_screen(String lijn1, String lijn2){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(lijn1);
  lcd.setCursor(0,1);
  lcd.print(lijn2);
}

void read_rfid(){
  if (rfid.PICC_IsNewCardPresent()) { // new tag is available
    if (rfid.PICC_ReadCardSerial()) { // NUID has been readed
      MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
      Serial.print("RFID/NFC Tag Type: ");
      Serial.println(rfid.PICC_GetTypeName(piccType));

      // print UID in Serial Monitor in the hex format
      Serial.print("UID:");
      for (int i = 0; i < rfid.uid.size; i++) {
        // Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
        // Serial.print(rfid.uid.uidByte[i], HEX);
        uid += rfid.uid.uidByte[i];
      }
      beep();
      if(uid==last_uid){
        Serial.println("Same card as before");
        uid="";
      }
      else{
        ldc_screen(time_now(),"test2");
        Serial.println();
        Serial.println(uid);
        last_uid = uid;
        uid = "";
        Serial.println(time_now());
        
      }
      rfid.PICC_HaltA(); // halt PICC
      rfid.PCD_StopCrypto1(); // stop encryption on PCD
    }
  }
}

void setup()
{
  Serial.begin(9600);

  pinMode(BUZZER_PIN,OUTPUT);
  digitalWrite(BUZZER_PIN,HIGH);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");

  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  

  SPI.begin(); // init SPI bus
  rfid.PCD_Init(); // init MFRC522

  Serial.println("Tap an RFID/NFC tag on the RFID-RC522 reader");

  lcd.init();
  lcd.backlight();
}


void loop()
{
  read_rfid();
  
}