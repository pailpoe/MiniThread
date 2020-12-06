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
  //**********************************************
  //No mode
  if(_eActualMode == NoMode)
  { 
    if(eState == State_No_Rotation)
    {
      //The motor is already stopped
      _n = 0;   
    }
    else
    {
      if( _n > 0 )
      {
        //The motor accelerate, need to start deccelerate
        if( StepToStop == 0 ) StepToStop = 1; // Correction BUG low speed
        _n = -StepToStop;
      }else if( _n == 0 )
      {
        //Motor is stopped
        eState = State_No_Rotation;
      }
    }
  }
  //**********************************************
  //SpeedModeUp
  if(_eActualMode == SpeedModeUp)
  {
    if(eState == State_Rotation_Negative && _n != 0)
    {
      //Wrong way !
      if( _n > 0 ) _n = -StepToStop; //Start deccelerate if accelerate
    }
    else
    {
      eState = State_Rotation_Positive;
      if ( _n < 0 ) _n = -_n; //Decelerate, we need to accelerate      
    }
  }
  //**********************************************
  //SpeedModeDown
  if(_eActualMode == SpeedModeDown)
  {
    if(eState == State_Rotation_Positive && _n != 0)
    {
      //Wrong way !
      if( _n > 0 ) _n = -StepToStop; //Start deccelerate if accelerate
    }
    else
    {
      eState = State_Rotation_Negative;
      if ( _n < 0 ) _n = -_n; //Decelerate, we need to accelerate        
    }
  }
  //**********************************************
  //PositionMode
  if(_eActualMode == PositionMode)
  {
    _ErrorPos = _AbsoluteCounter - _TargetPosition; 
   
    if( _ErrorPos < 0 )
    {
      if ( _n < 0 ) _n = -_n; //Decelerate, we need to accelerate 
      eState = State_Rotation_Positive;
      /*
      if(eState == State_Rotation_Negative)
      {
        //Wrong way !
        if( _n > 0 ) _n = -StepToStop; //Start deccelerate if accelerate
        if( _n == 0 )eState = State_Rotation_Positive;
      }
      else
      {
        eState = State_Rotation_Positive;
        if ( _n < 0 ) _n = -_n; //Decelerate, we need to accelerate      
      }
      */      
    }
   if( _ErrorPos > 0 )  
    {
      if ( _n < 0 ) _n = -_n; //Decelerate, we need to accelerate  
      eState = State_Rotation_Negative;
      /*
      if(eState == State_Rotation_Positive)
      {
        //Wrong way !
        if( _n > 0 ) _n = -StepToStop; //Start deccelerate if accelerate
        if( _n == 0 )eState = State_Rotation_Negative;
      }
      else
      {
        eState = State_Rotation_Negative;
        if ( _n < 0 ) _n = -_n; //Decelerate, we need to accelerate      
      } 
      */
    }
   if( _ErrorPos == 0 )  
    {
        eState = State_No_Rotation; 
        if ( _n > 0 ) _n = -StepToStop; 
        /*
        if ( _n == 1 || _n == 0  )
        {
          _n = 0;
          eState = State_No_Rotation;  
        }           
        if ( _n > 0 ) _n = -StepToStop;     
        */
    }
   
  }
  //**********************************************
  //End limit
  if(_UseEndLimit )
  {
    //Use the end limit
    if(eState == State_Rotation_Positive)
    {
      //Go to the max limit
      if(_AbsoluteCounter < _StopPositionMax)
      {
        DistanceToGo = _StopPositionMax - _AbsoluteCounter; 
        if( StepToStop >= DistanceToGo && _n > 0 )
        {
          //Start deccelerate because we approch from End limit max and we accelerate
          _n = -StepToStop;
        }
      }
      else
      {
        //At the end limit 
        eState = State_No_Rotation; 
        _n = 0; 
      }      
    }
    if(eState == State_Rotation_Negative)
    {
      //Go to the min limit
      if(_AbsoluteCounter > _StopPositionMin)
      {
      DistanceToGo = _AbsoluteCounter - _StopPositionMin;
      if( StepToStop >= DistanceToGo && _n > 0)
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
    }  
  }
  //**********************************************
  //Direction output
  if(eState == State_Rotation_Positive)
  {
    if(_Sens == false) digitalWrite(_PinDIR, LOW); 
    else digitalWrite(_PinDIR, HIGH);     
  }
  if(eState == State_Rotation_Negative)
  {
    if(_Sens == false) digitalWrite(_PinDIR, HIGH);
    else  digitalWrite(_PinDIR, LOW);     
  }
  //**********************************************
  //Calcul Step, Speed,...
  if(_n == 0)
  {
    // First step from stopped
    _cn = _c0 <= _cmin ? _cmin : _c0;
  }
  else
  { 
    if( _cn <= _cmin && _n > 0 )_n = StepToStop > 0 ? -StepToStop : -1; //Pour passage vitesse Haute à basse et avoir une rampe de decceleration
    _cn = _cn - (_cn*2)/(_n*4+1);
    if( _cn <= _cmin && _n > 0  ) _cn = _cmin; //Seulement si le moteur accelere pour éviter la d'ecceleration brutale
  }
  _n++;  
  if( (unsigned int)_cn >= 65535 ) _StepInterval = 65535;
  else  _StepInterval = (unsigned int)_cn;
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
float StepperMotor::GetStopPositionMin()
{
  return (float)_StopPositionMin/(float)_Resolution;  
}
float StepperMotor::GetStopPositionMax()
{
  return (float)_StopPositionMax/(float)_Resolution;  
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



