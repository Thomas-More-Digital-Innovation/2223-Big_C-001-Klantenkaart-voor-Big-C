#include <Arduino.h>
#include "SPI.h"//for tft
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "SPIFFS.h"
#include <JPEGDecoder.h>
#include <Fonts/FreeSans12pt7b.h>//until here for tft
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>//for the lcd1602a
#include <PN532_I2C.h>//for card reader
#include <PN532.h>//for card reader
#include "WiFi.h"
#include "time.h"//to get time
#include <HTTPClient.h>//for the post
#include <ArduinoJson.h>//for the post

DynamicJsonDocument doc(1024);//for the post response

String originalUrl = "https://the-big-c.pages.dev/api/register/";
String xApiKey = "fill in API KEY";

//buzzer
#define BUZZER 33

//wifi
const char* ssid = "TM_Infrastructuur";
const char* password =  "gbt4inaixLmKzznw";

//date and time
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

//card reader
PN532_I2C pn532i2c(Wire);
PN532 nfc(pn532i2c);

volatile bool connected = false;

//lcd
LiquidCrystal_I2C lcd(0x27, 16, 2);

// tft
#define TFT_DC 14
#define TFT_CS 15
#define TFT_RST 32
#define TFT_MOSI 18
#define TFT_CLK 5
#define TFT_MISO 19

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

unsigned long timeNowMillis = 1;
unsigned long timeLastCard = 0;
bool firstCardScanned = false;
bool alreadyDrawn = false;

///////////////////////////
//      FUNCTIONS        //
//////////////////////////
//this function returns the current time as a string, example "14:45:55"
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

/////////////////lcd screen////////////////////////////
//this function puts the 2 strings that are given as parameters on the screen
void lcd_screen(String line1, String line2){
  lcd.clear();
  lcd.setCursor(0,0);//cursor at beginning of line 1
  lcd.print(line1);
  lcd.setCursor(0,1);//cursor at beginning of line 2
  lcd.print(line2);
}



///////////////tft screen//////////////////////////////


// Return the minimum of two values a and b
#define minimum(a,b)     (((a) < (b)) ? (a) : (b))

//Decodes and paints onto the TFT screen
void jpegRender(int xpos, int ypos) {

  // retrieves infomration about the image
  uint16_t  *pImg;
  uint16_t mcu_w = JpegDec.MCUWidth;
  uint16_t mcu_h = JpegDec.MCUHeight;
  uint32_t max_x = JpegDec.width;
  uint32_t max_y = JpegDec.height;

  // Jpeg images are draw as a set of image block (tiles) called Minimum Coding Units (MCUs)
  // Typically these MCUs are 16x16 pixel blocks
  // Determine the width and height of the right and bottom edge image blocks
  uint32_t min_w = minimum(mcu_w, max_x % mcu_w);
  uint32_t min_h = minimum(mcu_h, max_y % mcu_h);

  // save the current image block size
  uint32_t win_w = mcu_w;
  uint32_t win_h = mcu_h;

  // record the current time so we can measure how long it takes to draw an image
  uint32_t drawTime = millis();

  // save the coordinate of the right and bottom edges to assist image cropping
  // to the screen size
  max_x += xpos;
  max_y += ypos;

  // read each MCU block until there are no more
  while ( JpegDec.read()) {

    // save a pointer to the image block
    pImg = JpegDec.pImage;

    // calculate where the image block should be drawn on the screen
    int mcu_x = JpegDec.MCUx * mcu_w + xpos;
    int mcu_y = JpegDec.MCUy * mcu_h + ypos;

    // check if the image block size needs to be changed for the right edge
    if (mcu_x + mcu_w <= max_x) win_w = mcu_w;
    else win_w = min_w;

    // check if the image block size needs to be changed for the bottom edge
    if (mcu_y + mcu_h <= max_y) win_h = mcu_h;
    else win_h = min_h;

    // copy pixels into a contiguous block
    if (win_w != mcu_w)
    {
      for (int h = 1; h < win_h-1; h++)
      {
        memcpy(pImg + h * win_w, pImg + (h + 1) * mcu_w, win_w << 1);
      }
    }

    // draw image MCU block only if it will fit on the screen
    if ( ( mcu_x + win_w) <= tft.width() && ( mcu_y + win_h) <= tft.height())
    {
      tft.drawRGBBitmap(mcu_x, mcu_y, pImg, win_w, win_h);
    }

    // Stop drawing blocks if the bottom of the screen has been reached,
    // the abort function will close the file
    else if ( ( mcu_y + win_h) >= tft.height()) JpegDec.abort();

  }

  // calculate how long it took to draw the image
  drawTime = millis() - drawTime;

  // print the results to the serial port
  Serial.print  ("Total render time was    : "); Serial.print(drawTime); Serial.println(" ms");
  Serial.println("=====================================");
}


