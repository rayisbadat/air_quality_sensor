# Summary
Will take air sensor value reading for Temp, Humidity, CO2 (ppm), and Air Quality (VOC raw val and an index 0-500)

# Assembly
* Cable the sensors to the board in a daisy chain via the STEMMA QT / Qwiic cables. 
* Insert the sim card, and connect the attenae
* Cabling
  * Feather TX Pin <-> SIM 7xxx LTE Module RX Pin
  * Feather RX Pin <-> SIM 7xxx LTE Module TX Pin
  * Feather TX Pin <-> SIM 7xxx LTE Module RX Pin
  * Feather GND Pin <-> SIM 7xxx LTE Module GND Pin
  * Feather 3v3 Pin <-> SIM 7xxx LTE Module 5v Pin
  * Connect Battery to battery port
  * Connect the sensors via STEMMA QT / QWIIC cables in a chain. Order does not matter.
  * Connect the Celluar antenna to MAIN (SIM7600) or the LTE (SIM7060) depending on board labeling.
  * Optional:
    * If you hook up one of the control pins, or the other 3v3 pin to a pin on the SIM7xxxx board, and move a jumper you can turn the LTE board on and off programiclly.  I didnt need to do that so dont know which pins and jumper you need to do.
    * Connect the GPS antenna (SIM7600) to AUX or GNSS (SIM7060) depending on board labeling.

# Parts List
* Board
  * [Adafruit ESP32-s2 Reverse Feather](https://www.adafruit.com/product/5345)
    * Has built in Battery Monitor
    * Has built in TFT LCD screen  
    * Has built in WiFi if needed
* Sensors
  * [SGP40 MOX gas sensor](https://www.adafruit.com/product/4829) 
    * For the VOC raw and Quality Index
    * I dont know what if any units the raw value is in. I was assuming its ppm, but i havent dug through the library code to deeply to confirm that.  It might just be a raw sensor value that has no meaning outside the algorithm it runs it through to generate an air quality index.
    * `Please note, this sensor, like all VOC/gas sensors, has variability, and to get precise measurements you will want to calibrate it against known sources! That said, for general environmental sensors, it will give you a good idea of trends and comparison.` 
      * i didnt calibrate and have no known sources
  * [SCD40 Air Sensor](https://www.adafruit.com/product/5187)
    * For temp, humity, and CO2
    * Temp and Humidity seem accurateish,  
    * spec claims `The SCD-40 is lower cost, and is perfect for indoor/outdoor air quality and CO2 measurements. It has a range of 400~2000 ppm with an accuracy of ±(50 ppm + 5% of reading)`
      * I have no way to validate/calibrate CO2 levels
  * [Adafruit PMSA003I Air Quality Breakout](https://www.adafruit.com/product/4632)
* LTE module
  * 1st Try: [SIM-7600A](https://www.amazon.com/4G-HAT-SIM7600A-H-Communication-Positioning/dp/B07PLTP3M6)
    * Worked fine, but didnt have https support in TinyGSM
    * Use the v4 sketch file
    * I am unsure and it wasnt labeled but... I think the antenna has a SMA male connector, the board has a MHF or UFL female connector surface mounted.  And the kit came with the necessary conversion cable
  * 2nd Try:  [SIM7060G](https://www.amazon.com/SIM7070G-NB-IoT-Cat-M-GPRS-GNSS/dp/B08DVCX4PG)
    * Will try in v5 file
    * Hoping smaller formfactore will make final assembly easier..looks same size as a feather.
* Sim Card
  * (EiotClub)[https://www.amazon.com/EIOTCLUB-Data-SIM-Card-Days/dp/B09QKF2M2S]
* Misc
  * 2x STEMMA QT / Qwiic cables (No soldering needed)
  * Battery
* Helpful links
  * [The Feather Board Pinouts](https://learn.adafruit.com/esp32-s2-reverse-tft-feather/pinouts)
  * [7600 to Arduino connections](https://www.waveshare.com/wiki/7600X_connect_Arduino)
  * [SIM7600 Board](https://www.waveshare.com/wiki/SIM7600E-H_4G_HAT)
  * [Random Blog about https and TinyGSM](https://dogcomp.medium.com/send-https-request-from-sim-7600x-lte-module-4f76be19e900)
  * [TinyGSM code and exampels](https://github.com/vshymanskyy/TinyGSM)
  * [Adafruit IO API Docs](https://io.adafruit.com/api/docs/#create-data)
