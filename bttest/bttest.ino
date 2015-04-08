/*
This is a simple terminal for the BT module..
*/
#include <SoftwareSerial.h>
#include <PLabBTSerial.h>
#define rxPin 2 // Connect this to pin RXD on the BT unit.
#define txPin 3 // Connect this to pin TXD on the BT unit.
PLabBTSerial btSerial(txPin, rxPin);
void setup()
{
Serial.begin(9600);
btSerial.begin(9600);
}
void loop() {
  while (!Serial.available()) { }
  char c = Serial.read();
  btSerial.begin(9600);
  Serial.println("STEPF");
  btSerial.write("STEPF\r\n");
}

