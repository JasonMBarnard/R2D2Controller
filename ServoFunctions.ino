#include <Servo.h>

// Note: Servo's are attached and detached as needed.  This reduces battery usage, servo noise caused by residual pressure, and servo damage for kids pulling on them.

// Define states
enum GripperDoorState {GRIPPER_DOOR_CLOSED, GRIPPER_DOOR_OPENING, GRIPPER_DOOR_OPENED, GRIPPER_DOOR_CLOSING};
enum GripperArmState {GRIPPER_ARM_LOWERED, GRIPPER_ARM_RAISING, GRIPPER_ARM_RAISED, GRIPPER_ARM_LOWERING};
enum InterfaceDoorState {INTERFACE_DOOR_CLOSED, INTERFACE_DOOR_OPENING, INTERFACE_DOOR_OPENED, INTERFACE_DOOR_CLOSING};
enum InterfaceArmState {INTERFACE_ARM_LOWERED, INTERFACE_ARM_RAISING, INTERFACE_ARM_RAISED, INTERFACE_ARM_LOWERING};
enum DataportDoorState {DATAPORT_DOOR_CLOSED, DATAPORT_DOOR_OPENED};
enum ChargePanelDoorState {CHARGEPANEL_DOOR_CLOSED, CHARGEPANEL_DOOR_OPENED};
enum InterfaceState {INTERFACE_RETRACTED, INTERFACE_EXTENDED};
enum GripperState {GRIPPER_CLOSED, GRIPPER_OPENED};
GripperDoorState gripperDoorState = GRIPPER_DOOR_CLOSED;
GripperArmState gripperArmState = GRIPPER_ARM_LOWERED;
InterfaceDoorState interfaceDoorState = INTERFACE_DOOR_CLOSED;
InterfaceArmState interfaceArmState = INTERFACE_ARM_LOWERED;
DataportDoorState dataportDoorState = DATAPORT_DOOR_CLOSED;
ChargePanelDoorState chargepanelDoorState = CHARGEPANEL_DOOR_CLOSED;
InterfaceState interfaceState = INTERFACE_RETRACTED;
GripperState gripperState = GRIPPER_CLOSED;

// Assign SBus channels
const int TopUtilArmCh = 4;     // SBus channel used for the top utility arm
const int BotUtilArmCh = 3;     // SBus channel used for the bottom utility arm
const int IntDoorCh = 5;        // SBus channel used for both the interface arm and the door
const int GripDoorCh = 6;       // SBus channel used for both the gripper arm and the door
const int IntGripAction = 7;    // SBus channel used for the Interface and Gripper action.  This is a shared channel and will perform the action if the arm is open (up).
const int DataPortDoorCh = 8;   // SBus channel used for the data port door
const int ChargePanelDoorCh = 9; // SBus channel used for the charge panel door

// Assign Arduino pins
const int TopUtilArmPin = 3;        // The Arduino pin used for the top utility arm
const int BotUtilArmPin = 2;        // The Arduino pin used for the bottom utility arm
const int interfaceDoorPin = 4;     // The Arduino pin used for the interface arm door
const int gripperDoorPin = 5;       // The Arduino pin used for the gripper arm door
const int interfaceArmPin = 6;      // The Arduino pin used for the interface arm
const int gripperArmPin = 7;        // The Arduino pin used for the gripper arm
const int IntActionPin = 8;         // The Arduino pin used for the interface action
const int GripActionPin = 9;        // The Arduino pin used for the gripper action
const int DataPortDoorPin = 10;     // The Arduino pin used for the data port door
const int ChargePanelDoorPin = 11;  // The Arduino pin used for the charge panel door

// Assign Servo start and stop points
const int TopUtilArmClosed = 41;   // Servo angle when the top utility arm is closed, from 0-180
const int TopUtilArmOpen = 152;    // Servo angle when the top utility arm is open, from 0 = 180
const int BotUtilArmClosed = 41;
const int BotUtilArmOpen = 152;
const int interfaceDoorClosed = 140;
const int interfaceDoorOpened = 0;
const int interfaceArmLowered = 140;
const int interfaceArmRaised = 0;
const int InterfaceClosed = 0;
const int InterfaceOpen = 180;
const int gripperDoorClosed = 0;
const int gripperDoorOpened = 140;
const int gripperArmLowered = 0;
const int gripperArmRaised = 150;
const int GripperClosed = 70;
const int GripperOpen = 0;
const int DataPortDoorOpen = 0;
const int DataPortDoorClosed = 140;
const int ChargePanelDoorOpen = 0;
const int ChargePanelDoorClosed = 140;

