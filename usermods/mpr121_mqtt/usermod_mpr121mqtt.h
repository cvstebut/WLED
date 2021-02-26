#pragma once
#include "wled.h"
#include <Wire.h>
#include "Adafruit_BusIO_Register.h"
#include "Adafruit_MPR121.h"
#include "touchdecoder.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <bitset>

TouchDecoder decoder1;
TouchDecoder decoder2;

Adafruit_MPR121 cap1 = Adafruit_MPR121();
Adafruit_MPR121 cap2 = Adafruit_MPR121();

bool cap1present = false;
bool cap2present = false;

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
class Mpr121MqttUsermod : public Usermod
{
private:
  //Private class members. You can declare variables and functions only accessible to your usermod here
  unsigned long lastTime = 0;
  // mpr121 touch board timer
  long touchTimer = millis();

public:
  /*
    * process() will handle the events generated by the touch sensor "sensor" based on the output of TouchDecoder decoder.
    * reportingOffset is added to the sensor field id when displaying and reporting - used to offset second sensor.
    * Accessed global variables/functions:
    *  
    *  - mqtt->publish
   */
  void process(Adafruit_MPR121 *sensor, TouchDecoder *decoder, String mqttBaseTopic, int reportingOffset = 0)
  {
    // Get the currently touched pads
    std::bitset<12> shortPress = (*decoder).shortPress();
    std::bitset<12> longPress = (*decoder).longPress();
    std::bitset<12> doubleShortPress = (*decoder).doubleShortPress();
    std::bitset<12> doubleLongPress = (*decoder).doubleLongPress();
    std::bitset<12> shortLongPress = (*decoder).shortLongPress();
    std::bitset<12> longShortPress = (*decoder).longShortPress();

    for (auto i = 0; i < (*decoder).width(); i++)
    {
      if (shortPress.test(i))
      {
        String t = String(mqttBaseTopic);
        t += "/touch/";
        t += i + reportingOffset;

        unsigned long pressTime = (*decoder).shortPressTime()[i];

        Serial.print(i);
        Serial.print(" shortPress (");
        Serial.print(reportingOffset);
        Serial.println(")");

        std::ostringstream s;
        s << "short-" << pressTime << std::endl;

        mqtt->publish(t.c_str(), 0, true, s.str().c_str());
      }
      if (longPress.test(i))
      {
        String t = String(mqttBaseTopic);
        t += "/touch/";
        t += i + reportingOffset;

        Serial.print(i);
        Serial.print(" longPress (");
        Serial.print(reportingOffset);
        Serial.println(")");
        mqtt->publish(t.c_str(), 0, true, "long");
      }
      if (doubleShortPress.test(i))
      {
        String t = String(mqttBaseTopic);
        t += "/touch/";
        t += i + reportingOffset;

        Serial.print(i);
        Serial.print(" doubleShortPress (");
        Serial.print(reportingOffset);
        Serial.println(")");
        mqtt->publish(t.c_str(), 0, true, "doubleShort");
      }
      if (doubleLongPress.test(i))
      {
        String t = String(mqttBaseTopic);
        t += "/touch/";
        t += i + reportingOffset;

        Serial.print(i);
        Serial.print(" doubleLongPress (");
        Serial.print(reportingOffset);
        Serial.println(")");
        mqtt->publish(t.c_str(), 0, true, "doubleLong");
      }
      if (shortLongPress.test(i))
      {
        String t = String(mqttBaseTopic);
        t += "/touch/";
        t += i + reportingOffset;

        Serial.print(i);
        Serial.print(" shortLongPress (");
        Serial.print(reportingOffset);
        Serial.println(")");
        mqtt->publish(t.c_str(), 0, true, "shortLong");
      }
      if (longShortPress.test(i))
      {
        String t = String(mqttBaseTopic);
        t += "/touch/";
        t += i + reportingOffset;

        Serial.print(i);
        Serial.print(" longShortPress (");
        Serial.print(reportingOffset);
        Serial.println(")");
        mqtt->publish(t.c_str(), 0, true, "longShort");
      }
    }
  }

  //Functions called by WLED
  /*
     * setup() is called once at boot. WiFi is not yet connected at this point.
     * You can use it to initialize variables, sensors or similar.
     */
  void setup()
  {
    Serial.println("Hello from usermod mpr121mqtt!");
    Serial.println("Adafruit MPR121 Capacitive Touch sensor test");
    // Default address is 0x5A, if tied to 3.3V its 0x5B
    // If tied to SDA its 0x5C and if SCL then 0x5D
    if (!cap1.begin(0x5A))
    {
      Serial.println("MPR121 - 0x5A - not found, check wiring?");
    }
    else
    {
      Serial.println("MPR121 - 0x5A - found");
      cap1present = true;
    }
    if (!cap2.begin(0x5C))
    {
      Serial.println("MPR121 - 0x5C - not found, check wiring?");
    }
    else
    {
      Serial.println("MPR121 - 0x5C - found");
      cap2present = true;
    }
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
      // Serial.println("mpr121mqtt - I'm alive!");
      lastTime = millis();
    }

    if (cap1present)
    {
      decoder1.push(cap1.touched(), millis());
      process(&cap1, &decoder1, mqttDeviceTopic, 0);
    }
    if (cap2present)
    {
      decoder2.push(cap2.touched(), millis());

      process(&cap2, &decoder2, mqttDeviceTopic, 100);
    }
  }
  /*
     * addToJsonInfo() can be used to add custom entries to the /json/info part of the JSON API.
     * Creating an "u" object allows you to add custom key/value pairs to the Info section of the WLED web UI.
     * Below it is shown how this could be used for e.g. a light sensor
     */
  /*
void addToJsonInfo(JsonObject &root)
{

  int reading = 42;
  //this code adds "u":{"Light":[20," lux"]} to the info object
  JsonObject user = root["u"];
  if (user.isNull())
    user = root.createNestedObject("u");

  JsonArray bmeArr = user.createNestedArray("BME-Temperature");   //name
  bmeArr.add(SensorTemperature);                                  //value
  bmeArr.add("°C");                                               //unit
  JsonArray bmeHum = user.createNestedArray("BME-Humidity");      //name
  bmeHum.add(SensorHumidity);                                     //value
  bmeHum.add("%RH");                                              //unit
  JsonArray bmePressure = user.createNestedArray("BME-Pressure"); //name
  bmePressure.add(SensorPressure);                                //value
}
*/
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
    return USERMOD_ID_MPR121MQTT;
  }

  //More methods can be added in the future, this example will then be extended.
  //Your usermod will remain compatible as it does not need to implement all methods from the Usermod base class!
};