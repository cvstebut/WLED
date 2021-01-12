#pragma once
#include "wled.h"
#include <Wire.h>
#include <BME280I2C.h> //BME280 sensor
#include "LedControl.h"
#include "LedControl_HW_SPI.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <array>

#define Celsius // Show temperature mesaurement in Celcius otherwise is in Fahrenheit

/* Recommended Modes -
   Based on Bosch BME280I2C environmental sensor data sheet.
Weather Monitoring :
   forced mode, 1 sample/minute
   pressure ×1, temperature ×1, humidity ×1, filter off
   Current Consumption =  0.16 μA
   RMS Noise = 3.3 Pa/30 cm, 0.07 %RH
   Data Output Rate 1/60 Hz
Humidity Sensing :
   forced mode, 1 sample/second
   pressure ×0, temperature ×1, humidity ×1, filter off
   Current Consumption = 2.9 μA
   RMS Noise = 0.07 %RH
   Data Output Rate =  1 Hz
Indoor Navigation :
   normal mode, standby time = 0.5ms
   pressure ×16, temperature ×2, humidity ×1, filter = x16
   Current Consumption = 633 μA
   RMS Noise = 0.2 Pa/1.7 cm
   Data Output Rate = 25Hz
   Filter Bandwidth = 0.53 Hz
   Response Time (75%) = 0.9 s
Gaming :
   normal mode, standby time = 0.5ms
   pressure ×4, temperature ×1, humidity ×0, filter = x16
   Current Consumption = 581 μA
   RMS Noise = 0.3 Pa/2.5 cm
   Data Output Rate = 83 Hz
   Filter Bandwidth = 1.75 Hz
   Response Time (75%) = 0.3 s
*/

// Default : forced mode aka Weather Mode, standby time = 1000 ms
// Oversampling = pressure ×1, temperature ×1, humidity ×1, filter off,
BME280I2C::Settings bmeSettings0x76(
    BME280::OSR_X1,
    BME280::OSR_X1,
    BME280::OSR_X1,
    BME280::Mode_Forced,
    BME280::StandbyTime_1000ms,
    BME280::Filter_Off,
    BME280::SpiEnable_False,
    BME280I2C::I2CAddr_0x76 // I2C address. (Either 0x76 or 0x77 - "normal": 0x76, Adafruit: 0x77)
);
// Default : forced mode aka Weather Mode, standby time = 1000 ms
// Oversampling = pressure ×1, temperature ×1, humidity ×1, filter off,
BME280I2C::Settings bmeSettings0x77(
    BME280::OSR_X1,
    BME280::OSR_X1,
    BME280::OSR_X1,
    BME280::Mode_Forced,
    BME280::StandbyTime_1000ms,
    BME280::Filter_Off,
    BME280::SpiEnable_False,
    BME280I2C::I2CAddr_0x77 // I2C address. (Either 0x76 or 0x77 - "normal": 0x76, Adafruit: 0x77)
);

BME280I2C bme76(bmeSettings0x76);
BME280I2C bme77(bmeSettings0x77);

#ifdef ARDUINO_ARCH_ESP32 //ESP32 boards
uint8_t SCL_PIN = 22;
uint8_t SDA_PIN = 21;
#else //ESP8266 boards
uint8_t SCL_PIN = 5;
uint8_t SDA_PIN = 4;
// uint8_t RST_PIN = 16; // Uncoment for Heltec WiFi-Kit-8
#endif

int csPinDisplay0 = 5;   
int csPinDisplay1 = 17;  // tested PINs: 17, 27  - should be "true GPIO" Pins
LedControl_HW_SPI lcBme0 = LedControl_HW_SPI();
LedControl_HW_SPI lcBme1 = LedControl_HW_SPI();

