#include "SoftwareSerial.h"
// RX: Arduino pin 2, XBee pin DOUT.  TX:  Arduino pin 3, XBee pin DIN
#define rxPin 10
#define txPin 11

SoftwareSerial XBee(rxPin, txPin);

void XBeesetup() {
  // Baud rate MUST match XBee settings (as set in XCTU)
  //pinMode(rxPin, Input);
  //pinMode(txPin, Output),
  XBee.begin(9600);
}

void XBeeloop() {
  if (XBee.available() > 0) {
      char c = XBee.read();
      Serial.print(c);
  }
  if (XBee.overflow()) {
    Serial.println("XBee overflow!");
  }
}
