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
	StepperMotor(unsigned int  Resolution , boolean Sens, char STEP_Pin, char DIR_Pin, char EN_Pin,voidFuncPtr ChangeStepInterval);
	//Change the motor parameter
  void ChangeParameter(unsigned int  Resolution , boolean Sens);
  
	void 		TimeToPrepareToMove (); //Be call every 100µs 
	void 		TimeToMove (); 					//Be call every 10µs after TimeToPrepareToMove() 
  unsigned int NewInterval ();    //New interval in µs
  

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

  
  
  //Electronic end limit min and max functions 
  void    UseEndLimit ( boolean State );
	void 		ChangeStopPositionMaxStep (long Stop_Position);
  void    ChangeStopPositionMaxReal (float Stop_Position);  
	void 		ChangeStopPositionMinStep (long Stop_Position);  
  void    ChangeStopPositionMinReal (float Stop_Position);
	boolean AreYouAtMaxPos();
	boolean AreYouAtMinPos(); 
  long    GetStopPositionMinStep();
  long    GetStopPositionMaxStep();

  //Change the max speed in step/s
	void 		     ChangeMaxSpeed (float MaxSpeed);
  //Get the max speed in step/s
  float GetMaxSpeed ();
  
  //Change the Acceleration in step/s²
	void 		     ChangeAcceleration (float Acceleration);
  //Get the max speed in step/s
  float GetAcceleration ();  
  
  

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
  voidFuncPtr _ChangeStepInterval; 
  boolean   _UseEndLimit;       //If true, use end limit
  teMotorMode _eActualMode;     //Motor mode
	long 			_AbsoluteCounter; 	//Absolute position in step
	long 			_TargetPosition; 		//Target position in step
  long      _ErrorPos;          //Error absolute - target
	long 			_StopPositionMax; 	//End limit Max position in step
	long 			_StopPositionMin;		//End limit Min position in step
	boolean 	_Sens; 							//Sens de rotation
  char      _PinDIR;            //IO for Dir
  char      _PinSTEP;           //IO for Step
  char      _PinEN;             //IO for Enable
	eMS_Motor eState; 						//Motor machine state
	unsigned int _Resolution; 		//Resolution/tr or mm 
  
  float _Speed;                 //Actual speed
 	float _MaxSpeed;    			    //Max speed in step/s 
  float _Acceleration;          //Acceleration in step/s²
  unsigned int _StepInterval;   //Step interval in µs
  long _n;                      //The step counter for speed calculations
  float _c0;                    //Intial step size in µs
  float _cn;                    //Last step size in µs
  float _cmin;                  //Min step size in microseconds based on maxSpeed
};
    
#endif
