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
#include "src/Various/Various.h"

#include <EEPROM.h>

#define TEXT_MAIN_MENU_TITLE "MiniThread"
#define TEXT_AUTHOR_SOFT "Pailpoe"
#define TEXT_VERSION_SOFT "1.0.0 Dev"

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

//Keyboard
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



//Struct and Enum def  ******************************************
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
} tsConfigDro;
const tsConfigDro csConfigDefault = {false,false,false,true,512,512,1200,false,1600,200,60000.0,12000};

//Threading machine state
typedef enum
{
  MS_THREAD_IDLE = 0, //Idle
  MS_THREAD_WAIT_THE_START = 1, //Wait the action to start
  MS_THREAD_WAIT_THE_SPLINDLE_ZERO = 2, // Wait spindle zero
  MS_THREAD_IN_THREAD = 3, // In threat
  MS_THREAD_END_THREAD = 4, // Wait the button to return
  MS_THREAD_IN_RETURN = 5 // In return
} teMS_ThreadingMode;

//Parameter for the thread
typedef struct
{
  long Numerator;
  long Denominator;
  long Offset;  
} tsThreadCalc;

//Screen choose
#define  SCREEN_DRO 0
#define  SCREEN_MOT1 1
#define  SCREEN_DEBUG 2
#define  SCREEN_END_LIST 1

//Motor mode
#define MOTOR_MODE_NO_MODE 0
#define MOTOR_MODE_MANUAL  1
#define MOTOR_MODE_AUTO    2
#define MOTOR_MODE_LEFT    3

// Variables global ******************************************
tsConfigDro  sGeneralConf;
float       TestFloat = 999.2;
byte        bToolChoose = 0; //Tool selection
boolean     bRelativeModeActived = false; //Relative or absolute mode
float       fAxeXPos = 0; // X position
float       fAxeYPos = 0; // Y position
float       fAxeCSpeed = 0; // C speed
byte        bMotorMode = 0;
boolean     bUseMotor = false;
float       fMotorStopMin = 0.0;
float       fMotorStopMax = 200.0;
boolean     bUseMotorEndLimit = true;
float       fMotorCurrentPos = 0;
int         iMotorSpeed = 1000;
int         iMotorThread = 100;
float       fMotor1ThreadOffset = 0.0;
boolean     bMotor1ThreadUseY = false;
float       fMotor1ThreadDiameter = 0.0;
float       fMotor1ThreadAngle = 30.0;
float       fM1ActualSpeed; // Motor Actual Speed
float       fM1MaxThreadSpeed; // Motor Max spindle speed for thread
byte        eScreenChoose = SCREEN_DRO;
teMS_ThreadingMode  eMS_Thread = MS_THREAD_IDLE;
tsThreadCalc sThreadCalc; 

// Forward declarations Funtions  ******************************************
void CalcMotorParameterForThread(); 
void CalcMotorMaxSpeedForThread();
void UsbSerial_Pos(); 
void Display_UpdateRealTimeData(); 
void ActionMotorSpeedUp();
void ActionMotorSpeedDown();
void Display_StartScreen(); 
void ActionRestoreSettingsInFlash(); 
void ActionSaveSettingsInFlash(); 
void ActionDro(); 
void ActionDebug(); 
void applyTool(); 
void ActionChangeRelaticeMode();
void ActionResetX(); 
void ActionResetY(); 
void ActionAxeXPos(); 
void ActionAxeYPos(); 
void ActionUseMotor(); 
void applyMotorMode(); 
void ActionMotorStopMin(); 
void ActionMotorStopMax(); 
void ActionUseMotorEndLimit(); 
void ActionMotorCurrentPos(); 
void ActionMotorMotorSpeed(); 
void ActionSetCurrentToMax(); 
void ActionSetCurrentToMin(); 
void ActionResetCurrentPos(); 
void ActionMotorChangeThread(); 
void ActionChangeMotor1Offset(); 
void ActionMotor1ThreadUseY();
void ActionChangeMotor1ThreadDiameter(); 
void ActionChangeMotor1ThreadAngle(); 
boolean M1_AreYouOkToStartTheThread();
boolean M1_AreYouOkToReturnAfterThread();
void ActionScreenMode(); 
void ActionChangeScreen();
void IT_Timer1_Overflow(); 
void IT_Timer2_Overflow(); 
void IT_Timer3_Overflow(); 
void Update_Overlfow_Timer4();
void Display_X_Informations(); 
void Display_Y_Informations();
void Display_C_Informations();
void Display_M_Informations();
void Display_Extra_Informations();
void Display_Debug_Informations();
void Display_Notice_Informations(char* str);

