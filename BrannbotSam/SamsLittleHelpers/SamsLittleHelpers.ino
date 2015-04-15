//IR Diode (sender): Use pin 3.
//Reflectance IR (fire detecter): A0.
#define BTrxPin 2   // Only on master-bot. Connect this to pin RXD on the BT unit.
#define BTtxPin 6   // Only on master-bot. Connect this to pin TXD on the BT unit.
const int IRReceiverPin = 11; // Only on master-bot, connect this to IR Remote receiver
const int ledPin = 13;
// True: IR      False: Serial / internet
const boolean USE_IR = false;
const boolean DEBUG = false;

/* The following must be true for the serial/internet communication to work:
  PLab Simple Internet.
  The PLab_Internet library takes care of message passing in a circular 
  configuration of Arduinos using the serial port on pin 0 (Rx) and pin 1 (Tx).
  
  Connect the Arduinos with common ground.
  Connect Tx from Arduino to Rx on the next, and continue. 
  Connect Tx on the last to Rx on the first.
  
  With three Arduions with address 0, 1 and 2 it will look like:
  
  GND on Arduino 0 to GND on Arduino 1
  GND on Arduino 1 to GND on Arduino 2 
  
  Pin 1 (Tx) on Arduino 0  --> Pin 0 (Rx) on Arduino 1
  Pin 1 (Tx) on Arduino 1  --> Pin 0 (Rx) on Arduino 2
  Pin 1 (Tx) on Arduino 2  --> Pin 0 (Rx) on Arduino 3
  
  The PLab_Internet object has the following interface:
     PLab_Internet(byte my_address);   // Set address of this Arduino.
     void update();                    // update must be calles in loop.
     void sendMessage(byte receiver_address, String text);  // Send message
     void onReceive( void (*)(byte,String) );  // Define callback function
*/

#include <PLab_IRremote.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>
#include <PLabBTSerial.h>
#include <PLabInternet.h>

PLabBTSerial btSerial(BTtxPin, BTrxPin);
IRrecv irrecv(IRReceiverPin);
decode_results results;
byte destination = 0;

byte robotInDistress = 0;  // internet spesific
byte robotSaved = 0;

IRsend irsend;

const byte IDENTITY = EEPROM.read(0);
const byte MASTER_BOT_ID = 1;
boolean IS_MASTER_BOT;
unsigned long IR_BURNING;
unsigned long IR_PUT_OUT; // IDENTITY + 4

PLab_Internet internet(IDENTITY);

const int FIRE_THRESHOLD = 750;
const byte FIRE_DEBOUNCE = 20;  // Number of measurements in a row which must be 'positive' for the fire to be registered

const byte STATE_INIT = 0;
const byte STATE_IDLE = 1;
const byte STATE_ON_FIRE = 2;
const byte STATE_WARN = 3;
const byte STATE_RETURN = 4;
const byte STATE_ERROR = 99;
byte state;

