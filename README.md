# MiniThread
## Presentation
A Digital readout (DRO) display for lathe with threading :
- A STM32F103CBT6 microcontroler
- 3 quadrature encoder input
- 2 stepper motor output
- A Oled SSD1306 128 x 64 screen with hardware I2C communication
- 6 push buttons for interaction with the User. 
- Use the Arduino ide (STM32duino)

## Input / Output
### Keyboard
The system use 6 push buttons for the navigation (joystick configuration) and it's possible to add 10 extra buttons for other functions  
- Up button : Navigate up through the menu list, select next value of the digit/char of editable variable, or previous option in select.
- Down button : Navigate down through the menu items list, select previous value of the digit/char of editable variable, or next option in select.
- Left button : Navigate through the Back button to the previous menu page, select previous digit/char of editable variable
- Right button : Navigate through the link to another (child) menu page, select next digit/char of editable variable, execute code associated with button
- Ok button : Toggle boolean menu item, enter edit mode of the associated non-boolean variable, exit edit mode with saving the variable, execute code associated with button
- Cancel button : Navigate to the previous (parent) menu page, exit edit mode without saving the variable, exit context loop if allowed within context's settings

The GEM menu library is used : https://github.com/Spirik/GEM

### Quadrature encoder input
The system can handle 3 quadrature encoder : 
- Quadrature decoder X on Timer1 ( A=PA8 and B=PA9) for the longitudinal
- Quadrature decoder Y on Timer3 ( A=PA6 and B=PA7) for the transversal
- Quadrature decoder C on Timer2 ( A=PA0 and B=PA1) for the Spindle
### Oled display
The system use a Oled SSD1306 128 x 64 screen with hardware I2C communication. 
- SCL = PB6 ( need a 1,5kOhm external resistance to vcc) 
- SDA = PB7 ( need a 1,5kOhm external resistance to vcc)

The graphical library used is u8g2 : https://github.com/olikraus/u8g2

## Installation
1. Install ARDUINO IDE V1.8.8
2. Download a zip file containing the Arduino STM32 files : https://github.com/rogerclarkmelbourne/Arduino_STM32/archive/master.zip
3. Unzip the content of the zip file, and place the Arduino_STM32 folder to [Arduino sketches folder]/[hardware]/[Arduino_STM32]. 
Create the 'hardware' folder if it does not exist yet.
Example how the path should look like: C:\Users\<user>\Documents\Arduino\hardware\Arduino_STM32  
4. Restart ARDUINO IDE
5. Choose : Tools > Board:"Generic ST32F103C series"
6. Choose : Tools > Variant:"STMF32C103CB (20k RAM, 128k Flash)"
7. Choose : Tools > Upload Method:"STLink"
8. Choose : Tools > CPU Speed Mhz:"72Mhz (Normal)"
9. Add additionnal board manager URL : File > Preferences > Additional Boards Manager URLs : "http://dan.drown.org/stm32duino/package_STM32duino_index.json"
10. Install Arduino SAM Boards : Tools > Boards Manager > Install Arduino SAM boards ( 32bits cortex M3)
11. Install the libraries (Tools > Manage libraries ): u8g2 
12. Compile ...