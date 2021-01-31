# PRISM
An ESP-32 based led controller which provides an adaptable web interface for control and can be integrated into IOT networks.

## Dependencies
* **ArduinoJSON [bblanchon]**
* **ESPAsyncWebServer [me-no-dev]**
* **AsyncTCP [me-no-dev]**
* **FastLED [FastLED]**

## Installation
First install the dependancies listed above (FastLED and ArduinoJSON are available through the Arduino package manager)
The ESP asyncronous web server and the tcp library can be found on github under the user **me-no-dev**.
Installation instructions for those can be found on their respective pages.

After all dependencies are installed clone the repository and open src/server.cpp. Ensure that all dependencies are being resolved from within the Arduino IDE or PlatformIO

```git clone https://github.com/logansurface/prism```

Build and upload to the ESP. A new network should appear under your WiFi manger named PRISM Web Setup, the password is defaultpassword1234.
Open up a serial monitor and set the baud rate to 115200. You should see an ip address for the ESP displayed there. Navigate to *http://ip-address:80/* and follow the instructions displayed on the web page there.
