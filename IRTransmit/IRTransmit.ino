#include "PLab_IRremote.h"
IRsend irsend;

void setup()
{
  Serial.begin(9600);
}

void loop()
{
  if (Serial.read() != -1) {
    irsend.sendNEC(IR_UP, 32);
  }
}
