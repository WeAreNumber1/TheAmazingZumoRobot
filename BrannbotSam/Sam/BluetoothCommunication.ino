/**
 * Code for Sam, allowing him to communicate with master-helper-bot.
 */
#include <SoftwareSerial.h>
#include <PLabBTSerial.h>

#define rxPin 2   // Connect this to pin RXD on the BT unit.
#define txPin 6   // Connect this to pin TXD on the BT unit.
const byte maxDestination = 4; // This is hard-coded to be 4 in SamHelperBots (IR-messages), please don't change
const byte ASCII_NUM_OFFSET = 48;
const byte BT_RETURN = maxDestination + 1;
const byte BT_NO_DESTINATION = 0;
byte BT_CURRENT_DESTINATION = BT_NO_DESTINATION;
boolean BT_SHALL_RETURN = false;
boolean BT_DESTINATION_CHANGED = false;

PLabBTSerial btSerial(txPin, rxPin);



///////////////
// BT-functions
///////////////

/**
 * Use this when Sam has returned home, before running BT_update() again.
 */
void BT_sendHasReturned()
{
  btSerial.write("HR\r\n");
  BT_SHALL_RETURN = false;
}

/**
 * Use this to get the "address" of the helper-bot in need of help.
 * Returns current destination, as last reported by master-helper-bot.
 * Returns: BT_NO_DESTINATION = 0 = false, if no destination is received.
 * or the number of the destination (1, 2, 3 or 4).
 */
int BT_getDestination()
{
  return BT_CURRENT_DESTINATION;
}

/**
 * Use this to check if Sam should travel home after trying to put out a fire.
 * Returns true if Sam should travel home (because fire is put out)
 * Returns false if Sam already is at home, or fire is not put out.
 */
boolean BT_shallReturn()
{
  return BT_SHALL_RETURN;
}

/**
 * Use this when Sam is home, to check if some bot needs help.
 * Returns true if Sam needs to travel to a new place (except home).
 * Returns false if nothing is reported, or if Sam needs to return home (use shallReturn)
 */
boolean BT_hasNewDestination()
{
  return BT_DESTINATION_CHANGED;
}

/**
 * Checks for new messages from master-helper-bot.
 * Run this before any other BT-functions in loop().
 */
void BT_update()
{
  int availableCount = btSerial.available();
  byte newDestination = BT_CURRENT_DESTINATION;
  if (availableCount > 0) {
    for (int i = 0; i < availableCount; ++i)
    {
      char c = btSerial.read();
    
      if (c == 'G')
      {
        byte potentialDestination = btSerial.read() - ASCII_NUM_OFFSET;
        if (potentialDestination > 0 && potentialDestination <= maxDestination)
        {
          // New destination is accepted!
          newDestination = potentialDestination;
        }
      }
      else if (c == 'R')
      {
        newDestination = BT_NO_DESTINATION;
        BT_SHALL_RETURN = true;
      }
    }
  }
  if (BT_CURRENT_DESTINATION != newDestination)
  {
    BT_DESTINATION_CHANGED = true && newDestination != 0;
    BT_CURRENT_DESTINATION = newDestination;
  } else {
    BT_DESTINATION_CHANGED = false;
  }
}
