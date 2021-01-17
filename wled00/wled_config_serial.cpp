#include "wled.h"

/*
 * Configuration using the serial port
 */

void handleConfigSerial()
{
#ifdef WLED_ENABLE_CONFIG_SERIAL

  if (Serial.available() > 0)
  {
    yield();
    String command = Serial.readStringUntil('\n');
    if (command.startsWith("ss02cspin="))
    {
      Serial.println("Command detected: ss02cspin");
    }
    else
    {
      Serial.println("Invalid command.");
    }
  }
#endif
}
