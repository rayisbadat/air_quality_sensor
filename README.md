# Summary
Will take air sensor value reading for Temp, Humidity, CO2 (ppm), and Air Quality (VOC raw val and an index 0-500)

# Assembly
Cable the two sensors to the board in a daisy chain via the STEMMA QT / Qwiic cables

# Parts List
* [Adafruit ESP32-s2 Reverse Feather](https://www.adafruit.com/product/5345)
  * Has built in Battery Monitor
  * Has built in TFT LCD screen  
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
* 2x STEMMA QT / Qwiic cables (No soldering needed)
* Battery


