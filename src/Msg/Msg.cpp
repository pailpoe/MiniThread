#include <Arduino.h>
#include "Msg.h"
#include "../Language/Language.h"

#define DEFINE_SIZE_BUFFER 200

Msg::Msg(U8G2& u8g2_)
: _u8g2(u8g2_)
{

}
void  Msg::DisplayMsg(char* Msg_, teTypeMsg eType, unsigned int TimeMs_)
{
  char buffer[DEFINE_SIZE_BUFFER];
  
  unsigned int uiBufferPos = 0;
  unsigned int Line = 15;
  for(uiBufferPos = 0; uiBufferPos++; uiBufferPos<DEFINE_SIZE_BUFFER)
  {
    buffer[uiBufferPos] = 'v';
  }
  
  
  _u8g2.firstPage();
  _u8g2.setFontPosTop();
  do 
  { 
    _u8g2.setFont(u8g2_font_6x12_tr); // choose a suitable font
    _u8g2.drawRFrame(0,0 ,128,64,0);
    _u8g2.drawBox(0,0 ,128,13);
    _u8g2.setColorIndex(0);
    if(eType == Warning)
    {
        _u8g2.drawStr(4,2,GetTxt(Id_Msg_Warning));  
    }
    if(eType == Info)
    {
        _u8g2.drawStr(4,2,GetTxt(Id_Msg_Info));  
    }
    _u8g2.setColorIndex(1);    
    
    
    while( *Msg_ != 0 && uiBufferPos < DEFINE_SIZE_BUFFER )
    { 
      if(*Msg_ == '\n')
      {
        if(uiBufferPos != 0)
        {
          buffer[uiBufferPos] = 0;
          _u8g2.drawStr(2,Line,&buffer[0]);
          Line += 10;
          uiBufferPos = 0;
        }
      }
      else
      {
        buffer[uiBufferPos] = *Msg_;
        uiBufferPos++;
      }
      Msg_++;        
    }
    buffer[uiBufferPos] = *Msg_;
    _u8g2.drawStr(2,Line,&buffer[0]);    
    //_u8g2.drawStr(3,20,Msg_);
  } while (_u8g2.nextPage());  
  delay(TimeMs_);   
  
}