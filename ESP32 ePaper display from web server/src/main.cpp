#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <EEPROM.h>

#include <GxEPD.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>
#include <GxGDEW075Z09/GxGDEW075Z09.h> 

#include <secrets.h>

static const uint8_t EPD_BUSY = 25;
static const uint8_t EPD_CS   = 15;
static const uint8_t EPD_RST  = 26; 
static const uint8_t EPD_DC   = 27; 
static const uint8_t EPD_SCK  = 13;
static const uint8_t EPD_MISO = 12; // Master-In Slave-Out not used, as no data from display
static const uint8_t EPD_MOSI = 14;

GxIO_Class io(SPI, /*CS=*/ EPD_CS, /*DC=*/ EPD_DC, /*RST=*/ EPD_RST); // arbitrary selection of 17, 16
GxEPD_Class display(io, /*RST=*/ EPD_RST, /*BUSY=*/ EPD_BUSY); // arbitrary selection of (16), 4

const uint16_t W = 640;
const uint16_t H = 384;

enum colors : uint8_t {
  WHITE = 0xFF,
  BLACK = 0x00,
  RED   = 0X0F
};

void setup() {
  Serial.begin(115200);
  Serial.print("Setup.");
  SPI.begin(EPD_SCK, EPD_MISO, EPD_MOSI, EPD_CS);
  Serial.print('.');
  display.init();
  Serial.print('.');
  EEPROM.begin(sizeof(uint32_t));
  Serial.print('.');
  Serial.println("done!");
}

int read32(WiFiClient &client){
  uint32_t out;
  client.readBytes((char*)&out, 4);
  return out;
}

void loop() {
  Serial.print("Connecting to wifi...");
  WiFi.begin(WiFi_SSID, WiFi_PASS);
  
  for(int timeout = 1000; WiFi.status() != WL_CONNECTED; delay(10)){
    if(timeout-- <= 0){
      Serial.println("Can't connect to wifi!");
      return;
    }
  }
  Serial.println("done!");

  Serial.print("Connecting to server..");
  WiFiClient client;

  if(!client.connect(Server_host, Server_port, 1000)){
      Serial.println("Can't connect to server!");
      return;
  }
  Serial.print('.');
  for(int timeout = 10; ! client.available(); delay(500)){
    if(timeout-- <= 0){
      client.stop();
      Serial.println("Server not responding!");
      return;
    }
  }
  Serial.println("done!");

  uint32_t hash = read32(client);

  Serial.print("Got hash: ");
  Serial.println(hash, HEX);

  uint32_t oldHash;
  EEPROM.get(0, oldHash);
  Serial.print("Old one is: ");
  Serial.println(oldHash, HEX);
  if(hash != oldHash){
    Serial.println("New image incoming");
    EEPROM.put(0, hash);
    EEPROM.commit();

    Serial.print("Reciving data...");
    display.fillScreen(GxEPD_WHITE);

    for(int i = 0; i < W * H; i++){
      while(client.available() == 0 ) delay(1);
      uint8_t pixel = client.read();
      // Serial.print("(" + (String)(i%W) + ", " + (String)(i/W) + ")");
      switch(pixel){
        case BLACK: display.writePixel(i % W, i / W, GxEPD_BLACK); break;
        case RED:  display.drawPixel(i % W, i / W, GxEPD_RED); break;
        case WHITE: break;
      }
    }
    Serial.println("done!");
    Serial.print("Updating screen...");
    display.update();
    Serial.println("done!");
  }

  client.stop();
  delay(10000);
}