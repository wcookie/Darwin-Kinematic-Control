/*
When running this controller in the real robot
do not interface via SSh client (i,e, PuTTY)
The result will be on a segmentation fault error.

Instead interface with the robot via remote desktop
(i.e. VNC). Open a terminal window, compile controller
and run.
*/
#include "Walk.hpp"
#include <webots/LED.hpp>
#include <webots/Accelerometer.hpp>
#include <webots/Gyro.hpp>
#include <webots/utils/Motion.hpp>
#include <webots/Speaker.hpp>
#include <webots/Camera.hpp>
#include <webots/Motor.hpp>
#include <webots/PositionSensor.hpp>
#include <webots/Keyboard.hpp>
#include <RobotisOp2MotionManager.hpp>
#include <RobotisOp2GaitManager.hpp>
#include <RobotisOp2VisionManager.hpp>

#include <cassert>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <fstream>

using namespace webots;
using namespace managers;
using namespace std;
static double minMotorPositions[NMOTORS];
static double maxMotorPositions[NMOTORS];


static const char *motorNames[NMOTORS] = {
  "ShoulderR" /*ID0 */, "ShoulderL" /*ID1 */, "ArmUpperR" /*ID2 */, "ArmUpperL" /*ID3 */,
  "ArmLowerR" /*ID4 */, "ArmLowerL" /*ID5 */, "PelvYR"    /*ID6 */, "PelvYL"    /*ID7 */,
  "PelvR"     /*ID8 */, "PelvL"     /*ID9*/, "LegUpperR" /*ID10*/, "LegUpperL" /*ID11*/,
  "LegLowerR" /*ID12*/, "LegLowerL" /*ID13*/, "AnkleR"    /*ID14*/, "AnkleL"    /*ID15*/,
  "FootR"     /*ID16*/, "FootL"     /*ID17*/, "Neck"      /*ID18*/, "Head"      /*ID19*/
};

Walk::Walk():
    Robot()
{
  mTimeStep = getBasicTimeStep();

  mEyeLED = getLED("EyeLed");
  mEyeLED->set(0x00FF00);
  getLED("HeadLed")->set(0xFF0000);
  mCamera = getCamera("Camera");
  mCamera->enable(2*mTimeStep);
  
  mAccelerometer = getAccelerometer("Accelerometer");
  mAccelerometer->enable(mTimeStep);

  getGyro("Gyro")->enable(mTimeStep);
  mSpeaker = getSpeaker("Speaker");

  for (int i=0; i<NMOTORS; i++) {
    mMotors[i] = getMotor(motorNames[i]);
    string sensorName = motorNames[i];
    sensorName.push_back('S');
    mPositionSensors[i] = getPositionSensor(sensorName);
    mPositionSensors[i]->enable(mTimeStep);
    minMotorPositions[i] = mMotors[i]->getMinPosition();
    maxMotorPositions[i] = mMotors[i]->getMaxPosition();
  }

  mKeyboard = getKeyboard();
  mKeyboard->enable(mTimeStep);

  mMotionManager = new RobotisOp2MotionManager(this);
  mGaitManager   = new RobotisOp2GaitManager(this, "config.ini");
  mVisionManager = new RobotisOp2VisionManager(mCamera->getWidth(), mCamera->getHeight(), 28, 20, 50, 45, 0, 30);
}

Walk::~Walk() {
}

void Walk::myStep() {
  int ret = step(mTimeStep);
  if (ret == -1)
    exit(EXIT_SUCCESS);
}

void Walk::wait(int ms) {
  double startTime = getTime();
  double s = (double) ms / 1000.0;
  while (s + startTime >= getTime())
    myStep();
}



