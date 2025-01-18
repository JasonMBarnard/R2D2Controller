#include <Sabertooth.h>

//Syren and Sabertooth defines
#define SYREN_ADDR         128      // Serial Address for Dome Syren
#define SABERTOOTH_ADDR    129      // Serial Address for Foot Sabertooth (if used)
#define Serial_ST Serial2       //Serial port used by Sabertooth (feet) and/or Syren (dome) (only uses the TX pin from the Arduino)
#define motorControllerBaudRate 9600 //baud rate for Syren and/or Sabertooth

// Assign SBus channelss
const int ThrottleCh = 0;        // SBus channel used for throttle
const int SteeringCh = 1;        // SBus channel used for steering
const int DomeCh = 2;            // SBus channel used for the dome
const int Automate = 14;         // SBus channel used for dome automation



// Dome Automation Variables
const int minAutodomeSpeed = 75;
const int maxAutodomeSpeed = 80;
#define minAutodomeMoveMillis 700
#define maxAutodomeMoveMillis 4000
boolean domeAutomation = true;
int domeTurnDirection = 1;  // 1 = positive turn, -1 negative turn
float domeTargetPosition = 0; // (0 - 359) - degrees in a circle, 0 = home
unsigned long domeStopTurnTime = 0;    // millis() when next turn should stop
unsigned long domeStartTurnTime = 0;  // millis() when next turn should start
long rndNum;
int domeSpeed;
unsigned int autodomeMoveMillis;
int domeStatus = 0;  // 0 = stopped, 1 = prepare to turn, 2 = turning
int domeAutoSpeed;   // Speed will be randomly generated - Valid Values: 50 - 100
int time360DomeTurn = 5000;  // milliseconds for dome to complete 360 turn at domeAutoSpeed - Valid Values: 2000 - 8000 (2000 = 2 seconds)


int throttleVal;
int steeringVal;
int domeRotationSpeed;
int driveSpeed;

Sabertooth *ST=new Sabertooth(SABERTOOTH_ADDR, Serial_ST);
Sabertooth *SyR=new Sabertooth(SYREN_ADDR, Serial_ST);

void Sabertoothsetup () {
  //Setup for Motor Controllers - Sabertooth (Feet) and Syren10 (Dome)
  Serial_ST.begin(motorControllerBaudRate);
  ST->setRamping(true);         // Enable ramping for smoother acceleration
  ST->setDeadband(5);           // Set deadband for precise control
  ST->drive(0);
  ST->turn(0);
  SyR->motor(1, 0);
}

void Sabertoothloop () {
  // Limit Drive Speed
  if (data.ch[15] > 1400) {   // High Ppeed
    driveSpeed = 127;
  } else if (data.ch[15] > 700) {   // Medium Speed
    driveSpeed = 80;
  } else { 
    driveSpeed = 40;  //Low Speed
  }
 

  // Assign channel values to feet and dome control variables
  throttleVal = map(data.ch[ThrottleCh], 172, 1811, -driveSpeed, driveSpeed);
  steeringVal = map(data.ch[SteeringCh], 172, 1811, -driveSpeed, driveSpeed);
  domeRotationSpeed = map(data.ch[DomeCh], 172, 1811, -127, 127);

  // Control feet
  if (data.ch[ThrottleCh] >= 172 and data.ch[SteeringCh] >= 172) {  //Prevents feet motors from turning when controller is off
    ST->drive(throttleVal);
    ST->turn(steeringVal);
  }


  if (data.ch[DomeCh] >= 172 and data.ch[Automate] <= 1800) {
    domeAutomation = false;
    SyR->motor(domeRotationSpeed);
  }
  else {
    domeAutomation = true;
  }

  // If dome automation is enabled - Call function
  if (domeAutomation && time360DomeTurn > 1999 && time360DomeTurn < 8001) // && domeAutoSpeed > 49 && domeAutoSpeed < 101)  
  {
    autoDome(); 
  }   

  if (DebugMode) {
    //Uncomment the following lines to debug Sabertooth issues
    //Serial.print("DriveSpeed: ");
    //Serial.print(driveSpeed);
    //Serial.print("\t");
    //Serial.print("Throttle: ");
    //Serial.print(data.ch[ThrottleCh]);
    //Serial.print(" / ");
    //Serial.print(throttleVal);
    //Serial.print("\t");
    //Serial.print("Steering: ");
    //Serial.print(data.ch[SteeringCh]);
    //Serial.print(" / ");   
    //Serial.print(steeringVal);
    //Serial.print("\t");
    //Serial.print("DomeSpeed: ");
    //Serial.print(data.ch[DomeCh]);
    //Serial.print(" / ");
    //Serial.print(domeRotationSpeed);
    //Serial.print("\t");
    //Serial.print("Automation: ");
    //Serial.println(domeAutomation);
  }
}


void autoDome () {
 if (domeStatus == 0) { // Dome is currently stopped - prepare for a future turn
  if (domeTargetPosition == 0) { // Dome is currently in the home position - prepare to turn away
    domeStartTurnTime = millis() + (random(3, 10) * 1000);
    rndNum = random(5,354);
    domeTargetPosition = rndNum;  // set the target position to a random degree of a 360 circle - shaving off the first and last 5 degrees
    domeAutoSpeed=random(minAutodomeSpeed,maxAutodomeSpeed);  // a random dome speed
    if (domeTargetPosition < 180) { // Turn the dome in the positive direction
      domeTurnDirection = 1;
      domeStopTurnTime = domeStartTurnTime + ((domeTargetPosition / 360) * time360DomeTurn);
    }
    else { // Turn the dome in the negative direction
      domeTurnDirection = -1;
      domeStopTurnTime = domeStartTurnTime + (((360 - domeTargetPosition) / 360) * time360DomeTurn);
    }
  }
  else { // Dome is not in the home position - send it back to home
    domeStartTurnTime = millis() + (random(3, 10) * 1000);
    if (domeTargetPosition < 180) {
      domeTurnDirection = -1;
      domeStopTurnTime = domeStartTurnTime + ((domeTargetPosition / 360) * time360DomeTurn);
    } 
    else {
      domeTurnDirection = 1;
      domeStopTurnTime = domeStartTurnTime + (((360 - domeTargetPosition) / 360) * time360DomeTurn);
    }
    domeTargetPosition = 0;
  }
  domeStatus = 1;  // Set dome status to preparing for a future turn
  }        
    
  if (domeStatus == 1) { // Dome is prepared for a future move - start the turn when ready
    if (domeStartTurnTime < millis()) {
      domeStatus = 2; 
    }
  }
    
  if (domeStatus == 2) { // Dome is now actively turning until it reaches its stop time
    if (domeStopTurnTime > millis()) {
      domeSpeed = domeAutoSpeed * domeTurnDirection;
      SyR->motor(domeSpeed);
    }
    else { // turn completed - stop the motor
      domeStatus = 0;
      SyR->stop();
    } 
  }
}