// Include application, user and local libraries
#include "SPI.h"
#include "TFT_22_ILI9225.h"
#include <avr/io.h>
#include <EEPROM.h>
#define TFT_RST 7
#define TFT_RS  9
#define TFT_CS  10  // SS
#define TFT_SDI 11  // MOSI
#define TFT_CLK 13  // SCK
#define TFT_LED 5

#define BUTTON_1 PORTD2
#define BUTTON_2 PORTD3
#define SCROLL PORTC0
#define LED1 PORTD4
#define LED2 PORTD6
// Use hardware SPI (faster - on Uno: 13-SCK, 12-MISO, 11-MOSI)
TFT_22_ILI9225 tft = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS, TFT_LED);

// Variables and constants
uint16_t x, y;
boolean flag = false;
String level;
int addr = 0;
uint16_t highestScore = EEPROM.read(addr);

 // variables for the ball//
int rad = 5;        // Width of the shape
float xpos =3;
float ypos =3;    // Starting position of shape    

float xspeed = 0;  // Speed of the shape
float yspeed = 0;  // Speed of the shape
float nextPos =1;
int xdirection = 1;  // Left or Right
int ydirection = 1;  // Top to Bottom

float prevx = 1;
float prevy = 1;
int width = 176;
int height = 220;

// variables for the paddle//
int xpos1 = 1;
int ypos1 = 215;

int prevx1 = 0;
int prevy1 = 0;
int score = 0;

volatile uint8_t action = 0;
volatile uint8_t action1 = 0;

uint8_t esp = 0;
int pot = 0;
void initInterrupt(){
  PCICR |= (1 << PCIE2); // enable interrupt for portd pins
  PCMSK2 |= (1 << PCINT18) | (1 << PCINT19); // set pd2 and pd3 to trigger interruupt on state change
}

ISR(PCINT2_vect){
  uint8_t i = 0; 
  if((PIND & (1 << BUTTON_1)) == 0){
    delayMicroseconds(1000);
     if((PIND & (1 << BUTTON_1)) == 0){
      action = 1;
     }
    
  }
   if((PIND & (1 << BUTTON_2)) == 0){
     delayMicroseconds(1000);
      if((PIND & (1 << BUTTON_2)) == 0){
        action1++;
      }
     
  }
}
void setup() {
  tft.begin();
  Serial.begin(9600);
  PORTD |= (1 << BUTTON_1) | (1 << BUTTON_2);  // enable internal pull-up resistors for buttons
  DDRD  |= (1 << LED1) | (1 << LED2);
  initInterrupt();
  xpos = width/2;
  ypos = height/2;

}