//Menu item ******************************************
GEMPage menuPageSettings("Settings"); // Settings submenu
GEMItem menuItemMainSettings("Settings", menuPageSettings);
GEMItem menuItemDirX("X dir:", sGeneralConf.Inverted_X);
GEMItem menuItemDirY("Y dir:", sGeneralConf.Inverted_Y);
GEMItem menuItemDirZ("C dir:", sGeneralConf.Inverted_Z);
GEMItem menuItemDiamY("Y diameter:", sGeneralConf.Diameter_Mode_Y);
GEMItem menuItemResoX("X step/mm:", sGeneralConf.Reso_X);
GEMItem menuItemResoY("Y step/mm:", sGeneralConf.Reso_Y);
GEMItem menuItemResoZ("C step/tr:", sGeneralConf.Reso_Z);
GEMItem menuItemDirM1("M1 dir:", sGeneralConf.Inverted_M1);
GEMItem menuItemResoM1("M1 step/tr:", sGeneralConf.Reso_M1);
GEMItem menuItemThreadM1("M1 thread:", sGeneralConf.thread_M1);
GEMItem menuItemAccelM1("M1 accel:", sGeneralConf.Accel_M1);
GEMItem menuItemSpeedM1("M1 speed:", sGeneralConf.Speed_M1);
GEMItem menuItemButtonRestoreSettings("Restore settings", ActionRestoreSettingsInFlash);
GEMItem menuItemButtonSaveSettings("Save settings", ActionSaveSettingsInFlash);
GEMItem menuItemButtonDro("Return to Screen", ActionDro);
GEMPage menuPageMain(TEXT_MAIN_MENU_TITLE);
GEMPage menuPageDebug("Debug tools"); // Debug submenu
GEMItem menuItemDebug("Debug tools", menuPageDebug);
GEMItem menuItemButtonDebug("Debug screen", ActionDebug);
GEMItem menuItemTestFloat("Float:", TestFloat);
GEMPage menuPageAxe("Axe Functions"); // Axe submenu
GEMItem menuItemAxe("Axe Functions", menuPageAxe);
SelectOptionByte selectToolOptions[] = {{"Tool_0", 0}, {"Tool_1", 1}, {"Tool_2", 2}, {"Tool_3", 3}, {"Tool_4", 4}, {"Tool_5", 5}};
GEMSelect selectTool(sizeof(selectToolOptions)/sizeof(SelectOptionByte), selectToolOptions);
GEMItem menuItemTool("Tool:", bToolChoose, selectTool, applyTool);
GEMItem menuItemRelativeMode("Relative:", bRelativeModeActived,ActionChangeRelaticeMode);
GEMItem menuItemButtonResetX("Set X to zero", ActionResetX);
GEMItem menuItemButtonResetY("Set Y to zero", ActionResetY);
GEMItem menuItemAxeXPos("X Pos:", fAxeXPos,ActionAxeXPos);
GEMItem menuItemAxeYPos("Y Pos:", fAxeYPos,ActionAxeYPos);
GEMPage menuPageMotor("Motor Functions"); // Motor submenu
GEMItem menuItemMotor("Motor Functions", menuPageMotor);
GEMItem menuItemUseMotor("Use motor:", bUseMotor,ActionUseMotor);
SelectOptionByte selectMotorModeOptions[] = {{"NoMode", 0}, {"Manual", 1},{"Auto", 2}, {"Left", 3}};
GEMSelect selectMotorMode(sizeof(selectMotorModeOptions)/sizeof(SelectOptionByte), selectMotorModeOptions);
GEMItem menuItemMotorMode("Motor mode:", bMotorMode, selectMotorMode, applyMotorMode);
GEMItem menuItemMotorStopMin("Stop Min:", fMotorStopMin,ActionMotorStopMin);
GEMItem menuItemMotorStopMax("Stop Max:", fMotorStopMax,ActionMotorStopMax);
GEMItem menuItemUseMotorEndLimit("Use limit:", bUseMotorEndLimit,ActionUseMotorEndLimit);
GEMItem menuItemMotorCurrentPos("CurrentPos:", fMotorCurrentPos,ActionMotorCurrentPos);
GEMItem menuItemMotorSpeed("Speed:", iMotorSpeed,ActionMotorMotorSpeed);
GEMItem menuItemButtonSetPosToMax("CurrentPos -> Max", ActionSetCurrentToMax);
GEMItem menuItemButtonSetPosToMin("CurrentPos -> Min", ActionSetCurrentToMin);
GEMItem menuItemButtonResetCurrentPos("Reset CurrentPos", ActionResetCurrentPos);
GEMPage menuPageThreadParameters("Thread parameters"); // Thread parameters submenu
GEMItem menuItemThreadParameters("Thread parameters", menuPageThreadParameters);
GEMItem menuItemMotorThread("Thread:", iMotorThread,ActionMotorChangeThread);
GEMItem menuItemMotor1ThreadOffset("Offset:", fMotor1ThreadOffset,ActionChangeMotor1Offset);
GEMItem menuItemMotor1ThreadUseY("Use Y:", bMotor1ThreadUseY,ActionMotor1ThreadUseY);
GEMItem menuItemMotor1ThreadDiameter("Diameter:", fMotor1ThreadDiameter,ActionChangeMotor1ThreadDiameter);
GEMItem menuItemMotor1ThreadAngle("Angle:", fMotor1ThreadAngle,ActionChangeMotor1ThreadAngle);
GEMItem menuItemMotor1ThreadInfo("Speed max:", fM1MaxThreadSpeed,true);
SelectOptionByte selectScreenOptions[] = {{"DroXYC", 0}, {"Mot1", 1}, {"Debug", 2}};
GEMSelect selectScreenMode(sizeof(selectScreenOptions)/sizeof(SelectOptionByte), selectScreenOptions);
GEMItem menuItemScreenMode("Screen:", eScreenChoose, selectScreenMode, ActionScreenMode);

