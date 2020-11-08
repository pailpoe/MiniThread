/*********************************************************************
Project Name    :   MiniThread
Hard revision   :   V1.0
Soft revision   :   /
Description     :   
Chip            :   STM32F103CBT6
freq uc         :   72Mhz (use 8Mhz external oscillator with PLL ) 
Compiler        :   Arduino IDE 1.8.3
Author          :   G.Pailleret, 2020 
Remark          :  
Revision        :

*********************************************************************/

#include "src/GEM/GEM_u8g2.h"
#include "src/QuadDecoder/QuadDecoder.h"
#include "src/Keypad/Keypad.h"
#include "src/StepperMotor/StepperMotor.h"
#include "src/Various/Splash.h"
#include <EEPROM.h>

#define TEXT_MAIN_MENU_TITLE "MiniThread V1.0"
#define TEXT_AUTHOR_SOFT "Pailpoe"
#define TEXT_VERSION_SOFT "V1.0"

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

//Instance Screen 
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, PIN_RES_SCR);

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
char hexaKeys[ROWS][COLS] = {
  {'Z'             ,'Z'           ,GEM_KEY_UP   ,'Z'            },
  {GEM_KEY_CANCEL  ,GEM_KEY_LEFT  ,GEM_KEY_OK   ,GEM_KEY_RIGHT  },
  {'Z'             ,'Z'           ,GEM_KEY_DOWN ,'Z'            },
  {'Z'             ,'Z'           ,'Z'          ,'Z'            }
};
byte rowPins[ROWS] = {PIN_SW_LIN_1, PIN_SW_LIN_2, PIN_SW_LIN_3, PIN_SW_LIN_4};
byte colPins[COLS] = {PIN_SW_COL_1, PIN_SW_COL_2, PIN_SW_COL_3, PIN_SW_COL_4};
//initialize an instance of class NewKeypad
Keypad customKeypad ( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);


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
} sConfigDro;
const sConfigDro csConfigDefault = {false,false,false,true,512,512,1200,false,1600,200,60000.0,12000};

// Variable
sConfigDro ConfigDro;

//Settings sub menu ******************************************
GEMPage menuPageSettings("Settings"); // Settings submenu
GEMItem menuItemMainSettings("Settings", menuPageSettings);
GEMItem menuItemDirX("X dir:", ConfigDro.Inverted_X);
GEMItem menuItemDirY("Y dir:", ConfigDro.Inverted_Y);
GEMItem menuItemDirZ("C dir:", ConfigDro.Inverted_Z);
GEMItem menuItemDiamY("Y diameter:", ConfigDro.Diameter_Mode_Y);
GEMItem menuItemResoX("X step/mm:", ConfigDro.Reso_X);
GEMItem menuItemResoY("Y step/mm:", ConfigDro.Reso_Y);
GEMItem menuItemResoZ("C step/tr:", ConfigDro.Reso_Z);
GEMItem menuItemDirM1("M1 dir:", ConfigDro.Inverted_M1);
GEMItem menuItemResoM1("M1 step/tr:", ConfigDro.Reso_M1);
GEMItem menuItemThreadM1("M1 thread:", ConfigDro.thread_M1);
GEMItem menuItemAccelM1("M1 accel:", ConfigDro.Accel_M1);
GEMItem menuItemSpeedM1("M1 speed:", ConfigDro.Speed_M1);
void ActionRestoreSettingsInFlash(); // Forward declaration
GEMItem menuItemButtonRestoreSettings("Restore settings", ActionRestoreSettingsInFlash);
void ActionSaveSettingsInFlash(); // Forward declaration
GEMItem menuItemButtonSaveSettings("Save settings", ActionSaveSettingsInFlash);

void ActionDro(); // Forward declaration
GEMItem menuItemButtonDro("Return to Screen", ActionDro);

//Main Page Menu
GEMPage menuPageMain(TEXT_MAIN_MENU_TITLE);

//Debug Page Menu
GEMPage menuPageDebug("Debug tools"); // Debug submenu
GEMItem menuItemDebug("Debug tools", menuPageDebug);
void ActionDebug(); // Forward declaration
GEMItem menuItemButtonDebug("Debug screen", ActionDebug);
float TestFloat = 999.2;
GEMItem menuItemTestFloat("Float:", TestFloat);

//Axe Functions ******************************************
GEMPage menuPageAxe("Axe Functions"); // Axe submenu
GEMItem menuItemAxe("Axe Functions", menuPageAxe);
byte ToolChoose = 0;
SelectOptionByte selectToolOptions[] = {{"Tool_0", 0}, {"Tool_1", 1}, {"Tool_2", 2}, {"Tool_3", 3}, {"Tool_4", 4}, {"Tool_5", 5}};
GEMSelect selectTool(sizeof(selectToolOptions)/sizeof(SelectOptionByte), selectToolOptions);
void applyTool(); // Forward declaration
GEMItem menuItemTool("Tool:", ToolChoose, selectTool, applyTool);
boolean RelativeMode = false;
void ActionChangeRelaticeMode();
GEMItem menuItemRelativeMode("Relative:", RelativeMode,ActionChangeRelaticeMode);
void ActionResetX(); // Forward declaration
GEMItem menuItemButtonResetX("Set X to zero", ActionResetX);
void ActionResetY(); // Forward declaration
GEMItem menuItemButtonResetY("Set Y to zero", ActionResetY);
float fAxeXPos = 0;
void ActionAxeXPos(); // Forward declaration
GEMItem menuItemAxeXPos("X Pos:", fAxeXPos,ActionAxeXPos);
float fAxeYPos = 0;
void ActionAxeYPos(); // Forward declaration
GEMItem menuItemAxeYPos("Y Pos:", fAxeYPos,ActionAxeYPos);

