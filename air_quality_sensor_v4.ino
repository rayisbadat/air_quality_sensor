/*
 * v1 First Pass
 * v2 Adding the aqi ppm sensor
 * v3 button controls
 * v4 adding lte support via tinyGMS
*/

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial

// Hardware-specific library for ST7789
#include <Adafruit_ST7789.h> 
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

// Adafruit PM2.5 sensor
#include "Adafruit_PM25AQI.h"
Adafruit_PM25AQI aqi = Adafruit_PM25AQI();

/////////////////
//LTE
#define TINY_GSM_MODEM_SIM7600
#define SerialAT Serial1

#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false
#define GSM_PIN ""

#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>
#include "./config.h"

TinyGsm        modem(SerialAT);
TinyGsmClient  client(modem);

#define GSM_AUTOBAUD_MIN 9600
#define GSM_AUTOBAUD_MAX 115200

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVER_PORT  80
#define AIO_SERVER_SSLPORT  443 //ssl

//////
bool displayOn = true;

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
    SerialMon.print("Error trying to execute readMeasurement(): ");
    errorToString(error, errorMessage, 256);
    SerialMon.println(errorMessage);
  } else if (air_values.co2 == 0) {
      SerialMon.println("Invalid sample detected, skipping.");
  } 

  return air_values;
}

struct voc_val get_voc(float temperature, float relative_humidity) {
  // Get VOC index
  struct voc_val voc_values;
  
  voc_values.sraw = sgp.measureRaw(temperature, relative_humidity);
  voc_values.voc_index = sgp.measureVocIndex(temperature, relative_humidity);

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
  tft.print(F("CO2:")); tft.print(co_two); tft.println(" ppm");

  tft.setTextColor(ST77XX_CYAN);
  tft.print("VOC Idx: "); tft.println(voc_index);

  tft.setTextColor(ST77XX_MAGENTA);
  tft.print(F("PM1.0: ")); tft.print(pm_values.pm10_env);tft.println(" ppm");
  tft.print(F("PM2.5: ")); tft.print(pm_values.pm25_env);tft.println(" ppm");
  tft.print(F("PM10: ")); tft.print(pm_values.pm100_env);tft.println(" ppm");

  text_size = 1;
  tft.setTextSize(text_size);
   
  tft.setTextColor(ST77XX_ORANGE);
  tft.print(F("Chrg: ")); tft.print(batt_perc,1); tft.print("%");
  tft.print(F(" Batt Volt: ")); tft.print(batt_volt,1); tft.println("V");
      
}

void pushMetrics(struct air_val air_values,struct voc_val voc_values, struct battery_val battery_values,PM25_AQI_Data pm_values) {
    HttpClient http(client, AIO_SERVER, AIO_SERVER_SSLPORT);

    String feedTemp = String("/api/v2/") + String(AIO_USERNAME) + String("/feeds/") + String("airquality.temp") + String("/data.json");
    String feedHumidity = String("/api/v2/") + String(AIO_USERNAME) + String("/feeds/") + String("airquality.humidity") + String("/data.json");
    String feedCO2 = String("/api/v2/") + String(AIO_USERNAME) + String("/feeds/") + String("airquality.co2") + String("/data.json");
    String feedVOC = String("/api/v2/") + String(AIO_USERNAME) + String("/feeds/") + String("airquality.voc") + String("/data.json");
    String feedPM10 = String("/api/v2/") + String(AIO_USERNAME) + String("/feeds/") + String("airquality.pm10") + String("/data.json");
    String feedPM25 = String("/api/v2/") + String(AIO_USERNAME) + String("/feeds/") + String("airquality.pm25") + String("/data.json");
    String feedPM100 = String("/api/v2/") + String(AIO_USERNAME) + String("/feeds/") + String("airquality.pm100") + String("/data.json");

    pushToAdafruit( feedTemp, String( air_values.temperature ) );
    pushToAdafruit( feedHumidity, String( air_values.relative_humidity ) );
    pushToAdafruit( feedCO2, String( air_values.co2 ) );
    pushToAdafruit( feedVOC, String( voc_values.voc_index ) );
    pushToAdafruit( feedPM10, String( pm_values.pm10_env ) );
    pushToAdafruit( feedPM25, String( pm_values.pm25_env ) );
    pushToAdafruit( feedPM100, String( pm_values.pm100_env ) );

}

void pushToAdafruit( String feedUrl, String value ) {
    HttpClient http(client, AIO_SERVER, AIO_SERVER_PORT);
    
    String contentType = "content-type: application/json;";    
    String stringBody = String("\{ \"value\": \"") + String(value) + String( "\" \}" );
    
    SerialMon.print(F("Trying to push Value: "));
    SerialMon.print(stringBody);
    SerialMon.print(F(" to URL Path: "));
    SerialMon.println(feedUrl);
    
    http.beginRequest();
      http.post(feedUrl);
      http.sendHeader(contentType);
      http.sendHeader("Accept: */*");
      http.sendHeader("X-AIO-Key",AIO_KEY);
      http.sendHeader("Content-Length: " + String(stringBody.length()));
      http.beginBody();
      http.print(stringBody);
    http.endRequest();

    int status = http.responseStatusCode();
    String respBody = http.responseBody();
    SerialMon.print(F("Response status code: "));
    SerialMon.println(status);
}

