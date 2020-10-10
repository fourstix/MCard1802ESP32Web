/*
 * ESP32 Web based front end using an MSCP23017 for data
 * lines and an  MCP23008 for the Control lines of an 
 * 1802 Membership card.
 * 
 * Copyright (c) 2020 by Gaston Williams
 * 
 * Based on the ESP32 remote relay controller program
 * by Kazuhiro Ouchi  @kanpapa
 * 
 * ESP32 Remote relay controller program
 * Copyright (c) 2020  by Kazuhiro Ouchi
 * 
 * Based on ESP32 tutorials published by Rui Santos
 * Copyright (c) 2013-2020 by RandomNerdTutorials.com
 * 
 * ESP32 tutorials available at 
 * https://randomnerdtutorials.com/
 * 
 * Based on the ESPAsyncWebServer
 * Copyright (c) 2016-2020 by me-no-dev@github.com
 * 
 * The ESPAsyncWebServer is available at:
 * https://github.com/me-no-dev/ESPAsyncWebServer
 * 
 * Based on the 1802 Membership card hardware by Lee Hart. 
 * The 1802 Membership card is available here: 
 * http://www.sunrise-ev.com/1802.htm 
 * 
 * The 1802 Membership Card Microcomputer 
 * Copyright (c) 2006-2020 by Lee A. Hart.
 * 
 * All libraries and hardware designs are copyright their respective authors.
 * 
 * Adafruit MCP23008 GPIO Expander Library
 * Copyright (c) 2012-2020 by Adadruit Industries
 * Written by Limor Fried/Ladyada for Adafruit Industries.  
 * 
 * The Arduino-MCP23017 library
 * Copyright (c) 2017 by Bertrand Lemasle
 * 
 * ESP32 Remote relay controller program
 * Copyright (c) 2020  by Kazuhiro Ouchi
 * 
 * ESPAsyncWebServer
 * Copyright (c) 2016-2020 by me-no-dev@github
 * 
 * The 1802 Membership Card Microcomputer hardware design
 * Copyright (c) 2006-2020 by Lee A. Hart
 * 
 * Many thanks to the original authors for making their designs and code avaialble.
 * 
 */

// Import required libraries
#include <Wire.h>
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"
#include "secrets.h"

//Change debug token from 0 to 1 to include debug code in compile
#define DEBUG 0

//Define masks for Hex Digit
#define LO_NIBBLE  0x000F
#define HI_NIBBLE  0x00F0

//Hold ef4 low for minimum of 75 mSec to simulate keypress
#define KEY_PRESS_DURATION 75

//Read Data every 100 mSec
#define DATA_READ_INTERVAL 100 

//State variables from inputs
boolean runState     = false;
boolean loadState    = false;
boolean memProtState = false;

//buffer for characters typed on the keypad
uint16_t key_buffer = 0x00;

//allows continuously held input button
boolean hold_input = false; 

//External Flag 4 staus used for keyboard input key
boolean input_pressed = false;

//time when input key was pressed
unsigned long t_input_down = millis();

//time when data was read from MCP23008
unsigned long t_data_read = millis();

//data written to 1802 data in lines on MCP23017 Port A
byte data_in = 0x00;
//previous data written to MCP23017 Port A
byte old_data_in = 0x00;

//data read from 1802 data out lines on MCP23017 Port B
byte data_out = 0x00;
//previous data read from MCP23017 Port B
byte old_data_out = 0x00;

//Q Bit read from Control port
boolean q_bit = false;
//previous Q bit read Control port
boolean old_q_bit = false;

//Not implemented yet
//EF2 flag goes LOW when asserted
boolean assert_ef2 = false;

//byte for control lines
byte control_data = 0x00;

//previous control line data
byte old_control_data = 0x00;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

//Get state of control flags
String getControlState() {
  String ctrlState = "Reset";
  if (runState && !loadState) {
    ctrlState = "Run";
  } else if (runState && loadState) {
    ctrlState = "Wait";
  } else if (!runState && loadState) {
    ctrlState = "Load";
  } //if-else if
  
  return ctrlState; 
} //getControlState

//Get the state of the M/P flag
String getMemoryState() {
  String memState = "Read/Write";
  
  if (memProtState) {
    memState = "Protected";
  } //if
  return memState;
} //getMemoryState

//Get the high hex digit icon for the data byte
String getHiDigit() {
  //high hex digit is second from right (next to last) nibble
  byte hi_digit = (data_out & HI_NIBBLE) >> 4;
  return String(hi_digit, HEX);
} //getHiDigit

//Get the lower hex digit ico for the data byte
String getLoDigit() {
  //low hex digit is rightmost (last) nibble
  byte lo_digit = data_out & LO_NIBBLE;
  return String(lo_digit, HEX);  
} //getLoDigit