//Class instance ******************************************
GEM_u8g2 menu(u8g2,GEM_POINTER_ROW,5,10,10,75); // menu
QuadDecoder Quad_Y(3,QuadDecoder::LinearEncoder,512,false,false,IT_Timer3_Overflow); //Quad Y with timer 3
QuadDecoder Quad_Z(2,QuadDecoder::RotaryEncoder,1200,true,false,IT_Timer2_Overflow); //Quad Z with timer 2
QuadDecoder Quad_X(1,QuadDecoder::LinearEncoder,512,false,false,IT_Timer1_Overflow); //Quad X with timer 1
HardwareTimer MotorControl(4);  //for motor control with timer 4
StepperMotor Motor1(800,false,PIN_MOT1_STEP,PIN_MOT1_DIR,PIN_MOT1_EN, Update_Overlfow_Timer4);// Motor 1

//Interrupt handler functions ******************************************
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
  //Serial.begin(115200); // Ignored by Maple. But needed by boards using hardware serial via a USB to Serial adaptor
  
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
  ActionDro(); //Start with dro screen
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
  //Create sub menu Thread parameter form menu Motor
  menuPageThreadParameters.setParentMenuPage(menuPageMotor);
  menuPageThreadParameters.addMenuItem(menuItemMotorThread);
  menuPageThreadParameters.addMenuItem(menuItemMotor1ThreadInfo);  
  menuPageThreadParameters.addMenuItem(menuItemMotor1ThreadOffset);
  menuPageThreadParameters.addMenuItem(menuItemMotor1ThreadUseY);
  menuPageThreadParameters.addMenuItem(menuItemMotor1ThreadDiameter);
  menuPageThreadParameters.addMenuItem(menuItemMotor1ThreadAngle);
  //Add Sub menu Motor
  menuPageMain.addMenuItem(menuItemMotor);
  menuPageMotor.addMenuItem(menuItemUseMotor);
  menuPageMotor.addMenuItem(menuItemMotorMode);
  menuPageMotor.addMenuItem(menuItemMotorStopMin);
  menuPageMotor.addMenuItem(menuItemMotorStopMax);
  menuPageMotor.addMenuItem(menuItemUseMotorEndLimit);
  menuPageMotor.addMenuItem(menuItemMotorCurrentPos);
  menuPageMotor.addMenuItem(menuItemMotorSpeed);
  menuPageMotor.addMenuItem(menuItemThreadParameters); //Sub menu thread parameter
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
        if(customKeypad.isPressed(GEM_KEY_OK))Motor1.ChangeMaxSpeed(sGeneralConf.Speed_M1);
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
        if(customKeypad.isPressed(GEM_KEY_OK))Motor1.ChangeMaxSpeed(sGeneralConf.Speed_M1);
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
          if (key == GEM_KEY_LEFT )
          {
            if(M1_AreYouOkToStartTheThread() == true)
            {
              //Calcul the motor parameter for Thread before start
              CalcMotorParameterForThread();
              eMS_Thread = MS_THREAD_WAIT_THE_SPLINDLE_ZERO;                
            }
          }   
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
            if(M1_AreYouOkToReturnAfterThread() == true)
            {
              eMS_Thread = MS_THREAD_IN_RETURN;
              Motor1.ChangeTheMode(StepperMotor::SpeedModeDown);              
            }
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
      sprintf(buffer,"Numerator:%ld",sThreadCalc.Numerator);
      u8g2.drawStr(0,0,buffer);
      sprintf(buffer,"Denominator:%ld",sThreadCalc.Denominator);
      u8g2.drawStr(0,10,buffer);
      sprintf(buffer,"Offset:%ld",sThreadCalc.Offset);
      u8g2.drawStr(0,20,buffer);  
      CalcMotorMaxSpeedForThread();
      sprintf(buffer,"speed:%f",fM1MaxThreadSpeed);
      u8g2.drawStr(0,30,buffer);  

           
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
  ReadConfigInFlash(&sGeneralConf);
  //Dispatch the config
  Dispatch_Config(&sGeneralConf);
}
void SaveConfigInFlash(tsConfigDro *pConf)
{
  unsigned int uiCount;
  char *pt;
  EEPROM.format();
  pt = (char*)pConf; 
  for(uiCount=0;uiCount<sizeof(tsConfigDro);uiCount++)
  {
    EEPROM.write(uiCount,*pt);
    pt++;  
  } 
}
void ReadConfigInFlash(tsConfigDro *pConf)
{
  unsigned int uiCount;
  uint16 uiState;
  uint16 value;
  char *pt;
  uiState = EEPROM_OK;
  pt = (char*)pConf; 
  for(uiCount=0;uiCount<sizeof(tsConfigDro);uiCount++)
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
void Dispatch_Config(tsConfigDro *pConf)
{
  Quad_X.SetSens( pConf->Inverted_X );  
  Quad_Y.SetSens( pConf->Inverted_Y );
  Quad_Z.SetSens( pConf->Inverted_Z );
  Quad_Y.SetDiameterMode(pConf->Diameter_Mode_Y);
  Quad_X.SetResolution(pConf->Reso_X);
  Quad_Y.SetResolution(pConf->Reso_Y);
  Quad_Z.SetResolution(pConf->Reso_Z);
  Motor1.ChangeParameter((unsigned int)((long)(  pConf->Reso_M1*100/pConf->thread_M1)) , pConf->Inverted_M1);
  Motor1.ChangeAcceleration(pConf->Accel_M1);
  CalcMotorMaxSpeedForThread(); 
}
void ActionSaveSettingsInFlash()
{
  //Store config in memort
  SaveConfigInFlash(&sGeneralConf);
  //Dispatch config to function
  Dispatch_Config(&sGeneralConf); 
  //PrintInformationOnScreen("Save in flash");
  //delay(100);   
}
void ActionRestoreSettingsInFlash()
{
  //Save default config in flash
  SaveConfigInFlash((tsConfigDro*)&csConfigDefault);
  Restore_Config();  
}


// ***************************************************************************************
// ***************************************************************************************
// *** Usb Serial functions *****************************************************************
void UsbSerial_Pos()
{
  /*
  char bufferChar[30];
  if(Serial.isConnected())
  {
    sprintf(bufferChar,"X%0.3f:",fAxeXPos); 
    Serial.print(bufferChar);
    sprintf(bufferChar,"Y%0.3f",fAxeYPos); 
    Serial.print(bufferChar);
    Serial.print("\n");   
  }
  */

  //Serial.print(':');
}
// ***************************************************************************************
// ***************************************************************************************
// *** Display functions *****************************************************************
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
    u8g2.drawStr(70,55,TEXT_VERSION_SOFT);   
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
  sprintf(bufferChar,"%+5.5d",(int)fAxeCSpeed);
  u8g2.drawStr(13,37,bufferChar);  // write something to the internal memory
  u8g2.setFont(u8g2_font_profont10_mr); // choose a suitable font
  sprintf(bufferChar,"%07.3f",(float)Quad_Z.GetValuePos()/sGeneralConf.Reso_Z*360.0);
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
    sprintf(bufferChar,"|%d", (unsigned int)fM1ActualSpeed);
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
  u8g2.drawStr(0,54,selectTool.getSelectedOptionName((byte*)&bToolChoose ));
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
  if(bRelativeModeActived==true)u8g2.drawStr(108,54,"|Rel");
  else u8g2.drawStr(108,54,"|Abs");  
}
void Display_UpdateRealTimeData()
{
  fMotorCurrentPos = Motor1.GetPositionReal();
  fM1ActualSpeed = Motor1.GetMaxSpeed(); 
  fAxeXPos = Quad_X.GetValue();
  fAxeYPos = Quad_Y.GetValue();
  fAxeCSpeed = (float)Quad_Z.GiveMeTheSpeed();     
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
  sprintf(bufferChar,"_c0:%f",Motor1._c0);
  u8g2.drawStr(0,27,bufferChar);  // write something to the internal memory
  sprintf(bufferChar,"_cmin:%f",Motor1._cmin);
  u8g2.drawStr(0,36,bufferChar);  // write something to the internal memory
}
void Display_Notice_Informations(char* str)
{
  u8g2.firstPage();
  u8g2.setFontPosTop();
  do 
  {
    u8g2.drawRFrame(0,0 ,128,64,4);
    u8g2.setFont(u8g2_font_6x12_tr); // choose a suitable font
    u8g2.drawStr(3,3,str);
  } while (u8g2.nextPage());  
  delay(2000); 
}
// ***************************************************************************************
// ***************************************************************************************
// *** Action functions from menu ********************************************************

