/*
 * v1 First Pass
 * v2 Adding the aqi ppm sensor
*/
//#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

// Use dedicated hardware SPI pins
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

//Battery Monitor 
#include "Adafruit_MAX1704X.h"
Adafruit_MAX17048 maxlipo;

//SCD40 air sensor
#include <Wire.h>
#include "SparkFun_SCD4x_Arduino_Library.h" 
SCD4x mySensor;
#include <Arduino.h>
#include <SensirionI2CScd4x.h>
#include <Wire.h>
SensirionI2CScd4x scd4x;

//SGP MOX gas sensor
#include "Adafruit_SGP40.h"
#include "Adafruit_SHT31.h"
Adafruit_SGP40 sgp;
Adafruit_SHT31 sht31;

/* Test sketch for Adafruit PM2.5 sensor with UART or I2C */
#include "Adafruit_PM25AQI.h"
Adafruit_PM25AQI aqi = Adafruit_PM25AQI();


struct battery_val {
  float volt;
  float percent;
};

struct air_val {
  uint16_t co2;
  float temperature;
  float relative_humidity;
};

struct voc_val {
  uint16_t sraw;
  int32_t voc_index;
};

struct air_val get_air_values() {
  uint16_t error;
  char errorMessage[256];
  struct air_val air_values;

  error = scd4x.readMeasurement(air_values.co2, air_values.temperature, air_values.relative_humidity);
  if (error) {
    Serial.print("Error trying to execute readMeasurement(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  } else if (air_values.co2 == 0) {
      Serial.println("Invalid sample detected, skipping.");
  } 

  return air_values;
}

struct voc_val get_voc(float temperature, float relative_humidity) {
  // Get VOC index
  struct voc_val voc_values;
  
  voc_values.sraw = sgp.measureRaw(temperature, relative_humidity);

  Serial.print("Raw measurement: ");
  Serial.println(voc_values.sraw);

  voc_values.voc_index = sgp.measureVocIndex(temperature, relative_humidity);
  Serial.print("Voc Index: ");
  Serial.println(voc_values.voc_index);

