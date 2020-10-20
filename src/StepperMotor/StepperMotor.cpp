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
  _eActualMode = NoMode;
  _UseEndLimit = true;
}
void StepperMotor::TimeToPrepareToMove ()
{
    //Release step pin
    digitalWrite(_PinSTEP, HIGH);
    
    if((_AbsoluteCounter < _TargetPosition && _eActualMode == PositionMode) || _eActualMode == SpeedModeUp )
    {
      if(_AbsoluteCounter < _StopPositionMax || !_UseEndLimit )
      {
        eState = State_Rotation_Positive; 
      }
      else
      {
        eState = State_No_Rotation;
        _LoopSpeedCount = _MaxSpeed;            
      }
      if(_Sens == false) digitalWrite(_PinDIR, LOW); 
      else digitalWrite(_PinDIR, HIGH); 
    }
    else if ((_AbsoluteCounter > _TargetPosition && _eActualMode == PositionMode) || _eActualMode == SpeedModeDown)
    {
      //Negatif
      if(_AbsoluteCounter > _StopPositionMin || !_UseEndLimit )
      {
        eState = State_Rotation_Negative;
      }
      else
      {
        eState = State_No_Rotation;
        _LoopSpeedCount = _MaxSpeed;          
      }
      if(_Sens == false) digitalWrite(_PinDIR, HIGH);
      else  digitalWrite(_PinDIR, LOW);    
    }
    else
    {
      //At postion
      eState = State_No_Rotation;
      _LoopSpeedCount = _MaxSpeed;     
    }   
}
void StepperMotor::TimeToMove () 
{
  if(_LoopSpeedCount == 0)
  {
    //Ok for a pulse 
    _LoopSpeedCount = (_MaxSpeed - 1);   
    if( eState == State_Rotation_Positive)
    {
      _AbsoluteCounter++;  
      digitalWrite(_PinSTEP, LOW); 
    }
    else if ( eState == State_Rotation_Negative)
    {
      _AbsoluteCounter--; 
      digitalWrite(_PinSTEP, LOW); 
    }    
  }
  else
  {
    _LoopSpeedCount-- ;    
  }    
}


void StepperMotor::ChangeTheMode(teMotorMode eMode)
{
  switch ( eMode )
  {
    case NoMode:
      _eActualMode = NoMode;
    break;
    case SpeedModeUp:
      _eActualMode = eMode; 
    break;
    case SpeedModeDown:
      _eActualMode = eMode;     
    break;
    case PositionMode:
      _TargetPosition = _AbsoluteCounter;
      _eActualMode = eMode; 
    break;
    default:
      _eActualMode = NoMode;
    break;
  }
}
StepperMotor::teMotorMode StepperMotor::ReturnTheMode()
{
  return _eActualMode; 
}
void    StepperMotor::UseEndLimit ( boolean State )
{
    _UseEndLimit = State;
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


void StepperMotor::ChangeTargetPositionStep (long Target_Position)
{
  _TargetPosition = Target_Position;   
}
void StepperMotor::ChangeTargetPositionReal (float Target_Position)
{
  _TargetPosition = (long) (Target_Position * _Resolution);   
}

void 		StepperMotor::ChangeCurrentPositionStep (long Position)
{
  _AbsoluteCounter = Position; 
  
}
void    StepperMotor::ChangeCurrentPositionReal (float Position)
{
  _AbsoluteCounter = (long) (Position * _Resolution);    
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
long    StepperMotor::GetStopPositionMinStep()
{
    return _StopPositionMin; 
}
long    StepperMotor::GetStopPositionMaxStep()
{
    return _StopPositionMax; 
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