//Motor Functions ****************************************
GEMPage menuPageMotor("Motor Functions"); // Motor submenu
GEMItem menuItemMotor("Motor Functions", menuPageMotor);
boolean bUseMotor = false;
void ActionUseMotor(); // Forward declaration
GEMItem menuItemUseMotor("Use motor:", bUseMotor,ActionUseMotor);
#define MOTOR_MODE_NO_MODE 0
#define MOTOR_MODE_MANUAL  1
#define MOTOR_MODE_AUTO    2
#define MOTOR_MODE_LEFT    3
byte bMotorMode = 0;
SelectOptionByte selectMotorModeOptions[] = {{"NoMode", 0}, {"Manual", 1},{"Auto", 2}, {"Left", 3}};
GEMSelect selectMotorMode(sizeof(selectMotorModeOptions)/sizeof(SelectOptionByte), selectMotorModeOptions);
void applyMotorMode(); // Forward declaration
GEMItem menuItemMotorMode("Motor mode:", bMotorMode, selectMotorMode, applyMotorMode);
float fMotorStopMin = -200.0;
void ActionMotorStopMin(); // Forward declaration
GEMItem menuItemMotorStopMin("Stop Min:", fMotorStopMin,ActionMotorStopMin);
float fMotorStopMax = 200.0;
void ActionMotorStopMax(); // Forward declaration
GEMItem menuItemMotorStopMax("Stop Max:", fMotorStopMax,ActionMotorStopMax);
boolean bUseMotorEndLimit = true;
void ActionUseMotorEndLimit(); // Forward declaration
GEMItem menuItemUseMotorEndLimit("Use limit:", bUseMotorEndLimit,ActionUseMotorEndLimit);
float fMotorCurrentPos = 0;
void ActionMotorCurrentPos(); // Forward declaration
GEMItem menuItemMotorCurrentPos("CurrentPos:", fMotorCurrentPos,ActionMotorCurrentPos);
int iMotorSpeed = 1000;
void ActionMotorMotorSpeed(); // Forward declaration
GEMItem menuItemMotorSpeed("Speed:", iMotorSpeed,ActionMotorMotorSpeed);
int iMotorThread = 100;
void ActionMotorChangeThread(); // Forward declaration
GEMItem menuItemMotorThread("Thread:", iMotorThread,ActionMotorChangeThread);
void ActionSetCurrentToMax(); // Forward declaration
GEMItem menuItemButtonSetPosToMax("CurrentPos -> Max", ActionSetCurrentToMax);
void ActionSetCurrentToMin(); // Forward declaration
GEMItem menuItemButtonSetPosToMin("CurrentPos -> Min", ActionSetCurrentToMin);
void ActionResetCurrentPos(); // Forward declaration
GEMItem menuItemButtonResetCurrentPos("Reset CurrentPos", ActionResetCurrentPos);
//Screen choose
#define  SCREEN_DRO 0
#define  SCREEN_MOT1 1
#define  SCREEN_DEBUG 2
#define  SCREEN_END_LIST 2 
byte eScreenChoose = SCREEN_DRO;
SelectOptionByte selectScreenOptions[] = {{"DroXYC", 0}, {"Mot1", 1}, {"Debug", 2}};
GEMSelect selectScreenMode(sizeof(selectScreenOptions)/sizeof(SelectOptionByte), selectScreenOptions);
void ActionScreenMode(); // Forward declaration
GEMItem menuItemScreenMode("Screen:", eScreenChoose, selectScreenMode, ActionScreenMode);
void ActionChangeScreen();// Forward declaration

//Threading state
typedef enum
{
  MS_THREAD_IDLE = 0, //Idle
  MS_THREAD_WAIT_THE_START = 1, //Wait the action to start
  MS_THREAD_WAIT_THE_SPLINDLE_ZERO = 2, // Wait spindle zero
  MS_THREAD_IN_THREAD = 3, // In threat
  MS_THREAD_END_THREAD = 4, // Wait the button to return
  MS_THREAD_IN_RETURN = 5 // In return
} teMS_ThreadingMode;
teMS_ThreadingMode eMS_Thread = MS_THREAD_IDLE;
typedef struct
{
  long Numerator;
  long Denominator;
  long Offset;  
} tsThreadCalc;
tsThreadCalc sThreadCalc; 


// Create menu object of class GEM_u8g2. Supply its constructor with reference to u8g2 object we created earlier
GEM_u8g2 menu(u8g2,GEM_POINTER_ROW,5,10,10,75);

