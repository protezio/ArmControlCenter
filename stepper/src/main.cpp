/* Example sketch to control a 28BYJ-48 stepper motor with ULN2003 driver board, AccelStepper and Arduino UNO: continuous rotation. More info: https://www.makerguides.com */

// Include the AccelStepper library:
#include <AccelStepper.h>

// Motor pin definitions:
#define motorPin1  2      // IN1 on the ULN2003 driver
#define motorPin2  3      // IN2 on the ULN2003 driver
#define motorPin3  4     // IN3 on the ULN2003 driver
#define motorPin4  5     // IN4 on the ULN2003 driver
#define motorPinStep  11 
#define motorPinDir   10
// Define the AccelStepper interface type; 4 wire motor in half step mode:
#define MotorInterfaceType 4
#define MotorInterfaceType2 1
// Initialize with pin sequence IN1-IN3-IN2-IN4 for using the AccelStepper library with 28BYJ-48 stepper motor:
AccelStepper stepper = AccelStepper(MotorInterfaceType, motorPin1, motorPin3, motorPin2, motorPin4);
AccelStepper stepper2 = AccelStepper(MotorInterfaceType2, motorPinStep, motorPinDir);
void setup() {
  // Set the maximum steps per second:
   stepper.setMaxSpeed(500);
   stepper.setAcceleration(100);
   stepper.setCurrentPosition(5000);
  
   stepper2.setMaxSpeed(100);
   stepper2.setAcceleration(10);
   stepper2.setCurrentPosition(5000);
  

   }


void loop()
{
    // If at the end of travel go to the other end
    if (stepper.distanceToGo() == 0)
      stepper.moveTo(-stepper.currentPosition());

    stepper.run();

  if (stepper2.distanceToGo() == 0)
      stepper2.moveTo(-stepper2.currentPosition());

    stepper2.run();

}