# MCard1802ESP32Web
This repository contains  Web based front panel for the 1802 Membership Card using ESPAsynchWebServer running on an ESP32.

My first computer was a Netronics Elf II. It was an RCA 1802 based single board computer that was sold as a kit,
with a hexadecimal keypad, an LED, video and an expansion bus. Like many high school kids, I mowed yards
and spent my hard-earned dollars to buy a kit from Netronics [based on their ads.](http://www.cosmacelf.com/gallery/netronics-ads/)

My orignal Elf II was lost in a move long ago, but today Lee Hart's [1802 Membership card.](http://www.sunrise-ev.com/1802.htm) 
duplicates the orignal elf hardware. I saw a very cool demo of a project named [ESP32 Web Hex Keypad](https://github.com/kanpapa/esp32_web_hex_keypad)
that used an ESP32 based web interface and relays to drive an 1802 VIP computer.

I wanted to do something similar to provide a web interface for the 1802 Membership Card using an ESP32. The web page layout
and graphics simulates the Netronics Elf II interface.  The ESP32 based web code communicates through an I2C based interface card 
to the 1802 Membership card and uses AJAX to update the web display.  The Arduino IDE with the ESP32 and SPIFFS extensions
was used to develop the code and upload it to the ESP32.

Description:
------------

For the I2C interface, this code uses an updated version of the [MCard1802ArduinoV2](https://github.com/fourstix/MCard1802ArduinoV2)
Front Panel card.  The Front Panel Card consists of an MCP23008 I2C 8 bit port expander to drive the 1802 Control lines,
a 7400 Quad Nand logic chip for Write Enable and Serial Communication logic, and an MCP23017 I2C dual port IO expander to communicate
with the 1802 Membership Card's data in and data out lines. The [Gerber](https://github.com/fourstix/MCard1802ESP32Web/blob/main/hardware/v1/MCard1802I2CFrontPanelv1Gerbers.zip) files,
the [KiCad](https://github.com/fourstix/MCard1802ESP32Web/blob/main/hardware/v1/MCardI2CFrontPanelv1KiCad.zip) files and
[schematic](https://github.com/fourstix/MCard1802ESP32Web/blob/main/docs/MCard1802I2CFrontPanel.pdf) for the printed circuit board are
available in this project.


The MCard1802ESP32Web code uses the [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer) to serve the web pages from the
[ESP32 SPIFFS file system.](https://github.com/me-no-dev/arduino-esp32fs-plugin)  There are two files in SPIFFS, the style sheet style.css 
and the HTML file index.html.  The style sheet contains the two button styles and the HTML file contains all the 
JavaScript for the AJAX (Asynchronous JavaScript and XML) logic for updating the web.

The ESP32 code contains three tabs.  The main tab file MCard1802ESP32Web.ino contains all the definitions for the Web Server, including the REST API
called by the buttons and AJAX functions.  It also includes all the SPIFFS definitions for the graphics files.  The ESPAsynchWebServer is
completely defined in the setup routine.  The loop routine queries the status of the Membership Card periodically and updates the 
status variables.

The second tab file MCard1802I2CFrontEnd.ino contains the logic for the MCP23008 and MCP23017 GPIO extenders used to communicate to the Membership Card.
The loop function periodically calls these functions to get status from the membership card and also invokes these functions whenever a button
is pressed.

The third tab file secrets.h contains the network SSID and password constants used to connect to the network.  Please be sure to replace the dummy
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

Quick Installation
------------------
The [ESP32 SPIFFS Web Server](https://randomnerdtutorials.com/esp32-web-server-spiffs-spi-flash-file-system/) tutorial
gives a very good explanation of how to install all the prerequesits for this project.  Random Nerd Tutorials also 
has a very good tutorial for [Getting Started with the ESP32](https://randomnerdtutorials.com/getting-started-with-esp32/)
that includes information on setting up the Arduino IDE.

The basic steps are:
* Install the [ESP32 board definition on the Arduino IDE](https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/)
* Install the [ESP32 Filesystem Uploader on the Arduino IDE](https://randomnerdtutorials.com/install-esp32-filesystem-uploader-arduino-ide/)
* Install the [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)
* Install the [AsyncTCP](https://github.com/me-no-dev/AsyncTCP) library required by ESPAsyncWebServer.

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
setup() function to serve the style sheet from SPIFFS.  The style sheet and html files are located in the data subdirectory underneath the ESP32 project code directory.
```arduino
  //Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });
```

The script section of the HTML file contains the definitions for the AJAX function updateFrontPanel(url, cbFunction).  This function
will invoke the REST API and then pass the response to the appropriate call back function.  Each call back function
will then update the web element.  The setInterval() function updates the Q LED, data bus byte graphics, memory
status text and control status text every 500 milliseconds.

The script section also defines the handler for each input key press.  The keypress function will invoke the input REST API and pass
the key value as a query parameter.  The following server route defined in the [MCard1802ESP32Web.ino](https://github.com/fourstix/MCard1802ESP32Web/blob/main/src/MCard1802ESP32Web/MCard1802ESP32Web.ino)
setup() function strips the key's character value from the query parameter and passes it to the prossesChar() function which
performs the action for the key.
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

The HTML body consists of a layout table to define the input key button definitions.  The keypress function is invoked for the onclick event.
The input button also invokes keypress for onmousedown and ontouchstart events to simulate cases where the input button is held down for a period of time.

I2C Front Panel
---------------
The I2C Front Panel card provides the MCP23008 for control lines, a 7400 Quad Nand logic chip for Memory Protect and inverse Q for serial communication TX line and MCP23017 IO Expander for data lines
along with a voltage regulator to provide 5v to the Membership Card if designed.  Jumpers can select if the Membership Card runs at the I2C voltage level or the 5v level.
The GPIO expanders support 3.3v to 5v level shifting, so the I2C inputs can be either 3.3v level or 5v level.  This allows the ESP32 to communicate with the Membership Card
running at 5v without separate level shifters for the I2C lines.  The I2C Front Panel card was based on the Front Panel Card design from the
[MCard1802ArduinoV2](https://github.com/fourstix/MCard1802ArduinoV2) project.

<table class="table table-hover table-striped table-bordered">
  <tr align="center">
   <td><img src="https://github.com/fourstix/MCard1802ESP32Web/blob/main/pics/WebPage.jpg"></td> 
   <td><img src="https://github.com/fourstix/MCard1802ESP32Web/blob/main/pics/I2CFrontPanelCard.jpg"></td> 
  </tr>
  <tr align="center">
    <td>ESP32 Web Page shown in Firefox browser</td>
    <td>ESP3 connected to the I2C Front Panel card with 1802 Membership Card.</td>
  </tr>
  <tr align="center">
    <td colspan="2"><img src="https://github.com/fourstix/MCard1802ESP32Web/blob/main/pics/I2CFrontPanelSchematic.jpg"></td>
  </tr>
  <tr align="center">
      <td colspan="2">I2C Front Panel Card Schematic</td>
  </tr>
</table>

Repository Contents
-------------------
* **/src/MCard1802ESP32Web/**
  * MCard1802ESP32Web.ino -- Main file for program that defines the ESP32Async Web Server including the server routes for REST API and AJAX functions.
  * MCard1802I2CFrontEnd.ino -- I2C Front End functions that process key input and communicate with the Membership Card.
  * secrets.h -- constants for network SSID and password. **Replace the dummy values with the correct private network values.**
* **/src/MCard1802ESP32Web/data/** -- SPIFFS files served by the Web Server
  * style.css -- Cascading style sheet that defines the two button styles
  * index.html -- HTML file that defines the page layout, JavaScript and AJAX functions.
* **/hardware/v1** -- PCB source files
  * MCard1802I2CFrontPanelv1Gerbers.zip -- Gerber files submitted to [JCLPCB](https://jlcpcb.com/) to build version 1 of the I2C Front Panel Card.
  * MCardI2CFrontPanelv1KiCad.zip -- KiCad 4.07 project files for version 1 of the I2C Front Panel Card.
* **/docs** -- documentation files
  * MCard1802I2CFrontPanel.pdf -- schematic for IC2 Front Panel Card.
* **/pics** -- pictures of sample configurations and examples


License Information
-------------------

This code is public domain under the MIT License, but please buy me a beer
if you use this and we meet someday (Beerware).

References to any products, programs or services do not imply
that they will be available in all countries in which their respective owner operates.

Sparkfun, the Sparkfun logo, and other Sparkfun products and services are
trademarks of the Sparkfun Electronics Corporation, in the United States,
other countries or both. 

Adafruit, the Adafruit logo, and other Adafruit products and services are
trademarks of the Adafruit Industries, in the United States,other countries or both.

Other company, product, or services names may be trademarks or services marks of others.

All libraries used in this code are copyright their respective authors.
  
Adafruit MCP23008 GPIO Expander Library
Copyright (c) 2012-2019 Adadruit Industries
Written by Limor Fried/Ladyada for Adafruit Industries.  

MCP23017 Arduino Library
Copyright (c) 2017 Bertrand Lemasle

ESP32 Remote relay controller program
Copyright (c) 2020  by Kazuhiro Ouchi
 
ESPAsyncWebServer
Copyright (c) 2016-2020 by me-no-dev@github

The Random Nerd ESP32 Tutorials published by Rui Santos
Copyright (c) 2013-2020 by RandomNerdTutorials.com
 
The 1802 Membership Card Microcomputer hardware design
Copyright (c) 2006-2020 by Lee A. Hart

Many thanks to the original authors for making their designs and code avaialble as open source.
 
This code, firmware, and software is released under the [MIT License](http://opensource.org/licenses/MIT).

The MIT License (MIT)

Copyright (c) 2020 by Gaston Williams

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

**THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.**