void setup()
{
  pinMode(ledPin, OUTPUT);
  if (DEBUG) Serial.begin(9600);

  state = STATE_IDLE;
  
  if (!USE_IR)
  {
    internet.onReceive(onMessageReceived);
  }

  // Okay, so what are we?
  byte isMasterBot = EEPROM.read(sizeof(byte));
  if (isMasterBot != 0 && isMasterBot != 1)
  {
    IS_MASTER_BOT = false;
    state = STATE_ERROR;
  } else {
    IS_MASTER_BOT = (boolean) isMasterBot;
  }
  // Update IR_BURNING and IR_PUT_OUT (IR-signals)
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
    if (USE_IR) 
    {
      irrecv.enableIRIn(); // Start the receiver
      irrecv.blink13(false); // Do not blink pin 13 as feedback.
    }
  }
  if (DEBUG)
  {
    Serial.print("Starting up. Number: ");
    Serial.print(IDENTITY);
    Serial.print(". Is master-bot:");
    Serial.println(IS_MASTER_BOT);
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

// Internet callbackfunction
void onMessageReceived(byte senderID, String message)
{
  if (IS_MASTER_BOT)
  {
    byte number = byte(message.charAt(0))-48;
    if (number > 0 && number < 5 && state == STATE_IDLE)
    {
      destination = number;
      state = STATE_WARN;
    } else if (number > 4 && number < 9 && state == STATE_WARN)
    {
      if ((number-4) == destination)
      {
        state = STATE_RETURN;
        destination = 0;
      }
    }
  }
}


// Bluetooth communication
const char BT_GOTO[] = "G";
const char BT_RETURN[] = "R";
const char BT_HAS_RETURNED[] = "R";
const char BT_END[] = "\r\n";

unsigned long bluetoothSendTime = 0;

byte lastAskedState = 0;
boolean firstRun = true;
// Returns true the first time it's called, after a state change
boolean isFirstRun()
{
  return firstRun;
}

void updateFirstRun()
{
  if (lastAskedState != state)
  {
    lastAskedState = state;
    firstRun = true;
  }
  else
  {
    firstRun = false;
  }
}

// Function to blink led
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


// Called during idle-state. Checks for fire.
byte fireStartedCounter = 0;
boolean fireWasPutOut = false;
unsigned long timeToSendPutOutMessage = 0;
void loopIdle()
{
  if (!IS_MASTER_BOT && fireWasPutOut && millis() > timeToSendPutOutMessage)
  {
    timeToSendPutOutMessage = millis() + 1000;
    // Tell master-bot that the fire is no more
    if (USE_IR)
    {
      irsend.sendNEC(IR_PUT_OUT, 32);
    } else {
      internet.sendMessage(MASTER_BOT_ID, (String) char(IDENTITY+48+4));
    }
  }

  // Fire detected?
  if(analogRead(0)< FIRE_THRESHOLD){
    ++fireStartedCounter;
  } else {
    fireStartedCounter = 0;
  }

  // Fire detected for some time, warn
  if (fireStartedCounter > FIRE_DEBOUNCE) {
    // WE'RE BURNING
    // TELL SAMBOT
    if (IS_MASTER_BOT)
    {
      destination = IDENTITY; // master bot is burning
    }
    state = STATE_ON_FIRE;
    if (DEBUG) Serial.println("OMFG I'M ON FIRE");
  }
//    Serial.println(analogRead(0));
    /*if (fireIsPutOut())
    {
      generateNewTimeToBurn();
    }*/
  /*}*/
}

// Part of idle loop, specific for master bot - act on messages from other bots
void loopIdleMB()
{
  if (state == STATE_ON_FIRE)
  {
    return;
  }
  if (USE_IR && irrecv.decode(&results)) // have we received an IR signal?
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

    irrecv.resume(); // receive the next value
  } else if (!USE_IR && robotInDistress)
  {
    destination = robotInDistress;
    robotInDistress = 0;
  }
  
  if (destination != 0)
  {
    state = STATE_WARN;
    if (DEBUG)
    {
      Serial.print("Sam!!! There's a fire at helperbot #0");
      Serial.println(destination);
    }
  }
    
}

// Called during fire state. Warn the master bot or Sam about our own fire.
byte firePutOutCounter = 0;
unsigned long triggerTimeInternetWarning = 0;
void loopOnFire()
{
  // Tell masterbot about our suffering
  if (!IS_MASTER_BOT)
  {
    if (USE_IR)
    {
      irsend.sendNEC(IR_BURNING, 32);
    } else if(millis() > triggerTimeInternetWarning) {
      internet.sendMessage(MASTER_BOT_ID, (String) char(IDENTITY+48));
      triggerTimeInternetWarning = millis() + 1000;
    }
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
      destination = 0;
      state = STATE_RETURN;
      if (DEBUG) Serial.println("Fire put out. Get back home, Sam!");
    } else {
      fireWasPutOut = true;
      state = STATE_IDLE;
      if (DEBUG) Serial.println("Fire put out. Great work, Sam!");
    }
  }
}

// Master-bot, alerts Sam about a fire at another helper bot.
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
  byte sender = 0;
  if (USE_IR && irrecv.decode(&results)) // have we received an IR signal?
  {
    
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
    
    irrecv.resume(); // receive the next value
  } else if(robotSaved)
  {
      sender = robotSaved;
      robotSaved = 0;
  }

  if (sender == destination)
  {
    // The fire is out. Get Sam back home
    state = STATE_RETURN;
    Serial.println("The fire is put out, I hear. Get back home, Sam!");
  }
}


// Master-bot, call and wait for Sam to return home.
void loopReturn()
{
  if (isFirstRun())
  {
    bluetoothSendTime = millis() + 200;
    btSerial.write(BT_RETURN);
    btSerial.write("\r\n");
  }

  // Has Sam returned home?
  int result = -1;
  while (btSerial.available() > 1)
  {
    if (btSerial.read() == 'H')
    {
      if (btSerial.read() == 'R')
      {
        state = STATE_IDLE;
        if (DEBUG) Serial.println("Welcome back, Sam!");
      }
    }
  }
}

// This is where the action takes place
void loop()
{
  if (!USE_IR)
  {
    internet.update();
  }
  
  updateFirstRun();
  
  if (isFirstRun())
  {
    timeToSwitch = millis();
    delay(1);
  }
  
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
        //loopIdle(); // check if we are burning
        //loopIdleMB(); // check if anyone else is burning
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