void setup(void) {
  SerialMon.begin(9600);
  SerialMon.println(F("Serial Terminal Initialized"));
  SerialMon.println(F("Starting Initialization of TFT Display"));

  pinMode(0,INPUT_PULLUP);
  pinMode(1,INPUT_PULLDOWN);
  pinMode(2,INPUT_PULLDOWN);
  
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
  SerialMon.println(F("Initialized TFT Display"));

  ////////////////////////////////////////
  //check for battery
  if (!maxlipo.begin()) {
    SerialMon.println(F("Couldnt find Adafruit MAX17048?\nMake sure a battery is plugged in!"));
    while (1) delay(10);
  }
  SerialMon.print(F("Found MAX17048")); Serial.print(F(" with Chip ID: 0x")); Serial.println(maxlipo.getChipID(), HEX);
  ////////////////////////////////////////

  ////////////////////////////////////////
  // Initialize SCD40
  SerialMon.println(F("Starting Initialization of SC40 Air Sensor"));
  Wire.begin();

  uint16_t error;
  char errorMessage[256];

  scd4x.begin(Wire);

  // stop potentially previously started measurement
  error = scd4x.stopPeriodicMeasurement();
  if (error) {
    SerialMon.print("Error trying to execute stopPeriodicMeasurement(): ");
    errorToString(error, errorMessage, 256);
    SerialMon.println(errorMessage);
  }
  ////////////////////////////////////////

  ////////////////////////////////////////
  // Initialize SCD40
  SerialMon.println(F("Starting Initialization of SC40 Air Sensor"));
  uint16_t serial0;
  uint16_t serial1;
  uint16_t serial2;
  error = scd4x.getSerialNumber(serial0, serial1, serial2);
  if (error) {
    SerialMon.print("Error trying to execute getSerialNumber(): ");
    errorToString(error, errorMessage, 256);
    SerialMon.println(errorMessage);
  } 

  // Start Measurement
  error = scd4x.startPeriodicMeasurement();
  if (error) {
    SerialMon.print("Error trying to execute startPeriodicMeasurement(): ");
    errorToString(error, errorMessage, 256);
    SerialMon.println(errorMessage);
  }
  //delay(5000);
  SerialMon.println(F("Initialized of SC40 Air Sensor"));
  ////////////////////////////////////////

  ////////////////////////////////////////
  // Start the MOX sensoe
  SerialMon.println("SGP40 test with SHT31 compensation, it takes time to start and get enough values");

  if (! sgp.begin()){
    SerialMon.println("SGP40 sensor not found :(");
    while (1);
  }
  ////////////////////////////////////////

  ////////////////////////////////////////
  delay(2000);
  aqi.begin_I2C();
  ////////////////////////////////////////

  ////////////////////////////////////////
  // Set GSM module baud rate
  TinyGsmAutoBaud(SerialAT, GSM_AUTOBAUD_MIN, GSM_AUTOBAUD_MAX);
  modem.restart();
  ////////////////////////////////////////

  delay(5000);
  SerialMon.println("done");

}

void loop() {

  struct battery_val battery_values;
  struct air_val air_values;
  struct voc_val voc_values;
  PM25_AQI_Data pm_values;

  bool pin1 = digitalRead(1);
  bool pin2 = digitalRead(2);

  if ( pin1 == 1 ) {
    //Send sleep mode command
    SerialMon.println("Button 2 pressed");
    tft.enableSleep( true );
    
    // turn off backlite
    pinMode(TFT_BACKLITE, OUTPUT);
    digitalWrite(TFT_BACKLITE, LOW);

    displayOn = false;

  } 
  if ( pin2 == 1 ){
    //send wake mode command
    SerialMon.println("Button 3 pressed");
    tft.enableSleep( false );
    
    // turn on backlite
    pinMode(TFT_BACKLITE, OUTPUT);
    digitalWrite(TFT_BACKLITE, HIGH); 

    displayOn = true;
  }
  
  // get batter status
  battery_values.volt = maxlipo.cellVoltage();
  battery_values.percent = maxlipo.cellPercent();
  air_values = get_air_values();
  voc_values = get_voc(air_values.temperature, air_values.relative_humidity);
  aqi.read(&pm_values);
 
  updateTFT(air_values,voc_values,battery_values,pm_values);

  SerialMon.print("displayOn:"); SerialMon.println(displayOn); 

  ////
  IPAddress local;
  SerialMon.println("Checking if modem ready");
  if (not modem.isNetworkConnected()) {
    //modem.gprsConnect(apn, gprsUser, gprsPass);
    SerialMon.print("Waiting for network...");
    if (!modem.waitForNetwork(600000L)) {  // You may need lengthen this in poor service areas
      SerialMon.println(F("waitForNetwork [fail]"));
      //delay(10000);
    }else{
      SerialMon.println(F("waitForNetwork [OK]"));
    }
  }

  if (modem.isNetworkConnected()){
    if (not modem.isGprsConnected()){
      modem.gprsConnect(apn, gprsUser, gprsPass);
    }
  }

  if( modem.isGprsConnected() ){  
    pushMetrics(air_values,voc_values,battery_values,pm_values);
  }

  delay(5000);
}
