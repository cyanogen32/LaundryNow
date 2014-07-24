/**
 * Code for WashingMachine simulator device
 * Arduino attached to servo to simulate vibration for LaundryNow demo
 */


#include <Servo.h>

Servo VibrationServo; // create servo object to control a servo
Servo SpinCycle;
int pos = 0;    // variable to store the servo position
int pos2 = 0;
const int buttonPin = 2;
int buttonState = 0;
int buttonPushCounter = 0;
int lastButtonState = 0;

void setup()
{
  VibrationServo.attach(9);  // attaches the servo on pin 9 to the servo object
  SpinCycle.attach(7);
  pinMode(buttonPin, INPUT);
}

void loop()
{
  // read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);

if (buttonState != lastButtonState)
{
  if (buttonState == HIGH)
    {
    buttonPushCounter++;
    }
  else
  {
  delay(100);
  }
}
if (buttonPushCounter % 4 == 0)
{
   VibrationServo.write(125);
   delay(100);
   VibrationServo.write(175);
   delay(100);
}
else
{
   VibrationServo.write(150);
   delay(100);
}
lastButtonState = buttonState;
}
