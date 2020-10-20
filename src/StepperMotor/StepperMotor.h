#ifndef StepperMotor_h
#define StepperMotor_h   
#include <Arduino.h>
/**
 *  \file StepperMotor.h
 *  \brief Stepper motor class
 *  \author G.pailleret
 *  \version 1.0
 *  \date 22/10/2019
 */
 
class StepperMotor
{
public:
	StepperMotor(unsigned int  Resolution , boolean Sens, char STEP_Pin, char DIR_Pin, char EN_Pin);
	//Change the motor parameter
  void ChangeParameter(unsigned int  Resolution , boolean Sens);
  
	void 		TimeToPrepareToMove (); //Be call every 100µs 
	void 		TimeToMove (); 					//Be call every 10µs after TimeToPrepareToMove() 

	enum teMotorMode 
	{   
      NoMode,
      SpeedModeUp,
      SpeedModeDown,
      PositionMode
  };

  //Change the actual mode
  void    ChangeTheMode(teMotorMode eMode);
  //Return the actual mode
  teMotorMode ReturnTheMode();
  
   //Change the On/off state  ( false = motor disabled no power)
  void    MotorChangePowerState ( boolean State);  

  //Change the target position in "PositionMode"
	void 		ChangeTargetPositionStep (long Target_Position);
  void    ChangeTargetPositionReal (float Target_Position);

  //Change the Current motor position
	void 		ChangeCurrentPositionStep (long Position);
  void    ChangeCurrentPositionReal (float Position);

  
  
  //Electronic end limit min and max 
  void    UseEndLimit ( boolean State );
	void 		ChangeStopPositionMaxStep (long Stop_Position);
  void    ChangeStopPositionMaxReal (float Stop_Position);  
	void 		ChangeStopPositionMinStep (long Stop_Position);  
  void    ChangeStopPositionMinReal (float Stop_Position);
	boolean AreYouAtMaxPos();
	boolean AreYouAtMinPos(); 
  long    GetStopPositionMinStep();
  long    GetStopPositionMaxStep();

  //Change the max speed
	//MaxSpeed from 1 to x ( 1 = max speed)
	void 		     ChangeMaxSpeed (unsigned int MaxSpeed);
  unsigned int GetMaxSpeed ();

  //Get the absolute position of the motor
	long 		 GetPositionStep();
  float    GetPositionReal(); // mm,turn
  
  
private:
	enum eMS_Motor 
	{   
      State_Rotation_Positive,
      State_Rotation_Negative, 
      State_No_Rotation 
  };

  boolean   _UseEndLimit;
  teMotorMode _eActualMode;      //Motor mode
	long 			_AbsoluteCounter; 	//Absolute position of the stepper motor
	long 			_TargetPosition; 		//Target position
	long 			_StopPositionMax; 	//Security Max position
	long 			_StopPositionMin;		//Security Min position
	boolean 	_Sens; 							//Sens de rotation
  char      _PinDIR;             //IO for Dir
  char      _PinSTEP;            //IO for Step
  char      _PinEN;              //IO for Enable
	eMS_Motor eState; 						//Motor machine state
	unsigned int _MaxSpeed; 			//Max speed ( 1= max 10Khz) 
	unsigned int _LoopSpeedCount; //Counter for Max speed 
	unsigned int _Resolution; 		//Resolution/tr or mm
};
    
#endif