void loop() {
 //main menu
   tft.setBackgroundColor(COLOR_BLACK);
   for (uint8_t i = 0; i < 127; i++)
   tft.drawPixel(random(tft.maxX()), random(tft.maxY()), random(0xffff));
   tft.setFont(Terminal12x16);
   tft.drawText(55, 30, "PONG", COLOR_BLUE);
   tft.drawRectangle(35, 117, 175, 140, COLOR_GREEN);
   tft.drawText(40, 60, ">> easy", COLOR_WHITE);
   tft.drawText(40, 90, ">> medium", COLOR_WHITE);
   tft.drawText(40, 120, ">> hard", COLOR_WHITE);
   while(!action){
    if (action1 == 0){
      //easy
      tft.drawRectangle(35, 57, 175, 80, COLOR_GREEN);
      tft.drawRectangle(35, 87, 175, 110, COLOR_BLACK);
      tft.drawRectangle(35, 117, 175, 140, COLOR_BLACK);
    }
    else if (action1 == 1){
      //medium
      tft.drawRectangle(35, 57, 175, 80, COLOR_BLACK);
      tft.drawRectangle(35, 87, 175, 110, COLOR_GREEN);
      tft.drawRectangle(35, 117, 175, 140, COLOR_BLACK);
    }
    else if (action1 == 2){
      //hard
      tft.drawRectangle(35, 57, 175, 80, COLOR_BLACK);
      tft.drawRectangle(35, 87, 175, 110, COLOR_BLACK);
      tft.drawRectangle(35, 117, 175, 140, COLOR_GREEN);
    }
    else{
      action1 = 0;
    }
   }
   if (action1 == 0){
    xspeed = 2.0;
    yspeed = 2.0;
    level = "Easy";
   }
   else if (action1 == 1){
    xspeed = 3.0;
    yspeed = 3.0;
    level = "Medium";
   }
   else if (action1 == 2){
    xspeed = 4.0;
    yspeed = 4.0;
    level = "Hard";
   }
   //set action back to zero
   action = 0;
   
   delay(1000);
   tft.clear();
   tft.drawRectangle(0, 0, 175, 20, COLOR_WHITE);
   tft.setFont(Terminal6x8);
   tft.drawText(5, 10, "SCORE: ",COLOR_RED);
   tft.drawText(80, 10, level,COLOR_WHITE);
   while(1){
    drawBall();
    drawPaddle(); 
    tft.drawText(47, 10,String(score));  
    if( esp == 1){
      tft.clear(); 
      break; 
    }
   }
   esp = 0;
   // set ball to the starting position 
   xpos =100;
   ypos = 100;
} 

void drawBall() { 
// Update the position of the shape
  xpos = xpos + ( xspeed * xdirection );
  ypos = ypos + ( yspeed * ydirection );
  
  // Test to see if the shape exceeds the boundaries of the screen
  // If it does, reverse its direction by multiplying by -1
  if (xpos > width-rad || xpos < rad) {
    xdirection *= -1;
  }
  if ( ypos < 30) {
    ydirection *= -1;
  } 
  if(xpos > xpos1 && xpos < xpos1+40 &&ypos >= ypos1-5 && ypos < ypos1 + 2){  // added an offset of -5 since the ball was entering the paddle before changing dir
    ydirection *= -1;
    score++;
   
  }
  else if(ypos > ypos1 +2){
    tft.clear();
    tft.fillRectangle(0, 0, tft.maxX() - 1, tft.maxY() - 1, COLOR_WHITE);
    for (uint8_t i = 0; i < 127; i++)
    tft.drawPixel(random(tft.maxX()), random(tft.maxY()), random(0xffff));
    if (score > highestScore){
      highestScore = score; 
    }
    EEPROM.write(addr, highestScore);
    
    while(!action){
      tft.setBackgroundColor(COLOR_WHITE);
      tft.setFont(Terminal12x16);
      tft.drawText(40, 30, "GAME OVER", COLOR_BLACK);
      tft.setFont(Terminal6x8);
      tft.drawText(65, 50, "SCORE: ",COLOR_BLACK);
      tft.drawText(107, 50,String(score),COLOR_BLACK);
       tft.drawText(40, 65, "HIGHEST SCORE:",COLOR_BLACK);
       tft.drawText(135, 65,String(highestScore),COLOR_BLACK);
    }
    // set score value back to zero
    score = 0;
    action = 0;
    esp = 1;
  }

  // Draw the shape
 tft.drawCircle(prevx, prevy, rad,COLOR_BLACK);
 tft.drawCircle(xpos, ypos, rad,COLOR_GREEN);
 prevx = xpos;
 prevy = ypos;
}
void drawPaddle(){
  pot = analogRead(SCROLL);
  nextPos = map(pot, 0, 1023, 0, 136);   
// Update the position of the shape
  xpos1 = nextPos;

  // Draw the shape
 tft.fillRectangle(prevx1, 215, prevx1+40, 217, COLOR_BLACK);
 tft.fillRectangle(xpos1, ypos1, xpos1+40, ypos1 + 2, COLOR_RED);
 prevx1 = xpos1;
}

