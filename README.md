# Installation and preparation

Setup packages for Arduino support for ESP8266 *(File -> Preferences -> Aditional Boards Manager URL). Remember to put a comma in between urls.
```
https://dl.espressif.com/dl/package_esp32_index.json,http://arduino.esp8266.com/stable/package_esp8266com_index.json
```

Download and import libraries *(Arduino IDE -> Sketch -> Include Library -> Add .ZIP Library)*
- <https://github.com/me-no-dev/ESPAsyncWebServer><br>
- <https://github.com/me-no-dev/ESPAsyncTCP>

*(Arduino IDE -> Sketch -> Include Library -> Manage Libraries...)*
- <code>ArduinoJson</code>
- <code>AsyncElegantOta</code>

Setup SPIFFS for Arduino IDE (Arduino version below  2)<br>
<https://github.com/esp8266/arduino-esp8266fs-plugin/releases/tag/0.5.0>
<ol>
    <li>Download last release and unzip it.</li>
    <li>Go to Documents/Arduino and create a folder named 'tools' (ignore step if folder already exists)</li>
    <li>Paste the ESP8266FS folder into the new created folder.</li>
    <li>Restart Arduino IDE</li>
</ol>


# Adding files SPIFFS
<ol>
    <li>Rename folder <code>data.example</code> and all files inside removing the <strong>.example</strong> extension.</li>
    <li>Open every <code>.dat</code> file and fill with your own configuration.</li>
    <li>Setup <strong>board</strong> and <strong>port</strong> configuration in Arduino IDE</li>
    <li>Load files by clicking <em>Tools -> ESP8266 Sketch Data Upload</em> </li>
    <li>Load program into board and enjoy!</li>
</ol>