void ActionChangeRelaticeMode()
{  
  if( bRelativeModeActived == true )
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

void CalcMotorParameterForThread()
{
  long lnumber;
  float OffsetFixe; //Constant offset
  sThreadCalc.Numerator = sGeneralConf.Reso_M1 * iMotorThread;  
  sThreadCalc.Denominator = sGeneralConf.thread_M1 * sGeneralConf.Reso_Z ; 
  lnumber = GCD_Function(sThreadCalc.Numerator,sThreadCalc.Denominator);
  sThreadCalc.Numerator = sThreadCalc.Numerator / lnumber;
  sThreadCalc.Denominator = sThreadCalc.Denominator / lnumber;   
  //Calcul of the offset
  OffsetFixe = (float)(fMotor1ThreadOffset*iMotorThread*sGeneralConf.Reso_M1) /(float)(360*sGeneralConf.thread_M1); 
  sThreadCalc.Offset = Motor1.GetStopPositionMinStep() - (long)OffsetFixe ;  
}
void CalcMotorMaxSpeedForThread()
{
  fM1MaxThreadSpeed = (float)(sGeneralConf.Speed_M1*60.0*sGeneralConf.thread_M1/(sGeneralConf.Reso_M1*iMotorThread));  
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
        //Use Max speed in the setting
        Motor1.ChangeMaxSpeed(sGeneralConf.Speed_M1); 
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
{ //20%
  iMotorSpeed = iMotorSpeed + iMotorSpeed / 5 ; 
}
void ActionMotorSpeedDown()
{ //20%
  iMotorSpeed = iMotorSpeed - iMotorSpeed / 5 ; 
}
void ActionMotorMotorSpeed()
{
  if(iMotorSpeed < 1)iMotorSpeed=1;
  if(iMotorSpeed > sGeneralConf.Speed_M1)iMotorSpeed = sGeneralConf.Speed_M1;
  Motor1.ChangeMaxSpeed(iMotorSpeed);    
}
void ActionMotorChangeThread()
{
  if(iMotorThread<=0)iMotorThread = 100;
  CalcMotorMaxSpeedForThread();  
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
void ActionChangeMotor1Offset()
{
  if(fMotor1ThreadOffset < 0)fMotor1ThreadOffset = 0.0;
  if(fMotor1ThreadOffset > 360)fMotor1ThreadOffset = 360.0;   
}
void ActionMotor1ThreadUseY()
{
}
void ActionChangeMotor1ThreadDiameter()
{
  if(fMotor1ThreadDiameter<0) fMotor1ThreadDiameter = -fMotor1ThreadDiameter;
}
void ActionChangeMotor1ThreadAngle()
{
  if(fMotor1ThreadAngle < 0)fMotor1ThreadAngle = 0.0;
  if(fMotor1ThreadAngle > 45)fMotor1ThreadAngle = 45; 
}
boolean M1_AreYouOkToStartTheThread()
{
  boolean result = true;
  if(bMotor1ThreadUseY == true)
  {
    if( fAxeYPos > fMotor1ThreadDiameter)
    {
      result = false;
      Display_Notice_Informations("Move Y : Y > Dia");     
    }   
  } 
  CalcMotorMaxSpeedForThread(); //Check the speed
  if( fAxeCSpeed >= fM1MaxThreadSpeed)
  {
    result = false;
    Display_Notice_Informations("Reduce spindle speed");     
  }
  if( fAxeCSpeed < 0 )
  {
    result = false;
    Display_Notice_Informations("Spindle wrong dir");     
  }   
  return result;
}
boolean M1_AreYouOkToReturnAfterThread()
{
  boolean result = true;
  if(bMotor1ThreadUseY == true)
  {
    if( fAxeYPos < fMotor1ThreadDiameter)
    {
      result = false;
      Display_Notice_Informations("Move Y : Y < Dia");
    }
      
  }
  return result;  
  
}