/*
 * Usermods allow you to add own functionality to WLED more easily
 * See: https://github.com/Aircoookie/WLED/wiki/Add-own-functionality
 * 
 * This is an example for a v2 usermod.
 * v2 usermods are class inheritance based and can (but don't have to) implement more functions, each of them is shown in this example.
 * Multiple v2 usermods can be added to one compilation easily.
 * 
 * Creating a usermod:
 * This file serves as an example. If you want to create a usermod, it is recommended to use usermod_v2_empty.h from the usermods folder as a template.
 * Please remember to rename the class and file to a descriptive name.
 * You may also use multiple .h and .cpp files.
 * 
 * Using a usermod:
 * 1. Copy the usermod into the sketch folder (same folder as wled00.ino)
 * 2. Register the usermod by adding #include "usermod_filename.h" in the top and registerUsermod(new MyUsermodClass()) in the bottom of usermods_list.cpp
 */

//class name. Use something descriptive and leave the ": public Usermod" part :)
class Bme280MqttUsermod : public Usermod
{
private:
  //Private class members. You can declare variables and functions only accessible to your usermod here
  unsigned long lastTime = 0;
  // BME280 sensor timer
  long sensorTimer = millis();
  long lastMqttReport = 0;
  long lastMeasurement = 0;
  long reportMqttPeriod = 60000; // publish measurement every bmeMqttPeriod milli seconds
  long measurePeriod = 60000;     // get measurement every measurePeriod milli seconds

  static const int sensorCount = 2;

  class Sensor
  {
  public:
    Sensor()
    {
      sensorType = "undef";
      sensorId = "undef";
      sensor = bme76;
      detected = false;
      displayOnSevenSegment = false;
      temperature = 0.0F;
      humidity = 0.0F;
      pressure = 0.0F;
    };

    Sensor(String sT, String sId, BME280I2C *bme, bool present, bool display, int displayId)
    {
      sensorType = sT;
      sensorId = sId;
      sensor = *bme;
      detected = present;
      displayOnSevenSegment = display;
    };

    String sensorType;
    String sensorId;
    BME280I2C sensor;
    bool detected = false;
    bool displayOnSevenSegment = false;
    float pressure;
    float temperature;
    float humidity;

    void writeQuad(LedControl_HW_SPI *lc, int group, float value)
    {
      // convert value to 4 char string with one decimal
      int digit = group * 4 - 1;
      int groupLength = 5;

      // round to 1 decimal place
      value = round(value * 10.0) / 10.0;

      std::ostringstream strStream;
      std::string stringValue = "-----";
      // handle overflow by setting value to "-----"
      if (value >= 1000.0)
      {
        stringValue = "-----";
      }
      else
      {
        strStream << std::setw(groupLength) << std::setfill(' ') << std::fixed << std::setprecision(1) << value;
        stringValue = strStream.str();
        strStream.str("");
      }
      //    Serial.println(stringValue.c_str());

      (*lc).setChar(0, digit--, stringValue[groupLength - 5], false);
      (*lc).setChar(0, digit--, stringValue[groupLength - 4], false);
      (*lc).setChar(0, digit--, stringValue[groupLength - 3], true);
      (*lc).setChar(0, digit--, stringValue[groupLength - 1], false);
    }

    void writeOct(LedControl_HW_SPI *lc, float value)
    {
      // convert value to 8 char string with one decimal
      int digit = 7;
      int groupLength = 9;

      // round to 1 decimal place
      value = round(value * 10.0) / 10.0;

      std::ostringstream strStream;
      std::string stringValue = "---------";
      // handle overflow by setting value to "-----"
      if (value >= 10000000.0)
      {
        stringValue = "---------";
      }
      else
      {
        strStream << std::setw(groupLength) << std::setfill(' ') << std::fixed << std::setprecision(1) << value;
        stringValue = strStream.str();
        strStream.str("");
      }
      //    Serial.println(stringValue.c_str());

      (*lc).setChar(0, digit--, stringValue[groupLength - 9], false);
      (*lc).setChar(0, digit--, stringValue[groupLength - 8], false);
      (*lc).setChar(0, digit--, stringValue[groupLength - 7], false);
      (*lc).setChar(0, digit--, stringValue[groupLength - 6], false);
      (*lc).setChar(0, digit--, stringValue[groupLength - 5], false);
      (*lc).setChar(0, digit--, stringValue[groupLength - 4], false);
      (*lc).setChar(0, digit--, stringValue[groupLength - 3], true);
      (*lc).setChar(0, digit--, stringValue[groupLength - 1], false);
    }