//Quadrature decoder
void IT_Timer1_Overflow(); // Forward declaration
void IT_Timer2_Overflow(); // Forward declaration
void IT_Timer3_Overflow(); // Forward declaration
QuadDecoder Quad_Y(3,QuadDecoder::LinearEncoder,512,false,false,IT_Timer3_Overflow); //Timer 3
QuadDecoder Quad_Z(2,QuadDecoder::RotaryEncoder,512,true,false,IT_Timer2_Overflow); //Timer 2
QuadDecoder Quad_X(1,QuadDecoder::LinearEncoder,512,false,false,IT_Timer1_Overflow); //Timer 1
void IT_Timer1_Overflow(){Quad_X.IT_OverflowHardwareTimer();}
void IT_Timer3_Overflow(){Quad_Y.IT_OverflowHardwareTimer();}
void IT_Timer2_Overflow()
{
  Quad_Z.IT_OverflowHardwareTimer();
  if(eMS_Thread == MS_THREAD_WAIT_THE_SPLINDLE_ZERO)
  {
    Quad_Z.ResetAllTheCounter();
    eMS_Thread = MS_THREAD_IN_THREAD;   
  }  
}
//Hardware timer 4 for motor control
HardwareTimer MotorControl(4); 
//Motor Class

void Update_Overlfow_Timer4();
StepperMotor Motor1(800,false,PIN_MOT1_STEP,PIN_MOT1_DIR,PIN_MOT1_EN, Update_Overlfow_Timer4);

//Timer 4 overflow for Step motor
void handler_Timer4_overflow()
{ 
  if(eMS_Thread == MS_THREAD_IN_THREAD)
  {
    Motor1.ChangeTargetPositionStep ((Quad_Z.GetValueLong()*sThreadCalc.Numerator ) / sThreadCalc.Denominator + sThreadCalc.Offset );
  }  
  Motor1.TimeToPrepareToMove();  
}
//Timer 4 channel 3 compare interrupt (10µs after overflow)
void handler_Timer4_compare3()
{
  Motor1.TimeToMove();
}
//Timer 4 change overflow value in µs
void Update_Overlfow_Timer4()
{
  MotorControl.setOverflow(Motor1.NewInterval());  
}

// ***************************************************************************************
// Forward declarations Funtions
 
long GCD_Function ( long n1, long n2); //Forward declaration
void CalcMotorParameterForThread(); //Forward declaration
void UsbSerial_Pos(); 
void Display_UpdateRealTimeData(); //Forward declarations
void ActionMotorSpeedUp();//Forward declarations
void ActionMotorSpeedDown();//Forward declarations
void Display_StartScreen(); //Forward declarations

