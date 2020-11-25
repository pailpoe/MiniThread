#include <Arduino.h>
#include "Msg.h"

Msg::Msg(U8G2& u8g2_)
: _u8g2(u8g2_)
{

}
void  Msg::DisplayMsg(char* Msg_, teTypeMsg eType, unsigned int TimeMs_)
{
  _u8g2.firstPage();
  _u8g2.setFontPosTop();
  do 
  {
    _u8g2.drawRFrame(0,0 ,128,64,4);
    _u8g2.drawLine(0,13 ,128,13);
    if(eType == Warning)
    {
        _u8g2.drawStr(4,2,"WARNING !!");  
    }
    if(eType == Info)
    {
        _u8g2.drawStr(4,2,"INFORMATION");  
    }    
    _u8g2.setFont(u8g2_font_6x12_tr); // choose a suitable font
    _u8g2.drawStr(3,20,Msg_);
  } while (_u8g2.nextPage());  
  delay(TimeMs_);   
  
}