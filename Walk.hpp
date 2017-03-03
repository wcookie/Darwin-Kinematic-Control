// Description:   Example showing how to use the gait manager
//                and keyboard inputs

#ifndef WALK_HPP
#define WALK_HPP

#define NMOTORS 20

#include <webots/Robot.hpp>

namespace managers {
  class RobotisOp2MotionManager;
  class RobotisOp2GaitManager;
    class RobotisOp2VisionManager;
}

namespace webots {
  class Motor;
  class PositionSensor;
  class LED;
  class Camera;
  class Accelerometer;
  class Gyro;
  class Keyboard;
  class Speaker;
};

class Walk : public webots::Robot {
  public:
                                     Walk();
    virtual                         ~Walk();
    void                             run();
    void                             checkIfFallen();
    void                             move(double speed, double angle);
    void                             timedWalk(double speed, double angle);
    void                             stopMov();
    void                             headmove(double xcoord, double ycoord);
    void                             moveShoulders();

  private:
    int                              mTimeStep;

    void                             myStep();
    void                             wait(int ms);

    webots::Motor                   *mMotors[NMOTORS];
    webots::PositionSensor *mPositionSensors[NMOTORS];
    webots::Accelerometer           *mAccelerometer;
    webots::Keyboard                *mKeyboard;
        webots::LED *mEyeLED;
      webots::Speaker *mSpeaker;
      webots::Camera                   *mCamera;
 
    managers::RobotisOp2MotionManager *mMotionManager;
    managers::RobotisOp2GaitManager   *mGaitManager;
    managers::RobotisOp2VisionManager  *mVisionManager;
};

#endif