// Variables to track timing
unsigned long gripperDoorLastActionMillis = 0;
unsigned long gripperArmLastActionMillis = 0;
unsigned long interfaceDoorLastActionMillis = 0; 
unsigned long interfaceArmLastActionMillis = 0;
unsigned long dataportDoorLastActionMillis = 0;
unsigned long chargepanelDoorLastActionMillis = 0;
unsigned long interfaceLastActionMillis = 0;
unsigned long gripperLastActionMillis = 0;

// Define time intervals for actions (in milliseconds)
const unsigned long gripperArmActionTime = 500;
const unsigned long gripperDoorActionTime = 600;
const unsigned long interfaceArmActionTime = 500;
const unsigned long interfaceDoorActionTime = 600;
const unsigned long idleDetachTime = 2000; // Time after which servos will be detached if idle

// Variables to track switch positions
int currentGripperSwitchPosition = 0;
int currentInterfaceSwitchPosition = 0;
int currentDataportSwitchPosition = 0;
int currentChargePanelSwitchPosition = 0;
int gripperInterfaceActionSwitchPosition = 0;

int TopUtilArmVal = TopUtilArmClosed;
int PrevTopUtilArmVal = TopUtilArmClosed;
int TopUtilArmDetachCount = 0;
int BotUtilArmVal = BotUtilArmClosed;
int PrevBotUtilArmVal = BotUtilArmClosed;
int BotUtilArmDetachCount = 0;

Servo TopUtilArm;
Servo BotUtilArm;
Servo interfaceDoorServo;
Servo gripperDoorServo;
Servo interfaceArmServo;
Servo gripperArmServo;
Servo interfaceServo;
Servo gripperServo;
Servo dataportDoorServo;
Servo chargepanelDoorServo;

void ServoSetup() {
  // Ensure all servos are in a closed status on startup.
  TopUtilArm.attach(TopUtilArmPin);
  TopUtilArm.write(TopUtilArmClosed);
  TopUtilArm.detach();
  BotUtilArm.attach(BotUtilArmPin);
  BotUtilArm.write(BotUtilArmClosed);
  BotUtilArm.detach();
  lowerGripperArm();
  closeGripperDoor();
  lowerInterfaceArm(); 
  closeInterfaceDoor(); 
  closeDataportDoor();
  closeChargePanelDoor();
}

void openGripperDoor() {
  if (!gripperDoorServo.attached()) {
    gripperDoorServo.attach(gripperDoorPin);
  }
  gripperDoorServo.write(gripperDoorOpened);
  gripperDoorLastActionMillis = millis();
  gripperDoorState = GRIPPER_DOOR_OPENING;
}

void closeGripperDoor() {
  if (!gripperDoorServo.attached()) {
    gripperDoorServo.attach(gripperDoorPin);
  }
  gripperDoorServo.write(gripperDoorClosed);
  gripperDoorLastActionMillis = millis();
  gripperDoorState = GRIPPER_DOOR_CLOSING;
}

void raiseGripperArm() {
  if (!gripperArmServo.attached()) {
    gripperArmServo.attach(gripperArmPin);
  }
  gripperArmServo.write(gripperArmRaised);
  gripperArmLastActionMillis = millis();
  gripperArmState = GRIPPER_ARM_RAISING;
}

void lowerGripperArm() {
  if (!gripperArmServo.attached()) {
    gripperArmServo.attach(gripperArmPin);
  }
  gripperArmServo.write(gripperArmLowered);
  gripperArmLastActionMillis = millis();
  gripperArmState = GRIPPER_ARM_LOWERING;
}

void openInterfaceDoor() {
  if (!interfaceDoorServo.attached()) {
    interfaceDoorServo.attach(interfaceDoorPin);
    //interfaceDoorServoAttached = true;
  }
  interfaceDoorServo.write(interfaceDoorOpened); // Example position for open door
  interfaceDoorLastActionMillis = millis();
  interfaceDoorState = INTERFACE_DOOR_OPENING;
}

