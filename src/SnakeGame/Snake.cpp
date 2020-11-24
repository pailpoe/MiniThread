#include <Arduino.h>
#include "Snake.h"

Snake::Snake(U8G2& u8g2_)
: _u8g2(u8g2_)
{

}
void Snake::draw() 
{
  drawSnake();
  drawFood();
}
void Snake::draw2() 
{
  drawText("Game Over", 30, 30);
}
void Snake::drawText(String text, int x, int y) 
{
  //char array to print
  char charBuff[10];
  //convert the string to char array
  text.toCharArray(charBuff, 10);
  //prepare the font
  _u8g2.setFont(u8g_font_5x8);
  //print the text
  _u8g2.drawStr(x, y, charBuff);
}

void Snake::drawSnake() 
{
  for (int i = 0; i < snakeSize; i++) {
    _u8g2.drawFrame(snake[i].X, snake[i].Y, gameItemSize, gameItemSize);
  }
}
void Snake::drawFood() 
{
  _u8g2.drawBox(snakeFood.X, snakeFood.Y, gameItemSize, gameItemSize);
}

void Snake::handleColisions() 
{
  //check if snake eats food
  if (snake[0].X == snakeFood.X && snake[0].Y == snakeFood.Y) {
    //increase snakeSize
    snakeSize++;
    //regen food
    spawnSnakeFood();
  }

  //check if snake collides with itself
  else {
    for (int i = 1; i < snakeSize; i++) {
      if (snake[0].X == snake[i].X && snake[0].Y == snake[i].Y) {
        state = gameOver;
      }
    }
  }
  //check for wall collisions
  if ((snake[0].X < 1) || (snake[0].Y < 1) || (snake[0].X > 127) || (snake[0].Y > 63)) {
    state = gameOver;
  }
}
void Snake::spawnSnakeFood() 
{
  //generate snake Food position
  do {
    snakeFood.X = random(2, 126);
  } while (snakeFood.X % 4 != 0);
  do {
    snakeFood.Y = random(2, 62);
  } while (snakeFood.Y % 4 != 0);
}

void Snake::gameSetup() {
  //Reset snake size
  snakeSize = 5;
  //snakeHead initial position
  int sHeadX;
  int sHeadY;
  do {
    sHeadX = random(2, 126);
    
  } while (sHeadX % gameItemSize != 0);
  do {
    sHeadY = random(2, 62);
  } while (sHeadY % gameItemSize != 0);

  //set first part of the snake
  snake[0].X = sHeadX;
  snake[0].Y = sHeadY;
  //generate random positions for the rest of the snake
  for (int i = 1; i < snakeSize; i++) {
    //choose random direction to build snake
    int dir = random(0, 3);

    if (dir == 0)//means left
    {
      snake[i].X = snake[i-1].X - gameItemSize;
      snake[i].Y = snake[i - 1].Y;
    }

    else if (dir == 1)//means right
    {
      snake[i].X = snake[i - 1].X + gameItemSize;
      snake[i].Y = snake[i - 1].Y;
    }

    else if (dir == 2)//means up
    {
      snake[i].X = snake[i - 1].X;
      snake[i].Y = snake[i - 1].Y - gameItemSize;
    }

    else if (dir == 3)//means down
    {
      snake[i].X = snake[i - 1].X;
      snake[i].Y = snake[i - 1].Y + gameItemSize;
    }
  }
  //generate snake Food position
  spawnSnakeFood();

  //change state to inGame
  state = pausedGame;
}

void Snake::handleInput() 
{
  if (KeyCode == SNAKE_KEY_LEFT && snakeDir != 1) {
    snakeDir = 0;
  }

  if (KeyCode == SNAKE_KEY_RIGHT && snakeDir != 0) {
    snakeDir = 1;
  }

  if (KeyCode == SNAKE_KEY_DOWN && snakeDir != 3) {
    snakeDir = 2;
  }

  if (KeyCode == SNAKE_KEY_UP && snakeDir != 2) {
    snakeDir = 3;
  }
}

void Snake::updateValues() 
{
  //update all body parts of the snake excpet the head
  for (int i = snakeSize - 1; i > 0; i--) {
    snake[i] = snake[i - 1];
  }

  //Now update the head
  //move left
  if (snakeDir == 0) {

    snake[0].X -= gameItemSize;
  }
  //move right
  else if (snakeDir == 1) {

    snake[0].X += gameItemSize;
  }

  //move down
  else if (snakeDir == 2) {

    snake[0].Y += gameItemSize;
  }

  //move up
  else if (snakeDir == 3) {

    snake[0].Y -= gameItemSize;
  }
}

void Snake::playGame() 
{
  handleColisions();
  handleInput();
  updateValues(); 
  _u8g2.firstPage();
  do {
    
    draw();
    delay(50);
  } while (_u8g2.nextPage());

}

void Snake::gamePaused() 
{
  if (KeyCode != SNAKE_KEY_NONE) 
  {
    state = inGame;
  }
  _u8g2.firstPage();
  do {

    draw();
    delay(50);
  } while (_u8g2.nextPage());
}
void Snake::gameLost() 
{
  _u8g2.firstPage();
  do {
    draw2();
    delay(50);
  } while (_u8g2.nextPage());
  delay(2000);
  state = setupGame;
}

void Snake::loop(byte keyCode) 
{
  KeyCode = keyCode; 
  switch (state)
  {
  case setupGame: gameSetup();
    break;
  case pausedGame: gamePaused();
    break;
  case inGame: playGame();
    break;
  case gameOver: gameLost();
    break;
  }
}
