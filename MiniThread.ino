/*********************************************************************
Project Name    :   MiniThread
Hard revision   :   V1.0
Soft revision   :   1.1.0
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
#include "src/Language/Language.h"
#include "src/SnakeGame/Snake.h"
#include "src/Msg/Msg.h"
#include <EEPROM.h>

#define TEXT_MAIN_MENU_TITLE "MiniThread 1.1.0"
#define TEXT_AUTHOR_SOFT "Pailpoe"
#define TEXT_VERSION_SOFT "1.1.0"

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
#define MOTOR_MODE_NO_MODE  0
#define MOTOR_MODE_MANUAL   1
#define MOTOR_MODE_AUTO     2
#define MOTOR_MODE_TH_EXT_N 3
#define MOTOR_MODE_TH_EXT_I 4
#define MOTOR_MODE_TH_INT_N 5
#define MOTOR_MODE_TH_INT_I 6

// Variables global ******************************************
const tsConfigDro csConfigDefault = {false,false,false,true,512,512,1200,false,1600,200,60000.0,12000,LANG_FR,true};
tsConfigDro  sGeneralConf;
boolean     bSettingsNeedToBeSaved = false;
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
float       fMotor1ThreadAngle = 0.0;
float       fM1ActualSpeed; // Motor Actual Speed
float       fM1MaxThreadSpeed; // Motor Max spindle speed for thread
byte        eScreenChoose = SCREEN_DRO;
teMS_ThreadingMode  eMS_Thread = MS_THREAD_IDLE;
tsThreadCalc sThreadCalc; 

// Forward declarations Funtions  ******************************************
void CalcMotorParameterForThread(); 
void CalcMotorParameterOffsetForThread();
void CalcMotorMaxSpeedForThread();
void UsbSerial_Pos(); 
void Display_UpdateRealTimeData(); 
void ActionMotorSpeedUp();
void ActionMotorSpeedDown();
void Display_StartScreen(); 
void ActionDro(); 
void ActionDebug();
void ActionLaunchSnakeGame();
void SnakeContextEnter();
void SnakeContextLoop();
void SnakeContextExit(); 
void applyTool(); 
void NeedToSave();
void ActionUpdateMenuTitle();

void ActionShortcutsResetX();
void ActionShortcutsResetY();
void ActionShortcutsResetM1();
void ActionShortcutsSetCurrentToMax();
void ActionShortcutsSetCurrentToMin();
void ActionShortcutsM1inManual();
void ActionShortcutsM1inAuto();

void ActionChangeDirX();
void ActionChangeDirY();
void ActionChangeDirZ();
void ActionChangeDirM1();
void ActionChangeResoX();
void ActionChangeResoY();
void ActionChangeResoZ();
void ActionChangeResoM1();
void ActionChangeDiamY();
void ActionChangeThreadM1();
void ActionChangeAccelM1();
void ActionChangeSpeedM1();
void ActionChangeLang();
void ActionChangeUseUSB();
void ActionRestoreSettingsInFlash(); 
void ActionSaveSettingsInFlash(); 
void ActionChangeRelaticeMode();
void ActionResetX(); 
void ActionResetY(); 
void ActionAxeXPos(); 
void ActionAxeYPos(); 
void SetReadOnlyMotorFunctions(boolean state); // true = Read only
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
void ActionIncMotor1Offset();
void ActionDecMotor1Offset(); 
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


//Menu item ******************************************
GEMPage menuPageShortcuts(""); // Shortcuts submenu
GEMItem menuItemShortcuts("", menuPageShortcuts);
GEMItem menuItemButtonShortcutsResetX("X = 0", ActionShortcutsResetX);
GEMItem menuItemButtonShortcutsResetY("Y = 0", ActionShortcutsResetY);
GEMItem menuItemButtonShortcutsResetM1("M1 = 0", ActionShortcutsResetM1);
GEMItem menuItemButtonShortcutsSetPosToMax("M1 -> M1 max", ActionShortcutsSetCurrentToMax);
GEMItem menuItemButtonShortcutsSetPosToMin("M1 -> M1 min", ActionShortcutsSetCurrentToMin);
GEMItem menuItemButtonShortcutsM1inManual("", ActionShortcutsM1inManual);
GEMItem menuItemButtonShortcutsM1inAuto("", ActionShortcutsM1inAuto);
GEMPage menuPageSettings(""); // Settings submenu
GEMItem menuItemMainSettings("", menuPageSettings);
GEMItem menuItemDirX("", sGeneralConf.Inverted_X,ActionChangeDirX);
GEMItem menuItemDirY("", sGeneralConf.Inverted_Y,ActionChangeDirY);
GEMItem menuItemDirZ("", sGeneralConf.Inverted_Z,ActionChangeDirZ);
GEMItem menuItemDiamY("", sGeneralConf.Diameter_Mode_Y,ActionChangeDiamY);
GEMItem menuItemResoX("", sGeneralConf.Reso_X,ActionChangeResoX);
GEMItem menuItemResoY("", sGeneralConf.Reso_Y,ActionChangeResoY);
GEMItem menuItemResoZ("", sGeneralConf.Reso_Z,ActionChangeResoZ);
GEMItem menuItemDirM1("", sGeneralConf.Inverted_M1,ActionChangeDirM1);
GEMItem menuItemResoM1("", sGeneralConf.Reso_M1,ActionChangeResoM1);
GEMItem menuItemThreadM1("", sGeneralConf.thread_M1,ActionChangeThreadM1);
GEMItem menuItemAccelM1("", sGeneralConf.Accel_M1,ActionChangeAccelM1);
GEMItem menuItemSpeedM1("", sGeneralConf.Speed_M1,ActionChangeSpeedM1);
SelectOptionByte selectLangOptions[] = {{"Fr", LANG_FR}, {"Eng", LANG_EN}};
GEMSelect selectLang(sizeof(selectLangOptions)/sizeof(SelectOptionByte), selectLangOptions);
GEMItem menuItemLang("", sGeneralConf.Lang, selectLang, ActionChangeLang);
GEMItem menuItemUseUSB("", sGeneralConf.UseUSBFunctions, ActionChangeUseUSB);
GEMItem menuItemButtonRestoreSettings("", ActionRestoreSettingsInFlash);
GEMItem menuItemButtonSaveSettings("", ActionSaveSettingsInFlash);
GEMItem menuItemButtonSnakeGame("Snake game !", ActionLaunchSnakeGame);
GEMItem menuItemButtonDro("", ActionDro);
GEMPage menuPageMain(TEXT_MAIN_MENU_TITLE);
GEMPage menuPageDebug(""); // Debug submenu
GEMItem menuItemDebug("", menuPageDebug);
GEMItem menuItemButtonDebug("Debug screen", ActionDebug);
GEMItem menuItemTestFloat("Float", TestFloat);
GEMPage menuPageAxe(""); // Axe submenu
GEMItem menuItemAxe("", menuPageAxe);
SelectOptionByte selectToolOptions[] = {{"Ref_0", 0}, {"Tool_1", 1}, {"Tool_2", 2}, {"Tool_3", 3}, {"Tool_4", 4}, {"Tool_5", 5}};
GEMSelect selectTool(sizeof(selectToolOptions)/sizeof(SelectOptionByte), selectToolOptions);
GEMItem menuItemTool("", bToolChoose, selectTool, applyTool);
GEMItem menuItemRelativeMode("", bRelativeModeActived,ActionChangeRelaticeMode);
GEMItem menuItemButtonResetX("X = 0", ActionResetX);
GEMItem menuItemButtonResetY("Y = 0", ActionResetY);
GEMItem menuItemAxeXPos("X = ?", fAxeXPos,ActionAxeXPos);
GEMItem menuItemAxeYPos("Y = ?", fAxeYPos,ActionAxeYPos);
GEMPage menuPageMotor(""); // Motor submenu
GEMItem menuItemMotor("", menuPageMotor);
GEMItem menuItemUseMotor("", bUseMotor,ActionUseMotor);
SelectOptionByte selectMotorModeOptions[] = {{"------", 0}, {"MANU", 1},{"AUTO", 2},{"TH EX N", 3},{"TH EX I", 4},{"TH IN N", 5},{"TH IN I", 6}};
GEMSelect selectMotorMode(sizeof(selectMotorModeOptions)/sizeof(SelectOptionByte), selectMotorModeOptions);
GEMItem menuItemMotorMode("Mode", bMotorMode, selectMotorMode, applyMotorMode);
GEMItem menuItemMotorStopMin("M1 min = ?", fMotorStopMin,ActionMotorStopMin);
GEMItem menuItemMotorStopMax("M1 max = ?", fMotorStopMax,ActionMotorStopMax);
GEMItem menuItemUseMotorEndLimit("", bUseMotorEndLimit,ActionUseMotorEndLimit);
GEMItem menuItemMotorCurrentPos("M1 = ?", fMotorCurrentPos,ActionMotorCurrentPos);
GEMItem menuItemMotorSpeed("", iMotorSpeed,ActionMotorMotorSpeed);
GEMItem menuItemButtonSetPosToMax("M1 -> M1 max", ActionSetCurrentToMax);
GEMItem menuItemButtonSetPosToMin("M1 -> M1 min", ActionSetCurrentToMin);
GEMItem menuItemButtonResetCurrentPos("M1 = 0", ActionResetCurrentPos);
GEMPage menuPageThreadParameters(""); // Thread parameters submenu
GEMItem menuItemThreadParameters("", menuPageThreadParameters);
GEMItem menuItemMotorThread("", iMotorThread,ActionMotorChangeThread);
GEMItem menuItemMotor1ThreadOffset("", fMotor1ThreadOffset,ActionChangeMotor1Offset);
GEMItem menuItemMotor1ThreadUseY("", bMotor1ThreadUseY,ActionMotor1ThreadUseY);
GEMItem menuItemMotor1ThreadDiameter("", fMotor1ThreadDiameter,ActionChangeMotor1ThreadDiameter);
GEMItem menuItemMotor1ThreadAngle("", fMotor1ThreadAngle,ActionChangeMotor1ThreadAngle);
GEMItem menuItemMotor1ThreadInfo("Vmax", fM1MaxThreadSpeed,true);
GEMItem menuItemMotorIncOffset("", ActionIncMotor1Offset);
GEMItem menuItemMotorDecOffset("", ActionDecMotor1Offset);
GEMPage menuPageProfilParameters(""); // Profil submenu
GEMItem menuItemProfil("", menuPageProfilParameters);
SelectOptionByte selectScreenOptions[] = {{"DroXYC", 0}, {"Mot1", 1}, {"Debug", 2}};
GEMSelect selectScreenMode(sizeof(selectScreenOptions)/sizeof(SelectOptionByte), selectScreenOptions);
GEMItem menuItemScreenMode("", eScreenChoose, selectScreenMode, ActionScreenMode);
//Class instance ******************************************
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0,/* reset=*/ U8X8_PIN_NONE); //Screen -->external reset (boot problem)
GEM_u8g2 menu(u8g2,GEM_POINTER_ROW,5,10,10,75); // menu
Snake MySnake(u8g2);
Msg MyMsg(u8g2);
QuadDecoder Quad_Y(3,QuadDecoder::LinearEncoder,512,false,false,IT_Timer3_Overflow); //Quad Y with timer 3
QuadDecoder Quad_Z(2,QuadDecoder::RotaryEncoder,1200,true,false,IT_Timer2_Overflow); //Quad Z with timer 2
QuadDecoder Quad_X(1,QuadDecoder::LinearEncoder,512,false,false,IT_Timer1_Overflow); //Quad X with timer 1
HardwareTimer MotorControl(4);  //for motor control with timer 4
StepperMotor Motor1(800,false,PIN_MOT1_STEP,PIN_MOT1_DIR,PIN_MOT1_EN, Update_Overlfow_Timer4);// Motor 1
Keypad customKeypad ( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); //Keypad
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
  afio_cfg_debug_ports(AFIO_DEBUG_SW_ONLY); //Only SWD
  Serial.begin(); // USB serial  
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

  //External reset for boot of the screen without problem
  pinMode(PIN_RES_SCR, OUTPUT);  //channel A
  digitalWrite(PIN_RES_SCR,0);
  delay(500);
  digitalWrite(PIN_RES_SCR,1);
  delay(500);
  u8g2.initDisplay();
  u8g2.setPowerSave(0);
  u8g2.clear();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();   


