#include <PLab_ZumoMotors.h>
#include <Pushbutton.h>
#include <QTRSensors.h>
#include <ZumoBuzzer.h>
#include <ZumoMotors.h>
#include <ZumoReflectanceSensorArray.h>

#include <SoftwareSerial.h>
#include <PLabBTSerial.h>
#include <PLab_ZumoMotors.h>


//Define states...?
int currentState;
#define WAIT 1
#define FIGHT 2
#define SEARCH 3

const int btUnitTxPin = 1; // Connected to tx on bt unit
const int btUnitRxPin = 0; // Connected to rx on bt unit
PLabBTSerial btSerial(btUnitTxPin, btUnitRxPin);

void setup(void)
{
  // Start communication with bluetooth unit
  btSerial.begin(9600);
}

void loop()
{
  // See if we have received a new character
  int availableCount = btSerial.available();
  if (availableCount > 0) {
    char text[availableCount];
    btSerial.read(text, availableCount);
    readCommand(text);
  }
  switch(currentState){
    case WAIT: delay(10); break;
    case FIGHT: fight(); break;
    case SEARCH: search(); break;
  }

}

void readCommand (char *text) {
  if (0 == strcmp("FIGHT", text)) {
      currentState = FIGHT;
    } else if (0 == strcmp("SEARCH", text)) {
      currentState = SEARCH;
  }
}

void fight(){
  PLab_motors.setSpeeds(100, 100);
}
void search(){
  PLab_motors.setSpeeds(-100, -100);
}