// ***************************************************************************************
// ***************************************************************************************
// *** setup, loop, ...  *****************************************************************
void setup() 
{
  //Delay  for boot of the screen without problem
  delay(200);
  pinMode(PIN_RES_SCR, OUTPUT);  //channel A
  digitalWrite(PIN_RES_SCR,0);
  delay(200);
  digitalWrite(PIN_RES_SCR,1);
  delay(200);

    
  u8g2.begin();
  //Display start screen
  Display_StartScreen();
  
  //Debug port...
  afio_cfg_debug_ports(AFIO_DEBUG_SW_ONLY); //Only SWD

  //USB Serial
  Serial.begin(115200); // Ignored by Maple. But needed by boards using hardware serial via a USB to Serial adaptor
  
  //Timer 4 for motor control
  MotorControl.pause(); //stop...
  MotorControl.setCompare(TIMER_CH3, 20); //10µs 
  MotorControl.setChannel3Mode(TIMER_OUTPUT_COMPARE);
  MotorControl.setPrescaleFactor(72); // 72Mhz, 1 = 1µs
  MotorControl.setOverflow(100); // default value 100µs overflow
  //MotorControl.setPeriod(100); //Period 100µs --> 10Khz
  MotorControl.attachCompare3Interrupt(handler_Timer4_compare3); //interrupt conmpare 3
  MotorControl.attachInterrupt(0, handler_Timer4_overflow); //Overflow interrupt  
  MotorControl.resume();
  //Restore config  
  Restore_Config();
  // Menu init, setup and draw
  menu.init();
  setupMenu();
  //menu.drawMenu(); //Start with menu screen
  ActionDro(); //Start with dro screen
  //ActionDebug(); //Start with debug screen
}
void setupMenu() {
  // Add menu items to menu page
  menuPageMain.addMenuItem(menuItemButtonDro);
  //Add Sub menu Axe
  menuPageMain.addMenuItem(menuItemAxe);
  menuPageAxe.addMenuItem(menuItemTool);
  menuPageAxe.addMenuItem(menuItemRelativeMode);
  menuPageAxe.addMenuItem(menuItemButtonResetX);
  menuPageAxe.addMenuItem(menuItemButtonResetY);
  menuPageAxe.addMenuItem(menuItemAxeXPos);
  menuPageAxe.addMenuItem(menuItemAxeYPos);
  menuPageAxe.setParentMenuPage(menuPageMain);
  //Add Sub menu Motor
  menuPageMain.addMenuItem(menuItemMotor);
  menuPageMotor.addMenuItem(menuItemUseMotor);
  menuPageMotor.addMenuItem(menuItemMotorMode);
  menuPageMotor.addMenuItem(menuItemMotorStopMin);
  menuPageMotor.addMenuItem(menuItemMotorStopMax);
  menuPageMotor.addMenuItem(menuItemUseMotorEndLimit);
  menuPageMotor.addMenuItem(menuItemMotorCurrentPos);
  menuPageMotor.addMenuItem(menuItemMotorSpeed);
  menuPageMotor.addMenuItem(menuItemMotorThread);
  menuPageMotor.addMenuItem(menuItemButtonSetPosToMax);
  menuPageMotor.addMenuItem(menuItemButtonSetPosToMin);
  menuPageMotor.addMenuItem(menuItemButtonResetCurrentPos);
  menuPageMotor.setParentMenuPage(menuPageMain);
  //Add Sub menu Settings
  menuPageMain.addMenuItem(menuItemMainSettings); 
  menuPageSettings.addMenuItem(menuItemDirX);
  menuPageSettings.addMenuItem(menuItemResoX);
  menuPageSettings.addMenuItem(menuItemDirY);
  menuPageSettings.addMenuItem(menuItemResoY);
  menuPageSettings.addMenuItem(menuItemDirZ);
  menuPageSettings.addMenuItem(menuItemResoZ);
  menuPageSettings.addMenuItem(menuItemDiamY);
  menuPageSettings.addMenuItem(menuItemDirM1);
  menuPageSettings.addMenuItem(menuItemResoM1);
  menuPageSettings.addMenuItem(menuItemThreadM1);
  menuPageSettings.addMenuItem(menuItemAccelM1);
  menuPageSettings.addMenuItem(menuItemSpeedM1);  
  menuPageSettings.addMenuItem(menuItemButtonRestoreSettings);
  menuPageSettings.addMenuItem(menuItemButtonSaveSettings);
  menuPageSettings.setParentMenuPage(menuPageMain);
  //Add item screen mode
  menuPageMain.addMenuItem(menuItemScreenMode);
  
  //Add Sub menu Debug
  menuPageMain.addMenuItem(menuItemDebug);
  menuPageDebug.addMenuItem(menuItemTestFloat);
  menuPageDebug.addMenuItem(menuItemButtonDebug); 
  menuPageDebug.setParentMenuPage(menuPageMain);
  // Add menu page to menu and set it as current
  menu.setMenuPageCurrent(menuPageMain);
}
void loop() {
  // This loop turn when i'm in the menu !
  if (menu.readyForKey()) 
  {
    Display_UpdateRealTimeData(); 
    menu.registerKeyPress(customKeypad.getKey());
  }
}
// ***************************************************************************************
// ***************************************************************************************
// *** DRO main context ******************************************************************
void ActionDro() {
  menu.context.loop = DroContextLoop;
  menu.context.enter = DroContextEnter;
  menu.context.exit = DroContextExit;
  menu.context.allowExit = false; // Setting to false will require manual exit from the loop
  menu.context.enter();
}
void DroContextEnter() {
  // Clear sreen
  u8g2.clear();
}
void DroContextLoop() 
{
  byte key = customKeypad.getKey();
  if (key == GEM_KEY_CANCEL) 
  { 
    // Exit Dro screen if GEM_KEY_CANCEL key was pressed
    menu.context.exit();
  } else 
  {
    if( bMotorMode == MOTOR_MODE_MANUAL || bMotorMode == MOTOR_MODE_AUTO )
    {
      if(key == GEM_KEY_UP)
      {
        ActionMotorSpeedUp();
        ActionMotorMotorSpeed();
      }
      if(key == GEM_KEY_DOWN)
      {
        ActionMotorSpeedDown(); 
        ActionMotorMotorSpeed();  
      }
    } 
    // Ok key for change the screen only when the motor is not in speed mode
    if(key == GEM_KEY_OK && (Motor1.ReturnTheMode()!=StepperMotor::SpeedModeUp))
    {
      if(key == GEM_KEY_OK && (Motor1.ReturnTheMode()!=StepperMotor::SpeedModeDown))
      {
        ActionChangeScreen();
      }  
    }
    //**** Manual mode Key *****
    if( bMotorMode == MOTOR_MODE_MANUAL)
    {
      if( customKeypad.isPressed(GEM_KEY_LEFT) || customKeypad.isPressed(GEM_KEY_RIGHT))
      {
        eScreenChoose = SCREEN_MOT1;
        //eScreenChoose = SCREEN_DEBUG;
        if( customKeypad.isPressed(GEM_KEY_LEFT))Motor1.ChangeTheMode(StepperMotor::SpeedModeUp);
        if( customKeypad.isPressed(GEM_KEY_RIGHT))Motor1.ChangeTheMode(StepperMotor::SpeedModeDown);
      }
      else Motor1.ChangeTheMode(StepperMotor::NoMode);
      //Fast Speed with OK pressed
      if(Motor1.ReturnTheMode()!=StepperMotor::SpeedModeUp || Motor1.ReturnTheMode()!=StepperMotor::SpeedModeDown)
      {
        if(customKeypad.isPressed(GEM_KEY_OK))Motor1.ChangeMaxSpeed(ConfigDro.Speed_M1);
        else Motor1.ChangeMaxSpeed(iMotorSpeed);   
      } 
    }
    //**** Auto mode Key *****
    if (bMotorMode == MOTOR_MODE_AUTO)
    {
      if (key == GEM_KEY_LEFT ) 
      {
        eScreenChoose = SCREEN_MOT1;
        if( Motor1.ReturnTheMode() == StepperMotor::NoMode ) Motor1.ChangeTheMode(StepperMotor::SpeedModeUp);
        else Motor1.ChangeTheMode(StepperMotor::NoMode);      
      }  
      if (key == GEM_KEY_RIGHT ) 
      { 
        eScreenChoose = SCREEN_MOT1;
        if( Motor1.ReturnTheMode() == StepperMotor::NoMode ) Motor1.ChangeTheMode(StepperMotor::SpeedModeDown);
        else Motor1.ChangeTheMode(StepperMotor::NoMode);  
      }
      //Fast Speed with OK pressed
      if(Motor1.ReturnTheMode()!=StepperMotor::SpeedModeUp || Motor1.ReturnTheMode()!=StepperMotor::SpeedModeDown)
      {
        if(customKeypad.isPressed(GEM_KEY_OK))Motor1.ChangeMaxSpeed(ConfigDro.Speed_M1);
        else Motor1.ChangeMaxSpeed(iMotorSpeed);   
      }    
     } 
     //**** Left threading mode Key *****
    if (bMotorMode == MOTOR_MODE_LEFT)
    {
      switch(eMS_Thread)
      {
        case MS_THREAD_IDLE:
          //No action here
        break;
        case MS_THREAD_WAIT_THE_START:
          if (key == GEM_KEY_LEFT ) eMS_Thread = MS_THREAD_WAIT_THE_SPLINDLE_ZERO;   
        break;
        case MS_THREAD_WAIT_THE_SPLINDLE_ZERO:
          //No action here
        break;
        case MS_THREAD_IN_THREAD:
          if ( Motor1.AreYouAtMaxPos() )
          {
            eMS_Thread = MS_THREAD_END_THREAD;
            Motor1.ChangeTheMode(StepperMotor::NoMode);    
          }   
        break;
        case MS_THREAD_END_THREAD:
          if (key == GEM_KEY_RIGHT )
          {
            eMS_Thread = MS_THREAD_IN_RETURN;
            Motor1.ChangeTheMode(StepperMotor::SpeedModeDown);    
          }
        break;
        case MS_THREAD_IN_RETURN:
          if ( Motor1.AreYouAtMinPos() )
          {
            eMS_Thread = MS_THREAD_WAIT_THE_START;
            Motor1.ChangeTheMode(StepperMotor::PositionMode);    
          }          
        break;   
      }
    }     
    DisplayDrawInformations();
    UsbSerial_Pos();   
  }
}
void DroContextExit() 
{
  menu.reInit();
  menu.drawMenu();
  menu.clearContext();
}
// ***************************************************************************************
// ***************************************************************************************
// *** Debug context *********************************************************************
void ActionDebug()
{
  menu.context.loop = DebugContextLoop;
  menu.context.enter = DebugContextEnter;
  menu.context.exit = DebugContextExit;
  menu.context.allowExit = false; // Setting to false will require manual exit from the loop
  menu.context.enter();    
}
void DebugContextEnter() 
{
  // Clear sreen
  u8g2.clear();
}
void DebugContextLoop() 
{
  byte key = customKeypad.getKey();
  u8g2.firstPage();
  do {
      u8g2.setColorIndex(1);
      u8g2.setFont(u8g2_font_profont10_mr); // choose a suitable font
      CalcMotorParameterForThread();
      char buffer[16];
      sprintf(buffer,"Numerator:%u",sThreadCalc.Numerator);
      u8g2.drawStr(0,0,buffer);
      sprintf(buffer,"Denominator:%u",sThreadCalc.Denominator);
      u8g2.drawStr(0,10,buffer);      
  } while (u8g2.nextPage());
  if (key == GEM_KEY_CANCEL) 
  { 
    menu.context.exit();
  }
}
void DebugContextExit() 
{
  Motor1.MotorChangePowerState(false);
  menu.reInit();
  menu.drawMenu();
  menu.clearContext();
}

