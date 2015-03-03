    ////////\\\\\\\\
   //////|||||\\\\\\\
  //««««WARNING»»»»»\\
 //DO NOT MERGE WITH \\
//  "MASTER BRANCH"   \\
///////////\\\\\\\\\\\\\\
//Note:
//This is an early implementation of Bluetooth Remote Controll.

//Bluetooth:#
#include <SoftwareSerial.h>
#include <PLabBTSerial.h>

//collision stuff
#include <Wire.h>
#include <LSM303.h>

#include <QTRSensors.h>
#include <ZumoReflectanceSensorArray.h>
#include <ZumoMotors.h>
#include <Pushbutton.h>
#include <NewPing.h>

//collision stuff
// initialize vector values
LSM303 compass;
LSM303::vector<int16_t> vectorOffset = {0, 0, 0};
LSM303::vector<int16_t> vectorValues = {0, 0, 0};
boolean collisionDetected = false;
#define COLLISION_THRESHOLD 4000   // MUST BE CHANGED/CALIBRATED?

#define REVERSE_SPEED     400 // 0 is stopped, 400 is full speed
#define TURN_SPEED        300
#define FORWARD_SPEED     300
#define ATTACK_SPEED     400
#define REVERSE_DURATION  200 // ms
#define TURN_DURATION     200 // ms
#define CHECK_DURATION    200 // ms
#define LEFT 1  // turn direction
#define RIGHT 2

// Pins:
const int echoPin = 6;
const int triggerPin = 3;
// Sonar sensor values
const int maxDistance = 300;
NewPing sonar(triggerPin, echoPin, maxDistance);

// Instansiate sensor, motor and pushbutton objects
ZumoReflectanceSensorArray reflectanceSensors;
ZumoMotors motors;
Pushbutton button(ZUMO_BUTTON);

// Set constants for PID control
unsigned int sensors[6];

// Define thresholds for border
#define BORDER_VALUE_LOW  800 // border low

// Define Robot STATES
int currentState;
#define WAIT 1
#define FIGHT 2
#define SEARCH 3

//Bluetooth things...
const int btUnitTxPin = 1; // Connected to tx on bt unit
const int btUnitRxPin = 0; // Connected to rx on bt unit
PLabBTSerial btSerial(btUnitTxPin, btUnitRxPin);

void setup() {
  // Start communication with bluetooth unit
  btSerial.begin(9600);

  currentState = WAIT; //Set this to FIGHT for autonomeous robot. (No-remote...)
  doVisionCalibration();
  delay(500);

  // Start up accelerometer
  Wire.begin();
  compass.init();
  compass.enableDefault();
  // Collect the current values - simple calibration
  calibrateAccelerator();
  turn(RIGHT);
  /*while(!button(ZUMO_BUTTON)){ //Wait for button press.
    delay(20);                 //There might be a better way...Cuz This Thing doesn't work...
  }*/
}

void loop() {
  // See if we have received a new character
  int availableCount = btSerial.available();
  if (availableCount > 0) {
    char text[availableCount];
    btSerial.read(text, availableCount);
    currentState = readCommand(text);
  }
  //Handle state transitions and execute action to current state
  switch (currentState) {
    case WAIT: wait(); break;
    case FIGHT: fight(); break;
    case SEARCH: search(); break;
  }
}

void search(){
  //Stops the robot
  //Makes it turn around
  //And when robot sees enemy, go to fightState and break;
  motors.setSpeeds(-150,150);
    if(enemyInSight()){
      currentState = FIGHT;
  }
}

int readCommand (char *text) {
  if (0 == strcmp("FIGHT", text)) {
    return FIGHT;
  } else if (0 == strcmp("SEARCH", text)) {
    return SEARCH;
  }
}

void wait(){
  //JUST CHILLING...
}

void doVisionCalibration() {
  //
  // -- Rotates to calibrate IR-reflectance sensors
  //
  reflectanceSensors.init();
  // Turn on LED to indicate we are in calibration mode
  /*pinMode(callibrateLedPin, OUTPUT);
  digitalWrite(callibrateLedPin, HIGH);*/

  // Wait 1 second and then begin automatic sensor calibration
  // by rotating in place to sweep the sensors over the line
  delay(1000);
  int i;
  for (i = 0; i < 80; i++) {
    if ((i > 10 && i <= 30) || (i > 50 && i <= 70))
      motors.setSpeeds(-50, -50);
    else
      motors.setSpeeds(50, 50);

    reflectanceSensors.calibrate();

    // Since our counter runs to 80, the total delay will be
    // 80*20 = 1600 ms.
    delay(20);
  }
}

void calibrateAccelerator()
{
  vectorOffset.x = compass.a.x;
  vectorOffset.y = compass.a.y;
  vectorOffset.z = compass.a.z;
}

// Reads new values, applies offset, etc
void updateVectorValues()
{
  vectorValues.x = compass.a.x - vectorOffset.x;
  vectorValues.y = compass.a.y - vectorOffset.y;
  vectorValues.z = compass.a.z - vectorOffset.z;
  collisionDetected = (abs(vectorValues.x) + abs(vectorValues.y) > COLLISION_THRESHOLD);
}

//
// Move Zumo backwards, then turn in given direction,
// and continue forward
//
void turn(int direction) {
  // Check wich direction to turn
  if (direction == RIGHT)
  {
    motors.setSpeeds(-REVERSE_SPEED, -REVERSE_SPEED);
    delay(REVERSE_DURATION);
    motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
    delay(TURN_DURATION);
    motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
  }
  else // turn left
  {
    motors.setSpeeds(-REVERSE_SPEED, -REVERSE_SPEED);
    delay(REVERSE_DURATION);
    motors.setSpeeds(-TURN_SPEED, TURN_SPEED);
    delay(TURN_DURATION);
    motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
  }
}

//STATE Search for prey. The robot....
//blablabla
bool turnRight = true;
void fight() {
  unsigned int position = 0;
  bool borderDetected = false;
    // Read the new accelerometer values
    compass.read();

    // Collect the new values
    updateVectorValues();

    //  read new sensor values
    position = reflectanceSensors.readLine(sensors);
    // Check to see if the border line has been reached. If so, turn and move in other direction.
    if (sensors[0] < BORDER_VALUE_LOW ) {
      // if leftmost sensor detects line, reverse and turn to the right
      borderDetected = true;
      turnRight = true;
      turn(RIGHT);
    }

    else if (sensors[5] < BORDER_VALUE_LOW) {
      // if rightmost sensor detects line, reverse and turn to the left
      borderDetected = true;
      turnRight = false;
      turn(LEFT);
    } else {
      if (collisionDetected) {
        //"FIGHT or Flight!"
        borderDetected = false;
      }
      // otherwise, FIGHT straight
      else if (enemyInSight()) {
        motors.setSpeeds(ATTACK_SPEED, ATTACK_SPEED);
      } else {//Tanken er aa holde farten oppe for en liten stund etter en kollisjon.
        if (false) {
          motors.setSpeeds(ATTACK_SPEED, ATTACK_SPEED);
        } else {
          if (turnRight){ //Makes the robot turn a little bit to the right while moving forward.
            motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED-150); //»»More effective Search for Prey.
          }else{
            motors.setSpeeds(FORWARD_SPEED-150, FORWARD_SPEED);
          }
        }
      }
    }
  }

bool enemyInSight() {
  unsigned long time = sonar.ping();
  float distance = sonar.convert_cm(time);
  if (distance < 60) {
    return true;
  }
  return false;
}
