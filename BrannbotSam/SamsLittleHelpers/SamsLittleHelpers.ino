//IR Diode: Use pin 3.
//Reflectance IR: A0.
#include "PLab_IRremote.h"

// Next line must be "#define MASTER_BOT" if this bot should communicate with Sam through Bluetooth.

const boolean IS_MASTER_BOT = true;

#include <SoftwareSerial.h>
#include <PLabBTSerial.h>

#define rxPin 2   // Connect this to pin RXD on the BT unit.
#define txPin 6   // Connect this to pin TXD on the BT unit.
#define IRReceiverPin 4

PLabBTSerial btSerial(txPin, rxPin);


IRrecv irrecv(IRReceiverPin);
decode_results results;

byte destination = 0;


IRsend irsend;

unsigned long IDENTITY = 4;
unsigned long IR_BURNING = IR_4;
unsigned long IR_PUT_OUT = IR_8; // IDENTITY + 4
const int ledPin = 13;

void setup()
{
  randomSeed(analogRead(7));  // Avoid same pattern every time
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
  // Set up BT
  if (IS_MASTER_BOT)
  {
    btSerial.begin(9600);
    irrecv.enableIRIn(); // Start the receiver
    irrecv.blink13(false); // Do not blink pin 13 as feedback.
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

// State-maskin

const byte STATE_INIT = 0;
const byte STATE_IDLE = 1;
const byte STATE_ON_FIRE = 2;
const byte STATE_WARN = 3;
const byte STATE_RETURN = 4;
byte state = STATE_IDLE;




      
  
  /*if (millis() > timeToBurn)
  {*/
void loopIdle()
{
    if(analogRead(0)< 1000){
      // WE'RE BURNING
      // TELL SAMBOT
      irsend.sendNEC(IDENTITY, 32);
      state = STATE_ON_FIRE;
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
    }
    
    irrecv.resume(); // receive the next value
  }
}


unsigned long timeToSwitch = 0;
boolean ledTurnedOn = false;
byte firePutOutCounter = 0;
void loopOnFire()
{
  // blink ligth
  if (timeToSwitch < millis())
  {
    if (ledTurnedOn)
    {
      digitalWrite(ledPin, LOW);
      ledTurnedOn = false;
      timeToSwitch = millis() + 450;
    } else {
      digitalWrite(ledPin, HIGH);
      ledTurnedOn = true;
      timeToSwitch = millis() + 50;
    }
  }
  
  // is the fire put out?
  if(analogRead(0) > 500){
      ++firePutOutCounter;
  }
  if (firePutOutCounter > 10)
  {
    // The fire is put out
    firePutOutCounter = 0;
    irsend.sendNEC(IR_PUT_OUT, 32);
    state = STATE_IDLE;
    digitalWrite(ledPin, LOW);
  }
}


const int BT_START = 0;
const int BT_GOTO = 16;
const int BT_RETURN = 17;
const int BT_HAS_RETURNED = 18;

void loopWarn()
{
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
    }
    
    irrecv.resume(); // receive the next value
  }
  btSerial.write((byte) BT_START);
  btSerial.write((byte) BT_GOTO);
  btSerial.write((byte) destination);
}

void loopReturn()
{
  btSerial.write((byte) BT_START);
  btSerial.write((byte) BT_RETURN);
  
  // Has Sam returned home?
  int result = -1;
  while (btSerial.available() && result != BT_START)
  {
    result = btSerial.read();
  };
  if (result == BT_START && btSerial.read() == BT_HAS_RETURNED)
  {
      state = STATE_IDLE;
  }
}


void loop()
{
  switch(state)
  {
    case STATE_IDLE:
      loopIdle();
      if (IS_MASTER_BOT)
      {
        loopIdleMB();
      }
      break;
    case STATE_ON_FIRE:
      loopOnFire();
      break;
    case STATE_WARN:
      if (IS_MASTER_BOT)
      {
        loopWarn();
      } else {
        state = 99;  // you shouldn't be warning us when you're just a slave, stupid
      }
      break;
    case STATE_RETURN:
      loopReturn();
      break;
    default:
      // Not recognized, just do some blinking
      digitalWrite(13, HIGH);
      delay(500);
      digitalWrite(13, LOW);
      delay(500);
      break;
  }
}