// ***************************************************************************************
// ***************************************************************************************
// *** Save / Restore config *************************************************************
void Restore_Config()
{
  //Read Config in Memory
  ReadConfigInFlash(&ConfigDro);
  //Dispatch the config
  Dispatch_Config(&ConfigDro);
}
void SaveConfigInFlash(sConfigDro *pConf)
{
  unsigned int uiCount;
  char *pt;
  EEPROM.format();
  pt = (char*)pConf; 
  for(uiCount=0;uiCount<sizeof(sConfigDro);uiCount++)
  {
    EEPROM.write(uiCount,*pt);
    pt++;  
  } 
}
void ReadConfigInFlash(sConfigDro *pConf)
{
  unsigned int uiCount;
  uint16 uiState;
  uint16 value;
  char *pt;
  uiState = EEPROM_OK;
  pt = (char*)pConf; 
  for(uiCount=0;uiCount<sizeof(sConfigDro);uiCount++)
  {
    uiState |= EEPROM.read(uiCount,&value);
    *pt = (char) value;
    pt++;  
  }
  if(uiState != EEPROM_OK)
  {
    //Problem, restore default  
    *pConf = csConfigDefault;
  } 
}
void Dispatch_Config(sConfigDro *pConf)
{
  Quad_X.SetSens( pConf->Inverted_X );  
  Quad_Y.SetSens( pConf->Inverted_Y );
  Quad_Z.SetSens( pConf->Inverted_Z );
  Quad_Y.SetDiameterMode(pConf->Diameter_Mode_Y);
  Quad_X.SetResolution(pConf->Reso_X);
  Quad_Y.SetResolution(pConf->Reso_Y);
  Quad_Z.SetResolution(pConf->Reso_Z);
  Motor1.ChangeParameter((unsigned int)((long)(  pConf->Reso_M1*100/pConf->thread_M1)) , pConf->Inverted_M1);
  Motor1.ChangeAcceleration(ConfigDro.Accel_M1); 
}
void ActionSaveSettingsInFlash()
{
  //Store config in memort
  SaveConfigInFlash(&ConfigDro);
  //Dispatch config to function
  Dispatch_Config(&ConfigDro); 
  //PrintInformationOnScreen("Save in flash");
  //delay(100);   
}
void ActionRestoreSettingsInFlash()
{
  //Save default config in flash
  SaveConfigInFlash((sConfigDro*)&csConfigDefault);
  Restore_Config();  
}


