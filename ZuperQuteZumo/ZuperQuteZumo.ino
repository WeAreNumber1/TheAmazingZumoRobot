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

//Bluetooth:#
#include <SoftwareSerial.h>
#include <PLabBTSerial.h>

//collision stuff
// initialize vector values
LSM303 compass;
LSM303::vector<int16_t> vectorOffset = {0, 0, 0};
LSM303::vector<int16_t> vectorValues = {0, 0, 0};
boolean collisionDetected = false;
#define COLLISION_THRESHOLD 4000   // MUST BE CHANGED/CALIBRATED?

#define REVERSE_SPEED     300 // 0 is stopped, 400 is full speed
#define TURN_SPEED        250
#define FORWARD_SPEED     250
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

//Bluetooth things...
const int btUnitTxPin = 1; // Connected to tx on bt unit
const int btUnitRxPin = 0; // Connected to rx on bt unit
PLabBTSerial btSerial(btUnitTxPin, btUnitRxPin);

// Define Robot STATES
int currentState;
<<<<<<< Updated upstream
#define WAIT 1
#define FIGHT 2
#define TR 3
#define TL 4
#define STOP 5

//Bluetooth things...
const int btUnitTxPin = 1; // Connected to tx on bt unit
const int btUnitRxPin = 0; // Connected to rx on bt unit
PLabBTSerial btSerial(btUnitTxPin, btUnitRxPin);

void setup() {
  // Start communication with bluetooth unit
  btSerial.begin(9600);

  currentState = WAIT; //Set this to FIGHT for autonomeous robot. (No-remote...)
=======
#define FIGHT 1
#define ATTACK 2

void setup() {
  currentState = FIGHT;
>>>>>>> Stashed changes
  doVisionCalibration();
  delay(500);

  // Start up accelerometer
  Wire.begin();
  compass.init();
  compass.enableDefault();
  // Collect the current values - simple calibration
  calibrateAccelerator();
  turn(RIGHT);
}

void loop() {
  // See if we have received a new character
<<<<<<< Updated upstream
  int availableCount = btSerial.available();
  if (availableCount > 0) {
    char text[availableCount];
    btSerial.read(text, availableCount);
    int a =  readCommand(text);
    if (currentState != a){
      currentState = a;
    }
  }
  if(button.isPressed()){
    delay(200); //Wait 2 sec
    currentState = FIGHT; //Fight
  }
  //Handle state transitions and execute action to current state
  if (currentState == TR){
    search(true);
  }else if (currentState == FIGHT) {
    fight();
  }else if (currentState ==TL){
    search(false);
  }else if (currentState == STOP){
    stopMotors();
  }
}

void search(bool a){
  //Stops the robot
  //Makes it turn around
  //And when robot sees enemy, go to fightState and break;
  if (a){
    motors.setSpeeds(300,-300);
  }else{
    motors.setSpeeds(-300,300);
  }
  unsigned long time = sonar.ping();
  float distance = sonar.convert_cm(time);
  if (distance < 45) {
      currentState = FIGHT;
  }
}

void stopMotors(){
  motors.setSpeeds(0,0);
}

int readCommand (char *text) {
  if (0 == strcmp("FIGHT", text)) {
    return FIGHT;
  } else if (0 == strcmp("TR", text)) {
    return TR;
  } else if(0 == strcmp("TL",text)){
    return TL;
  } else if (0 == strcmp("STOP", text)) {
    return STOP;
  }
}

void wait(){
  //JUST CHILLING...
  motors.setSpeeds(0,0);
}

=======
 int availableCount; //= btSerial.available();
  if (availableCount > 0) {
    char text[availableCount];
    btSerial.read(text, availableCount);
    currentState = readCommand(text);
  //Handle state transitions and execute action to current state
    switch (currentState) {
      case FIGHT: fight(); break;
    }
  }
}

int readCommand (char *text) {
  if (0 == strcmp("FIGHT", text)) {
    return FIGHT;
  } else if (0 == strcmp("SEARCH", text)) {
    return FIGHT;
  }
}


>>>>>>> Stashed changes
void doVisionCalibration() {
  // -- Drives back and fourth to calibrate IR-reflectance sensors
  reflectanceSensors.init();
  // Wait 1 second and then begin automatic sensor calibration
  delay(1000);
  int i;
  for (i = 0; i < 80; i++) {
    if ((i > 10 && i <= 30) || (i > 50 && i <= 70))
      motors.setSpeeds(-50, -50);
    else
      motors.setSpeeds(50, 50);
<<<<<<< Updated upstream
=======

>>>>>>> Stashed changes
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
    motors.setSpeeds(0, 0);
  }
  else // turn left
  {
    motors.setSpeeds(-REVERSE_SPEED, -REVERSE_SPEED);
    delay(REVERSE_DURATION);
    motors.setSpeeds(-TURN_SPEED, TURN_SPEED);
    delay(TURN_DURATION);
    motors.setSpeeds(0, 0);
  }
}

//STATE Search for prey. The robot....
//blablabla
bool turnRight = true;
void fight() {
<<<<<<< Updated upstream
=======
  while (true){
>>>>>>> Stashed changes
  unsigned int position = 0;
  bool borderDetected = false;
    // Read the new accelerometer values
    compass.read();

    // Collect the new values
    updateVectorValues(); //Todo: implement some delay so that it doesn't update while right after accelerating/(turning).

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
  if (distance < 40) {
    return true;
  }
  return false;
}