    void updateSensorData()
    {
      float temp(NAN), hum(NAN), pres(NAN);
#ifdef Celsius
      BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
#else
      BME280::TempUnit tempUnit(BME280::TempUnit_Fahrenheit);
#endif
      BME280::PresUnit presUnit(BME280::PresUnit_Pa);
      sensor.read(pres, temp, hum, tempUnit, presUnit);
      temperature = temp;
      humidity = hum;
      pressure = pres;
    }

    DynamicJsonDocument getJson()
    {
      DynamicJsonDocument jsonData(1024);

      jsonData[String("sensor_id")] = sensorId.c_str();
      jsonData[String("sensor_type")] = sensorType.c_str();
      jsonData[String("temperature")] = temperature;
      jsonData[String("pressure")] = pressure;
      if (sensorType == "bme280")
      {
        jsonData[String("humidity")] = humidity;
      }

      return jsonData;
    };

    void addInfoData(JsonObject *userData)
    {
      std::ostringstream s;
      s << sensorType.c_str() << "-" << sensorId.c_str();
      std::string arrayNamePrefix = s.str();

      std::string arrayName;

      if (detected)
      {
        arrayName = arrayNamePrefix + "-temperature";
        JsonArray bmeTemp = (*userData).createNestedArray(arrayName);
        bmeTemp.add(temperature); // value
        bmeTemp.add(" °C");

        arrayName = arrayNamePrefix + "-pressure";
        JsonArray bmePressure = (*userData).createNestedArray(arrayName);
        bmePressure.add(pressure);
        bmePressure.add(" Pa");

        if (sensorType == "bme280")
        {
          arrayName = arrayNamePrefix + "-humidity";
          JsonArray bmeHum = (*userData).createNestedArray(arrayName);
          bmeHum.add(humidity);
          bmeHum.add(" %RH");
        }
      }
      else
      {
        arrayName = arrayNamePrefix + "-sensor";
        JsonArray bmeTemp = (*userData).createNestedArray(arrayName);
        bmeTemp.add(0.0F); // value
        bmeTemp.add(" - not detected!");
      }
    }

    void displayData()
    {
      //          Serial.println("LED-Display: Output temperature and humidity");
      writeQuad(&lcBme0, 2, temperature);
      writeQuad(&lcBme0, 1, humidity);
      writeOct(&lcBme1, pressure);
    }
  };

public:
  std::array<Sensor, 2> sensors;
  bool sevenSegmentConnected = true;
  int sevenSegmentSensor = 0; // sensor array id [0|1]) that should be displayed

  bool sensorConnected = false;
  //Functions called by WLED
  /*
     * setup() is called once at boot. WiFi is not yet connected at this point.
     * You can use it to initialize variables, sensors or similar.
     */
  void setup()
  {
    Serial.println("bme280Usermod - Setup");

    sensors[0] = Sensor(String("bmX280"), String("0x76"), &bme76, false, false, 0);
    sensors[1] = Sensor(String("bmX280"), String("0x77"), &bme77, true, true, 1);

    Wire.begin(SDA_PIN, SCL_PIN);

    for (unsigned int i = 0; i < sensors.size(); i++)
    {
      if (sensors[i].sensor.begin())
      {
        Sensor *cs = &(sensors[i]);

        switch (cs->sensor.chipModel())
        {
        case BME280::ChipModel_BME280:
          cs->sensorType = "bme280";
          cs->detected = true;
          break;
        case BME280::ChipModel_BMP280:
          cs->sensorType = "bmp280";
          cs->detected = true;
          break;
        default:
          cs->sensorType = "unknown";
          cs->detected = false;
        }
      }
    };

    for (unsigned int i = 0; i < sensors.size(); i++)
    {
      Sensor *cs = &sensors[i];

      Serial.print("bme280Usermod - bme280/bmp280 sensor detection: ");
      //      Serial.print(cs->sensorType.c_str());
      Serial.print(" - I2C address ");
      Serial.print(cs->sensorId.c_str());

      if (cs->detected)
      {
        Serial.print(" - Detected sensor type: ");
        Serial.println(cs->sensorType.c_str());
      }
      else
      {
        Serial.println(" - No sensor detected");
      }
    };

    lcBme0.begin(csPinDisplay0);
    /*
    The MAX72XX is in power-saving mode on startup,
    we have to do a wakeup call
    */
    lcBme0.shutdown(0, false);
    /* Set the brightness to a medium values */
    lcBme0.setIntensity(0, 0);
    /* and clear the display */
    lcBme0.clearDisplay(0);

    lcBme1.begin(csPinDisplay1);
    lcBme1.shutdown(0, false);
    lcBme1.setIntensity(0, 0);
    lcBme1.clearDisplay(0);
  };