void closeInterfaceDoor() {
  if (!interfaceDoorServo.attached()) {
    interfaceDoorServo.attach(interfaceDoorPin);
    //interfaceDoorServoAttached = true;
  }
  interfaceDoorServo.write(interfaceDoorClosed); // Example position for closed door
  interfaceDoorLastActionMillis = millis();
  interfaceDoorState = INTERFACE_DOOR_CLOSING;
}

void raiseInterfaceArm() {
  if (!interfaceArmServo.attached()) {
    interfaceArmServo.attach(interfaceArmPin);
    //interfaceArmServoAttached = true;
  }
  interfaceArmServo.write(interfaceArmRaised); // Example position for raised arm
  interfaceArmLastActionMillis = millis();
  interfaceArmState = INTERFACE_ARM_RAISING;
}

void lowerInterfaceArm() {
  if (!interfaceArmServo.attached()) {
    interfaceArmServo.attach(interfaceArmPin);
  }
  interfaceArmServo.write(interfaceArmLowered); // Example position for lowered arm
  interfaceArmLastActionMillis = millis();
  interfaceArmState = INTERFACE_ARM_LOWERING;
}

void openDataportDoor() {
  if(!dataportDoorServo.attached()) {
    dataportDoorServo.attach(DataPortDoorPin);
  }
  dataportDoorServo.write(DataPortDoorOpen);
  dataportDoorLastActionMillis = millis();
  dataportDoorState = DATAPORT_DOOR_OPENED;
}

void closeDataportDoor() {
  if(!dataportDoorServo.attached()) {
    dataportDoorServo.attach(DataPortDoorPin);
  }  
  dataportDoorServo.write(DataPortDoorClosed);
  dataportDoorLastActionMillis = millis();
  dataportDoorState = DATAPORT_DOOR_CLOSED;  
}

void openChargePanelDoor() {
  if(!chargepanelDoorServo.attached()) {
    chargepanelDoorServo.attach(ChargePanelDoorPin);
  }
  chargepanelDoorServo.write(ChargePanelDoorOpen);
  chargepanelDoorLastActionMillis = millis();
  chargepanelDoorState = CHARGEPANEL_DOOR_OPENED;
}

void closeChargePanelDoor() {
  if(!chargepanelDoorServo.attached()) {
    chargepanelDoorServo.attach(ChargePanelDoorPin);
  }
  chargepanelDoorServo.write(ChargePanelDoorClosed);
  chargepanelDoorLastActionMillis = millis();
  chargepanelDoorState = CHARGEPANEL_DOOR_CLOSED;
}

void retractInterface() {
  if (!interfaceServo.attached()) {
    interfaceServo.attach(IntActionPin);
  }
  interfaceServo.write(InterfaceClosed);
  interfaceLastActionMillis = millis();
  interfaceState = INTERFACE_RETRACTED;
}

void extendInterface(){
  if (!interfaceServo.attached()) {
    interfaceServo.attach(IntActionPin);
  }
  interfaceServo.write(InterfaceOpen);
  interfaceLastActionMillis = millis();
  interfaceState = INTERFACE_EXTENDED;
}

void closeGripper() {
  if (!gripperServo.attached()) {
    gripperServo.attach(GripActionPin);
  }
  gripperServo.write(GripperClosed);
  gripperLastActionMillis = millis();
  gripperState = GRIPPER_CLOSED;
}

void openGripper() {
  if (!gripperServo.attached()) {
    gripperServo.attach(GripActionPin);
  }
  gripperServo.write(GripperOpen);
  gripperLastActionMillis = millis();
  gripperState = GRIPPER_OPENED;
}

