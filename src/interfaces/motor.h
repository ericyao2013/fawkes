
/***************************************************************************
 *  motor.h - Fawkes BlackBoard Interface - MotorInterface
 *
 *  Templated created:   Thu Oct 12 10:49:19 2006
 *  Copyright  2007  Martin Liebenberg, Tim Niemueller
 *
 *  $Id$
 *
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  along with this program; if not, write to the Free Software Foundation,
 *  Inc., 51 Franklin Street, Fifth floor, Boston, MA 02111-1307, USA.
 */

#ifndef __INTERFACES_MOTOR_H_
#define __INTERFACES_MOTOR_H_

#include <interface/interface.h>
#include <interface/message.h>

class MotorInterface : public Interface
{
 /// @cond INTERNALS
 INTERFACE_MGMT_FRIENDS(MotorInterface)
 /// @endcond
 public:
  /* constants */
  static const unsigned int MOTOR_ENABLED;
  static const unsigned int MOTOR_DISABLED;

 private:
  /** Internal data storage, do NOT modify! */
  typedef struct {
    int RightRPM; /**< 
      RPM of the motor on the right front of the robot.
     */
    int RearRPM; /**< 
      RPM of motor on the rear of the robot.
     */
    int LeftRPM; /**< 
      RPM of the motor on the left front of the robot.
     */
    unsigned long int ControllerThreadID; /**< 
     The ID of the controlling thread.
     Only from this thread command messages are accepted.
     */
    unsigned int MotorState : 1; /**< 
      The current state of the motor.
     */
    char ControllerThreadName[64]; /**< 
     The name of the controlling thread.
   */
  } MotorInterface_data_t;

  MotorInterface_data_t *data;

 public:
  /* messages */
  class SetMotorStateMessage : public Message
  {
   private:
    /** Internal data storage, do NOT modify! */
    typedef struct {
      unsigned int MotorState; /**< 
      The new motor state to set. Use the MOTOR_* constants.
     */
    } SetMotorStateMessage_data_t;

    SetMotorStateMessage_data_t *data;

   public:
    SetMotorStateMessage(unsigned int iniMotorState);
    SetMotorStateMessage();
    ~SetMotorStateMessage();

    /* Methods */
    unsigned int getMotorState();
    void setMotorState(const unsigned int newMotorState);
  };

  class AquireControlMessage : public Message
  {
   private:
    /** Internal data storage, do NOT modify! */
    typedef struct {
      unsigned long int ThreadID; /**< 
      The thread ID of the thread which is allowed to control the motors.
      Set to zero to use the data of the current thread (the message is zeroed at
      creation automatically, so if you do not set anything the sending thread
      aquires the control.
     */
      char ThreadName[64]; /**< 
      The thread name of the aquiring thread.
     */
    } AquireControlMessage_data_t;

    AquireControlMessage_data_t *data;

   public:
    AquireControlMessage(unsigned long int iniThreadID, char * iniThreadName);
    AquireControlMessage();
    ~AquireControlMessage();

    /* Methods */
    unsigned long int getThreadID();
    void setThreadID(const unsigned long int newThreadID);
    char * getThreadName();
    void setThreadName(const char * newThreadName);
  };

  class TransRotRPMMessage : public Message
  {
   private:
    /** Internal data storage, do NOT modify! */
    typedef struct {
      float Forward; /**< The forward command. */
      float Sideward; /**< The sideward command. */
      float Rotation; /**< The rotation command. */
      float Speed; /**< The speed command. */
    } TransRotRPMMessage_data_t;

    TransRotRPMMessage_data_t *data;

   public:
    TransRotRPMMessage(float iniForward, float iniSideward, float iniRotation, float iniSpeed);
    TransRotRPMMessage();
    ~TransRotRPMMessage();

    /* Methods */
    float getForward();
    void setForward(const float newForward);
    float getSideward();
    void setSideward(const float newSideward);
    float getRotation();
    void setRotation(const float newRotation);
    float getSpeed();
    void setSpeed(const float newSpeed);
  };

