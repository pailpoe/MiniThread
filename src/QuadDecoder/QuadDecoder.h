#ifndef QuadDecoder_h
#define QuadDecoder_h
    
#include <Arduino.h>
#include "HardwareTimer.h"

#define Overflow_Size_default 65536    

class QuadDecoder
{
public:
  enum teTypeEncoder
  {
    LinearEncoder,
    RotaryEncoder
  };
QuadDecoder(unsigned int  TimerChannel,teTypeEncoder  eTypeEncoder, unsigned int  Resolution , boolean Sens,boolean DiameterMode,voidFuncPtr handler);

//To update value
void IT_OverflowHardwareTimer(); //IT overflow hardware timer


void SetAbsolut();
void SetRelative();
void SetAbsolutZero();
void SetRelativeZero();
void SetZeroActiveMode();
void SwitchMode(); //Absolute or relative
boolean RelativeModeActived(); // true if actived

void SetValue(float Value); //Absolut or reltive, depend of the mode and the resolution
float GetValue(); //Absolut or reltive, depend of the mode and the resolution
long  GetValueLong(); //Absolut or reltive, depend of the mode and the resolution
unsigned int GetValuePos(); //Pos without overflow...
int GiveMeTheSpeed(); // calcul the speed

void SetResolution(unsigned int  Resolution);
void SetSens(boolean Sens);
void ToggleSens();
void SetDiameterMode (boolean DiameterMode);
void ToggleDiameterMode();
void ResetAllTheCounter();


private:
  void ComputeAbsoluteValue();
  void InitSpeedMeasure();
  void UpdatOverflowSize();
  teTypeEncoder _eTypeEncoder;
  unsigned int _CountValue; //Encoder count value
  unsigned int _Overflow_Size; //Overflow size default = 65536
  long _AbsoluteCounter; //Absolute counter
  long _AbsoluteCounterZero; //Zero Absolu
  long _RelativeCounterZero; //Zero Relatif
  unsigned int _Resolution; //Resolution par mm
  long _TimeCalcSpeed;
  long _PosCalcSpeed;
  int _Speed;
  boolean _Sens; //Sens de comptage
  boolean _RelativeModeActived; // if true, relative mode else absolute mode
  boolean _DiameterMode; // Mode Diameter true or false
  int _Overflow; // Comptage overflow or underflow 16bits counter
  HardwareTimer *_HardwareTimer;
};
    
#endif