void Servoloop() {
  // Capture servo channel data
  TopUtilArmVal = map(data.ch[TopUtilArmCh], 172, 1811, TopUtilArmOpen, TopUtilArmClosed);
  BotUtilArmVal = map(data.ch[BotUtilArmCh], 172, 1811, BotUtilArmOpen, BotUtilArmClosed);
  currentGripperSwitchPosition = data.ch[GripDoorCh];
  currentInterfaceSwitchPosition = data.ch[IntDoorCh];
  currentDataportSwitchPosition = data.ch[DataPortDoorCh];
  currentChargePanelSwitchPosition = data.ch[ChargePanelDoorCh];
  gripperInterfaceActionSwitchPosition = data.ch[IntGripAction];

  // Top utility arm control
  if (TopUtilArmVal != PrevTopUtilArmVal) {
    TopUtilArm.attach(TopUtilArmPin);
    if (TopUtilArmVal >= TopUtilArmClosed and TopUtilArmVal <= TopUtilArmOpen) {
      TopUtilArm.write(TopUtilArmVal);
    }
    PrevTopUtilArmVal = TopUtilArmVal;
  }
  else if (TopUtilArmVal = PrevTopUtilArmVal and TopUtilArm.attached()) {
    TopUtilArmDetachCount ++;
    if (TopUtilArmDetachCount > 80) {
      TopUtilArm.detach();
      TopUtilArmDetachCount = 0;
    }
  }

  // Bottom utility arm control
  if (BotUtilArmVal != PrevBotUtilArmVal) {
    BotUtilArm.attach(BotUtilArmPin);
    if (BotUtilArmVal >= BotUtilArmClosed and BotUtilArmVal <= BotUtilArmOpen) {
      BotUtilArm.write(BotUtilArmVal);
    }
    PrevBotUtilArmVal = BotUtilArmVal;
  }
  else if (BotUtilArmVal = PrevBotUtilArmVal and BotUtilArm.attached()) {
    BotUtilArmDetachCount ++;
    if (BotUtilArmDetachCount > 80) {
      BotUtilArm.detach();
      BotUtilArmDetachCount = 0;
    }
  }

  // Data Port Door control
  if (currentDataportSwitchPosition < 500 && dataportDoorState != DATAPORT_DOOR_CLOSED) {
    closeDataportDoor();
  }
  if (currentDataportSwitchPosition >= 500 && dataportDoorState != DATAPORT_DOOR_OPENED) {
    openDataportDoor();
  }

  // Charge Panel Door control
  if (currentChargePanelSwitchPosition < 500 && chargepanelDoorState != CHARGEPANEL_DOOR_CLOSED) {
    closeChargePanelDoor();
  }
  if (currentChargePanelSwitchPosition >= 500 && chargepanelDoorState != CHARGEPANEL_DOOR_OPENED) {
    openChargePanelDoor();
  }

  // Check if gripper switch positions have changed
  if (
  (currentGripperSwitchPosition < 500 && (gripperDoorState != GRIPPER_DOOR_CLOSED or gripperArmState != GRIPPER_ARM_LOWERED))
  or (currentGripperSwitchPosition >= 500 && currentGripperSwitchPosition < 1500 && (gripperDoorState != GRIPPER_DOOR_OPENED or gripperArmState != GRIPPER_ARM_LOWERED))
  or (currentGripperSwitchPosition >= 1500 && (gripperDoorState != GRIPPER_DOOR_OPENED or gripperArmState != GRIPPER_ARM_RAISED))
  ) {
    handleGripperSwitchChange(currentGripperSwitchPosition);
  }

  // Check if interface switch positions have changed
  if (
  (currentInterfaceSwitchPosition < 500 && (interfaceDoorState != INTERFACE_DOOR_CLOSED or interfaceArmState != INTERFACE_ARM_LOWERED))
  or (currentInterfaceSwitchPosition >= 500 && currentInterfaceSwitchPosition < 1500 && (interfaceDoorState != INTERFACE_DOOR_OPENED or interfaceArmState != INTERFACE_ARM_LOWERED))
  or (currentInterfaceSwitchPosition >= 1500 && (interfaceDoorState != INTERFACE_DOOR_OPENED or interfaceArmState != INTERFACE_ARM_RAISED))
  ) {
    handleInterfaceSwitchChange(currentInterfaceSwitchPosition);
  }

  //Interface and Gripper mechanism action
  if (gripperInterfaceActionSwitchPosition < 500) {
    if (gripperState == GRIPPER_OPENED) {
      closeGripper();
    }
    if (interfaceState == INTERFACE_EXTENDED) {
      retractInterface();
    }
  }
  if (gripperInterfaceActionSwitchPosition >= 500) {
    if (gripperState == GRIPPER_CLOSED && gripperArmState == GRIPPER_ARM_RAISED) {
      openGripper();
    }
    if (interfaceState == INTERFACE_RETRACTED && interfaceArmState == INTERFACE_ARM_RAISED) {
      extendInterface();
    }
  }
 
  // Kill attached Servos to quiet buzz
  if (gripperArmServo.attached() && currentMillis >= gripperArmLastActionMillis + idleDetachTime) {
    gripperArmServo.detach();
  }
  if (gripperDoorServo.attached() && currentMillis >= gripperDoorLastActionMillis + idleDetachTime) {
    gripperDoorServo.detach();
  }
  if (interfaceArmServo.attached() && currentMillis >= interfaceArmLastActionMillis + idleDetachTime) {
    interfaceArmServo.detach();
  }
  if (interfaceDoorServo.attached() && currentMillis >= interfaceDoorLastActionMillis + idleDetachTime) {
    interfaceDoorServo.detach();
  }
  if (gripperServo.attached() && currentMillis >= gripperLastActionMillis + idleDetachTime) {
    gripperServo.detach();
  }
  if (interfaceServo.attached() && currentMillis >= interfaceLastActionMillis + idleDetachTime) {
    interfaceServo.detach();
  }
  if (dataportDoorServo.attached() && currentMillis >= dataportDoorLastActionMillis + idleDetachTime) {
    dataportDoorServo.detach();
  }
  if (chargepanelDoorServo.attached() && currentMillis >= chargepanelDoorLastActionMillis + idleDetachTime) {
    chargepanelDoorServo.detach();
  }
}

