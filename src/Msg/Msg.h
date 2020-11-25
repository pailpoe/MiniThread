#ifndef Msg_h
#define Msg_h   
#include <Arduino.h>
#include <U8g2lib.h>
 
 
class Msg
{
public:
  enum teTypeMsg
  {
    Warning,
    Info
  };
	Msg(U8G2& u8g2_);
  void DisplayMsg(char* Msg_, teTypeMsg eType, unsigned int TimeMs_);
   
private:
  U8G2& _u8g2;

};
    
#endif
