#ifndef StepperMotor_h
#define StepperMotor_h   
#include <Arduino.h>
/**
 *  \file StepperMotor.h
 *  \brief Classe de gestion du moteur pas à pas
 *  \author G.pailleret
 *  \version 1.0
 *  \date 22/10/2019
 */
 
class StepperMotor
{
public:
	StepperMotor(unsigned int  Resolution , boolean Sens, char STEP_Pin, char DIR_Pin, char EN_Pin);
	
	void 		TimeToPrepareToMove (); //Be call every 100µs 
	void 		TimeToMove (); 					//Be call every 10µs after TimeToPrepareToMove() 
	
	/**
	 *  \brief Change the target position
	 *  \param [in] Target_Position Target position of the motor
	 */
	void 		ChangeTargetPositionStep (long Target_Position);
	void 		ChangeStopPositionMaxStep (long Stop_Position);
	void 		ChangeStopPositionMinStep (long Stop_Position);
  void    ChangeTargetPositionReal (float Target_Position);
  void    ChangeStopPositionMaxReal (float Stop_Position);
  void    ChangeStopPositionMinReal (float Stop_Position);

  void    MotorChangePowerState ( boolean State); 
 
	boolean AreYouAtMaxPos();
	boolean AreYouAtMinPos();
  
  void ChangeParameter(unsigned int  Resolution , boolean Sens);
	/**
	 *  \fn void ChangeMaxSpeed (unsigned int MaxSpeed); 
	 *  \brief Change the max speed
	 *  \param [in] MaxSpeed from 1 to x ( 1 = max speed)
	 */
	void 		ChangeMaxSpeed (unsigned int MaxSpeed);
  unsigned int GetMaxSpeed ();
	/**
	 *  \fn long GetPosition();
	 *  \brief Return the absolute position of the motor
	 *  \return absolute position of the motor
	 */  
	long 		GetPositionStep();
  float    GetPositionReal(); // mm,turn
private:
	enum eMS_Motor 
	{   
      State_Rotation_Positive,
      State_Rotation_Negative, 
      State_No_Rotation 
  };
	void 			PrepareToTurnPos();
	void 			TurnPos();
	void 			PrepareToTurnNeg(); 
	void 			TurnNeg(); 
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
	unsigned int _Resolution; 		//Resolution par tour
};
    
#endif