MyMsg.DisplayMsg(GetTxt(Id_Msg_Warning_SpeedTooHigh),Msg::Warning,7000);
MyMsg.DisplayMsg(GetTxt(Id_Msg_Warning_WrongDirection),Msg::Warning,7000);
MyMsg.DisplayMsg(GetTxt(Id_Msg_Warning_NoEndLimit),Msg::Warning,7000);
MyMsg.DisplayMsg(GetTxt(Id_Msg_Warning_NoAtMinPos),Msg::Warning,7000);
MyMsg.DisplayMsg(GetTxt(Id_Msg_Warning_YINFDIA),Msg::Warning,7000);
MyMsg.DisplayMsg(GetTxt(Id_Msg_Warning_YSUPDIA),Msg::Warning,7000);

  //Display start screen
  Display_StartScreen(); 

  // Menu init, setup and draw
  menu.init();
  setupMenu();
  ActionDro(); //Start with dro screen
}
void setupMenu() {
  // Add menu items to menu page
  menuPageMain.addMenuItem(menuItemButtonDro);
  //Add Sub menu shortcuts
  menuPageMain.addMenuItem(menuItemShortcuts);
  menuPageShortcuts.addMenuItem(menuItemButtonShortcutsResetX);
  menuPageShortcuts.addMenuItem(menuItemButtonShortcutsResetY);
  menuPageShortcuts.addMenuItem(menuItemButtonShortcutsResetM1);
  menuPageShortcuts.addMenuItem(menuItemButtonShortcutsSetPosToMax);
  menuPageShortcuts.addMenuItem(menuItemButtonShortcutsSetPosToMin);
  menuPageShortcuts.addMenuItem(menuItemButtonShortcutsM1inManual);
  menuPageShortcuts.addMenuItem(menuItemButtonShortcutsM1inAuto);
  menuPageShortcuts.setParentMenuPage(menuPageMain);
  //Add Sub menu Axe
  menuPageMain.addMenuItem(menuItemAxe);
  menuPageAxe.addMenuItem(menuItemTool);
  menuPageAxe.addMenuItem(menuItemRelativeMode);
  menuPageAxe.addMenuItem(menuItemButtonResetX);
  menuPageAxe.addMenuItem(menuItemButtonResetY);
  menuPageAxe.addMenuItem(menuItemAxeXPos);
  menuItemAxeXPos.setPrecision(3);
  menuPageAxe.addMenuItem(menuItemAxeYPos);
  menuItemAxeYPos.setPrecision(3);
  menuPageAxe.setParentMenuPage(menuPageMain);
  //Create sub menu Thread parameter for menu Motor
  menuPageThreadParameters.setParentMenuPage(menuPageMotor);
  menuPageThreadParameters.addMenuItem(menuItemMotorThread);
  menuPageThreadParameters.addMenuItem(menuItemMotor1ThreadInfo);
  menuItemMotor1ThreadInfo.setPrecision(1);  
  menuPageThreadParameters.addMenuItem(menuItemMotor1ThreadOffset);
  menuItemMotor1ThreadOffset.setPrecision(2);
  menuPageThreadParameters.addMenuItem(menuItemMotor1ThreadUseY);
  menuPageThreadParameters.addMenuItem(menuItemMotor1ThreadDiameter);
  menuItemMotor1ThreadDiameter.setPrecision(2);
  menuPageThreadParameters.addMenuItem(menuItemMotor1ThreadAngle);
  menuItemMotor1ThreadAngle.setPrecision(2);
  menuPageThreadParameters.addMenuItem(menuItemMotorIncOffset);
  menuPageThreadParameters.addMenuItem(menuItemMotorDecOffset);
  //Create sub menu Profil parameter for menu Motor  
  menuPageProfilParameters.setParentMenuPage(menuPageMotor);

 
  //Add Sub menu Motor
  menuPageMain.addMenuItem(menuItemMotor);
  menuPageMotor.addMenuItem(menuItemUseMotor);
  menuPageMotor.addMenuItem(menuItemUseMotorEndLimit);
  menuPageMotor.addMenuItem(menuItemMotorMode);
  menuPageMotor.addMenuItem(menuItemMotorSpeed); 
  menuPageMotor.addMenuItem(menuItemMotorCurrentPos);
  menuItemMotorCurrentPos.setPrecision(2); 
  menuPageMotor.addMenuItem(menuItemMotorStopMin);
  menuItemMotorStopMin.setPrecision(2);
  menuPageMotor.addMenuItem(menuItemMotorStopMax);
  menuItemMotorStopMax.setPrecision(2);
  menuPageMotor.addMenuItem(menuItemThreadParameters); //Sub menu thread parameter
  menuPageMotor.addMenuItem(menuItemProfil); //Sub menu profil parameter
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
  menuPageSettings.addMenuItem(menuItemLang);
  menuPageSettings.addMenuItem(menuItemUseUSB);
  menuPageSettings.addMenuItem(menuItemButtonRestoreSettings);
  menuPageSettings.addMenuItem(menuItemButtonSaveSettings);
  menuPageSettings.setParentMenuPage(menuPageMain);
  //Add item screen mode
  menuPageMain.addMenuItem(menuItemScreenMode);
  //Add snake game button
  menuPageMain.addMenuItem(menuItemButtonSnakeGame);
  //Add Sub menu Debug
  menuPageMain.addMenuItem(menuItemDebug);
  menuPageDebug.addMenuItem(menuItemTestFloat);
  menuPageDebug.addMenuItem(menuItemButtonDebug); 
  menuPageDebug.setParentMenuPage(menuPageMain);
  // Add menu page to menu and set it as current
  menu.setMenuPageCurrent(menuPageMain);
  //Set read only because it's OFF
  SetReadOnlyMotorFunctions(true);
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
        if( customKeypad.isPressed(GEM_KEY_LEFT))
        {
          Motor1.ChangeTheMode(StepperMotor::SpeedModeUp);
          if(customKeypad.isPressed(GEM_KEY_UP))
          {
            //Move the end limit Max
            fMotorStopMax += 0.05;
            ActionMotorStopMax();     
          }
          if(customKeypad.isPressed(GEM_KEY_DOWN))
          {
            //Move the end limit Max
            fMotorStopMax -= 0.05;
            ActionMotorStopMax();     
          }    
        }
        if( customKeypad.isPressed(GEM_KEY_RIGHT))
        {
          Motor1.ChangeTheMode(StepperMotor::SpeedModeDown);
          if(customKeypad.isPressed(GEM_KEY_UP))
          {
            //Move the end limit Max
            fMotorStopMin += 0.05;
            ActionMotorStopMin();     
          }
          if(customKeypad.isPressed(GEM_KEY_DOWN))
          {
            //Move the end limit Max
            fMotorStopMin -= 0.05;
            ActionMotorStopMin();     
          }  
        }
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
     if ( bMotorMode == MOTOR_MODE_TH_EXT_N ||
          bMotorMode == MOTOR_MODE_TH_EXT_I ||
          bMotorMode == MOTOR_MODE_TH_INT_N ||
          bMotorMode == MOTOR_MODE_TH_INT_I )
    {
      switch(eMS_Thread)
      {
        case MS_THREAD_IDLE:
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
// *** Snake game for fun !! context *****************************************************
void ActionLaunchSnakeGame()
{
  menu.context.loop = SnakeContextLoop;
  menu.context.enter = SnakeContextEnter;
  menu.context.exit = SnakeContextExit;
  menu.context.allowExit = false; // Setting to false will require manual exit from the loop
  menu.context.enter();    
}
void SnakeContextEnter()
{
  u8g2.clear();//Clear screen
}
void SnakeContextLoop() 
{
  byte key = customKeypad.getKey();
  MySnake.loop(key);
  if (key == GEM_KEY_CANCEL) 
  { 
    menu.context.exit();
  }
}
void SnakeContextExit() 
{
  menu.reInit();
  menu.drawMenu();
  menu.clearContext();
}
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
      char buffer[16];
      sprintf(buffer,"Numerator:%ld",sThreadCalc.Numerator);
      u8g2.drawStr(0,0,buffer);
      sprintf(buffer,"Denominator:%ld",sThreadCalc.Denominator);
      u8g2.drawStr(0,10,buffer);
      sprintf(buffer,"Offset:%ld",sThreadCalc.Offset);
      u8g2.drawStr(0,20,buffer);  
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
  ActionChangeLang();
}
void ActionSaveSettingsInFlash()
{
  //Store config in memort
  SaveConfigInFlash(&sGeneralConf);
  //Dispatch config to function
  Dispatch_Config(&sGeneralConf); 
  //PrintInformationOnScreen("Save in flash");
  //delay(100);  
  bSettingsNeedToBeSaved = false;
  MyMsg.DisplayMsg(GetTxt(Id_Msg_Save),Msg::Info,1000);
  menu.drawMenu(); //Refresh screen after  
}
void ActionRestoreSettingsInFlash()
{
  //Save default config in flash
  SaveConfigInFlash((tsConfigDro*)&csConfigDefault);
  Restore_Config();
  bSettingsNeedToBeSaved = false;
  MyMsg.DisplayMsg(GetTxt(Id_Msg_Restore),Msg::Info,1000);
  menu.drawMenu(); //Refresh screen after 
}
// ***************************************************************************************
// ***************************************************************************************
// *** Usb Serial functions *****************************************************************
void UsbSerial_Pos()
{
  char bufferChar[30];
  if(Serial.isConnected() && sGeneralConf.UseUSBFunctions)
  {
    sprintf(bufferChar,"X%0.3f:",fAxeXPos); 
    Serial.print(bufferChar);
    sprintf(bufferChar,"Y%0.3f:",fAxeYPos); 
    Serial.print(bufferChar);
    sprintf(bufferChar,"C%0.3f:",fAxeCSpeed); 
    Serial.print(bufferChar);
    sprintf(bufferChar,"M%0.3f",fMotorCurrentPos); 
    Serial.print(bufferChar);
    Serial.print("\n");   
  }
}
// ***************************************************************************************
// ***************************************************************************************
// *** Display functions *****************************************************************
void Display_StartScreen() 
{
  u8g2.clear();
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
  u8g2.clear();
  u8g2.firstPage();
  MyMsg.DisplayMsg(GetTxt(Id_Msg_Start),Msg::Warning,3000);
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
        u8g2.drawStr(57,37,"|------");
      break;
      case MOTOR_MODE_MANUAL:
        u8g2.drawStr(57,37,"|MANU");
      break;
      case MOTOR_MODE_AUTO:
        u8g2.drawStr(57,37,"|AUTO");
      break;
      case MOTOR_MODE_TH_EXT_N:
        u8g2.drawStr(57,37,"|THEX N"); 
      break;
      case MOTOR_MODE_TH_EXT_I:
        u8g2.drawStr(57,37,"|THEX I"); 
      break;
      case MOTOR_MODE_TH_INT_N:
        u8g2.drawStr(57,37,"|THIN N"); 
      break;
      case MOTOR_MODE_TH_INT_I:
        u8g2.drawStr(57,37,"|THIN I"); 
      break;      
    }
    //Motor speed
    //if motor is Left mode, display the speed from the settings (Max speed )
    sprintf(bufferChar,"|%d", (unsigned int)fM1ActualSpeed);
    //sprintf(bufferChar,"|%d",iMotorSpeed);
    u8g2.drawStr(95,37,bufferChar);    
    
    //End limit 
    sprintf(bufferChar,"%+09.3f <> %+09.3f",fMotorStopMax,fMotorStopMin);
    if(bUseMotorEndLimit)u8g2.drawStr(13,45,bufferChar);
    else u8g2.drawStr(16,45,GetTxt(Id_Msg_Motor_NoEndLimit));       
  }
  else
  {
    u8g2.setFont(u8g2_font_profont10_mr); // choose a suitable font
    u8g2.drawStr(16,40,GetTxt(Id_Msg_Motor_Disabled));   
  }  
  u8g2.drawRFrame(11,36,116,18,3);    
}
void Display_Extra_Informations()
{
  char bufferChar[10];
  u8g2.setFont(u8g2_font_profont10_mr); // choose a suitable font
  u8g2.drawStr(0,54,selectToolOptions[bToolChoose].name);
  //Display thread Masterstate 
  if (  bMotorMode == MOTOR_MODE_TH_EXT_N ||
        bMotorMode == MOTOR_MODE_TH_EXT_I ||
        bMotorMode == MOTOR_MODE_TH_INT_N ||
        bMotorMode == MOTOR_MODE_TH_INT_I )
  {
    switch(eMS_Thread)
    {
      case MS_THREAD_IDLE:
        u8g2.drawStr(30,54,"|IDLE");
      break;  
      case MS_THREAD_WAIT_THE_START:
        u8g2.drawStr(30,54,"|WAIT START");
      break;  
      case MS_THREAD_WAIT_THE_SPLINDLE_ZERO:
        u8g2.drawStr(30,54,"|WAIT SYNC");
      break;  
      case MS_THREAD_IN_THREAD:
        u8g2.drawStr(30,54,"|IN THREAD");
      break;  
      case MS_THREAD_END_THREAD:
        u8g2.drawStr(30,54,"|WAIT RETURN");
      break;             
      case MS_THREAD_IN_RETURN:
        u8g2.drawStr(30,54,"|IN RETURN");
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

// ***************************************************************************************
// ***************************************************************************************
// *** Action functions from menu ********************************************************

void NeedToSave()
{
  bSettingsNeedToBeSaved = true;
}
void ActionChangeDirX()
{
  NeedToSave();  
}
void ActionChangeDirY()
{
  NeedToSave();
}
void ActionChangeDirZ()
{
  NeedToSave();
}
void ActionChangeDirM1()
{
  NeedToSave();
}
void ActionChangeResoX()
{
  NeedToSave();
  if(sGeneralConf.Reso_X < 1)sGeneralConf.Reso_X = 1;
  if(sGeneralConf.Reso_X > 10000)sGeneralConf.Reso_X = 10000;     
}
void ActionChangeResoY()
{
  NeedToSave();
  if(sGeneralConf.Reso_Y < 1)sGeneralConf.Reso_Y = 1;
  if(sGeneralConf.Reso_Y > 10000)sGeneralConf.Reso_Y = 10000;  
}
void ActionChangeResoZ()
{
  NeedToSave();
  if(sGeneralConf.Reso_Z < 1)sGeneralConf.Reso_Z = 1;
  if(sGeneralConf.Reso_Z > 10000)sGeneralConf.Reso_Z = 10000;   
}
void ActionChangeResoM1()
{
  NeedToSave();
  if(sGeneralConf.Reso_M1 < 1)sGeneralConf.Reso_M1 = 1;
  if(sGeneralConf.Reso_M1 > 10000)sGeneralConf.Reso_M1 = 10000;  
}
void ActionChangeDiamY()
{
  NeedToSave();
}
void ActionChangeThreadM1()
{
  NeedToSave();
  if(sGeneralConf.thread_M1 < 1)sGeneralConf.thread_M1 = 1;
  if(sGeneralConf.thread_M1 > 10000)sGeneralConf.thread_M1 = 10000;
}
void ActionChangeAccelM1()
{
  NeedToSave();
  if(sGeneralConf.Accel_M1 < 1.0)sGeneralConf.Accel_M1 = 1.0;
  if(sGeneralConf.Accel_M1 > 500000.0)sGeneralConf.Accel_M1 = 500000.0;
}
void ActionChangeSpeedM1()
{
  NeedToSave();
  if(sGeneralConf.Speed_M1 < 1)sGeneralConf.Speed_M1 = 1;
  if(sGeneralConf.Speed_M1 > 30000)sGeneralConf.Speed_M1 = 30000;
}
void ActionChangeUseUSB()
{
  
  
}

void ActionShortcutsResetX()
{
  Quad_X.SetZeroActiveMode();
  ActionDro();   
}
void ActionShortcutsResetY()
{
  Quad_Y.SetZeroActiveMode();
  ActionDro();  
}
void ActionShortcutsResetM1()
{
  ActionResetCurrentPos();
  ActionDro(); 
}
void ActionShortcutsSetCurrentToMax()
{
  ActionSetCurrentToMax();
  ActionDro();  
}
void ActionShortcutsSetCurrentToMin()
{
  ActionSetCurrentToMin();
  ActionDro();  
}
void ActionShortcutsM1inManual()
{
  bMotorMode = MOTOR_MODE_MANUAL; 
  applyMotorMode();  
  ActionDro();  
}
void ActionShortcutsM1inAuto()
{
  bMotorMode = MOTOR_MODE_AUTO; 
  applyMotorMode();  
  ActionDro();     
}

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
void SetReadOnlyMotorFunctions(boolean state)
{
  menuItemMotorMode.setReadonly(state);  
  menuItemMotorStopMin.setReadonly(state);
  menuItemMotorStopMax.setReadonly(state);
  menuItemUseMotorEndLimit.setReadonly(state);
  menuItemMotorCurrentPos.setReadonly(state);
  menuItemMotorSpeed.setReadonly(state);
  menuItemButtonSetPosToMax.setReadonly(state);
  menuItemButtonSetPosToMin.setReadonly(state);
  menuItemButtonResetCurrentPos.setReadonly(state);
  menuItemThreadParameters.setReadonly(state);
  menuItemProfil.setReadonly(state);
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
    SetReadOnlyMotorFunctions(false); 
  }
  else
  {
    Motor1.ChangeTheMode(StepperMotor::NoMode);
    bMotorMode = MOTOR_MODE_NO_MODE; 
    Motor1.MotorChangePowerState(false);
    eMS_Thread = MS_THREAD_IDLE;
    eScreenChoose = SCREEN_DRO; 
    SetReadOnlyMotorFunctions(true);   
  }     
}
void ActionUseMotorEndLimit()
{
  Motor1.UseEndLimit(bUseMotorEndLimit);  
}

void CalcMotorParameterForThread()
{
  long lGCD;
  //Calc Numerator and Denominator with simplification
  sThreadCalc.Numerator = sGeneralConf.Reso_M1 * iMotorThread;  
  sThreadCalc.Denominator = sGeneralConf.thread_M1 * sGeneralConf.Reso_Z ; 
  lGCD = GCD_Function(sThreadCalc.Numerator,sThreadCalc.Denominator);
  sThreadCalc.Numerator = sThreadCalc.Numerator / lGCD;
  sThreadCalc.Denominator = sThreadCalc.Denominator / lGCD;   
  //If reverse direction
  if( bMotorMode == MOTOR_MODE_TH_EXT_I || bMotorMode == MOTOR_MODE_TH_INT_I)
  {
    sThreadCalc.Numerator = -sThreadCalc.Numerator;
  }
  CalcMotorParameterOffsetForThread();
}
void CalcMotorParameterOffsetForThread()
{
  float OffsetFixe = 0.0; //Constant offset
  float OffsetVariable = 0.0; //variable offset
  float fTemp = 0.0; // pour calcul du demi pas
  //Calc of the fixe offset
  OffsetFixe = (float)((360.0 - fMotor1ThreadOffset)*iMotorThread*sGeneralConf.Reso_M1) /(float)(360*sGeneralConf.thread_M1); 
  //Calcul of the variable offset ( depend of the diameter and Y position).
  if(bMotor1ThreadUseY == true)
  {
    if(bMotorMode == MOTOR_MODE_TH_EXT_N || bMotorMode == MOTOR_MODE_TH_EXT_I)
    {
      if(fAxeYPos <= fMotor1ThreadDiameter)
      {
        OffsetVariable = (float)((fMotor1ThreadDiameter - fAxeYPos)/2.0 * tan(fMotor1ThreadAngle * 0.01745)); //Pi/180 = 0.01745
        OffsetVariable = OffsetVariable *(float)(sGeneralConf.Reso_M1*100.0/sGeneralConf.thread_M1) ; 
      }else
      {
        OffsetVariable = 0.0;  
      }          
    }
    if(bMotorMode == MOTOR_MODE_TH_INT_N || bMotorMode == MOTOR_MODE_TH_INT_I)
    {
      if(fAxeYPos >= fMotor1ThreadDiameter)
      {
        OffsetVariable = (float)((fAxeYPos - fMotor1ThreadDiameter)/2.0 * tan(fMotor1ThreadAngle * 0.01745)); //Pi/180 = 0.01745
        OffsetVariable = OffsetVariable *(float)(sGeneralConf.Reso_M1*100.0/sGeneralConf.thread_M1) ; 
      }else
      {
        OffsetVariable = 0.0;  
      }          
    }
    //L'offset variable ne peut pas dépasser le demi pas demandé 
    fTemp = (float)(sGeneralConf.Reso_M1*iMotorThread /(sGeneralConf.thread_M1 * 2.0)) ; 
    if( OffsetVariable > fTemp ) OffsetVariable = fTemp;  
    
  }
  //Global offset
  sThreadCalc.Offset = Motor1.GetStopPositionMinStep() - (long)OffsetFixe + (long)OffsetVariable ;
}
void CalcMotorMaxSpeedForThread()
{
  fM1MaxThreadSpeed = (float)(sGeneralConf.Speed_M1*60.0*sGeneralConf.thread_M1/(sGeneralConf.Reso_M1*iMotorThread));  
}
void applyMotorMode()
{
  switch (bMotorMode)
  {
    case MOTOR_MODE_TH_EXT_N :
    case MOTOR_MODE_TH_EXT_I:
    case MOTOR_MODE_TH_INT_N:
    case MOTOR_MODE_TH_INT_I:
      eMS_Thread = MS_THREAD_WAIT_THE_START;
      //Use Max speed in the setting
      Motor1.ChangeMaxSpeed(sGeneralConf.Speed_M1); 
      //Motor in position mode
      Motor1.ChangeTheMode(StepperMotor::PositionMode);    
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
  if(iMotorSpeed == iMotorSpeed + iMotorSpeed / 5) iMotorSpeed++;
  else iMotorSpeed = iMotorSpeed + iMotorSpeed / 5 ;   
}
void ActionMotorSpeedDown()
{ //-20%
  if(iMotorSpeed == iMotorSpeed - iMotorSpeed / 5) iMotorSpeed--;
  else iMotorSpeed = iMotorSpeed - iMotorSpeed / 5 ;  
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
}
void ActionResetY()
{
  Quad_Y.SetZeroActiveMode();  
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
  ///Possibility to change the offset durring the threading and spindle off  
  if(eMS_Thread == MS_THREAD_IN_THREAD && fAxeCSpeed == 0)
  {
    CalcMotorParameterOffsetForThread();    
  } 
}
void ActionIncMotor1Offset()
{
  fMotor1ThreadOffset = fMotor1ThreadOffset+2;
  ActionChangeMotor1Offset();
}
void ActionDecMotor1Offset()
{
  fMotor1ThreadOffset = fMotor1ThreadOffset-2;
  ActionChangeMotor1Offset();
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
  //Check position of Y
  if(bMotor1ThreadUseY == true)
  {
    if( bMotorMode == MOTOR_MODE_TH_EXT_N || bMotorMode == MOTOR_MODE_TH_EXT_I)
    {
//      if( fAxeYPos > fMotor1ThreadDiameter)
//      {
//        result = false;
//        Display_Notice_Informations("Move Y : Y > Dia");     
//      }    
    }
    if( bMotorMode == MOTOR_MODE_TH_INT_N || MOTOR_MODE_TH_INT_I)
    {
//      if( fAxeYPos < fMotor1ThreadDiameter)
//      {
//        result = false;
//        Display_Notice_Informations("Move Y : Y < Dia");     
//      }    
    }    
  } 
  CalcMotorMaxSpeedForThread(); //Check the speed
  //Check the max speed
  if( fAxeCSpeed >= fM1MaxThreadSpeed)
  {
    result = false;
    MyMsg.DisplayMsg(GetTxt(Id_Msg_Warning_SpeedTooHigh),Msg::Warning,2000);    
  }
  //Check the direction
  if( bMotorMode == MOTOR_MODE_TH_EXT_N || bMotorMode == MOTOR_MODE_TH_INT_N)
  {
    if( fAxeCSpeed < 0 )
    {
      result = false;
      MyMsg.DisplayMsg(GetTxt(Id_Msg_Warning_WrongDirection),Msg::Warning,2000);    
    }      
  }
  if( bMotorMode == MOTOR_MODE_TH_EXT_I || bMotorMode == MOTOR_MODE_TH_INT_I)
  {
    if( fAxeCSpeed > 0 )
    {
      result = false;
      MyMsg.DisplayMsg(GetTxt(Id_Msg_Warning_WrongDirection),Msg::Warning,2000);    
    }       
  }   
  //Check if endlimit is on
  if( !bUseMotorEndLimit)
  {
    result = false;
    MyMsg.DisplayMsg(GetTxt(Id_Msg_Warning_NoEndLimit),Msg::Warning,2000);   
  } 
  //Check if the motor is at min pos
  if( !Motor1.AreYouAtMinPos())
  {
    result = false;
    MyMsg.DisplayMsg(GetTxt(Id_Msg_Warning_NoAtMinPos),Msg::Warning,2000);     
  }   
  return result;
}
boolean M1_AreYouOkToReturnAfterThread()
{
  boolean result = true;
  if(bMotor1ThreadUseY == true)
  {
    if( bMotorMode == MOTOR_MODE_TH_EXT_N || bMotorMode == MOTOR_MODE_TH_EXT_I)
    {
      if( fAxeYPos < fMotor1ThreadDiameter)
      {
        result = false;
        MyMsg.DisplayMsg(GetTxt(Id_Msg_Warning_YINFDIA),Msg::Warning,2000);
      }    
    }
    if( bMotorMode == MOTOR_MODE_TH_INT_N || bMotorMode == MOTOR_MODE_TH_INT_I)
    {
      if( fAxeYPos > fMotor1ThreadDiameter)
      {
        result = false;
        MyMsg.DisplayMsg(GetTxt(Id_Msg_Warning_YSUPDIA),Msg::Warning,2000);
      }   
    }      
  }
  return result;    
}
void ActionChangeLang()
{  
  ChangeLang(sGeneralConf.Lang); 
  ActionUpdateMenuTitle();  
}
void ActionUpdateMenuTitle()
{
  menuPageSettings.setTitle(GetTxt(Id_Msg_TEXT_MENU_SETTINGS));
  menuItemMainSettings.setTitle(GetTxt(Id_Msg_TEXT_MENU_SETTINGS));
  menuItemButtonDro.setTitle(GetTxt(Id_Msg_TEXT_MENU_RETURN_SCREEN));
  menuItemAxe.setTitle(GetTxt(Id_Msg_TEXT_MENU_AXE_FUNCTIONS));
  menuItemMotor.setTitle(GetTxt(Id_Msg_TEXT_MENU_MOTOR_FUNCTIONS));  
  menuItemScreenMode.setTitle(GetTxt(Id_Msg_TEXT_MENU_ECRAN));
  menuItemButtonRestoreSettings.setTitle(GetTxt(Id_Msg_TEXT_MENU_RESTORE_SETTINGS) );
  menuItemButtonSaveSettings.setTitle(GetTxt(Id_Msg_TEXT_MENU_SAVE_SETTINGS));
  menuItemShortcuts.setTitle(GetTxt(Id_Msg_TEXT_MENU_FAST_FUNCTIONS));
  menuPageShortcuts.setTitle(GetTxt(Id_Msg_TEXT_MENU_FAST_FUNCTIONS));
  menuItemButtonShortcutsM1inManual.setTitle(GetTxt(Id_Msg_TEXT_MENU_FAST_M1MANU));
  menuItemButtonShortcutsM1inAuto.setTitle(GetTxt(Id_Msg_TEXT_MENU_FAST_M1AUTO));
  menuPageAxe.setTitle(GetTxt(Id_Msg_TEXT_MENU_AXE_FUNCTIONS));
  menuPageMotor.setTitle(GetTxt(Id_Msg_TEXT_MENU_MOTOR_FUNCTIONS));
  menuPageDebug.setTitle(GetTxt(Id_Msg_TEXT_MENU_DEBUG));
  menuItemDebug.setTitle(GetTxt(Id_Msg_TEXT_MENU_DEBUG));
  menuPageThreadParameters.setTitle(GetTxt(Id_Msg_TEXT_MENU_THREAD_PARAMETERS));
  menuItemThreadParameters.setTitle(GetTxt(Id_Msg_TEXT_MENU_THREAD_PARAMETERS));
  menuItemUseMotorEndLimit.setTitle(GetTxt(Id_Msg_TEXT_MENU_MOTORDLIMIT));
  menuItemUseMotor.setTitle(GetTxt(Id_Msg_TEXT_MENU_MOTORABLED));
  menuItemMotorSpeed.setTitle(GetTxt(Id_Msg_TEXT_MENU_MOTOR_SPEED)); 
  menuItemMotorThread.setTitle(GetTxt(Id_Msg_TEXT_MENU_THREAD_THREAD));
  menuItemMotor1ThreadOffset.setTitle(GetTxt(Id_Msg_TEXT_MENU_THREAD_OFFSET));
  menuItemMotor1ThreadUseY.setTitle(GetTxt(Id_Msg_TEXT_MENU_THREAD_USEY));
  menuItemMotor1ThreadDiameter.setTitle(GetTxt(Id_Msg_TEXT_MENU_THREAD_DIAMETER));
  menuItemMotor1ThreadAngle.setTitle(GetTxt(Id_Msg_TEXT_MENU_THREAD_ANGLE));
  menuItemMotorIncOffset.setTitle(GetTxt(Id_Msg_TEXT_MENU_THREAD_INC));
  menuItemMotorDecOffset.setTitle(GetTxt(Id_Msg_TEXT_MENU_THREAD_DEC));
  menuItemTool.setTitle(GetTxt(Id_Msg_TEXT_MENU_AXE_TOOL));
  menuItemRelativeMode.setTitle(GetTxt(Id_Msg_TEXT_MENU_AXE_REL));
  menuItemDirX.setTitle(GetTxt(Id_Msg_TEXT_MENU_SETTINGS_XDIR));
  menuItemDirY.setTitle(GetTxt(Id_Msg_TEXT_MENU_SETTINGS_YDIR));
  menuItemDirZ.setTitle(GetTxt(Id_Msg_TEXT_MENU_SETTINGS_CDIR));
  menuItemDiamY.setTitle(GetTxt(Id_Msg_TEXT_MENU_SETTINGS_YDIAM));
  menuItemResoX.setTitle(GetTxt(Id_Msg_TEXT_MENU_SETTINGS_XSTEP));
  menuItemResoY.setTitle(GetTxt(Id_Msg_TEXT_MENU_SETTINGS_YSTEP));
  menuItemResoZ.setTitle(GetTxt(Id_Msg_TEXT_MENU_SETTINGS_CSTEP));
  menuItemDirM1.setTitle(GetTxt(Id_Msg_TEXT_MENU_SETTINGS_M1DIR));
  menuItemResoM1.setTitle(GetTxt(Id_Msg_TEXT_MENU_SETTINGS_M1STEP));
  menuItemThreadM1.setTitle(GetTxt(Id_Msg_TEXT_MENU_SETTINGS_M1TH));
  menuItemAccelM1.setTitle(GetTxt(Id_Msg_TEXT_MENU_SETTINGS_M1ACC));
  menuItemSpeedM1.setTitle(GetTxt(Id_Msg_TEXT_MENU_SETTINGS_M1SPE));
  menuItemLang.setTitle(GetTxt(Id_Msg_TEXT_MENU_SETTINGS_M1LAN));
  menuItemUseUSB.setTitle(GetTxt(Id_Msg_TEXT_MENU_SETTINGS_USB));
  menuPageProfilParameters.setTitle(GetTxt(Id_Msg_TEXT_MENU_PROFIL)); 
  menuItemProfil.setTitle(GetTxt(Id_Msg_TEXT_MENU_PROFIL));
}
