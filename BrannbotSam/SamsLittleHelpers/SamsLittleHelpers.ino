//IR Diode: Use pin 3.
//Reflectance IR: A0.
#include "PLab_IRremote.h"
#include <EEPROM.h>

// Next line must be "#define MASTER_BOT" if this bot should communicate with Sam through Bluetooth.

#include <SoftwareSerial.h>
#include <PLabBTSerial.h>

#define rxPin 2   // Connect this to pin RXD on the BT unit.
#define txPin 6   // Connect this to pin TXD on the BT unit.
const int IRReceiverPin = 11;

PLabBTSerial btSerial(txPin, rxPin);


IRrecv irrecv(IRReceiverPin);
decode_results results;

byte destination = 0;


IRsend irsend;

byte IDENTITY;
boolean IS_MASTER_BOT;
unsigned long IR_BURNING;
unsigned long IR_PUT_OUT; // IDENTITY + 4
const int ledPin = 13;

const int FIRE_THRESHOLD = 1000;
const byte FIRE_DEBOUNCE = 10;  // Number of measurements in a row which must be 'positive' for the fire to be registered

const byte STATE_INIT = 0;
const byte STATE_IDLE = 1;
const byte STATE_ON_FIRE = 2;
const byte STATE_WARN = 3;
const byte STATE_RETURN = 4;
const byte STATE_ERROR = 99;
byte state;

void setup()
{
  randomSeed(analogRead(7));  // Avoid same pattern every time
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
  
  state = STATE_IDLE;
  
  // Okay, so what are we?
  IDENTITY = EEPROM.read(0);
  byte isMasterBot = EEPROM.read(sizeof(byte));
  if (isMasterBot != 0 && isMasterBot != 1)
  {
    IS_MASTER_BOT = false;
    state = STATE_ERROR;
  } else {
    IS_MASTER_BOT = (boolean) isMasterBot;
  }
  // Update IR_BURNING and IR_PUT_OUT
  switch(IDENTITY)
  {
    case 1:
      IR_BURNING = IR_1;
      IR_PUT_OUT = IR_5;
      break;
    case 2:
      IR_BURNING = IR_2;
      IR_PUT_OUT = IR_6;
      break;
    case 3:
      IR_BURNING = IR_3;
      IR_PUT_OUT = IR_7;
      break;
    case 4:
      IR_BURNING = IR_4;
      IR_PUT_OUT = IR_8;
      break;
    default:
      state = STATE_ERROR;
      break;
  }
  
  // Set up BT
  if (IS_MASTER_BOT)
  {
    btSerial.begin(9600);
    irrecv.enableIRIn(); // Start the receiver
    irrecv.blink13(false); // Do not blink pin 13 as feedback.
    Serial.println("Starting up.");
  }
}

/*unsigned long timeToBurn = 1500;

void generateNewTimeToBurn()
{
  // Will start to burn some time between 15 and 40 sec
  timeToBurn = millis() + (1000 * random(15, 40));
}

boolean fireIsPutOut()
{
  // Right now: Fire is automatically put out after 13 sec
  return (millis() > (timeToBurn + (13 * 1000)));
}*/

// Bluetooth communication
const char BT_GOTO[] = "G";
const char BT_RETURN[] = "R";
const char BT_HAS_RETURNED[] = "HR";
const char BT_END[] = "\r\n";

unsigned long bluetoothSendTime = 0;

byte lastAskedState = 0;

boolean isFirstRun()
{
  if (lastAskedState != state)
  {
    lastAskedState = state;
    return true;
  }
  else
  {
    return false;
  }
}

byte fireStartedCounter = 0;
boolean fireWasPutOut = false;
unsigned long timeToSendIR = 0;
void loopIdle()
{
  if (fireWasPutOut && millis() > timeToSendIR)
  {
    // Tell master-bot that the fire is no more
    irsend.sendNEC(IR_PUT_OUT, 32);
    timeToSendIR = millis() + 500;
  }
  
  if(analogRead(0)< FIRE_THRESHOLD){
    ++fireStartedCounter;
  } else {
    fireStartedCounter = 0;
  }
  
  if (fireStartedCounter > FIRE_DEBOUNCE) {
    // WE'RE BURNING
    // TELL SAMBOT
    if (IS_MASTER_BOT)
    {
      destination = IDENTITY; // WE are the ones burning
    }
    state = STATE_ON_FIRE;
    Serial.println("OMFG I'M ON FIRE");
  }
//    Serial.println(analogRead(0));
    /*if (fireIsPutOut())
    {
      generateNewTimeToBurn();
    }*/
  /*}*/
}

