#pragma once
#include "wled.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <array>

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
class ReportStateDataMqttUsermod : public Usermod
{
private:
  //Private class members. You can declare variables and functions only accessible to your usermod here
  unsigned long lastTime = 0;
  long reportTimer = millis();
  long reportMqttPeriod = 10000; // publish measurement every bmeMqttPeriod milli seconds

public:
  bool mqttReportEnabled = true;
  JsonArray reportData;
  //Functions called by WLED
  /*
     * setup() is called once at boot. WiFi is not yet connected at this point.
     * You can use it to initialize variables, sensors or similar.
     */
  void setup()
  {
    Serial.println("reportStateDataMqttUsermod - Setup");
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
    reportTimer = millis();

    if (mqttReportEnabled && (reportTimer - lastTime > reportMqttPeriod))
    {
      lastTime = reportTimer;
      if (mqtt != nullptr)
      {
        // Create string populated with user defined device topic from the UI, and the read temperature, humidity and pressure. Then publish to MQTT server.
        String d = String(mqttDeviceTopic);
        d += "/data";

        DynamicJsonDocument dDoc(JSON_BUFFER_SIZE);
        JsonObject root = dDoc.to<JsonObject>();
        usermods.addToJsonState(root);
//        serializeState(root, false, false, false);

        String rootString;
        serializeJson(root, rootString);
        Serial.print("reportStateDataMqtt - display current usermod json state: ");
        Serial.println(rootString);

        reportData = {};
        if (root.containsKey("bme280mqtt"))
        {
          if (root["bme280mqtt"].containsKey("bme280-0x76"))
          {
            reportData.add(root["bme280mqtt"]["bme280-0x76"]);
          }
          else
          {
            Serial.println("reportStateDataMqtt - extractSensorData from root - no sensor key bme280-0x76 - nothing reported");
          }
          if (root["bme280mqtt"].containsKey("bme280-0x77"))
          {
            reportData.add(root["bme280mqtt"]["bme280-0x77"]);
          }
          else
          {
            Serial.println("reportStateDataMqtt - extractSensorData from root - no sensor key bme280-0x77 - nothing reported");
          }
        }
        else
        {
          Serial.println("reportStateDataMqtt - extractSensorData from root - no root key bme280mqtt");
        }

        if (!reportData.isNull())
        {
          String output;
          serializeJson(reportData, output);

          Serial.print("reportStateDataViaMqttUsermod - mqtt connected - message: ");
          Serial.println(output);

          mqtt->publish(d.c_str(), 0, true, output.c_str());
        }
      }
      else
      {
        Serial.println("reportStateDataViaMqttUsermod - mqtt not connected!");
      }
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
*/
  /*
     * addToJsonState() can be used to add custom entries to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
  /*
  void addToJsonState(JsonObject &root)
  {
  }
*/
  /*
     * readFromJsonState() can be used to receive data clients send to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */

  void readFromJsonState(JsonObject &root)
  {
    Serial.println("reportStateDataMqtt - readFromJsonState called!");
  }

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