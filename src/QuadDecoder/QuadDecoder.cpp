#include <Arduino.h>
#include "QuadDecoder.h"


void QuadDecoder::IT_OverflowHardwareTimer()
{
  if(_HardwareTimer->getDirection())
  {
     _Overflow--;
  } else
  {
    _Overflow++;
  }  
}


QuadDecoder::QuadDecoder(unsigned int  TimerChannel,teTypeEncoder  eTypeEncoder, unsigned int  Resolution , boolean Sens, boolean DiameterMode,voidFuncPtr handler)
{
  _Resolution = Resolution;
  _Overflow = 0;
  _eTypeEncoder = eTypeEncoder;
  UpdatOverflowSize();     
  _AbsoluteCounter = 0;
  _Sens = Sens;
  _AbsoluteCounterZero = 0;
  _RelativeCounterZero = 0;
  _DiameterMode = DiameterMode; 
  _RelativeModeActived = false;

  //Config Hardware timer   
  _HardwareTimer = new HardwareTimer(TimerChannel);
  
  if( TimerChannel == 1 )
  {
    pinMode(PA8, INPUT_PULLUP);  //channel A
    pinMode(PA9, INPUT_PULLUP);  //channel B      
  }
  else if ( TimerChannel == 2 )
  {
    pinMode(PA0, INPUT_PULLUP);  //channel A
    pinMode(PA1, INPUT_PULLUP);  //channel B  
  }
  else if ( TimerChannel == 3 )
  {
    pinMode(PA6, INPUT_PULLUP);  //channel A
    pinMode(PA7, INPUT_PULLUP);  //channel B  
              
  }
  _HardwareTimer->setMode(1, TIMER_ENCODER); //set mode, the channel is not used when in this mode. 
  _HardwareTimer->pause(); //stop...
  _HardwareTimer->setPrescaleFactor(1); //normal for encoder to have the lowest or no prescaler. 
  _HardwareTimer->setOverflow(_Overflow_Size);    
  _HardwareTimer->setCount(0); //reset the counter. 
  _HardwareTimer->setEdgeCounting(TIMER_SMCR_SMS_ENCODER3); //or TIMER_SMCR_SMS_ENCODER1 or TIMER_SMCR_SMS_ENCODER2. This uses both channels to count and ascertain direction. 
  _HardwareTimer->attachInterrupt(0, handler); //Overflow interrupt (extern function)  
  _HardwareTimer->resume();//start the encoder...
  _HardwareTimer->refresh(); 
  InitSpeedMeasure();
   
}

void QuadDecoder::SetValue(float Value)
{
  long temp;
  ComputeAbsoluteValue();
  temp = (long)(Value * _Resolution);  
  if ( _RelativeModeActived == false)
  {
    //Absolute
    _AbsoluteCounterZero = _AbsoluteCounter - temp;   
  }
  else
  {
    //relative  
    _RelativeCounterZero= _AbsoluteCounter - temp;
  }  
}
float QuadDecoder::GetValue()
{
  long temp;
  ComputeAbsoluteValue();
  if ( _RelativeModeActived == false)
  {
    //Absolute
    temp = _AbsoluteCounter-_AbsoluteCounterZero;   
  }
  else
  {
    //relative  
    temp = _AbsoluteCounter-_RelativeCounterZero;
  } 
  
  return  (float)temp/_Resolution; 
}
long  QuadDecoder::GetValueLong()
{
  long temp;
  ComputeAbsoluteValue();
  if ( _RelativeModeActived == false)
  {
    //Absolute
    temp = _AbsoluteCounter-_AbsoluteCounterZero;   
  }
  else
  {
    //relative  
    temp = _AbsoluteCounter-_RelativeCounterZero;
  } 
  
  return  temp;   
}
unsigned int QuadDecoder::GetValuePos()
{
  if( _Sens == true )
  {
    return (_Overflow_Size-_HardwareTimer->getCount()); 
  }
  else
  {
    return _HardwareTimer->getCount();   
  }       
}

void QuadDecoder::SetAbsolut(){ _RelativeModeActived = false;}
void QuadDecoder::SetRelative(){ _RelativeModeActived = true;}
void QuadDecoder::SwitchMode(){_RelativeModeActived = !_RelativeModeActived;}
boolean QuadDecoder::RelativeModeActived(){return _RelativeModeActived;}
void QuadDecoder::SetDiameterMode (boolean DiameterMode){_DiameterMode = DiameterMode;}
void QuadDecoder::ToggleSens(){_Sens = !_Sens;}
void QuadDecoder::ToggleDiameterMode(){_DiameterMode = !_DiameterMode;}

void QuadDecoder::SetAbsolutZero()
{
  ComputeAbsoluteValue();
  _AbsoluteCounterZero = _AbsoluteCounter;  
  
}
void QuadDecoder::SetRelativeZero()
{
  ComputeAbsoluteValue();   
  _RelativeCounterZero = _AbsoluteCounter; 
}
void QuadDecoder::SetZeroActiveMode()
{
  if(_RelativeModeActived == false)SetAbsolutZero();
  else SetRelativeZero();     
}

void QuadDecoder::ComputeAbsoluteValue()
{
   _CountValue = _HardwareTimer->getCount();
   _AbsoluteCounter = _CountValue + _Overflow * (_Overflow_Size + 1);
  if(_Sens == true)
  {
    _AbsoluteCounter = -_AbsoluteCounter;      
  }
  if(_DiameterMode == true)
  {
    _AbsoluteCounter = _AbsoluteCounter*2;    
  }  
}

void QuadDecoder::SetResolution(unsigned int  Resolution)
{
  _HardwareTimer->pause(); //stop...
  _Resolution = Resolution;   
  UpdatOverflowSize();
  _HardwareTimer->setOverflow(_Overflow_Size);    
  _HardwareTimer->setCount(0); //reset the counter. 
  _HardwareTimer->resume();//start the encoder... 
  _HardwareTimer->refresh();
  ResetAllTheCounter();  
}
void QuadDecoder::SetSens(boolean Sens)
{
  _Sens = Sens;    
}

void QuadDecoder::ResetAllTheCounter()
{
  _Overflow = 0; 
  _AbsoluteCounter = 0;
  _AbsoluteCounterZero = 0;
  _RelativeCounterZero = 0; 
  InitSpeedMeasure();  
}

void QuadDecoder::InitSpeedMeasure()
{
  ComputeAbsoluteValue();
  _TimeCalcSpeed = millis();
  _Speed = 0;
  _PosCalcSpeed = _AbsoluteCounter;  
}
int QuadDecoder::GiveMeTheSpeed()
{
  long Time;
  long Pos;
  Time = millis();
  ComputeAbsoluteValue();
  Pos = _AbsoluteCounter;  
  if(Time > (_TimeCalcSpeed+200) )
  {
    _Speed = (int)((long)(Pos - _PosCalcSpeed)*60000)/(long)((Time - _TimeCalcSpeed)*_Resolution); 
    //Spindle_Speed = DeltaPos;   
    _TimeCalcSpeed = Time;
    _PosCalcSpeed = Pos;      
  }
  return _Speed; 
}
void QuadDecoder::UpdatOverflowSize()
{
  if(_eTypeEncoder==LinearEncoder)_Overflow_Size = 0xFFFF;
  else _Overflow_Size = _Resolution-1;   

}
