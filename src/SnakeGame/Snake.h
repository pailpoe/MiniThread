#ifndef Snake_h
#define Snake_h   
#include <Arduino.h>
#include <U8g2lib.h>
 

#define SNAKE_KEY_NONE    0 
#define SNAKE_KEY_UP      U8X8_MSG_GPIO_MENU_UP     
#define SNAKE_KEY_RIGHT   U8X8_MSG_GPIO_MENU_NEXT  
#define SNAKE_KEY_DOWN    U8X8_MSG_GPIO_MENU_DOWN   
#define SNAKE_KEY_LEFT    U8X8_MSG_GPIO_MENU_PREV 
 
class Snake
{
public:
	Snake(U8G2& u8g2_);
  void loop(byte keyCode);
  
  
private:
  void draw();
  void draw2();
  void drawText(String text, int x, int y);
  void drawSnake();
  void drawFood();
  void handleColisions();
  void spawnSnakeFood();
  void gameSetup();
  void updateValues();
  void playGame();
  void gamePaused();
  void gameLost();
  void handleInput(); 
  U8G2& _u8g2;
  struct gameItem {
    int X; // x position
    int Y;  //y position
  };  //Last step size in µs
//GameStates
  enum gameState {
    setupGame,
    pausedGame,
    inGame,
    gameOver,
  };
  //Key
  int KeyCode = 0;
  //snakedirection:
  //0: left
  //1: right
  //2: down
  //3: up
  int snakeDir = 1;
  //diameter of the gameItemSize
  int gameItemSize = 4;
  //default snake size
  unsigned int snakeSize = 5;
  //array to store all snake body part positions
  gameItem snake[100];
  //snake food item
  gameItem snakeFood;
  gameState state = setupGame;
};
    
#endif
