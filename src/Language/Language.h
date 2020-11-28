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
    Id_Msg_TEXT_MENU_THREAD_THREAD = 22,
    Id_Msg_TEXT_MENU_THREAD_OFFSET = 23,
    Id_Msg_TEXT_MENU_THREAD_USEY = 24,
    Id_Msg_TEXT_MENU_THREAD_DIAMETER = 25,
    Id_Msg_TEXT_MENU_THREAD_ANGLE = 26,  
    Id_Msg_TEXT_MENU_THREAD_INC = 27, 
    Id_Msg_TEXT_MENU_THREAD_DEC = 28,
    Id_Msg_TEXT_MENU_AXE_TOOL = 29,
    Id_Msg_TEXT_MENU_AXE_REL = 30,     
    Id_Msg_TEXT_MENU_SETTINGS_XDIR = 31,
    Id_Msg_TEXT_MENU_SETTINGS_XSTEP = 32,
    Id_Msg_TEXT_MENU_SETTINGS_YDIR = 33,
    Id_Msg_TEXT_MENU_SETTINGS_YSTEP = 34,
    Id_Msg_TEXT_MENU_SETTINGS_CDIR = 35,
    Id_Msg_TEXT_MENU_SETTINGS_CSTEP = 36,
    Id_Msg_TEXT_MENU_SETTINGS_YDIAM = 37,
    Id_Msg_TEXT_MENU_SETTINGS_M1DIR = 38,
    Id_Msg_TEXT_MENU_SETTINGS_M1STEP = 39,
    Id_Msg_TEXT_MENU_SETTINGS_M1TH = 40,
    Id_Msg_TEXT_MENU_SETTINGS_M1ACC = 41,
    Id_Msg_TEXT_MENU_SETTINGS_M1SPE = 42,
    Id_Msg_TEXT_MENU_SETTINGS_M1LAN = 43,
    Id_Msg_Warning_YINFDIA = 44,
    Id_Msg_Warning_YSUPDIA = 45,  
    Id_Msg_Warning_SpeedTooHigh = 46,
    Id_Msg_Warning_WrongDirection = 47,
    Id_Msg_Warning_NoEndLimit = 48,
    Id_Msg_Warning_NoAtMinPos = 49,
    
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
    { "Axe menu",//Id_Msg_TEXT_MENU_AXE_FUNCTIONS   "Axe menu"
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
    { "WARNING:No end limits",//Id_Msg_Motor_NoEndLimit
      "ATTENTION:Pas de Fdc"},     
    { "Thread",//Id_Msg_TEXT_MENU_THREAD_THREAD
      "Pas"},
    { "Offset",//Id_Msg_TEXT_MENU_THREAD_OFFSET
      "Decalage"}, 
    { "Use Y",//Id_Msg_TEXT_MENU_THREAD_USEY
      "Utiliser Y"}, 
    { "Diameter",//Id_Msg_TEXT_MENU_THREAD_DIAMETER
      "Diametre"}, 
    { "Angle",//Id_Msg_TEXT_MENU_THREAD_ANGLE
      "Angle"}, 
    { "Inc. Offset +2",//Id_Msg_TEXT_MENU_THREAD_INC
      "Inc. decalage +2"}, 
    { "Dec. Offset -2",//Id_Msg_TEXT_MENU_THREAD_DEC
      "Dec. decalage -2"},   
    { "Tools",//Id_Msg_TEXT_MENU_AXE_TOOL
      "Outils"}, 
    { "Relative",//Id_Msg_TEXT_MENU_AXE_REL
      "Relatif"},
    { "X dir.",//Id_Msg_TEXT_MENU_SETTINGS_XDIR
      "X direction"},
    { "X step/mm",//Id_Msg_TEXT_MENU_SETTINGS_XSTEP
      "X pas/mm"},
    { "Y dir.",//Id_Msg_TEXT_MENU_SETTINGS_YDIR
      "Y direction"},
    { "Y step/mm",//Id_Msg_TEXT_MENU_SETTINGS_YSTEP
      "Y pas/mm"},
    { "C dir.",//Id_Msg_TEXT_MENU_SETTINGS_CDIR
      "C direction"},
    { "C step/tr",//Id_Msg_TEXT_MENU_SETTINGS_CSTEP
      "C pas/tr"},
    { "Y diameter",//Id_Msg_TEXT_MENU_SETTINGS_YDIAM
      "Y diametre"},
    { "M1 dir.",//Id_Msg_TEXT_MENU_SETTINGS_M1DIR
      "M1 dir."},
    { "M1 step/tr",//Id_Msg_TEXT_MENU_SETTINGS_M1STEP
      "M1 pas/tr"},
    { "M1 thread",//Id_Msg_TEXT_MENU_SETTINGS_M1TH
      "M1 pas"},
    { "M1 accel.",//Id_Msg_TEXT_MENU_SETTINGS_M1ACC
      "M1 accel."},
    { "M1 speed",//Id_Msg_TEXT_MENU_SETTINGS_M1SPE
      "M1 vitesse"},
    { "Language",//Id_Msg_TEXT_MENU_SETTINGS_M1LAN
      "Langue"},
    { "Move Y\nY < Dia ext",//Id_Msg_Warning_YINFDIA
      "Reculer l'axe Y\nY < Dia exterieur"},
    { "Move Y\nY > Dia int",//Id_Msg_Warning_YSUPDIA
      "Avancer l'axe Y\nY > Dia interieur"},
    { "Reduce spindle speed",//Id_Msg_Warning_SpeedTooHigh
      "Reduire la vitesse\nde la broche"},
    { "Spindle wrong dir",//Id_Msg_Warning_WrongDirection
      "Changer le sens de \nrotation de la broche"},
    { "No end limit",//Id_Msg_Warning_NoEndLimit
      "Activer les fins de\ncourse du moteur"},
    { "No at min pos",//Id_Msg_Warning_NoAtMinPos
      "Positionner le moteur\na sa position min"},

  }; 


void ChangeLang(char Lang);
char* GetTxt(teMsgId eIdMsg); 

#endif