# MCard1802ESP32Web
This repository contains  Web based front panel for the 1802 Membership Card using ESPAsynchWebServer running on an ESP32 

My first computer was a Netronics Elf II. It was an RCA 1802 based single board computer that was sold as a kit,
but it had a hexadecimal keypad, an LED, video and an expansion bus. Like many high school kids, I mowed yards
and spent my hard-earned dollars to buy a kit from Netronics [based on their ads.](http://www.cosmacelf.com/gallery/netronics-ads/)

My orignal Elf II was lost in a move long ago, but today Lee Hart's [1802 Membership card.](http://www.sunrise-ev.com/1802.htm) 
duplicates the orignal elf hardware. I saw a very cool demo of a project named [ESP32 Web Hex Keypad](https://github.com/kanpapa/esp32_web_hex_keypad)
that used an ESP32 based web interface and relays to drive an 1802 VIP computer.

I wanted to do something similar to simulate the Netronics Elf II interface using a web interface and an ESP32.
The ESP32 based code communicates through an I2C based interface to the 1802 Membership card and uses AJAX to update
the web display.

Description:
------------

For the I2C interface, this code uses an updated version of the [MCard1802ArduinoV2](https://github.com/fourstix/MCard1802ArduinoV2)
Front Panel card.  The Front Panel Card consists of an MCP23008 I2C 8 bit port expander to drive the 1802 Control lines,
a 7400 Quad Nand logic chip for Write Enable and Serial Communication logic, and an MCP23017 I2C dual port IO expander to communicate
with the 1802 Membership Card's data in and data out lines. The [Gerber files](https://github.com/fourstix/MCard1802ESP32Web/tree/main/hardware)
and [schematic](https://github.com/fourstix/MCard1802ESP32Web/blob/main/docs/MCard1802I2CFrontPanel.pdf) for the printed circuit board are
available in this project.


The MCard1802ESP32Web code uses the [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer) to serve the web pages from the
[ESP32 SPIFFS file system.](https://github.com/me-no-dev/arduino-esp32fs-plugin)  There are two files in SPIFFS, the style sheet style.css 
and the HTML file index.html.  The style sheet contains the two button styles and the HTML file contains all the 
JavaScript for the AJAX (Asynchronous JavaScript and XML) logic for updating the web.

The ESP32 code contains three tabs.  The main tab MCard1802ESP32Web contains all the definitions for the Web Server, including the REST API
called by the buttons and AJAX functions.  It also includes all the SPIFFS definitions for the graphics files.  The ESPAsynchWebServer is
completely defined in the setup routine.  The loop routine queries the status of the Membership Card periodically and updates the 
status variables.

The second tab MCard1802I2CFrontEnd contains the logic for the MCP23008 and MCP23017 GPIO extenders used to communicate to the Membership Card.
The loop function periodically calls these functions to get status from the membership card and also invokes these functions whenever a button
is pressed.

The third tab secrets.h contains the network SSID and password constants used to connect to the network.  Please be sure to replace the dummy
values in this file with the actual values required to connect to your own Wifi network.

Introduction
-------------

A very good source of information on the RCA 1802 chip and Cosmac Elf computer can be found on the 
[CosmacElf web page.](http://www.cosmacelf.com) The 1802 was a fantastic microprocessor that still has quite a 
dedicated following today.

The 1802 Membership card is available from Lee Hart on his [website.](http://www.sunrise-ev.com/1802.htm)  
Additional documentation and other information are availble from Herb Johnson's 
[1802 Membership Card wesite.](http://www.retrotechnology.com/memship/memship.html)

Information on the ESP32AsynchWeb server can be found at Random Nerd Tutorials in the
[ESP32 SPIFFS Web Server](https://randomnerdtutorials.com/esp32-web-server-spiffs-spi-flash-file-system/) tutorial.

This code largely follows that example except that it uses AJAX rather than templates to update the web display.
Using AJAX allows the Membership Card to update the web display asynchronously without any user action like a page
reresh.  Information about AJAX can be found at w3schools [AJAX Information](https://www.w3schools.com/js/js_ajax_intro.asp) page.
W3Schools also has a very good [CSS](https://www.w3schools.com/css/default.asp), [JavaScript](https://www.w3schools.com/js/default.asp)
and [HTML](https://www.w3schools.com/html/default.asp) tutorials.

HTML
----

The HTML file [index.html](https://github.com/fourstix/MCard1802ESP32Web/blob/main/src/MCard1802ESP32Web/data/index.html)
contains the web page layout for the input buttons and the AJAX logic to invoke the server REST API and update
the web page with the Membership Card status.

The tags in the header section are standard tags plus the link to the style sheet in SPIFFS.
```html
<link rel="stylesheet" type="text/css" href="style.css">
```
This link invokes the following server route definition in the [MCard1802ESP32Web.ino](https://github.com/fourstix/MCard1802ESP32Web/blob/main/src/MCard1802ESP32Web/MCard1802ESP32Web.ino)
setup() function to serve the style sheet.
```arduino
  //Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });
```

The script section of the HTML file contains the definitions for the AJAX function updateFrontPanel(url, cbFunction).  This function
will invoke the REST API and then pass the response to the appropriate call back function.  Each call back function
will then update the web element.  The setInterval() function updates the Q LED, Data byte display graphics, memory
status text and control status text every 500 milliseconds.

The script section also defines the handler for each input key press.  The keypress function will invoke the input REST API and pass
the key value as a query parameter.  The following server route defined in the [MCard1802ESP32Web.ino](https://github.com/fourstix/MCard1802ESP32Web/blob/main/src/MCard1802ESP32Web/MCard1802ESP32Web.ino)
setup() function strips the key's character value from the query parameter and passes it to the prossesChar() function.
```arduino
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

```

The HTML body consists of a layout table to define the input key button definitions.  The keypress function is invoked by the onclick method.
The input button also invokes keypress onmousedown and ontouchstart to simulate cases where the input button is held down for a period of time.