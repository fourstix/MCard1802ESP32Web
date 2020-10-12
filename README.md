# MCard1802ESP32Web
This repository contains  Web based front panel for the 1802 Membership Card using ESPAsynchWebServer running on an ESP32 

My first computer was a Netronics Elf II. It was an RCA 1802 based single board computer that was sold as a kit,
but it had a hexadecimal keypad, an LED, video and an expansion bus. Like many high school kids, I mowed yards
and spent my hard-earned dollars to buy a kit from Netronics [based on their ads.](http://www.cosmacelf.com/gallery/netronics-ads/)

My orignal Elf II was lost in a move long ago, but today Lee Hart's [1802 Membership card.]((http://www.sunrise-ev.com/1802.htm) 
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
[ESP32 SPIFFS file system.](https://github.com/me-no-dev/arduino-esp32fs-plugin)  