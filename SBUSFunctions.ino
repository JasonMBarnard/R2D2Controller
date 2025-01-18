#include "sbus.h"

/* SBUS object, reading SBUS */
bfs::SbusRx x8r_rx(&Serial1);
/* SBUS data */
bfs::SbusData data;

void SBUSsetup() {
  // begin the SBUS communication
  // if (DebugMode) {
    /* Serial to display data */
    Serial.begin(115200);
    Serial.println("Booting... ");
    while (!Serial) {}
  //}
  /* Begin the SBUS communication */
  x8r_rx.Begin();
  Serial.println("Connected to x8r receiver");
}

void SBUSloop() {
  if (x8r_rx.Read()) {
    /* Grab the received data */
    data = x8r_rx.data(); // <- USE THE "data" VARIABLE TO SEE VALUES FROM YOUR TRANSMITER
    /* Display the received data */
    if (DebugMode) {
      // Uncomment the following lines to debug SBus issues.
      for (int8_t i = 0; i < data.NUM_CH; i++) {
        Serial.print("Ch");
        Serial.print(i);
        Serial.print(": ");
        Serial.print(data.ch[i]);
        Serial.print("\t");
      }
      /* Display lost frames and failsafe data */
      Serial.print("LF: ");
      Serial.print(data.lost_frame);
      Serial.print("\t");
      Serial.print("FS: ");
      Serial.println(data.failsafe);
    }
  }

}