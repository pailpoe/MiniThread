/*********************************************************************
Project Name    :   Mini Dro 2
Hard revision   :   V1.0
Soft revision   :   /
Description     :   Dro system for lathe or milling machine with 3 quadrature decoder, Oled SSD1306 display and 6 push buttons for navigation
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
#include <EEPROM.h>

#define USE_KEYPAD_KEYBOARD

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);


#ifdef USE_KEYPAD_KEYBOARD
  const byte ROWS = 4; //four rows
  const byte COLS = 4; //four columns
  char hexaKeys[ROWS][COLS] = {
    {'Z'             ,'Z'           ,GEM_KEY_UP   ,'Z'            },
    {GEM_KEY_CANCEL  ,GEM_KEY_LEFT  ,GEM_KEY_OK   ,GEM_KEY_RIGHT  },
    {'Z'             ,'Z'           ,GEM_KEY_DOWN ,'Z'            },
    {'Z'             ,'Z'           ,'Z'          ,'Z'            }
  };
  byte rowPins[ROWS] = {PA5, PA4, PA3, PA2}; //connect to the row pinouts of the keypad
  byte colPins[COLS] = {PB0, PB1, PB10, PB11}; //connect to the column pinouts of the keypad
  //initialize an instance of class NewKeypad
  Keypad customKeypad ( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
  //customKeypad.getKey();
#endif

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
} sConfigDro;
const sConfigDro csConfigDefault = {false,false,false,false,512,512,512,false,1600,200};
// Variable
sConfigDro ConfigDro;

GEMItem menuItemDirX("X dir:", ConfigDro.Inverted_X);
GEMItem menuItemDirY("Y dir:", ConfigDro.Inverted_Y);
GEMItem menuItemDirZ("Z dir:", ConfigDro.Inverted_Z);
GEMItem menuItemDiamY("Diameter Y", ConfigDro.Diameter_Mode_Y);
GEMItem menuItemResoX("X step/mm:", ConfigDro.Reso_X);
GEMItem menuItemResoY("Y step/mm:", ConfigDro.Reso_Y);
GEMItem menuItemResoZ("Z step/mm:", ConfigDro.Reso_Z);
GEMItem menuItemDirM1("M1 dir:", ConfigDro.Inverted_M1);
GEMItem menuItemResoM1("M1 step/tr:", ConfigDro.Reso_M1);
GEMItem menuItemThreadM1("M1 thread:", ConfigDro.thread_M1);

void ActionRestoreSettingsInFlash(); // Forward declaration
GEMItem menuItemButtonRestoreSettings("Restore settings", ActionRestoreSettingsInFlash);
void ActionSaveSettingsInFlash(); // Forward declaration
GEMItem menuItemButtonSaveSettings("Save settings", ActionSaveSettingsInFlash);

void ActionDro(); // Forward declaration
GEMItem menuItemButtonDro("DRO screen", ActionDro);

void ActionDebug(); // Forward declaration
GEMItem menuItemButtonDebug("Debug screen", ActionDebug);

//Main Page Menu
GEMPage menuPageMain("Dro Menu");

//Settings Page Menu
GEMPage menuPageSettings("Settings"); // Settings submenu
GEMItem menuItemMainSettings("Settings", menuPageSettings);

//For tool selection
byte ToolChoose = 0;
SelectOptionByte selectToolOptions[] = {{"Tool_0", 0}, {"Tool_1", 1}, {"Tool_2", 2}, {"Tool_3", 3}, {"Tool_4", 4}, {"Tool_5", 5}};
GEMSelect selectTool(sizeof(selectToolOptions)/sizeof(SelectOptionByte), selectToolOptions);
void applyTool(); // Forward declaration
GEMItem menuItemTool("Tool:", ToolChoose, selectTool, applyTool);



boolean RelativeMode = false;
void UpdateRelAxe();
GEMItem menuItemRelativeMode("Relative mode", RelativeMode,UpdateRelAxe);


// Create menu object of class GEM_u8g2. Supply its constructor with reference to u8g2 object we created earlier
GEM_u8g2 menu(u8g2);

//Quadrature decoder
void IT_Timer1_Overflow(); // Forward declaration
void IT_Timer2_Overflow(); // Forward declaration
void IT_Timer3_Overflow(); // Forward declaration
QuadDecoder Quad_Y(3,QuadDecoder::LinearEncoder,512,false,false,IT_Timer3_Overflow); //Timer 3
QuadDecoder Quad_Z(2,QuadDecoder::RotaryEncoder,512,true,false,IT_Timer2_Overflow); //Timer 2
QuadDecoder Quad_X(1,QuadDecoder::LinearEncoder,512,false,false,IT_Timer1_Overflow); //Timer 1
void IT_Timer1_Overflow(){Quad_X.IT_OverflowHardwareTimer();}
void IT_Timer2_Overflow(){Quad_Z.IT_OverflowHardwareTimer();}
void IT_Timer3_Overflow(){Quad_Y.IT_OverflowHardwareTimer();}

//Hardware timer 4 for motor control
HardwareTimer MotorControl(4); 
//Motor Class
#define PIN_MOT1_STEP  PA10
#define PIN_MOT1_DIR   PB15
#define PIN_MOT1_EN    PA15
#define PIN_MOT2_STEP  PB13
#define PIN_MOT2_DIR   PB12
#define PIN_MOT2_EN    PB14
StepperMotor Motor1(800,false,PIN_MOT1_STEP,PIN_MOT1_DIR,PIN_MOT1_EN);
//Timer 4 overflow for Step motor
void handler_Timer4_overflow()
{ 
  Motor1.TimeToPrepareToMove(); 
}
//Timer 4 channel 3 compare interrupt (10µs after overflow)
void handler_Timer4_compare3()
{
  Motor1.TimeToMove();
}

void setup() {

  // U8g2 library init. Pass pin numbers the buttons are connected to.
  // The push-buttons should be wired with pullup resistors (so the LOW means that the button is pressed)
  #ifdef USE_KEYPAD_KEYBOARD
    u8g2.begin();
  #endif
  #ifndef USE_KEYPAD_KEYBOARD
    u8g2.begin(/*Select/OK=*/ PB14, /*Right/Next=*/ PB1, /*Left/Prev=*/ PB0, /*Up=*/ PB15, /*Down=*/ PB12, /*Home/Cancel=*/ PB13);
  #endif
  //Debug port...
  afio_cfg_debug_ports(AFIO_DEBUG_SW_ONLY); //Only SWD
  
  //Timer 4 for motor control
  MotorControl.pause(); //stop...
  MotorControl.setCompare(TIMER_CH3, 720); //10µs (720) after
  MotorControl.setChannel3Mode(TIMER_OUTPUT_COMPARE);
  MotorControl.setPeriod(100); //Period 100µs --> 10Khz
  MotorControl.attachCompare3Interrupt(handler_Timer4_compare3); //interrupt conmpare 3
  MotorControl.attachInterrupt(0, handler_Timer4_overflow); //Overflow interrupt  
  MotorControl.resume();

  Motor1.ChangeStopPositionMinReal(0);
  Motor1.ChangeMaxSpeed(2);
  Motor1.ChangeStopPositionMaxReal(1000);
  Motor1.MotorChangePowerState(true);

 
  //Restore config  
  Restore_Config();
  // Menu init, setup and draw
  menu.init();
  setupMenu();
  //menu.drawMenu(); //Start with menu screen
  ActionDro(); //Start with dro screen
  //ActionDebug(); //Start with dro screen
}