//   This function opens the Filing System Jpeg image file and primes the decoder
void drawFSJpeg(const char *filename, int xpos, int ypos) {

  Serial.print("Drawing file: "); Serial.println(filename);
  
  fs::File jpgFile = SPIFFS.open( filename, "r");    // File handle reference for SPIFFS

  if ( !jpgFile ) {
    Serial.print("ERROR: File \""); Serial.print(filename); Serial.println ("\" not found!");
    return;
  }

  boolean decoded = JpegDec.decodeFsFile(filename);
  if (decoded) {
    // render the image onto the screen at given coordinates
    jpegRender(xpos, ypos);
  }
  else {
    Serial.println("Jpeg file format not supported!");
  }
}


//show how many coffies to go until a free one
void tftCoffieLeft(int counter){
  drawFSJpeg("/koffieBonen.jpg", 0, 0);//draws foto on tft screen
  tft.fillRoundRect(20,60,280,120,9,ILI9341_BLACK);//draws a black rectancle on screen
  tft.setCursor(150,120);
  tft.setFont(&FreeSans12pt7b);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(3);
  tft.print(counter);

  tft.setCursor(50,160);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(1);
  tft.print("more for a free coffee");//writes text on screen(over black rectangle)
}

void post_request(String url){
  if(WiFi.status()== WL_CONNECTED){ //Check if wifi is connected
  
    HTTPClient http;   
    
    http.begin(url);  
    //set the headers
    http.addHeader("Content-Type", "application/json");             
    http.addHeader("x-api-key", xApiKey);
    
    int httpResponseCode = http.POST("post from esp32"); //do post request
    
    if(httpResponseCode>0){
      String response = http.getString(); //save response in a variable
      Serial.println(httpResponseCode);   //the response code
      Serial.println(response);
      deserializeJson(doc, response);
      JsonObject response_json = doc.as<JsonObject>();//make the response into a jsonObject

      bool freeCoffee = response_json["data"]["freeCoffee"];//get the needed info from the response
      int coffeeCount = response_json["data"]["count"];
      int coffeeToGo = response_json["data"]["countToGo"];

      Serial.println(freeCoffee);    
      Serial.println(coffeeCount);   
      //draw on the tft screen
      if(freeCoffee){
        lcd_screen(time_now(),"Free Coffee");
        drawFSJpeg("/freecoffee.jpg", 0, 0); 
      }
      else{
        lcd_screen(time_now(),"NO free coffee");
        tftCoffieLeft(coffeeToGo);
      }  
    }else{
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }
    
    http.end(); //stop the connection

   
  
 }else//wifi is not connected
 {
    lcd_screen("wifi","connecting");//show the barista there is a wifi problem
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) { //try to connect to wifi
      vTaskDelay(500/portTICK_PERIOD_MS);   
      Serial.println("Connecting to WiFi..");
    }
    lcd_screen("wifi","connected");//show the barista the wifi is ok again
    Serial.println("WiFi connected");
 }

}

