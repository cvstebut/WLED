#include "wled.h"
/*
 * Register your v2 usermods here!
 *   (for v1 usermods using just usermod.cpp, you can ignore this file)
 */

/*
 * Add/uncomment your usermod filename here (and once more below)
 * || || ||
 * \/ \/ \/
 */
//#include "usermod_v2_example.h"
#ifdef USERMOD_DALLASTEMPERATURE
#include "../usermods/Temperature/usermod_temperature.h"
#endif

//#include "usermod_v2_empty.h"

#ifdef USERMOD_BUZZER
#include "../usermods/buzzer/usermod_v2_buzzer.h"
#endif
#ifdef USERMOD_SENSORSTOMQTT
#include "usermod_v2_SensorsToMqtt.h"
#endif

#ifdef USERMOD_BME280MQTT
#include "../usermods/bme280_mqtt/usermod_bme280mqtt.h"
#endif

#ifdef USERMOD_MPR121MQTT
#include "../usermods/mpr121_mqtt/usermod_mpr121mqtt.h"
#endif

#ifdef USERMOD_SHT3XMQTT
#include "../usermods/sht3x_mqtt/usermod_sht3xmqtt.h"
#endif

// BME280 v2 usermod. Define "USERMOD_BME280" in my_config.h
#ifdef USERMOD_BME280
#include "../usermods/BME280_v2/usermod_bme280.h"
#endif

void registerUsermods()
{
  /*
   * Add your usermod class name here
   * || || ||
   * \/ \/ \/
   */
  //usermods.add(new MyExampleUsermod());

#ifdef USERMOD_DALLASTEMPERATURE
  usermods.add(new UsermodTemperature());
#endif

  //usermods.add(new UsermodRenameMe());

#ifdef USERMOD_BUZZER
  usermods.add(new BuzzerUsermod());
#endif

#ifdef USERMOD_BME280MQTT
  usermods.add(new Bme280MqttUsermod());
#endif

#ifdef USERMOD_MPR121MQTT
  usermods.add(new Mpr121MqttUsermod());
#endif

#ifdef USERMOD_SHT3XMQTT
  usermods.add(new Sht3xMqttUsermod());
#endif

#ifdef USERMOD_BME280
  usermods.add(new UsermodBME280());
#endif
#ifdef USERMOD_SENSORSTOMQTT
  usermods.add(new UserMod_SensorsToMQTT());
#endif
}