// ***************************************************************************************
// ***************************************************************************************
// *** Usb Serial functions *****************************************************************
void UsbSerial_Pos()
{
  char bufferChar[30];
  if(Serial.isConnected())
  {
    sprintf(bufferChar,"X%0.3f:",fAxeXPos); 
    Serial.print(bufferChar);
    sprintf(bufferChar,"Y%0.3f",fAxeYPos); 
    Serial.print(bufferChar);
    Serial.print("\n");   
  }

  //Serial.print(':');
}
// ***************************************************************************************
// ***************************************************************************************
// *** Display functions *****************************************************************


void Display_X_Informations(); //Forward declarations
void Display_Y_Informations(); //Forward declarations
void Display_C_Informations(); //Forward declarations
void Display_M_Informations(); //Forward declarations
void Display_Extra_Informations(); //Forward declarations
void Display_Debug_Informations(); //Forward declarations

void Display_StartScreen() 
{
  u8g2.firstPage();
  u8g2.setFontPosTop();
  do 
  {
    u8g2.drawXBMP((u8g2.getDisplayWidth() - logoMiniThread_width) / 2, (u8g2.getDisplayHeight() - logoMiniThread_height) / 2, logoMiniThread_width, logoMiniThread_height, logoMiniThread_bits);
  } while (u8g2.nextPage());  
  delay(500); 
  do 
  {
    u8g2.setFont(u8g2_font_6x12_tr); // choose a suitable font
    u8g2.drawStr(0,55,TEXT_AUTHOR_SOFT);
    u8g2.drawStr(80,55,TEXT_VERSION_SOFT);   
  } while (u8g2.nextPage()); 
  delay(2000);
  u8g2.firstPage();
  do 
  {
    u8g2.setCursor(0,0);
    u8g2.drawStr(0,0,"Use this system");
    u8g2.drawStr(0,10,"at your own risk !");
  } while (u8g2.nextPage());
  delay(4000); 
}

void DisplayDrawInformations()
{
  u8g2.firstPage();
  u8g2.setFontPosTop();
  do 
  {
    Display_UpdateRealTimeData();
    if( eScreenChoose == SCREEN_DRO )
    {
      Display_X_Informations();
      Display_Y_Informations();    
      Display_C_Informations();
      Display_Extra_Informations();
    }
    else if( eScreenChoose == SCREEN_MOT1 )  
    {
      Display_X_Informations();
      Display_Y_Informations();    
      Display_M_Informations();
      Display_Extra_Informations();      
    }
    else if( eScreenChoose == SCREEN_DEBUG )  
    {
      Display_Debug_Informations(); 
    }   
  } while (u8g2.nextPage());
}