void setupMenu() {
  // Add menu items to menu page
  menuPageMain.addMenuItem(menuItemButtonDro);
  menuPageMain.addMenuItem(menuItemButtonDebug);
  menuPageMain.addMenuItem(menuItemTool);
  menuPageMain.addMenuItem(menuItemRelativeMode);
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
  menuPageSettings.addMenuItem(menuItemButtonRestoreSettings);
  menuPageSettings.addMenuItem(menuItemButtonSaveSettings);
  // Specify parent menu page for the Settings menu page
  menuPageSettings.setParentMenuPage(menuPageMain);
  // Add menu page to menu and set it as current
  menu.setMenuPageCurrent(menuPageMain);
}

void loop() {
  // If menu is ready to accept button press...
  if (menu.readyForKey()) {
    // ...detect key press using U8g2 library
    // and pass pressed button to menu
    #ifndef USE_KEYPAD_KEYBOARD
    menu.registerKeyPress(u8g2.getMenuEvent());
    #endif
    #ifdef USE_KEYPAD_KEYBOARD
    menu.registerKeyPress(customKeypad.getKey());
    #endif    
  }
}

// *** DRO context with axe display
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
void DroContextLoop() {
  // Detect key press manually using U8g2 library
    #ifndef USE_KEYPAD_KEYBOARD
      byte key = u8g2.getMenuEvent();
    #endif
    #ifdef USE_KEYPAD_KEYBOARD
      byte key = customKeypad.getKey();
    #endif  
  if (key == GEM_KEY_CANCEL) 
  { 
    // Exit animation routine if GEM_KEY_CANCEL key was pressed
    menu.context.exit();
  } else 
  {
    if(key == GEM_KEY_UP)Quad_X.SetZeroActiveMode();
    if(key == GEM_KEY_DOWN)Quad_Z.SetZeroActiveMode();
    if(key == GEM_KEY_OK)Quad_Y.SetZeroActiveMode();
    DisplayDrawInformations();
  }
}
void DroContextExit() 
{
  menu.reInit();
  menu.drawMenu();
  menu.clearContext();
}


