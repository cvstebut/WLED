#pragma once

#include "wled.h"
#include <Wire.h>
#include "SHTSensor.h"
#include "LedControl.h"
#include "LedControl_HW_SPI.h"
#include <iomanip>
#include <sstream>

SHTSensor sht;
LedControl_HW_SPI lc = LedControl_HW_SPI();

#ifdef USERMOD_BME280MQTT
#else

#ifdef ARDUINO_ARCH_ESP32 //ESP32 boards
uint8_t SCL_PIN = 22;
uint8_t SDA_PIN = 21;
#else                     //ESP8266 boards
uint8_t SCL_PIN = 5;
uint8_t SDA_PIN = 4;
// uint8_t RST_PIN = 16; // Uncoment for Heltec WiFi-Kit-8
#endif
#endif
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
class Sht3xMqttUsermod : public Usermod
{
private:
  //Private class members. You can declare variables and functions only accessible to your usermod here
  unsigned long lastTime = 0;
  // BME280 sensor timer
  long sensorTimer = millis();
  long lastMqttReport = 0;
  long lastMeasurement = 0;
  long reportMqttPeriod = 20000; // publish measurement every reportMqttPeriod milli seconds
  long measurePeriod = 1000;     // get measurement every measurePeriod milli seconds
  String sensorType;

public:
  bool sensorConnected = false;
  //Functions called by WLED
  float SensorTemperature;
  float SensorHumidity;
  /*
     * setup() is called once at boot. WiFi is not yet connected at this point.
     * You can use it to initialize variables, sensors or similar.
     */
  void setup()
  {
    Serial.println("Hello from usermod sht3xmqtt!");
    lc.begin(5);
    /*
    The MAX72XX is in power-saving mode on startup,
    we have to do a wakeup call
    */
    lc.shutdown(0, false);
    /* Set the brightness to a medium values */
    lc.setIntensity(0, 2);
    /* and clear the display */
    lc.clearDisplay(0);

    Wire.begin(SDA_PIN, SCL_PIN);

    if (!sht.init())
    {
      Serial.println("Could not find SHT3x sensor!");
      sensorConnected = false;
    }
    else
    {
      Serial.println("SHT3x sensor detected");
      sensorConnected = true;
      sensorType = "sht35";
    }
  }

  void writeQuad(int group, float value)
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

    lc.setChar(0, digit--, stringValue[groupLength - 5], false);
    lc.setChar(0, digit--, stringValue[groupLength - 4], false);
    lc.setChar(0, digit--, stringValue[groupLength - 3], true);
    lc.setChar(0, digit--, stringValue[groupLength - 1], false);
  }

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
      // Serial.println("sht3x - I'm alive!");
      lastTime = millis();
    }

    // sensor MQTT publishing
    sensorTimer = millis();

    if (sensorConnected && (sensorTimer - lastMeasurement > measurePeriod))
    {
      lastMeasurement = sensorTimer;
      if (sht.readSample())
      {
        SensorTemperature = sht.getTemperature();
        SensorHumidity = sht.getHumidity();

        //          Serial.println("LED-Display: Output temperature and humidity");
        writeQuad(2, SensorTemperature);
        writeQuad(1, SensorHumidity);

      }
      else
      {
        Serial.println("sht3xmqtt: Error in readSample()");
      }
    }
    // - Timer to publish new sensor data every reportMqttPeriod seconds
    // - Only send mqtt data if sensor is connected
    if (sensorConnected && (sensorTimer - lastMqttReport > reportMqttPeriod))
    {
      lastMqttReport = sensorTimer;

      // Check if MQTT Connected, otherwise it will crash the 8266
      if (mqtt != nullptr)
      {
        // Create string populated with user defined device topic from the UI, and the read temperature, humidity and pressure. Then publish to MQTT server.
        String d = String(mqttDeviceTopic);
        d += "/data";
        //        Serial.print("mqtt topic - data: ");
        //        Serial.println(d);

        DynamicJsonDocument sensorData(1024);

        sensorData[String("sensor_type")] = sensorType.c_str();
        sensorData[String("temperature")] = SensorTemperature;
        sensorData[String("humidity")] = SensorHumidity;

        String output;
        serializeJson(sensorData, output);        
        Serial.print("MQTT connected!  - message payload: ");
        Serial.println(output);
        mqtt->publish(d.c_str(), 0, true, output.c_str());
      }
      else
      {
        Serial.println("sht3x - mqtt not connected!");
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
      user = root.createNestedObject("u");

    JsonArray sensorArr = user.createNestedArray("SHT-Temperature"); //name
    sensorArr.add(SensorTemperature);                                //value
    sensorArr.add("°C");                                             //unit
    JsonArray bmeHum = user.createNestedArray("SHT-Humidity");       //name
    bmeHum.add(SensorHumidity);                                      //value
    bmeHum.add("%RH");                                               //unit
  }

  /*
     * addToJsonState() can be used to add custom entries to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
  void addToJsonState(JsonObject &root)
  {
    //root["user0"] = userVar0;
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
    return USERMOD_ID_SHT3XMQTT;
  }

  //More methods can be added in the future, this example will then be extended.
  //Your usermod will remain compatible as it does not need to implement all methods from the Usermod base class!
};