void Display_X_Informations()
{
  char bufferChar[16];
  u8g2.setColorIndex(1);
  u8g2.setFont(u8g2_font_profont22_tf); // choose a suitable font
  u8g2.drawStr(0,1,"X");
  u8g2.setColorIndex(1);
  sprintf(bufferChar,"%+09.3f",fAxeXPos);  
  u8g2.drawStr(13,1,bufferChar);  // write something to the internal memory
  u8g2.drawRFrame(11,0,116,18,3);  
}
void Display_Y_Informations()
{
  char bufferChar[16];
  u8g2.setColorIndex(1);
  u8g2.setFont(u8g2_font_profont22_tf); // choose a suitable font
  u8g2.drawStr(0,19,"Y");
  sprintf(bufferChar,"%+09.3f",fAxeYPos);
  u8g2.drawStr(13,19,bufferChar);  // write something to the internal memory
  u8g2.drawRFrame(11,18,116,18,3);    
}
void Display_C_Informations()
{
  char bufferChar[16];
  u8g2.setColorIndex(1); 
  u8g2.setFont(u8g2_font_profont22_tf); // choose a suitable font 
  u8g2.drawStr(0,37,"C");
  sprintf(bufferChar,"%+5.5d",Quad_Z.GiveMeTheSpeed());
  u8g2.drawStr(13,37,bufferChar);  // write something to the internal memory
  u8g2.setFont(u8g2_font_profont10_mr); // choose a suitable font
  sprintf(bufferChar,"%07.3f",(float)Quad_Z.GetValuePos()/ConfigDro.Reso_Z*360.0);
  u8g2.drawStr(85,37,bufferChar);  // write something to the internal memory
  u8g2.drawStr(85,45,"tr/min");
  u8g2.drawRFrame(11,36,116,18,3);    
}
void Display_M_Informations()
{
  char bufferChar[30];
  u8g2.drawStr(0,37,"M");
  u8g2.setColorIndex(1);
  if( bUseMotor == true )
  {
    u8g2.setFont(u8g2_font_profont10_mr); // choose a suitable font
    sprintf(bufferChar,"%+09.3f",fMotorCurrentPos);
    u8g2.drawStr(13,37,bufferChar);  // write something to the internal memory
    switch ( bMotorMode )
    {
      case MOTOR_MODE_NO_MODE:
        u8g2.drawStr(57,37,"|NO");
      break;
      case MOTOR_MODE_MANUAL:
        u8g2.drawStr(57,37,"|Manual");
      break;
      case MOTOR_MODE_AUTO:
        u8g2.drawStr(57,37,"|Auto");
      break;
      case MOTOR_MODE_LEFT:
        u8g2.drawStr(57,37,"|Left"); 
      break;   
    }
    //Motor speed
    //if motor is Left mode, display the speed from the settings (Max speed )
    sprintf(bufferChar,"|%d", bMotorMode == MOTOR_MODE_LEFT ? ConfigDro.Speed_M1 : iMotorSpeed);
    //sprintf(bufferChar,"|%d",iMotorSpeed);
    u8g2.drawStr(90,37,bufferChar);    
    
    //End limit 
    sprintf(bufferChar,"%+09.3f <> %+09.3f",fMotorStopMax,fMotorStopMin);
    if(bUseMotorEndLimit)u8g2.drawStr(13,45,bufferChar);
    else u8g2.drawStr(13,45," WARNING : No limit");       
  }
  else
  {
    u8g2.setFont(u8g2_font_profont10_mr); // choose a suitable font
    u8g2.drawStr(13,37,"Motor is OFF");   
  }  
  u8g2.drawRFrame(11,36,116,18,3);    
}
void Display_Extra_Informations()
{
  char bufferChar[10];
  u8g2.setFont(u8g2_font_profont10_mr); // choose a suitable font
  u8g2.drawStr(0,54,selectTool.getSelectedOptionName((byte*)&ToolChoose ));
  //Display thread Mastersate 
  if (bMotorMode == MOTOR_MODE_LEFT)
  {
    switch(eMS_Thread)
    {
      case MS_THREAD_IDLE:
        u8g2.drawStr(30,54,"|Idle");
      break;  
      case MS_THREAD_WAIT_THE_START:
        u8g2.drawStr(30,54,"|Wait left sw");
      break;  
      case MS_THREAD_WAIT_THE_SPLINDLE_ZERO:
        u8g2.drawStr(30,54,"|Wait zero s");
      break;  
      case MS_THREAD_IN_THREAD:
        u8g2.drawStr(30,54,"|In thread");
      break;  
      case MS_THREAD_END_THREAD:
        u8g2.drawStr(30,54,"|Wait right sw");
      break;             
      case MS_THREAD_IN_RETURN:
        u8g2.drawStr(30,54,"|In return");
      break;      
    }    
  }
  //Display Abs / relative for axe X and Y 
  if(RelativeMode==true)u8g2.drawStr(108,54,"|Rel");
  else u8g2.drawStr(108,54,"|Abs");  
}
void Display_UpdateRealTimeData()
{
  fMotorCurrentPos = Motor1.GetPositionReal(); 
  fAxeXPos = Quad_X.GetValue();
  fAxeYPos = Quad_Y.GetValue();     
}
void Display_Debug_Informations()
{
  char bufferChar[30];
  u8g2.setFont(u8g2_font_profont10_mr); // choose a suitable font
  u8g2.drawStr(0,0,"Debug page !");
  sprintf(bufferChar,"I:%d",Motor1.NewInterval());
  u8g2.drawStr(0,9,bufferChar);  // write something to the internal memory
  sprintf(bufferChar,"_n:%ld",Motor1._n);
  u8g2.drawStr(0,18,bufferChar);  // write something to the internal memory
}



// ***************************************************************************************
// ***************************************************************************************
// *** Action functions from menu ********************************************************

void ActionChangeRelaticeMode()
{  
  if( RelativeMode == true )
  {
    Quad_X.SetRelative();  
    Quad_Y.SetRelative();
    //Quad_Z.SetRelative();
  }
  else
  {
    Quad_X.SetAbsolut();  
    Quad_Y.SetAbsolut();
    //Quad_Z.SetAbsolut();
  }      
}

