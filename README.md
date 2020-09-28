# Dro-Mini
## Presentation
A Digital readout (DRO) display for lathe or milling machine with :
- A STM32F103CBT6 microcontroler
- 3 quadrature encoder input
- A Oled SSD1306 128 x 64 screen with hardware I2C communication
- 6 push buttons for interaction with the User. 
- Use the Arduino ide (STM32duino)

## Input / Output
### Keyboard
The system use 6 push buttons for the navigation (joystick configuration) : 
- Up button ( pin PB15 )
- Down button ( pin PB12 )
- Left button ( pin PB0 ) 
- Right button ( pin PB1 )
- Ok button ( pin PB15 )
- Cancel button ( pin PB13 )

The GEM menu library is used : https://github.com/Spirik/GEM

### Quadrature encoder input
The system can handle 3 quadrature encoder : 
- Quadrature decoder X on Timer1 ( A=PA8 and B=PA9)
- Quadrature decoder Y on Timer3 ( A=PA6 and B=PA7)
- Quadrature decoder Z on Timer2 ( A=PA0 and B=PA1)
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