void handleGripperSwitchChange(int switchPosition) {
  currentMillis = millis();
  if (switchPosition < 500) { // Switch position 1, gripper Arm lowered and gripper door closed
    if (gripperState == GRIPPER_OPENED) {
      closeGripper();
    }
    if (gripperArmState != GRIPPER_ARM_LOWERED) {  //Check if the gripper arm is lowered
      switch (gripperArmState) {
        case GRIPPER_ARM_RAISING:
          lowerGripperArm();
        break;
        case GRIPPER_ARM_RAISED:
          lowerGripperArm();
        break;
        case GRIPPER_ARM_LOWERING:
          if (currentMillis >= gripperArmLastActionMillis + gripperArmActionTime) {
            gripperArmState = GRIPPER_ARM_LOWERED;
          }
        break;
      }
    }
    else if (gripperArmState == GRIPPER_ARM_LOWERED) {
      switch (gripperDoorState){
        case GRIPPER_DOOR_OPENING:
          closeGripperDoor();
        break;
        case GRIPPER_DOOR_OPENED:
          closeGripperDoor();
        break;
        case GRIPPER_DOOR_CLOSING:
          if (currentMillis >= gripperDoorLastActionMillis + gripperDoorActionTime) {
            gripperDoorState = GRIPPER_DOOR_CLOSED;
          }
        break;
      }
    }
  }
  if (switchPosition >= 500 && switchPosition < 1500) {
    if (gripperState == GRIPPER_OPENED) {
      closeGripper();
    }
    if (gripperArmState != GRIPPER_ARM_LOWERED) {  //Check if the gripper arm is lowered
      switch (gripperArmState) {
        case GRIPPER_ARM_RAISING:
          lowerGripperArm();
        break;
        case GRIPPER_ARM_RAISED:
          lowerGripperArm();
        break;
        case GRIPPER_ARM_LOWERING:
           if (currentMillis >= gripperArmLastActionMillis + gripperArmActionTime) {
            gripperArmState = GRIPPER_ARM_LOWERED;
          }       
        break;
      }
    }
    if (gripperDoorState != GRIPPER_DOOR_OPENED) {
      switch (gripperDoorState) {
        case GRIPPER_DOOR_CLOSING:
          openGripperDoor();
        break;
        case GRIPPER_DOOR_CLOSED:
          openGripperDoor();
        break;
        case GRIPPER_DOOR_OPENING:
          if (currentMillis >= gripperArmLastActionMillis + gripperArmActionTime) {
            gripperDoorState = GRIPPER_DOOR_OPENED;
          }
        break;
      }
    }
  }
  if (switchPosition >= 1500) {
    if (gripperDoorState != GRIPPER_DOOR_OPENED) {
      switch (gripperDoorState) {
        case GRIPPER_DOOR_CLOSING:
          openGripperDoor();          
        break;
        case GRIPPER_DOOR_CLOSED:
          openGripperDoor();         
        break;
        case GRIPPER_DOOR_OPENING:
          if (currentMillis >= gripperDoorLastActionMillis + gripperDoorActionTime) {
            gripperDoorState = GRIPPER_DOOR_OPENED;
          }
        break;
      }
    }
    if (gripperDoorState == GRIPPER_DOOR_OPENED && gripperArmState != GRIPPER_ARM_RAISED) {
      switch (gripperArmState) {
        case GRIPPER_ARM_LOWERING:
          raiseGripperArm();
        break;
        case GRIPPER_ARM_LOWERED:
          raiseGripperArm();
        break;
        case GRIPPER_ARM_RAISING:
          if (currentMillis >= gripperArmLastActionMillis + gripperArmActionTime) {
            gripperArmState = GRIPPER_ARM_RAISED;
          }
        break;
      }
    }
  }
}

