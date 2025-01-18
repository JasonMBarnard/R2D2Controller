/* Last Update 10/05/2023 */

#include "definitions.h"

bool DebugMode = false;
unsigned long currentMillis = 0;

void setup() {
  SBUSsetup();
  Sabertoothsetup();
  ServoSetup();
  marcDuinoSetup();
}

void loop () {
  currentMillis = millis();
  SBUSloop();
  Sabertoothloop();
  Servoloop();
  marcDuinoloop();
}
