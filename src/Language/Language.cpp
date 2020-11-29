#include <Arduino.h>
#include "Language.h"

char btActualLang = LANG_FR;
void ChangeLang(char Lang)
{
    btActualLang = Lang; 
}

char* GetTxt(teMsgId eIdMsg)
{
  if( btActualLang == LANG_FR)
  {
    return sLangData[eIdMsg].Fr;  
  }
  else
  {
    return sLangData[eIdMsg].En; 
  } 
} 