void handleInterfaceSwitchChange(int switchPosition) {
  currentMillis = millis();
  if (switchPosition < 500) { // Switch position 1, interface Arm lowered and interface door closed
    if (interfaceState == INTERFACE_EXTENDED) {
      retractInterface();
    }
    if (interfaceArmState != INTERFACE_ARM_LOWERED) {  //Check if the interface arm is lowered
      switch (interfaceArmState) {
        case INTERFACE_ARM_RAISING:
          lowerInterfaceArm();
        break;
        case INTERFACE_ARM_RAISED:
          lowerInterfaceArm();
        break;
        case INTERFACE_ARM_LOWERING:
          if (currentMillis >= interfaceArmLastActionMillis + interfaceArmActionTime) {
            interfaceArmState = INTERFACE_ARM_LOWERED;
          }
        break;
      }
    }
    else if (interfaceArmState == INTERFACE_ARM_LOWERED) {
      switch (interfaceDoorState){
        case INTERFACE_DOOR_OPENING:
          closeInterfaceDoor();
        break;
        case INTERFACE_DOOR_OPENED:
          closeInterfaceDoor();
        break;
        case INTERFACE_DOOR_CLOSING:
          if (currentMillis >= interfaceDoorLastActionMillis + interfaceDoorActionTime) {
            interfaceDoorState = INTERFACE_DOOR_CLOSED;
          }
        break;
      }
    }
  }
  if (switchPosition >= 500 && switchPosition < 1500) {
    if (interfaceState == INTERFACE_EXTENDED) {
      retractInterface();
    }
    if (interfaceArmState != INTERFACE_ARM_LOWERED) {  //Check if the interface arm is lowered
      switch (interfaceArmState) {
        case INTERFACE_ARM_RAISING:
          lowerInterfaceArm();
        break;
        case INTERFACE_ARM_RAISED:
          lowerInterfaceArm();
        break;
        case INTERFACE_ARM_LOWERING:
           if (currentMillis >= interfaceArmLastActionMillis + interfaceArmActionTime) {
            interfaceArmState = INTERFACE_ARM_LOWERED;
          }       
        break;
      }
    }
    if (interfaceDoorState != INTERFACE_DOOR_OPENED) {
      switch (interfaceDoorState) {
        case INTERFACE_DOOR_CLOSING:
          openInterfaceDoor();
        break;
        case INTERFACE_DOOR_CLOSED:
          openInterfaceDoor();
        break;
        case INTERFACE_DOOR_OPENING:
          if (currentMillis >= interfaceArmLastActionMillis + interfaceArmActionTime) {
            interfaceDoorState = INTERFACE_DOOR_OPENED;
          }
        break;
      }
    }
  }
  if (switchPosition >= 1500) {
    if (interfaceDoorState != INTERFACE_DOOR_OPENED) {
      switch (interfaceDoorState) {
        case INTERFACE_DOOR_CLOSING:
          openInterfaceDoor();          
        break;
        case INTERFACE_DOOR_CLOSED:
          openInterfaceDoor();         
        break;
        case INTERFACE_DOOR_OPENING:
          if (currentMillis >= interfaceDoorLastActionMillis + interfaceDoorActionTime) {
            interfaceDoorState = INTERFACE_DOOR_OPENED;
          }
        break;
      }
    }
    if (interfaceDoorState == INTERFACE_DOOR_OPENED && interfaceArmState != INTERFACE_ARM_RAISED) {
      switch (interfaceArmState) {
        case INTERFACE_ARM_LOWERING:
          raiseInterfaceArm();
        break;
        case INTERFACE_ARM_LOWERED:
          raiseInterfaceArm();
        break;
        case INTERFACE_ARM_RAISING:
          if (currentMillis >= interfaceArmLastActionMillis + interfaceArmActionTime) {
            interfaceArmState = INTERFACE_ARM_RAISED;
          }
        break;
      }
    }
  }
}