  return voc_values;
}

//void updateTFT(float temp, float humidity, uint16_t co_two, int32_t voc_index, uint16_t voc_raw , float batt_volt, float batt_perc) {
void updateTFT(struct air_val air_values,struct voc_val voc_values, struct battery_val battery_values,PM25_AQI_Data pm_values) {

  float temp = air_values.temperature;
  float humidity = air_values.relative_humidity;
  uint16_t co_two = air_values.co2; 
  int32_t voc_index = voc_values.voc_index;
  uint16_t voc_raw = voc_values.sraw;
  float batt_volt = battery_values.volt;
  float batt_perc = battery_values.percent;

  int text_size = 2;
  tft.setTextSize(text_size);

  tft.setCursor(0, 0);
  tft.fillScreen(ST77XX_BLACK);
  
  tft.setTextColor(ST77XX_WHITE);
  tft.print(F("Temp: ")); tft.print(temp); tft.print("C ");
  tft.print(((temp*9)/5) + 32); tft.println("F");

  tft.setTextColor(ST77XX_YELLOW);
  tft.print(F("Rel Humidity: ")); tft.print(humidity); tft.println("%");

  tft.setTextColor(ST77XX_GREEN);
  tft.print(F("CO2:")); tft.print(co_two); tft.println("ppm");

  tft.setTextColor(ST77XX_BLUE);
  //tft.print("VOC: "); tft.print(voc_raw); tft.println("ppm");
  //tft.print("VOC Raw: "); tft.print(voc_raw);
  tft.print("VOC Idx: "); tft.println(voc_index);

  text_size = 1;
  tft.setTextSize(text_size);
   
  tft.setTextColor(ST77XX_ORANGE);
  tft.print(F("Chrg: ")); tft.print(batt_perc,1); tft.print("% ");
  tft.print(F("Batt Volt: ")); tft.print(batt_volt,1); tft.println("V");
    

  tft.setTextColor(ST77XX_MAGENTA);
  tft.print(F("PM1.0: ")); tft.print(pm_values.pm10_env);tft.print("ppm ");
  tft.print(F("PM2.5: ")); tft.print(pm_values.pm25_env);tft.print("ppm ");
  tft.print(F("PM10: ")); tft.print(pm_values.pm100_env);tft.println("ppm");
}


void setup(void) {
  Serial.begin(9600);
  /*
  while (!Serial) {
        delay(100);
  }
  */
  Serial.println(F("Serial Terminal Initialized"));

  Serial.println(F("Starting Initialization of TFT Display"));
  // turn on backlite
  pinMode(TFT_BACKLITE, OUTPUT);
  digitalWrite(TFT_BACKLITE, HIGH);

  // turn on the TFT / I2C power supply
  pinMode(TFT_I2C_POWER, OUTPUT);
  digitalWrite(TFT_I2C_POWER, HIGH);
  delay(10);

  // initialize TFT
  tft.init(135, 240); // Init ST7789 240x135
  tft.setRotation(3);
  tft.fillScreen(ST77XX_BLACK);

  //Configure tft
  tft.setTextWrap(false);
  Serial.println(F("Initialized TFT Display"));

  ////////////////////////////////////////
  //check for battery
  if (!maxlipo.begin()) {
    Serial.println(F("Couldnt find Adafruit MAX17048?\nMake sure a battery is plugged in!"));
    while (1) delay(10);
  }
  Serial.print(F("Found MAX17048")); Serial.print(F(" with Chip ID: 0x")); Serial.println(maxlipo.getChipID(), HEX);
  ////////////////////////////////////////

  ////////////////////////////////////////
  // Initialize SCD40
  Serial.println(F("Starting Initialization of SC40 Air Sensor"));
  Wire.begin();

  uint16_t error;
  char errorMessage[256];

  scd4x.begin(Wire);

  // stop potentially previously started measurement
  error = scd4x.stopPeriodicMeasurement();
  if (error) {
    Serial.print("Error trying to execute stopPeriodicMeasurement(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  }
  ////////////////////////////////////////

  ////////////////////////////////////////
  // Initialize SCD40
  Serial.println(F("Starting Initialization of SC40 Air Sensor"));
  uint16_t serial0;
  uint16_t serial1;
  uint16_t serial2;
  error = scd4x.getSerialNumber(serial0, serial1, serial2);
  if (error) {
    Serial.print("Error trying to execute getSerialNumber(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  } 

  // Start Measurement
  error = scd4x.startPeriodicMeasurement();
  if (error) {
    Serial.print("Error trying to execute startPeriodicMeasurement(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  }
  //delay(5000);
  Serial.println(F("Initialized of SC40 Air Sensor"));
  ////////////////////////////////////////

  ////////////////////////////////////////
  // Start the MOX sensoe
    Serial.println("SGP40 test with SHT31 compensation, it takes time to start and get enough values");

  if (! sgp.begin()){
    Serial.println("SGP40 sensor not found :(");
    while (1);
  }

  if (! sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
  //if (! sht31.begin(0x45)) {   // Set to 0x45 for alternate i2c addr
    
    Serial.println("Couldn't find SHT31");
  //  while (1);
  }

  Serial.print("Found SHT3x + SGP40 serial #");
  Serial.print(sgp.serialnumber[0], HEX);
  Serial.print(sgp.serialnumber[1], HEX);
  Serial.println(sgp.serialnumber[2], HEX);
  ////////////////////////////////////////

  ///////////
  delay(2000);
  //if (! aqi.begin_I2C()) {      // connect to the sensor over I2C
  aqi.begin_I2C();
  //Serial.println("Could not find PM 2.5 sensor!");
  //  while (1) delay(10);
  //}

  
  delay(5000);
  Serial.println("done");
}


void loop() {

  struct battery_val battery_values;
  struct air_val air_values;
  struct voc_val voc_values;
  //struct pm_val pm_values;
  //PM25_AQI_Data data;
  PM25_AQI_Data pm_values;
  
  // get batter status
  battery_values.volt = maxlipo.cellVoltage();
  battery_values.percent = maxlipo.cellPercent();
 
  air_values = get_air_values();
  voc_values = get_voc(air_values.temperature, air_values.relative_humidity);

  
  aqi.read(&pm_values);
  /* 
  aqi.read(&data);
  pm_values.pm10_standard = data.pm10_standard;
  pm_values.pm25_standard = data.pm25_standard;
  pm_values.pm100_standard = data.pm100_standard;
  pm_values.pm10_env = data.pm10_env;
  pm_values.pm25_env = data.pm25_env;
  pm_values.pm100_env = data.pm100_env;
  
  Serial.print(F("PM 1.0: ")); Serial.print(data.pm10_env);
  Serial.print(F("\t\tPM 2.5: ")); Serial.print(data.pm25_env);
  Serial.print(F("\t\tPM 10: ")); Serial.println(data.pm100_env);
  Serial.println(F("---------------------------------------"));
  Serial.print(F("Particles > 0.3um / 0.1L air:")); Serial.println(data.particles_03um);
  Serial.print(F("Particles > 0.5um / 0.1L air:")); Serial.println(data.particles_05um);
  Serial.print(F("Particles > 1.0um / 0.1L air:")); Serial.println(data.particles_10um);
  Serial.print(F("Particles > 2.5um / 0.1L air:")); Serial.println(data.particles_25um);
  Serial.print(F("Particles > 5.0um / 0.1L air:")); Serial.println(data.particles_50um);
  Serial.print(F("Particles > 10 um / 0.1L air:")); Serial.println(data.particles_100um);
  Serial.println(F("---------------------------------------"));
  */
  //Update display
  //updateTFT(air.temperature,air.relative_humidity,air.co2,voc_values.voc_index,voc_values.sraw,battery.volt,battery.percent);
  updateTFT(air_values,voc_values,battery_values,pm_values);


  delay(5000);
  
}
