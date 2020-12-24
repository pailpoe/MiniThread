#ifndef Config_h
#define Config_h   
#include <Arduino.h>

#define TEXT_MAIN_MENU_TITLE "MiniThread 1.3.0"
#define TEXT_AUTHOR_SOFT "Pailpoe"
#define TEXT_VERSION_SOFT "1.3.0"

// IO def ( for quad decoder, define in class !)
#define PIN_RES_SCR    PB9
#define PIN_MOT1_STEP  PA10
#define PIN_MOT1_DIR   PB15
#define PIN_MOT1_EN    PA15
#define PIN_MOT2_STEP  PB13
#define PIN_MOT2_DIR   PB12
#define PIN_MOT2_EN    PB14
#define PIN_SW_LIN_1   PA5
#define PIN_SW_LIN_2   PA4
#define PIN_SW_LIN_3   PA3
#define PIN_SW_LIN_4   PA2
#define PIN_SW_COL_1   PB0
#define PIN_SW_COL_2   PB1
#define PIN_SW_COL_3   PB10
#define PIN_SW_COL_4   PB11 

//Screen choose
#define  SCREEN_DRO 0
#define  SCREEN_MOT1 1
#define  SCREEN_DEBUG 2
#define  SCREEN_END_LIST 1

//Motor mode
#define MOTOR_MODE_NO_MODE  0
#define MOTOR_MODE_MANUAL   1
#define MOTOR_MODE_AUTO     2
#define MOTOR_MODE_TH_EXT_N 3
#define MOTOR_MODE_TH_EXT_I 4
#define MOTOR_MODE_TH_INT_N 5
#define MOTOR_MODE_TH_INT_I 6
#define MOTOR_MODE_PROFIL   7

//Profil mode 
#define PROFIL_MODE_CONE    0
#define PROFIL_MODE_SPHERE  1

//Profil Direction 
#define PROFIL_LEFT   0
#define PROFIL_RIGHT  1

//Profil Position 
#define PROFIL_EXT   0
#define PROFIL_INT   1



//Data backup structure 
typedef struct
{
  boolean Inverted_X;  
  boolean Inverted_Y;
  boolean Inverted_Z;
  boolean Diameter_Mode_Y;
  int  Reso_X;
  int  Reso_Y;
  int  Reso_Z;
  boolean Inverted_M1;
  int  Reso_M1;
  int  thread_M1;
  float Accel_M1;
  int Speed_M1;
  float Backlash_M1;
  byte Lang;
  boolean UseUSBFunctions;
} tsConfigDro; 
//Threading machine state
typedef enum
{
  MS_THREAD_IDLE = 0, //Idle
  MS_THREAD_WAIT_THE_START = 1, //Wait the button to start
  MS_THREAD_WAIT_THE_SPLINDLE_ZERO = 2, // Wait spindle zero
  MS_THREAD_IN_THREAD = 3, // In threat
  MS_THREAD_END_THREAD = 4, // Wait the button to return
  MS_THREAD_IN_RETURN = 5 // In return
} teMS_ThreadingMode;
//Profil machine state
typedef enum
{
  MS_PROFIL_IDLE = 0,             // Idle
  MS_PROFIL_WAIT_START_CYCLE = 1, // Wait start cycle with "OK"
  MS_PROFIL_WAIT_THE_START = 2,   // Wait the button to start
  MS_PROFIL_IN_PROFIL = 3,        // In Profil
  MS_PROFIL_END_PROFIL = 4,       // Wait the button to return
  MS_PROFIL_IN_RETURN = 5,        // In return
  MS_PROFIL_BACKASH = 6,          // Backlash conpensation
  MS_PROFIL_END = 7               // End
} teMS_ProfilMode;

//Struct for the thread mode
typedef struct
{
  long Numerator;
  long Denominator;
  long Offset;  
} tsThreadCalc;

//Struct for the profil mode
typedef struct
{
  long DiamInProfilY;
  long DiamStartProfilY;
  long DiamEndProfilY;
  long DiamReturnY; 
  long StartPositionX;
  long EndPositionX;
  long BackLashPosX;
  long Passe; //sur le rayon !
  unsigned int Count;
  boolean LastPasse; 
  long Numerator;
  long Denominator;
} tsProfilData;
 
    
#endif
