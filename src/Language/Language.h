#ifndef Language_H_
#define Language_H_

//#define UseFrenchForText
#define UseEnglishForText

#ifdef UseEnglishForText
  #define TEXT_MENU_SETTINGS        "Settings"
  #define TEXT_MENU_RETURN_SCREEN   "Return to Screen"
  #define TEXT_MENU_AXE_FUNCTIONS   "Axe Functions"
  #define TEXT_MENU_MOTOR_FUNCTIONS "Motor Functions"
  #define TEXT_MENU_ECRAN           "Screen:"
#endif
#ifdef UseFrenchForText
  #define TEXT_MENU_SETTINGS        "Reglage"
  #define TEXT_MENU_RETURN_SCREEN   "Retour a l'ecran"
  #define TEXT_MENU_AXE_FUNCTIONS   "Fonctions des axes"
  #define TEXT_MENU_MOTOR_FUNCTIONS "Fonctions moteur"
  #define TEXT_MENU_ECRAN           "Ecran:"
#endif



 

#endif