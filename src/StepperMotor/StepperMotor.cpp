#include <Arduino.h>
#include "StepperMotor.h"

StepperMotor::StepperMotor(unsigned int  Resolution , boolean Sens, char STEP_Pin, char DIR_Pin, char EN_Pin,voidFuncPtr ChangeStepInterval)
{
  _StepInterval = 500;
  _Resolution = Resolution;
  _AbsoluteCounter = 0;
  _TargetPosition = 0;
  _StopPositionMax = 0;
  _StopPositionMin = 0; 
  _Sens = Sens;
  _PinDIR = DIR_Pin; 
  _PinSTEP = STEP_Pin; 
  _PinEN = EN_Pin; 
  pinMode(_PinDIR, OUTPUT);
  pinMode(_PinSTEP, OUTPUT);
  pinMode(_PinEN, OUTPUT); 
  eState = State_No_Rotation;  
  _eActualMode = NoMode;
  _UseEndLimit = true;
  _ChangeStepInterval = ChangeStepInterval;
  _n = 0;
  _Speed = 0.0;
  ChangeAcceleration(20000.0);
  ChangeMaxSpeed(10000.0);
  
  
}
void StepperMotor::TimeToPrepareToMove ()
{
    //Release step pin
    digitalWrite(_PinSTEP, HIGH);
    
    long DistanceToGo = 0;
    long StepToStop = (long)((_Speed * _Speed) / (2.0 * _Acceleration)); // Equation 16
    
  //No mode ************************************************ 
  if(_eActualMode == NoMode)
  { 
    if(eState == State_No_Rotation)
    {
      //The motor is already stopped
      _n = 0;   
    }
    else
    {
      //Afer SpeedModeUp -> NoMode or  SpeedModeDown -> NoMode 
      if( _n > 0 )
      {
        //The motor accelerate, need to start deccelerate
        _n = -StepToStop;
      }else if( _n == 0 )
      {
        //Motor is stopped
        eState = State_No_Rotation;
      }
    }
  }
  //Speed mode Up +++ ***************************************
  if(_eActualMode == SpeedModeUp)
  {
    if(_AbsoluteCounter < _StopPositionMax || !_UseEndLimit )
    {
      eState = State_Rotation_Positive;
      DistanceToGo = _StopPositionMax - _AbsoluteCounter; 
      if( StepToStop >= DistanceToGo && _n > 0 && _UseEndLimit  )
      {
        //Start deccelerate because we approch from End limit min
        _n = -StepToStop;
      }
      if ( StepToStop < DistanceToGo && _n < 0 )
      {
        //Need to acelerate  
        _n = -_n;
      }  
    }
    else
    {
      //At the end limit 
      eState = State_No_Rotation; 
      _n = 0; 
    }
    if(_Sens == false) digitalWrite(_PinDIR, LOW); 
    else digitalWrite(_PinDIR, HIGH); 
  }
  //Speed mode Down --- ***********************************
  if(_eActualMode == SpeedModeDown)
  {
    if(_AbsoluteCounter > _StopPositionMin || !_UseEndLimit )
    {
      eState = State_Rotation_Negative;
      DistanceToGo = _AbsoluteCounter - _StopPositionMin;
      if( StepToStop >= DistanceToGo && _n > 0 && _UseEndLimit )
      {
        //Start deccelerate because we approch from End limit min
        _n = -StepToStop;
      }
      if ( StepToStop < DistanceToGo && _n < 0 )
      {
        //Need to acelerate  
        _n = -_n;
      }  
    }
    else
    {
      //At the end limit
      eState = State_No_Rotation; 
      _n = 0; 
    }
    if(_Sens == false) digitalWrite(_PinDIR, HIGH);
    else  digitalWrite(_PinDIR, LOW);   
  }
  //Position mode ***********************************************************
  if((_AbsoluteCounter <= _TargetPosition && _eActualMode == PositionMode) )
  {
    //We need to accelerate
    if(_AbsoluteCounter < _StopPositionMax || !_UseEndLimit )
    {

      eState = State_Rotation_Positive;
      DistanceToGo = _StopPositionMax - _AbsoluteCounter;
      if(_n < 0)
      {
        //Need to accelerte
        _n = -_n;  
      }
      
      if( StepToStop >= DistanceToGo && _n > 0 )
      {
          //Start deccelerate because we approch from End limit min
          _n = -StepToStop;
      }   
    }
    else
    {
      //At the end limit
      eState = State_No_Rotation;
      _n = 0;     
    }
    if(_Sens == false) digitalWrite(_PinDIR, LOW); 
    else digitalWrite(_PinDIR, HIGH); 
  }
  else if ((_AbsoluteCounter > _TargetPosition && _eActualMode == PositionMode) )
  {
    //We need to decelerate
    if(_n > 0)
    {
      //Need to decellerate
      _n = -StepToStop; 
    }
 
  }
  if (_AbsoluteCounter >= _TargetPosition && _eActualMode == PositionMode && _n == 0)
  {
    //At position with speed min, no movment
    eState = State_No_Rotation;
        
  }
 

  //**********************************************
  if(_n == 0)
  {
    // First step from stopped
    _cn = _c0;
  }
  else
  { 

    _cn = _cn - (_cn*2)/(_n*4+1);
    if( _cn <= _cmin ) _cn = _cmin; 
  }
  _n++;  
  _StepInterval = _cn;
  _Speed = 1000000.0 / _cn;
  _ChangeStepInterval();
}
void StepperMotor::TimeToMove () 
{  
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
unsigned int StepperMotor::NewInterval ()
{
    return _StepInterval;
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
      _n = 0;
      eState = State_No_Rotation;
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



void StepperMotor::ChangeMaxSpeed (float MaxSpeed)
{
  if (MaxSpeed < 0.0)
    MaxSpeed = -MaxSpeed;
  if (_MaxSpeed != MaxSpeed)
  {
    _MaxSpeed = MaxSpeed;
    _cmin = 1000000.0 / MaxSpeed;
    // Recompute _n from current speed and adjust speed if accelerating or cruising
    if (_n > 0)
    {
      _n = (long)((_Speed * _Speed) / (2.0 * _Acceleration)); // Equation 16
    }
  }  
}
float StepperMotor::GetMaxSpeed ()
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
void StepperMotor::ChangeAcceleration (float Acceleration)
{
  if (Acceleration == 0.0)
    return;
  if (Acceleration < 0.0)
    Acceleration = -Acceleration;
  if (_Acceleration != Acceleration)
  {
    // Recompute _n 
    _n = _n * (_Acceleration / Acceleration);
    // New c0
    _c0 = 0.676 * sqrt(2.0 / Acceleration) * 1000000.0; // Equation 15
    _Acceleration = Acceleration;
  }  
}
float StepperMotor::GetAcceleration ()
{
  return _Acceleration;
} 