  /*
     * connected() is called every time the WiFi is (re)connected
     * Use it to initialize network interfaces
     */
  void connected()
  {
    //Serial.println("Connected to WiFi!");
  }

  /*
     * loop() is called continuously. Here you can check for events, read sensors, etc.
     * 
     * Tips:
     * 1. You can use "if (WLED_CONNECTED)" to check for a successful network connection.
     *    Additionally, "if (WLED_MQTT_CONNECTED)" is available to check for a connection to an MQTT broker.
     * 
     * 2. Try to avoid using the delay() function. NEVER use delays longer than 10 milliseconds.
     *    Instead, use a timer check as shown here.
     */
  void loop()
  {
    if (millis() - lastTime > 5000)
    {
      // Serial.println("bme280 - I'm alive!");
      lastTime = millis();
    }

    // BME280 sensor handling
    sensorTimer = millis();

    // "higher-frequency" measurements for display on seven segment display
    if (sevenSegmentConnected && (sensorTimer - lastMeasurement > measurePeriod))
    {
      lastMeasurement = sensorTimer;
      for (unsigned int i = 0; i < sensors.size(); i++)
      {
        Sensor *cs = &sensors[i];
        if ((*cs).displayOnSevenSegment)
        {
          (*cs).updateSensorData();
          (*cs).displayData();
        }
      }
    }

    // "lower-frequency" measurements for reporting via mqtt. Just display last high-frequency measurement, if
    //  seven segment is connected - otherwise: execute needed measurement
    // - Timer to publish new sensor data every bmeMqttPeriod seconds
    // - Only send mqtt data if sensor is connected
    if (sensorTimer - lastMqttReport > reportMqttPeriod)
    {
      lastMqttReport = sensorTimer;
      for (unsigned int i = 0; i < sensors.size(); i++)
      {
        Sensor *cs = &sensors[i];

        std::ostringstream s;
        s << "bme280Usermod - current sensor data - index: " << i << ", sensorId: " << cs->sensorId.c_str() << ", detected: " << cs->detected << std::endl;
        //        Serial.println(s.str().c_str());

        if (cs->detected)
        {
          // no need to execute measurement in case of display on seven segment. Just use the last measurement done in the "high-frequency"
          // display loop
          if (!(*cs).displayOnSevenSegment)
          {
            cs->updateSensorData();
          }

          // Check if MQTT Connected, otherwise it will crash the 8266
          if (mqtt != nullptr)
          {
            DynamicJsonDocument jsonData = cs->getJson();

            // Create string populated with user defined device topic from the UI, and the read temperature, humidity and pressure. Then publish to MQTT server.
            String d = String(mqttDeviceTopic);
            d += "/data";

            String output;
            serializeJson(jsonData, output);

            Serial.print("bme280Usermod - mqtt connected - message: ");
            Serial.println(output);
            mqtt->publish(d.c_str(), 0, true, output.c_str());
          }
          else
          {
            Serial.println("bme280Usermod - mqtt not connected!");
          }
        }
      }
    }
  }
  /*
     * addToJsonInfo() can be used to add custom entries to the /json/info part of the JSON API.
     * Creating an "u" object allows you to add custom key/value pairs to the Info section of the WLED web UI.
     * Below it is shown how this could be used for e.g. a light sensor
     */