void loopIdleMB()
{
  if (irrecv.decode(&results)) // have we received an IR signal?
  { 
    switch(results.value) 
    {
      // See https://github.com/IDI-PLab/Lecture-examples/blob/master/Forelesning_3_Gruppepakke/PLab_IRRemote_code_dump.ino for list of values
      case IR_1: destination = 1; break;
      case IR_2: destination = 2; break;
      case IR_3: destination = 3; break;
      case IR_4: destination = 4; break;
      default:
        destination = 0;
        break;
    }
    if (destination != 0)
    {
      state = STATE_WARN;
      Serial.print("Sam!!! There's a fire at helperbot #0");
      Serial.println(destination);
    }
    
    irrecv.resume(); // receive the next value
  }
}


unsigned long timeToSwitch = 0;
boolean ledTurnedOn = false;

void blinkLed(int durationOn, int durationOff) 
{
  // blink ligth
  if (timeToSwitch < millis())
  {
    if (ledTurnedOn)
    {
      digitalWrite(ledPin, LOW);
      ledTurnedOn = false;
      timeToSwitch = millis() + durationOff;
    } else {
      digitalWrite(ledPin, HIGH);
      ledTurnedOn = true;
      timeToSwitch = millis() + durationOn;
    }
  }
}


byte firePutOutCounter = 0;
void loopOnFire()
{
  // Tell masterbot about our suffering
  if (!IS_MASTER_BOT)
  {
    irsend.sendNEC(IR_BURNING, 32);
  } else if (isFirstRun())
  {
    btSerial.write(BT_GOTO);
    btSerial.write(char(destination+48));
    btSerial.write("\r\n");
  }
  
  // is the fire put out? Prevent random put-out status
  if(analogRead(0) > FIRE_THRESHOLD){
      ++firePutOutCounter;
  } else {
      firePutOutCounter = 0;
  }
  
  if (firePutOutCounter > FIRE_DEBOUNCE)
  {
    // The fire is put out
    firePutOutCounter = 0;
    digitalWrite(ledPin, LOW);
    if (IS_MASTER_BOT)
    {
      state = STATE_RETURN;
      Serial.println("Fire put out. Get back home, Sam!");
    } else {
      fireWasPutOut = true;
      state = STATE_IDLE;
      Serial.println("Fire put out. Great work, Sam!");
    }    
  }
}


void loopWarn()
{  
  if (isFirstRun())
  {
    //bluetoothSendTime = millis() + 200;
    btSerial.write(BT_GOTO);
    btSerial.write(char(destination+48));
    btSerial.write("\r\n");
  }
  
  // Should we change state?
  if (irrecv.decode(&results)) // have we received an IR signal?
  { 
    byte sender = 0;
    switch(results.value) 
    {
      // See https://github.com/IDI-PLab/Lecture-examples/blob/master/Forelesning_3_Gruppepakke/PLab_IRRemote_code_dump.ino for list of values
      case IR_1: 
      case IR_2: 
      case IR_3: 
      case IR_4:
      // Discard messages of fires elsewhere; we are already occupied
        break;
      
      // These messages indicate the fire is put out.
      case IR_5: sender = 1; break;
      case IR_6: sender = 2; break;
      case IR_7: sender = 3; break;
      case IR_8: sender = 4; break;
    }
    
    if (sender == destination)
    {
      // The fire is out. Get Sam back home
      state = STATE_RETURN;
      Serial.println("The fire is put out, I hear. Get back home, Sam!");
    }
    irrecv.resume(); // receive the next value
  }
}


void loopReturn()
{
  //if (millis() > bluetoothSendTime)
  if (isFirstRun())
  {
    bluetoothSendTime = millis() + 200;
    btSerial.write(BT_RETURN);
    btSerial.write("\r\n");
  }
  
  // Has Sam returned home?
  int result = -1;
  int availableCount = btSerial.available();
  if (availableCount > 0) {
    char text[availableCount];
    btSerial.read(text, availableCount); 
    if (0 == strcmp(BT_HAS_RETURNED, text))
    {
      state = STATE_IDLE;
      Serial.println("Welcome back, Sam!");
    }
  }
}


void loop()
{
  switch(state)
  {
    case STATE_IDLE:
      blinkLed(2000, 2000);
      loopIdle();  // Check if fire
      if (IS_MASTER_BOT)
      {
        loopIdleMB();  // Check if received message about fire
      }
      break;
    case STATE_ON_FIRE:
      blinkLed(50, 450);
      loopOnFire();  // Warn master-bot about fire, check if fire is put out
      break;
    case STATE_WARN:
      blinkLed(200,200);
      if (IS_MASTER_BOT)
      {
        loopWarn();  // Warn Sam about fire
      } else {
        state = STATE_ERROR;  // you shouldn't be warning us when you're just a slave, stupid
      }
      break;
    case STATE_RETURN:
      blinkLed(500, 200);
      if (IS_MASTER_BOT) 
      {
        loopReturn();  // ask Sam to go home
        loopIdle(); // check if we are burning
        loopIdleMB(); // check if anyone else is burning
      } else {
        state = STATE_ERROR;
      }
      break;
    default:
      // Not recognized, just do some blinking to alert our masters
      blinkLed(450, 50);
      break;
  }
}