  class DriveRPMMessage : public Message
  {
   private:
    /** Internal data storage, do NOT modify! */
    typedef struct {
      float FrontRight; /**< Rotation in RPM of the right front wheel. */
      float FrontLeft; /**< Rotation in RPM of the left front wheel. */
      float Rear; /**< Rotation in RPM of the rear wheel. */
    } DriveRPMMessage_data_t;

    DriveRPMMessage_data_t *data;

   public:
    DriveRPMMessage(float iniFrontRight, float iniFrontLeft, float iniRear);
    DriveRPMMessage();
    ~DriveRPMMessage();

    /* Methods */
    float getFrontRight();
    void setFrontRight(const float newFrontRight);
    float getFrontLeft();
    void setFrontLeft(const float newFrontLeft);
    float getRear();
    void setRear(const float newRear);
  };

  class TransMessage : public Message
  {
   private:
    /** Internal data storage, do NOT modify! */
    typedef struct {
      float VX; /**< Speed in X direction in m/s. */
      float VY; /**< Speed in Y direction in m/s. */
    } TransMessage_data_t;

    TransMessage_data_t *data;

   public:
    TransMessage(float iniVX, float iniVY);
    TransMessage();
    ~TransMessage();

    /* Methods */
    float getVX();
    void setVX(const float newVX);
    float getVY();
    void setVY(const float newVY);
  };

  class RotMessage : public Message
  {
   private:
    /** Internal data storage, do NOT modify! */
    typedef struct {
      float Omega; /**< Angle rotation in rad/s. */
    } RotMessage_data_t;

    RotMessage_data_t *data;

   public:
    RotMessage(float iniOmega);
    RotMessage();
    ~RotMessage();

    /* Methods */
    float getOmega();
    void setOmega(const float newOmega);
  };

  class TransRotMessage : public Message
  {
   private:
    /** Internal data storage, do NOT modify! */
    typedef struct {
      float VX; /**< Speed in X direction in m/s. */
      float VY; /**< Speed in Y direction in m/s. */
      float Omega; /**< Angle rotation in rad/s. */
    } TransRotMessage_data_t;

    TransRotMessage_data_t *data;

   public:
    TransRotMessage(float iniVX, float iniVY, float iniOmega);
    TransRotMessage();
    ~TransRotMessage();

    /* Methods */
    float getVX();
    void setVX(const float newVX);
    float getVY();
    void setVY(const float newVY);
    float getOmega();
    void setOmega(const float newOmega);
  };

  class OrbitMessage : public Message
  {
   private:
    /** Internal data storage, do NOT modify! */
    typedef struct {
      float PX; /**< Point's X coordinate to orbit. */
      float PY; /**< Point's Y coordinate to orbit. */
      float Omega; /**< Angular speed around point in rad/s. */
    } OrbitMessage_data_t;

    OrbitMessage_data_t *data;

   public:
    OrbitMessage(float iniPX, float iniPY, float iniOmega);
    OrbitMessage();
    ~OrbitMessage();

    /* Methods */
    float getPX();
    void setPX(const float newPX);
    float getPY();
    void setPY(const float newPY);
    float getOmega();
    void setOmega(const float newOmega);
  };

  virtual bool messageValid(const Message *message) const;
 private:
  MotorInterface();
  ~MotorInterface();

 public:
  /* Methods */
  unsigned int getMotorState();
  void setMotorState(const unsigned int newMotorState);
  int getRightRPM();
  void setRightRPM(const int newRightRPM);
  int getRearRPM();
  void setRearRPM(const int newRearRPM);
  int getLeftRPM();
  void setLeftRPM(const int newLeftRPM);
  unsigned long int getControllerThreadID();
  void setControllerThreadID(const unsigned long int newControllerThreadID);
  char * getControllerThreadName();
  void setControllerThreadName(const char * newControllerThreadName);

};

#endif
