#include <Arduino.h>
#include "StepperMotor.h"

StepperMotor::StepperMotor(unsigned int  Resolution , boolean Sens, char STEP_Pin, char DIR_Pin, char EN_Pin)
{
  _Resolution = Resolution;
  _MaxSpeed = 2;
  _LoopSpeedCount = 0;
  _AbsoluteCounter = 0;
  _TargetPosition = 0;
  _StopPositionMax = 0;
  _StopPositionMin = 0; 
  _Sens = Sens;
  eState = State_No_Rotation;
  _PinDIR = DIR_Pin; 
  _PinSTEP = STEP_Pin; 
  _PinEN = EN_Pin; 
  pinMode(_PinDIR, OUTPUT);
  pinMode(_PinSTEP, OUTPUT);
  pinMode(_PinEN, OUTPUT); 
}
void StepperMotor::TimeToPrepareToMove ()
{
    if(_AbsoluteCounter < _TargetPosition)
    {
      if(_AbsoluteCounter < _StopPositionMax )
      {
        eState = State_Rotation_Positive; 
        PrepareToTurnPos();
      }
      else
      {
        eState = State_No_Rotation;
        _LoopSpeedCount = _MaxSpeed;
        PrepareToTurnPos();         
      }    
    }
    else if (_AbsoluteCounter > _TargetPosition)
    {
      //Negatif
      if(_AbsoluteCounter > _StopPositionMin )
      {
        eState = State_Rotation_Negative;
        PrepareToTurnNeg(); 
      }
      else
      {
        eState = State_No_Rotation;
        _LoopSpeedCount = _MaxSpeed;
        PrepareToTurnPos();         
      }   
    }
    else
    {
      //At postion
      eState = State_No_Rotation;
      _LoopSpeedCount = _MaxSpeed;
      PrepareToTurnPos();     
    }   
}
void    StepperMotor::MotorChangePowerState ( boolean State)
{
  if(State == false)
  {
    digitalWrite(_PinEN, HIGH); 
  }else
  {
    digitalWrite(_PinEN, LOW);  
  }
} 
boolean StepperMotor::AreYouAtMaxPos()
{
  boolean result=false;
  if(_AbsoluteCounter==_StopPositionMax)result = true;
  return result;  
}
boolean StepperMotor::AreYouAtMinPos()
{
  boolean result=false;
  if(_AbsoluteCounter==_StopPositionMin)result = true;
  return result;  
}
void StepperMotor::TimeToMove () 
{
  if(_LoopSpeedCount == 0)
  {
    //Ok for a pulse 
    _LoopSpeedCount = (_MaxSpeed - 1);   
    if( eState == State_Rotation_Positive)
    {
      TurnPos(); 
    }
    else if ( eState == State_Rotation_Negative)
    {
      TurnNeg();
    }    
  }
  else
  {
    _LoopSpeedCount-- ;    
  }    
}
void StepperMotor::PrepareToTurnPos()
{
    digitalWrite(_PinSTEP, HIGH);
    if(_Sens == false) digitalWrite(_PinDIR, LOW); 
    else digitalWrite(_PinDIR, HIGH);  
}
void StepperMotor::TurnPos()
{
    _AbsoluteCounter++;  
    digitalWrite(_PinSTEP, LOW);
    if(_Sens == false) digitalWrite(_PinDIR, LOW); 
    else digitalWrite(_PinDIR, HIGH); 
}
void StepperMotor::PrepareToTurnNeg()
{
    digitalWrite(_PinSTEP, HIGH);   
    if(_Sens == false) digitalWrite(_PinDIR, HIGH);
    else  digitalWrite(_PinDIR, LOW);     
}
void StepperMotor::TurnNeg()
{
    _AbsoluteCounter--; 
    digitalWrite(_PinSTEP, LOW);   
    if(_Sens == false) digitalWrite(_PinDIR, HIGH);
    else  digitalWrite(_PinDIR, LOW);  
}
void StepperMotor::ChangeTargetPositionStep (long Target_Position)
{
  _TargetPosition = Target_Position;   
}
void StepperMotor::ChangeTargetPositionReal (float Target_Position)
{
  _TargetPosition = (long) (Target_Position * _Resolution);   
}
void StepperMotor::ChangeMaxSpeed (unsigned int MaxSpeed)
{
  _MaxSpeed = MaxSpeed;  
}
unsigned int StepperMotor::GetMaxSpeed ()
{
  return _MaxSpeed;   
}
void StepperMotor::ChangeStopPositionMaxStep (long Stop_Position)
{
  _StopPositionMax = Stop_Position;   
}
void StepperMotor::ChangeStopPositionMinStep (long Stop_Position)
{
  _StopPositionMin = Stop_Position;   
}
void StepperMotor::ChangeStopPositionMaxReal (float Stop_Position)
{
  _StopPositionMax = (long) (Stop_Position*_Resolution);   
}
void StepperMotor::ChangeStopPositionMinReal (float Stop_Position)
{
  _StopPositionMin = (long) (Stop_Position*_Resolution);   
}


long StepperMotor::GetPositionStep()
{
  return _AbsoluteCounter;   
  
}
float StepperMotor::GetPositionReal()
{
  return (float)_AbsoluteCounter/(float)_Resolution;    
}
void StepperMotor::ChangeParameter(unsigned int  Resolution , boolean Sens)
{
  _Resolution = Resolution;
  _Sens = Sens;  
}
