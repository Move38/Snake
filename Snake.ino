/*
   Blinks - Snake (rather inspired by)

   Rotate around a single Blink with a trail of light (or tail)
   Continue to rotate around Blink until button is pressed
   When pressed, send the snake into the neighboring Blink
   Rotation reverses in the neighboring blink to appear as a smooth 
   hand-off from one Blink to the other (looks like a Do-si-do)
   Snake can rotate through rainbow(HSB) and use its color to allow
   for picking up items... i.e. can only collect an apple of color x
   when the snake is ~color x. This requires staying alive while the 
   color changes.

   Snake can only rotate in a single Blink for up to 5 times before dying.
   Pressing a button spawns a new snake (only if tile is not already part of
   active game play)

   Game:
   Collect as many apples? as you can without running into a wall? (i.e. press when facing outward)
   Move to as many tiles as possible while avoiding obstacles (fire/bombs...)

   End game has the snake explode and trigger chain reaction
*/
#include "blinklib.h"
//#include "blinkani.h"

// Information to convey over message
// Game Mode (0-3) - 2 bits
// Passing of snake
//  snake hue (0-31) - 5 bits
//  snake length (3-31) - 5 bits?
//
// pass information in a byte
//
// 0 - 5 bits for hue - 2 bits for game mode
// i.e.
// 01000001 = hue of 16 and game mode of GAMEPLAY

enum Mode { 
    ATTRACT,        // No snake spawned, perform an attract mode animation... 
    GAMEPLAY,       // Snake spawned and moving from tile to tile, don't allow 2 snakes to spawn
    GAMEOVER,       // Snake died, show chain reaction
    SCORE           // Display number of targets acquired... or some endgame/score keeping 
};

enum Direction { 
    CLOCKWISE,            //  
    COUNTER_CLOCKWISE     //
};

static Mode mode = GAMEPLAY; 

static bool isSnake = false;
static byte snakeFace = 0;
static byte snakeHue = 0;
static byte snakeLength = 3;  // maybe can trail multiple tiles...
static Direction snakeDirection = CLOCKWISE;

static uint32_t snakeFaceIncrement_ms = 80;
static uint32_t nextSnakeFaceIncrement_ms = 0;

static uint32_t snakeHueIncrement_ms = 500;
static uint32_t nextSnakeHueIncrement_ms = 0;

void setup() {
  // put your setup code here, to run once:
  //blinkAniBegin();
  setColor(OFF);
}

void loop() {
  // put your main code here, to run repeatedly:
  uint32_t now = millis();
  
  if(mode == ATTRACT) {

//    setColor(GREEN); // DEBUG MODE
    
    if(buttonSingleClicked()){
      spawnSnake();
    }

    FOREACH_FACE(f) {
        if (irIsReadyOnFace(f)) {
          byte receivedMessage = irGetData(f);
//          if((receivedMessage & 3) == GAMEPLAY) {
            mode = GAMEPLAY;
            // alert neighbors to enter gameplay
            byte data = mode;     // mode in low bits
            irBroadcastData(data);

//          }
        }
    }
    
  }
  else if(mode == GAMEPLAY) {

    // if has snake, update snake
    if( isSnake ) {

      if(buttonSingleClicked()){
        passSnake();
      }
      if(buttonDoubleClicked()){
      }
      if(buttonMultiClicked()) {
      }

      // update snake position
      if( now >= nextSnakeFaceIncrement_ms ) {
        snakeFace = (snakeFace + 1 ) % 6;
        nextSnakeFaceIncrement_ms = now + snakeFaceIncrement_ms;
      }

      // update snake hue
      if( now >= nextSnakeHueIncrement_ms ) {
        snakeHue = (snakeHue + 1 ) % 32;
        nextSnakeHueIncrement_ms = now + snakeHueIncrement_ms;
      }

      // draw snake
      drawSnake(); 
    }
    
    else {
      // no snake
      if(buttonSingleClicked()){
        // blink to show recognition of touch?
      }
      if(buttonDoubleClicked()){
        spawnSnake();
      }
      if(buttonMultiClicked()) {
        if(buttonClickCount() == 3) {
          reset();
        }
      }

      setColor(OFF);
//      setColor(YELLOW); // DEBUG MODE

      
      FOREACH_FACE(f) {
        if (irIsReadyOnFace(f)) {
          byte receivedMessage = irGetData(f);    
          if (((receivedMessage & 32) >> 5) == COUNTER_CLOCKWISE) {
             snakeDirection = CLOCKWISE;
          }else {
             snakeDirection = COUNTER_CLOCKWISE;
          }

          snakeHue = 4 * ((receivedMessage & 31) >> 2);
          isSnake = true;

          // snake starts on face received on
          snakeFace = f;
        }
      }

    }
    
    // if snake received accept snake
    // if no snake, wait for snake
  }
  else if(mode == GAMEOVER) {
    // explode
    // pass explosion to neighboring Blinks
  }
  else if(mode == SCORE) {
  
  }
  
}

void reset() {
  isSnake = false;
  snakeFace = 0;
  snakeHue = 0;
  snakeLength = 3;
  mode = GAMEPLAY;
}

void spawnSnake() {
  isSnake = true;
  snakeFace = 0;
  snakeHue = 0;
  snakeLength = 3;
  mode = GAMEPLAY;
//  byte data = mode;     // mode in low bits
//  irBroadcastData(data);
}

void passSnake() {
  isSnake = false;
  byte data = (snakeDirection << 5) + ((snakeHue/4) << 2) + mode; // store hue in high bits, mode in low bits
//  irBroadcastData(data);
  irSendData( snakeFace, data);
}

void drawSnake() {
  // clear the display buffer
  setColor(OFF);

  // update the color of the snake
  int dir = 1;
  if(snakeDirection == COUNTER_CLOCKWISE) {
    dir = -1;
  }

  FOREACH_FACE(f) {
    
    byte brightness;
    byte distFromHead = (6 + snakeFace - (dir*f)) % 6;  // returns # of positions away from the head
    byte hueAdjusted = 8 * ((32 + snakeHue - distFromHead) % 32);
    
    if(distFromHead < snakeLength) {
      
      brightness = 255 - (64 * distFromHead); // scale the brightness to 8 bits and dimmer based on distance from head
    
    } else {
    
      brightness = 0; // don't show past the tail of the snake
    
    }
    
    setFaceColor(f, makeColorHSB( hueAdjusted, 255, brightness));
  }
}

