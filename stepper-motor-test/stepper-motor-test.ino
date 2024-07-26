#include <Stepper.h>

const int stepsPerRevolution = 200;  // change this to fit the number of steps per revolution
// for your motor

// initialize the stepper library on pins 8 through 11:
Stepper myStepper(stepsPerRevolution, 8,6,7,5);

bool setPos = false;
//                 1 2 3 4 5 6 7 8 9 0 9  8  7  6  5  4  3  2  1
int positions[] = {5,5,5,5,5,5,5,5,5,5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5};
const int positionCount = 20;

void setup() {
  // set the speed at 60 rpm:
  myStepper.setSpeed(260);
  // initialize the serial port:
  Serial.begin(9600);
}

void loop() {
  // step one revolution  in one direction:
  //Serial.println("clockwise");
  //myStepper.step(stepsPerRevolution);
  //delay(500);

  // step one revolution in the other direction:
  //Serial.println("counterclockwise");
  //myStepper.step(-stepsPerRevolution);
  //delay(500);
  if (!setPos) {
    myStepper.step(-stepsPerRevolution);
    setPos = true;
    myStepper.step(20);
  }

  for (int step = 0; step <= positionCount; step++) {
    myStepper.step(positions[step]);
    delay(20);
  }
}