//Get state icon for Q bit
String getQState() {
  String qState = "off";
  if (q_bit) {
      qState = "on";
    } //if
  return qState;  
} //getQState


void setup() {
  //Set up MCP23017 and MCP23008
  Wire.begin();   
  setupMcpCommunication();
  
  // Serial port for debugging purposes
  Serial.begin(115200);
  
  // Initialize SPIFFS
  if(!SPIFFS.begin(true)){
    Serial.println(F("An Error has occurred while mounting SPIFFS"));
    return;
  }

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println(F("Connecting to WiFi.."));
  }

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

  /* Define routes for web server */
  //Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });
  
  //Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });

  //Routes to load images for byte displays
  server.on("/0.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/0.png", "image/png");
  }); 
  server.on("/1.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/1.png", "image/png");
  });
  server.on("/2.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/2.png", "image/png");
  });
  server.on("/3.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/3.png", "image/png");
  });  
  server.on("/4.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/4.png", "image/png");
  }); 
  server.on("/5.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/5.png", "image/png");
  });
  server.on("/6.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/6.png", "image/png");
  });
  server.on("/7.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/7.png", "image/png");
  }); 
  server.on("/8.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/8.png", "image/png");
  }); 
  server.on("/9.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/9.png", "image/png");
  });
  server.on("/a.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/a.png", "image/png");
  });
  server.on("/b.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/b.png", "image/png");
  });  
  server.on("/c.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/c.png", "image/png");
  }); 
  server.on("/d.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/d.png", "image/png");
  });
  server.on("/e.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/e.png", "image/png");
  });
  server.on("/f.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/f.png", "image/png");
  }); 
  
  //Routes to load images for Q LED on and off
  server.on("/on.gif", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/on.gif", "image/gif");
  });
  server.on("/off.gif", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/off.gif", "image/gif");
  });  

  //Route for input keys
  server.on("/input", HTTP_GET, [](AsyncWebServerRequest *request){
    if(request->hasParam("key")) {
      AsyncWebParameter* param = request->getParam("key");
      char key =  param->value().charAt(0);
      
    #if DEBUG
      Serial.print(F("Key: "));
      Serial.println(key);
    #endif
    processChar(key);
    } //if request->hasParam("key")
    request->send(SPIFFS, "/index.html", "text/html");
  });

  //Routes for AJAX updates
  server.on("/ctrl", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", getControlState().c_str()); 
  });
  server.on("/memory", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", getMemoryState().c_str()); 
  });
  server.on("/qbit", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", getQState().c_str()); 
  });
  server.on("/hi", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", getHiDigit().c_str()); 
  });
  server.on("/lo", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", getLoDigit().c_str()); 
  });
  
  // Start server
  server.begin();
} //setup

void loop() {
    if (hold_input) {
    t_input_down = millis();
    #if DEBUG
      //Print debug message only once
      if (!input_pressed) {
        Serial.println(F("Input Button down!"));
      } //if !input_pressed
    #endif
    input_pressed = true;
     } else if (input_pressed && (millis() - t_input_down < KEY_PRESS_DURATION)) {
    //Continue to hold input key for duration of key press
    input_pressed = true;    
  } else {
    input_pressed = false;
  } // if hold_input - else if
  
  //Set up control data
  control_data = getControlData(input_pressed);
  
  if (control_data != old_control_data) {
    //write out conrol data to MCP23008
    writeControlData(control_data);
    //save control values after write
    old_control_data = control_data;  
  #if DEBUG
      Serial.print(F("Writing Control Data: "));
      Serial.println(control_data, HEX);
  #endif    
  } //if control_data != old_control_data

  
  //If the key data has changed write to 1802
  if (data_in != old_data_in) {
    writeDataIn(data_in);
    //Save key data after sending out
    old_data_in = data_in;    
    #if DEBUG
      Serial.print(F("Writing "));
      print2Hex(data_in);
      Serial.println(F(" to data bus."));
    #endif
   } //if data_in != old_data_in  

  //Read data out and q bit periodically
  if (millis() - t_data_read > DATA_READ_INTERVAL) {
    //Save previous data before reading
    old_data_out = data_out;
    //Read the input data
    data_out = readDataOut();

    //Save previous q value before reading
    old_q_bit = q_bit;

    q_bit = readQBit();

  #if DEBUG
    if (data_out != old_data_out) {
      Serial.println();
      Serial.print(F("Data Out: "));
      print2Hex(data_out);
      Serial.println();
    } // if data_out != old_data_out
    if (q_bit != old_q_bit) {
      Serial.println();
      Serial.print(F("Q: "));
      Serial.println(q_bit);
    } // if q_bit != old_q_bit
  #endif  
  
  //Set interval time to wait before next read
  t_data_read = millis();
  } //if time > DATA_READ_INTERVAL  
} //loop