  void addToJsonInfo(JsonObject &root)
  {
    //this code adds "u":{"Light":[20," lux"]} to the info object
    JsonObject user = root["u"];
    if (user.isNull())
    {
      user = root.createNestedObject("u");
    }

    for (unsigned int i = 0; i < sensors.size(); i++)
    {
      Sensor *cs = &sensors[i];
      (*cs).addInfoData(&user);
    }
  }

  /*
     * addToJsonState() can be used to add custom entries to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
  void addToJsonState(JsonObject &root)
  {
    const std::string usermodSection = "bme280mqtt";
    JsonObject user = root[usermodSection];
    if (user.isNull())
      user = root.createNestedObject(usermodSection);

    for (unsigned int i = 0; i < sensors.size(); i++)
    {
      Sensor *cs = &sensors[i];

      if ((*cs).detected)
      {
        std::ostringstream s;
        s << (*cs).sensorType.c_str() << "-" << (*cs).sensorId.c_str();
        std::string sensorSection = s.str();

        JsonObject sensorData = user[sensorSection];
        if (sensorData.isNull())
          sensorData = user.createNestedObject(sensorSection);

        sensorData.set((*cs).getJson().as<JsonObject>());
      }
    }
  }

  /*
     * readFromJsonState() can be used to receive data clients send to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
  /*
  void readFromJsonState(JsonObject &root)
  {
    userVar0 = root["user0"] | userVar0; //if "user0" key exists in JSON, update, else keep old value
    //if (root["bri"] == 255) Serial.println(F("Don't burn down your garage!"));
  }
  */
  /*
     * addToConfig() can be used to add custom persistent settings to the cfg.json file in the "um" (usermod) object.
     * It will be called by WLED when settings are actually saved (for example, LED settings are saved)
     * If you want to force saving the current state, use serializeConfig() in your loop().
     * 
     * CAUTION: serializeConfig() will initiate a filesystem write operation.
     * It might cause the LEDs to stutter and will cause flash wear if called too often.
     * Use it sparingly and always in the loop, never in network callbacks!
     * 
     * addToConfig() will also not yet add your setting to one of the settings pages automatically.
     * To make that work you still have to add the setting to the HTML, xml.cpp and set.cpp manually.
     * 
     * I highly recommend checking out the basics of ArduinoJson serialization and deserialization in order to use custom settings!
     */
  /*
  void addToConfig(JsonObject &root)
  {
    JsonObject top = root.createNestedObject("exampleUsermod");
    top["great"] = userVar0; //save this var persistently whenever settings are saved
  }
  */
  /*
     * readFromConfig() can be used to read back the custom settings you added with addToConfig().
     * This is called by WLED when settings are loaded (currently this only happens once immediately after boot)
     * 
     * readFromConfig() is called BEFORE setup(). This means you can use your persistent values in setup() (e.g. pin assignments, buffer sizes),
     * but also that if you want to write persistent values to a dynamic buffer, you'd need to allocate it here instead of in setup.
     * If you don't know what that is, don't fret. It most likely doesn't affect your use case :)
     */
  /*
  void readFromConfig(JsonObject &root)
  {
    JsonObject top = root["top"];
    userVar0 = top["great"] | 42; //The value right of the pipe "|" is the default value in case your setting was not present in cfg.json (e.g. first boot)
  }
  */

  /*
     * getId() allows you to optionally give your V2 usermod an unique ID (please define it in const.h!).
     * This could be used in the future for the system to determine whether your usermod is installed.
     */
  uint16_t getId()
  {
    return USERMOD_ID_BME280MQTT;
  }

  //More methods can be added in the future, this example will then be extended.
  //Your usermod will remain compatible as it does not need to implement all methods from the Usermod base class!
};