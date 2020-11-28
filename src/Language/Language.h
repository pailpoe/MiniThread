#ifndef Language_H_
#define Language_H_
#include <Arduino.h>


#define LANG_EN  0
#define LANG_FR  1

typedef enum
{
    Id_Msg_Start = 0,
    Id_Msg_Warning = 1,
    Id_Msg_Info = 2,
    Id_Msg_Save = 3,
    Id_Msg_Restore = 4,
    Id_Msg_TEXT_MENU_SETTINGS = 5,           
    Id_Msg_TEXT_MENU_RETURN_SCREEN = 6,       
    Id_Msg_TEXT_MENU_FAST_FUNCTIONS = 7,     
    Id_Msg_TEXT_MENU_FAST_M1MANU = 8,        
    Id_Msg_TEXT_MENU_FAST_M1AUTO = 9,        
    Id_Msg_TEXT_MENU_AXE_FUNCTIONS = 10,    
    Id_Msg_TEXT_MENU_MOTOR_FUNCTIONS = 11,    
    Id_Msg_TEXT_MENU_MOTORABLED = 12,      
    Id_Msg_TEXT_MENU_MOTORDLIMIT = 13,    
    Id_Msg_TEXT_MENU_MOTOR_SPEED = 14,      
    Id_Msg_TEXT_MENU_ECRAN = 15,              
    Id_Msg_TEXT_MENU_DEBUG = 16,              
    Id_Msg_TEXT_MENU_SAVE_SETTINGS = 17,      
    Id_Msg_TEXT_MENU_RESTORE_SETTINGS = 18,   
    Id_Msg_TEXT_MENU_THREAD_PARAMETERS = 19,
    Id_Msg_Motor_Disabled = 20,
    Id_Msg_Motor_NoEndLimit = 21,
}teMsgId;

typedef struct
{
    char* En;
    char* Fr;
}tsLang;
const tsLang sLangData[] = 
  {  
    { "Use this system\nat your own risk\nkeep cool !",//Id_Msg_Start
      "Utiliser ce systeme\na vos propre risques"},
    { "WARNING",//Id_Msg_Warning
      "ATTENTION"},
    { "INFORMATION",//Id_Msg_Info
      "INFORMATION"},
    { "Settings saved",//Id_Msg_Save
      "Param. sauvegardes"},
    { "Settings restored",//Id_Msg_Restore
      "Param. restaures"},
    { "Settings menu",//Id_Msg_TEXT_MENU_SETTINGS
      "Menu des reglages"},
    { "Working screen",//Id_Msg_TEXT_MENU_RETURN_SCREEN
      "Ecran de travail"},
    { "Shortcuts",//Id_Msg_TEXT_MENU_FAST_FUNCTIONS
      "Raccourcis"},
    { "Motor 1 manual",//Id_Msg_TEXT_MENU_FAST_M1MANU
      "Moteur 1 en manuel"},
    { "Motor 1 auto",//Id_Msg_TEXT_MENU_FAST_M1AUTO
      "Moteur 1 en auto"},
    { "Axe menu",//Id_Msg_TEXT_MENU_AXE_FUNCTIONS
      "Menu des axes"},
    { "Motor1 menu",//Id_Msg_TEXT_MENU_MOTOR_FUNCTIONS
      "Menu du moteur1"},
    { "Enabled",//Id_Msg_TEXT_MENU_MOTORABLED
      "Active"},
    { "Use limit",//Id_Msg_TEXT_MENU_MOTORDLIMIT
      "FinDeCourse"},
    { "Speed",//Id_Msg_TEXT_MENU_MOTOR_SPEED
      "Vitesse"},
    { "Screen",//Id_Msg_TEXT_MENU_ECRAN
      "Ecran"},
    { "Debug tools",//Id_Msg_TEXT_MENU_DEBUG
      "Outils pour dev."},
    { "Save settings",//Id_Msg_TEXT_MENU_SAVE_SETTINGS
      "Sauvegarder"},
    { "Restore settings",//Id_Msg_TEXT_MENU_RESTORE_SETTINGS
      "Restaurer"},
    { "Thread parameters",//Id_Msg_TEXT_MENU_RESTORE_SETTINGS
      "Para. de filetage"},
    { "Motor disabled",//Id_Msg_Motor_Disabled
      "Moteur desactive"},
    { "No end limits",//Id_Msg_Motor_NoEndLimit
      "Pas de Fdc"},     
      
  }; 



void ChangeLang(char Lang);
char* GetTxt(teMsgId eIdMsg); 

#endif