////////////////card reader///////////////////////
//this function returns true if a card can be read
bool connect() {
  
  nfc.begin();

  // checks if te card reader is found
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata)
  {
    Serial.println("PN53x card not found!");
    return false;
  }

  //prints the card reader version
  Serial.print("Found chip PN5"); Serial.println((versiondata >> 24) & 0xFF, HEX);
  Serial.print("Firmware version: "); Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print('.'); Serial.println((versiondata >> 8) & 0xFF, DEC);

  // Set the max number of retry attempts to read from a card
  nfc.setPassiveActivationRetries(0xFF);

  // configures the board to read RFID tags
  nfc.SAMConfig();

  Serial.println("Waiting for card (ISO14443A Mifare)...");
  Serial.println("");

  return true;
}
//this function reads the card uid and changes the screens accordingly
void readCard(void * parameter){
  for(;;){
    timeNowMillis = millis();
    if((timeNowMillis>(timeLastCard+8000)) && (!alreadyDrawn)){
      //draw the logo onto the screen
      drawFSJpeg("/theBigC.jpg", 0, 0); 
      alreadyDrawn = true;
    }
    boolean success;
    // Buffer to store the UID
    uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
    // length of UID, can be 4 or 7
    uint8_t uidLength;
    
    //reads the uid
    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);

    // If the card is detected, print the UID
    if (success)
    {
      tone(BUZZER, 1000); //sends a tone of 1kHz
      vTaskDelay(500/portTICK_PERIOD_MS);        
      noTone(BUZZER);//stops the tone
      String cardNumber = "";
      Serial.println("Card Detected");
      Serial.print("Size of UID: "); Serial.print(uidLength, DEC);
      Serial.println(" bytes");
      Serial.print("UID: ");
      for (uint8_t i = 0; i < uidLength; i++)
      {
        Serial.print(" 0x"); Serial.print(uid[i], HEX);
        cardNumber += uid[i];
      }
      
      Serial.println("");
      Serial.println(cardNumber);
      Serial.println("");
      String complete_url = originalUrl+cardNumber;//ads the card number to the post url
      post_request(complete_url);// makes the post request
      timeLastCard = millis();
      vTaskDelay(10/portTICK_PERIOD_MS);
      firstCardScanned = true;
      alreadyDrawn = false;
    }
  }
}


////////////////////////////////
////////////SETUP//////////////
///////////////////////////////
void setup()
{
  pinMode(TFT_RST,OUTPUT);
  Serial.begin(115200);
  //get lcd ready 
  lcd.init();
  lcd.backlight();

  vTaskDelay(10/portTICK_PERIOD_MS);
  lcd_screen("wifi","connecting");
  
  //make wifi connection
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    vTaskDelay(500/portTICK_PERIOD_MS);   
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");

  lcd_screen("wifi","connected");

  // Init the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  //get tft screen ready, set the orientation en make screen black
  tft.begin();
  tft.setRotation(3);  // 0 & 2 Portrait. 1 & 3 landscape
  tft.fillScreen(0);

  //init the file system
  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS initialisation failed!");
    while (1) yield(); // Stay here twiddling thumbs waiting
  }
  Serial.println("\r\nInitialisation done.");

  //draw the logo onto the screen
  drawFSJpeg("/theBigC.jpg", 0, 0); 

  //set the buzzer pin as output
  pinMode(BUZZER, OUTPUT);//make shure the card reader is ready
  //waits until card reader is ready
  while (!connected) {
    connected = connect();
  }
  Serial.println("now with millis");
  //makes te task that reads the card
  xTaskCreate(readCard,"readCard",10000,NULL,1,NULL);

}


////////////////////////////////
////////////LOOP//////////////
///////////////////////////////
void loop()
{
  //check if it's been 10 seconds sinds the last card was scanned
  //then go back to the standard image on the tft screen
  /*
  if (firstCardScanned)
  {
    timeNowMillis = millis();
    if((timeNowMillis>(timeLastCard+8000)) && (!alreadyDrawn)){
      //draw the logo onto the screen
      drawFSJpeg("/theBigC.jpg", 0, 0); 
      alreadyDrawn = true;
    }
  }*/ 
  
  vTaskDelay(1000/portTICK_PERIOD_MS);
  
}