// function containing the main feedback loop
void Walk::run() {
 
 
 cout<<"Min Head: "<<minMotorPositions[19]<<endl;
 cout<<"Max Head: "<<maxMotorPositions[19]<<endl;
 cout<<"Min Neck(left): "<<minMotorPositions[18]<<endl;
 cout<<"Max Neck(right): "<<maxMotorPositions[18]<<endl;
  // First step to update sensors values
  myStep();
  // play the hello motion
  mMotionManager->playPage(1);
  mMotionManager->playPage(9); // init position
  wait(200);
  

  // main loop
  int key = 0;
  bool isWalking = false;
  double currentNeckPos = 0;
  double currentHeadPos = 0;
  double currentRArmPos = 0;
  double currentLArmPos = 0;
  
  while (true) {
    checkIfFallen();

    mGaitManager->setXAmplitude(0.0);
    mGaitManager->setAAmplitude(0.0);

    // get keyboard key
    while((key = mKeyboard->getKey()) >= 0) {
      switch(key) {
        case ' ' : // Space bar
          if(isWalking) {
            mGaitManager->stop();
            isWalking = false;
            wait(200);
          }
          else {
            mGaitManager->start();
            isWalking = true;
            wait(200);
          }
          break;
        case Keyboard::UP :
          mGaitManager->setXAmplitude(1.0);
          //move(100,0);
          break;
        case Keyboard::DOWN :
          mGaitManager->setXAmplitude(-1.0);
          //move(-100,0);
          break;
        case Keyboard::RIGHT :
          //move(50,1);
          mGaitManager->setAAmplitude(-0.5);
          break;
        case Keyboard::LEFT :
          //move(50, -1);
          mGaitManager->setAAmplitude(0.5);
          break;
        case 'A' :
          if(currentNeckPos < (maxMotorPositions[18]-.1)){
          mMotors[18]->setPosition(currentNeckPos+0.05);
          currentNeckPos = currentNeckPos + 0.05;
          }
          break;
        case '1':
          move(200,200);
          break;
        case 'D' :
          if(currentNeckPos > (minMotorPositions[18]+.1)){
          mMotors[18]->setPosition(currentNeckPos-0.05);
          currentNeckPos = currentNeckPos - 0.05;
          }
          break;
        case 'W':
          if(currentHeadPos < (maxMotorPositions[19]-.1)){
            mMotors[19]->setPosition(currentHeadPos+0.05);
            currentHeadPos = currentHeadPos + 0.05;
          }
          break;
        case 'S':
          if(currentHeadPos > (minMotorPositions[19]+.1)){
            mMotors[19]->setPosition(currentHeadPos-0.05);
            currentHeadPos = currentHeadPos - 0.05;
          }
          break;
        case 'R':
          mEyeLED->set(0xFF0000);
          break;
        case 'G':
          mEyeLED->set(0x00FF00);
          break;
        case 'F':
          if(currentRArmPos > (minMotorPositions[1]+.1)){
            mMotors[1]->setPosition(currentRArmPos-0.005);
            mMotors[3]->setPosition(currentRArmPos-0.005);
            mMotors[5]->setPosition(currentRArmPos-0.005);
            currentRArmPos = currentRArmPos-0.005;
            myStep();
          }
          break;
        case 'H':
          //if(currentLArmPos > (minMotorPositions[2]+.1)){
          //mMotors[13]->setPosition(minMotorPositions[13]);
        //mMotors[1]->setPosition(1);
        //mMotors[3]->setPosition(-1);
        //mMotors[5]->setPosition(maxMotorPositions[5]);
        //currentLArmPos = currentLArmPos-0.005;
        //myStep();
        //}
          headmove(0,0);
          wait(3000);
          headmove(100,100);
          break;
          
        case 'J':
          if(currentLArmPos > (minMotorPositions[2]-.1)){
        mMotors[0]->setPosition(currentLArmPos+0.005);
        mMotors[2]->setPosition(currentLArmPos+0.005);
        mMotors[4]->setPosition(currentLArmPos+0.005);
        currentLArmPos = currentLArmPos+0.005;
        myStep();
        }
        break;
        
        case 'T':
        mSpeaker->speak("Hi, I am Darwin, an anthropomorphic robot.  At Northwestern University, I have been programmed to work with virtual reality and the HTC Vive.  Now you can control a robot surrogate in another location that will mimic your every movement, and allow you to see through my eyes.", 2.0);
        break;

        case 'E':
        return;
        break;
      }
    }

    mGaitManager->step(mTimeStep);

    // step
    myStep();
  }
}

