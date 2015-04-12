#ifndef BT_COMMUNICATION_SAM
#define BT_COMMUNICATION_SAM

#include <SoftwareSerial.h>
#include <PLabBTSerial.h>

void BT_sendHasReturned();
int BT_getDestination();
boolean BT_shallReturn();
boolean BT_hasNewDestination();
void BT_update();

extern BTSerial btSerial;

#endif