// *** Debug context
void ActionDebug()
{
  menu.context.loop = DebugContextLoop;
  menu.context.enter = DebugContextEnter;
  menu.context.exit = DebugContextExit;
  menu.context.allowExit = false; // Setting to false will require manual exit from the loop
  menu.context.enter();    
}
void DebugContextEnter() {
  // Clear sreen
  u8g2.clear();
}
void DebugContextLoop() {
  // Detect key press manually using U8g2 library
    #ifndef USE_KEYPAD_KEYBOARD
      byte key = u8g2.getMenuEvent();
    #endif
    #ifdef USE_KEYPAD_KEYBOARD
      byte key = customKeypad.getKey();
    #endif  
  u8g2.firstPage();
  do {
      u8g2.setColorIndex(1);
      u8g2.setFont(u8g2_font_profont10_mr); // choose a suitable font
      char buffer[16];
      sprintf(buffer,"Speed Z:%d",Quad_Z.GiveMeTheSpeed());
      u8g2.drawStr(2,1,buffer);
      sprintf(buffer,"millis():%ld",millis());
      u8g2.drawStr(2,10,buffer);
      sprintf(buffer,"pos Z:%d",Quad_Z.GetValuePos());
      u8g2.drawStr(2,19,buffer);
      sprintf(buffer,"Moteur1 pos:%ld",Motor1.GetPositionStep());
      u8g2.drawStr(2,28,buffer);
      sprintf(buffer,"Speed Moteur1 max:%d",Motor1.GetMaxSpeed());
      u8g2.drawStr(2,37,buffer);    
  } while (u8g2.nextPage());
  if (key == GEM_KEY_CANCEL) 
  { 
    // Exit animation routine if GEM_KEY_CANCEL key was pressed
    menu.context.exit();
  }
  if (key == GEM_KEY_LEFT) 
  { 
    Motor1.ChangeTargetPositionReal(10); 
  }
  if (key == GEM_KEY_RIGHT) 
  { 
    Motor1.ChangeTargetPositionReal(0);
  }
}
void DebugContextExit() 
{
  menu.reInit();
  menu.drawMenu();
  menu.clearContext();
}

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
}

void PrintInformationOnScreen( char* str)
{
  u8g2.clearBuffer();  
  u8g2.setCursor(0, 0);
  u8g2.print(str);
  u8g2.sendBuffer();  
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
void DisplayDrawInformations()
{
  char buffer_x[16];
  char buffer_y[16];
  char buffer_z[16];
  sprintf(buffer_x,"%+4.3f",Quad_X.GetValue());
  sprintf(buffer_y,"%+4.3f",Quad_Y.GetValue());
  sprintf(buffer_z,"%+4.3f",Quad_Z.GetValue());

  u8g2.firstPage();
  do {
  u8g2.setColorIndex(1);
  //u8g2.setFont(u8g2_font_t0_22_mr); // choose a suitable font
  u8g2.setFont(u8g2_font_profont22_tf); // choose a suitable font
  u8g2.drawStr(2,1,"X");
  u8g2.setColorIndex(1);  
  u8g2.drawStr(20,1,buffer_x);  // write something to the internal memory
  u8g2.drawRFrame(19,0,108,18,3); 
  u8g2.drawStr(2,19,"Y");
  u8g2.setColorIndex(1);
  u8g2.drawStr(20,19,buffer_y);  // write something to the internal memory
  u8g2.drawRFrame(19,18,108,18,3);
  u8g2.drawStr(2,37,"Z");
  u8g2.setColorIndex(1);
  u8g2.drawStr(20,37,buffer_z);  // write something to the internal memory
  u8g2.drawRFrame(19,36,108,18,3);

  u8g2.setFont(u8g2_font_profont10_mr); // choose a suitable font
  u8g2.drawStr(0,54,selectTool.getSelectedOptionName((byte*)&ToolChoose ));

  if(RelativeMode==true)u8g2.drawStr(80,54,"Relative");
  else u8g2.drawStr(80,54,"Absolute");
  //u8g2.sendBuffer();          // transfer internal memory to the display 
  } while (u8g2.nextPage());
}
void UpdateRelAxe()
{  
  if( RelativeMode == true )
  {
    Quad_X.SetRelative();  
    Quad_Y.SetRelative();
    Quad_Z.SetRelative();
  }
  else
  {
    Quad_X.SetAbsolut();  
    Quad_Y.SetAbsolut();
    Quad_Z.SetAbsolut();
  }      
}

void applyTool()
{
  
  
}