void Walk::checkIfFallen() {
  static int fup = 0;
  static int fdown = 0;
  static const double acc_tolerance = 80.0;
  static const double acc_step = 100;

  // count how many steps the accelerometer
  // says that the robot is down
  const double *acc = mAccelerometer->getValues();
  if (acc[1] < 512.0 - acc_tolerance)
    fup++;
  else
    fup = 0;

  if (acc[1] > 512.0 + acc_tolerance)
    fdown++;
  else
    fdown = 0;

  // the robot face is down
  if (fup > acc_step) {
    mMotionManager->playPage(10); // f_up
    mMotionManager->playPage(9); // init position
    fup = 0;
  }
  // the back face is down
  else if (fdown > acc_step) {
    mMotionManager->playPage(11); // b_up
    mMotionManager->playPage(9); // init position
    fdown = 0;
  }
}





//Function Wyatt Asked For 
//Walk indefinitely

void Walk::move(double speed, double angle){
 // First step to update sensors values
  myStep();
  // play the hello motion
 // mSpeaker->speak("Hello Wyatt",1.0);
  
  mMotionManager->playPage(1);
  mMotionManager->playPage(9); // init position
  wait(5000);
  

  // main loop
    mGaitManager->setXAmplitude(0.0);
    mGaitManager->setAAmplitude(0.0);

    mGaitManager->start();
    mGaitManager->step(mTimeStep);
    wait(200);

  while (true) {
    checkIfFallen();
    if(angle == 0){   
      mGaitManager->setXAmplitude(speed/100);
    }
    else if(angle > 0){
      mGaitManager->setAAmplitude(-speed/100);
    }
    else if(angle < 0){
      mGaitManager->setAAmplitude(speed/100);
    }
    mGaitManager->step(mTimeStep);
    // step
    myStep();
  }
  
}  



//Stop walking 
void Walk::stopMov(){

 mGaitManager->stop(); 
}

//Head and Neck Movement
//100x100 grid; (100,100) is top left; (0,0) is bottom right
void Walk::headmove(double xcoord,double ycoord){
  double x = ((xcoord/100)*1.29)-.36;
  double y = ((ycoord/100)*3.62)-1.81;
  
  mMotors[18]->setPosition(y);
  mMotors[19]->setPosition(x);
  myStep();
}

void Walk::timedWalk(double speed, double angle){
 double s;
 // First step to update sensors values
  myStep();
  // play the hello motion
  
  mMotionManager->playPage(1);
  mMotionManager->playPage(9); // init position
  wait(2000);
  

  // main loop
    mGaitManager->setXAmplitude(0.0);
    mGaitManager->setAAmplitude(0.0);

    mGaitManager->start();
    mGaitManager->step(mTimeStep);
    wait(200);

  s = getTime() + 5;
  while (getTime() < s) {
    checkIfFallen();
    if(angle == 0){   
      mGaitManager->setXAmplitude(speed/100);
    }
    else if(angle > 0){
      mGaitManager->setAAmplitude(-speed/100);
    }
    else if(angle < 0){
      mGaitManager->setAAmplitude(speed/100);
    }
    mGaitManager->step(mTimeStep);
    // step
    myStep();
  }
  mGaitManager->setXAmplitude(0);
  mGaitManager->setAAmplitude(0);
  mGaitManager->stop();
  wait(1000);
  return;
}  

void Walk::moveShoulders(){
  

}