void applyTool()
{
    
}
void ActionUseMotor()
{
  if( bUseMotor == true ) 
  {
    Motor1.ChangeTheMode(StepperMotor::NoMode);
    bMotorMode = MOTOR_MODE_NO_MODE; 
    ActionMotorStopMin();
    ActionMotorStopMax(); 
    ActionMotorCurrentPos();
    ActionMotorMotorSpeed();
    Motor1.UseEndLimit(bUseMotorEndLimit);
    Motor1.MotorChangePowerState(true);
    eMS_Thread = MS_THREAD_IDLE;
    eScreenChoose = SCREEN_MOT1; 
  }
  else
  {
    Motor1.ChangeTheMode(StepperMotor::NoMode);
    bMotorMode = MOTOR_MODE_NO_MODE; 
    Motor1.MotorChangePowerState(false);
    eMS_Thread = MS_THREAD_IDLE;
    eScreenChoose = SCREEN_DRO;   
  }     
}
void ActionUseMotorEndLimit()
{
  Motor1.UseEndLimit(bUseMotorEndLimit);  
}
//Returns the greatest common divisor of two integers
long GCD_Function ( long n1, long n2)
{
  long i,result;
  for(i=1;i<=n1 && i<=n2;i++)
  {
    if(n1%i==0 && n2%i==0)result = i;  
  }
  return result;     
}
void CalcMotorParameterForThread()
{
  long lnumber;
  sThreadCalc.Numerator = ConfigDro.Reso_M1 * iMotorThread;  
  sThreadCalc.Denominator = ConfigDro.thread_M1 * ConfigDro.Reso_Z ; 
  lnumber = GCD_Function(sThreadCalc.Numerator,sThreadCalc.Denominator);
  sThreadCalc.Numerator = sThreadCalc.Numerator / lnumber;
  sThreadCalc.Denominator = sThreadCalc.Denominator / lnumber;   
  //sThreadCalc.Numerator = 16 * iMotorThread;  
  //sThreadCalc.Denominator = 2400; 
  sThreadCalc.Offset = Motor1.GetStopPositionMinStep() ;  
}
void applyMotorMode()
{
  switch (bMotorMode)
  {
    case MOTOR_MODE_LEFT :
      //Left thread
      if( bUseMotorEndLimit && Motor1.AreYouAtMinPos())
      {
        //Need to have end limit and position at min pos to start
        eMS_Thread = MS_THREAD_WAIT_THE_START;
        //Calcul the motor parameter for Thread
        CalcMotorParameterForThread();
        //Use Max speed in the setting
        Motor1.ChangeMaxSpeed(ConfigDro.Speed_M1); 
        //Motor in position mode
        Motor1.ChangeTheMode(StepperMotor::PositionMode);    
      }
      else
      {
        bMotorMode = MOTOR_MODE_NO_MODE;  
      } 
    break; 
    case MOTOR_MODE_NO_MODE :
    case MOTOR_MODE_MANUAL :
    case MOTOR_MODE_AUTO :
    default:
      eMS_Thread = MS_THREAD_IDLE;
      ActionMotorMotorSpeed();
      Motor1.ChangeTheMode(StepperMotor::NoMode);  
    break;
  }  
}
void ActionMotorStopMin()
{
  Motor1.ChangeStopPositionMinReal(fMotorStopMin);  
}
void ActionMotorStopMax()
{
  Motor1.ChangeStopPositionMaxReal(fMotorStopMax);  
}
void ActionMotorCurrentPos()
{
  Motor1.ChangeCurrentPositionReal(fMotorCurrentPos);    
}
void ActionMotorSpeedUp()
{
  iMotorSpeed = iMotorSpeed+100;  
}
void ActionMotorSpeedDown()
{
  if(iMotorSpeed>100) iMotorSpeed = iMotorSpeed - 100;  
}
void ActionMotorMotorSpeed()
{
  if(iMotorSpeed<1)iMotorSpeed=1;
  if(iMotorSpeed>30000)iMotorSpeed=30000;
  Motor1.ChangeMaxSpeed(iMotorSpeed);    
}
void ActionMotorChangeThread()
{
  if(iMotorThread<=0)iMotorThread = 100;  
}
void ActionSetCurrentToMax()
{
  Display_UpdateRealTimeData();
  fMotorStopMax = fMotorCurrentPos;
  ActionMotorStopMax();    
}
void ActionSetCurrentToMin()
{
  Display_UpdateRealTimeData();
  fMotorStopMin = fMotorCurrentPos;
  ActionMotorStopMin();   
}
void ActionResetCurrentPos()
{
  fMotorCurrentPos = 0;
  ActionMotorCurrentPos();  
}
void ActionResetX()
{
  Quad_X.SetZeroActiveMode(); 
  ActionDro();
}
void ActionResetY()
{
  Quad_Y.SetZeroActiveMode();
  ActionDro();  
}
void ActionAxeXPos()
{
  Quad_X.SetValue(fAxeXPos);    
}
void ActionAxeYPos()
{
  Quad_Y.SetValue(fAxeYPos);   
}
void ActionScreenMode()
{
  
}
void ActionChangeScreen()
{
  if(eScreenChoose>=SCREEN_END_LIST)eScreenChoose = SCREEN_DRO;
  else eScreenChoose++; 
}
