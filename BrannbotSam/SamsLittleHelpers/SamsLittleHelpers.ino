#include "PLab_IRremote.h"
IRsend irsend;

void setup()
{
  /*Serial.begin(9600); */ //Dont think we need this...
}

void loop()
{
  if (Serial.read() != -1) { //TODO: if Robot detects Fire (or buttonpress)
    irsend.sendNEC(IR_UP, 32); //Send my position aka. destination
  }
}
