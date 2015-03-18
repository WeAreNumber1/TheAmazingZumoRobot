#include "PLab_IRremote.h"
IRsend irsend;

unsigned long IDENTITY = 1;

void setup()
{
  /*Serial.begin(9600); */ //Dont think we need this...
  randomSeed(analogRead(7));
}

unsigned long timeToBurn = 1500;

void generateNewTimeToBurn()
{
  // Will start to burn some time between 15 and 40 sec
  timeToBurn = millis() + (1000 * random(15, 40));
}

boolean fireIsPutOut()
{
  // Right now: Fire is automatically put out after 13 sec
  return (millis() > (timeToBurn + (13 * 1000)));
}

void loop()
{
  if (millis() > timeToBurn)
  {
    // WE'RE BURNING
    // TELL SAMBOT
    irsend.sendNEC(IDENTITY, 32);
    delay(500);
    if (fireIsPutOut())
    {
      generateNewTimeToBurn();
    }
  }
}
