#include <ESP32Lib.h>
#include <Ressources/Font6x8.h>
#include <Ressources/CodePage437_9x16.h>
#include <Ressources/CodePage437_8x14.h>

#include "redHeart.h"
#include "redPlayer.h"
#include "redBullet.h"

#include "blueHeart.h"
#include "bluePlayer.h"
#include "blueBullet.h"

//pin configuration
const int redPin = 14;
const int greenPin = 19;
const int bluePin = 27;
const int hsyncPin = 32;
const int vsyncPin = 33;

const int maxAmmo = 3;
const int shootInterval = 200;
const int startButton = 18;

const int redUp = 23;
const int redDown = 22;
const int redShoot = 21;
const int redBuzzer = 4;
int redPosY = 130;
int redHeartLeft = 5;
int redAmmo[3][3];
unsigned long redShootCD = 0;

const int blueUp = 0;
const int blueDown = 2;
const int blueShoot = 15;
int bluePosY = 130;
int blueHeartLeft = 5;
int blueAmmo[3][3];
unsigned long blueShootCD = 0;

const int width = 320;
const int height = 240;

unsigned long textBlinkInterval = 500;
bool textBlinkState = true;   // false = Don't, true = Display

int gameState = 0;       // 0 = Start, 1 = Game

//VGA Device
VGA3Bit vga;

void setup()
{
  
  Serial.begin(9600);

  pinMode(startButton, INPUT_PULLUP);

  pinMode(redBuzzer, OUTPUT);
  
  pinMode(redUp, INPUT_PULLUP);
  pinMode(redDown, INPUT_PULLUP);
  pinMode(redShoot, INPUT_PULLUP);
  
  pinMode(blueUp, INPUT_PULLUP);
  pinMode(blueDown, INPUT_PULLUP);
  pinMode(blueShoot, INPUT_PULLUP);
  
  vga.setFrameBufferCount(2);
  vga.init(vga.MODE320x240, redPin, greenPin, bluePin, hsyncPin, vsyncPin);
}

void loop()
{
  while (true)                                          // MENU
  {
    
    if ( gameState == 1 )
    {
      break;
    }
    vga.clear(0);
    vga.rect(0, 1, 320, 239, vga.RGB(255, 255, 255));
    vga.setFont(CodePage437_9x16);
    vga.setCursor(115, 60);
    vga.setTextColor(vga.RGB(255, 255, 255), vga.RGB(0, 0, 0));
    vga.print("1 - 1 POW?");

    vga.setFont(CodePage437_8x14);
    vga.setCursor(70, 140);
    if (textBlinkState)
    {
      vga.print("Press Start to continue");
    }
      
    if (millis() > textBlinkInterval)
    {
      textBlinkInterval = millis() + 800;
      textBlinkState = !textBlinkState;
    }
    
    redPlayer.drawMix(vga, 0, 30, 148);
    bluePlayer.drawMix(vga, 0, 290, 148);

    if ( digitalRead(startButton) == 0 )
    {
      delay(200);
      gameState = 1;
      unsigned long fading = 0;
      while (true)
      {
          vga.fillRect(fading-20, 1, fading-15, 239, vga.RGB(0, 0, 0));
          fading++;
          vga.show();

        if (fading >= 330)
        {
          break;
        }
      }
      break;
    }
    
    vga.show();
  }

  for (int i=0 ; i<maxAmmo ; i++)
    {
      redAmmo[i][0] = 0;
      blueAmmo[i][0] = 0;
    }
  redHeartLeft = 5;
  blueHeartLeft = 5;
  redPosY = 130;
  bluePosY = 130;
  redShootCD = millis() + shootInterval;
  blueShootCD = millis() + shootInterval;
  textBlinkState = false;
  textBlinkInterval = millis();

  while (true)                                          // GAME
  {
    vga.clear();
    
    int HPos = 15;
    for ( int i=1 ; i<=redHeartLeft ; i++ )
    {
      redHeart.drawMix(vga, 0, HPos, 13);
      HPos += 20;
    }

    HPos = 305;
    for ( int i=1 ; i<=blueHeartLeft ; i++ )
    {
      blueHeart.drawMix(vga, 0, HPos, 13);
      HPos -= 20;
    }
    
    if (digitalRead(redUp) == 0)
    {
      redPosY--;
      if (redPosY <= 42)
      {
        redPosY = 42;
      }
    }
    else if ( digitalRead(redDown) == 0 )
    {
      redPosY++;
      if (redPosY >= 222)
      {
        redPosY = 222;
      }
    }

    if (digitalRead(blueUp) == 0)
    {
      bluePosY--;
      if (bluePosY <= 42)
      {
        bluePosY = 42;
      }
    }
    else if ( digitalRead(blueDown) == 0 )
    {
      bluePosY++;
      if (bluePosY >= 222)
      {
        bluePosY = 222;
      }
    }

    if ( digitalRead(redShoot) == 0 && millis() > redShootCD )
    {
      redShootCD = millis() + shootInterval;
      for ( int i=0 ; i<maxAmmo ; i++ )
      {
        if (redAmmo[i][0] == 0)
        {
          redAmmo[i][0] = 1;
          redAmmo[i][1] = 60;
          redAmmo[i][2] = redPosY;
          break;
        }
      }
    }

    if ( digitalRead(blueShoot) == 0 && millis() > blueShootCD )
    {
      blueShootCD = millis() + shootInterval;
      for ( int i=0 ; i<maxAmmo ; i++ )
      {
        if (blueAmmo[i][0] == 0)
        {
          blueAmmo[i][0] = 1;
          blueAmmo[i][1] = 260;
          blueAmmo[i][2] = bluePosY;
          break;
        }
      }
    }

    for ( int i=0 ; i<maxAmmo ; i++ )
    {
      if ( redAmmo[i][0] == 1 && redHeartLeft != 0)
      {

        if ( redAmmo[i][1]+17 >= 265 && ( (redAmmo[i][2]+4 >= bluePosY-17 && redAmmo[i][2]+4 <= bluePosY+17) || (redAmmo[i][2]-4 >= bluePosY-17 && redAmmo[i][2]-4 <= bluePosY+17)))
        {
          redAmmo[i][0] = 0;
          blueHeartLeft--;
        }
        else
        {
          redBullet.draw(vga, 0, redAmmo[i][1], redAmmo[i][2]);
          redAmmo[i][1] += 3;
          if ( redAmmo[i][1] >= 321 )
          {
            redAmmo[i][0] = 0;
          }
        }
      }

      if ( blueAmmo[i][0] == 1 && blueHeartLeft != 0 )
      {

        if ( blueAmmo[i][1]-17 <= 55 && ( (blueAmmo[i][2]+4 >= redPosY-17 && blueAmmo[i][2]+4 <= redPosY+17) || (blueAmmo[i][2]-4 >= redPosY-17 && blueAmmo[i][2]-4 <= redPosY+17)))
        {
          blueAmmo[i][0] = 0;
          redHeartLeft--;
        }
        else
        {
          blueBullet.draw(vga, 0, blueAmmo[i][1], blueAmmo[i][2]);
          blueAmmo[i][1] -= 3;
          if ( blueAmmo[i][1] <= 0 )
          {
            blueAmmo[i][0] = 0;
          }
        }
      }
    }

    if ( redHeartLeft != 0 )
    {
      redPlayer.drawMix(vga, 0, 30, redPosY);
    }
    else
    {
      redPosY = 0;
    }
    

    if ( blueHeartLeft != 0 )
    {
      bluePlayer.drawMix(vga, 0, 290, bluePosY);
    }
    else
    {
      bluePosY = 0;
    }
    
    vga.rect(0, 25, 320, 215, vga.RGB(255, 255, 255));

    if ( redHeartLeft == 0 || blueHeartLeft == 0 )
    {
      vga.setFont(CodePage437_9x16);
      if ( blueHeartLeft == 0 )
      {
        vga.setCursor(121, 80);
        vga.setTextColor(vga.RGB(255, 0, 0), vga.RGB(0, 0, 0));
        vga.print("RED WINS!");
      }
      else
      {
        vga.setCursor(115, 80);
        vga.setTextColor(vga.RGB(0, 0, 255), vga.RGB(0, 0, 0));
        vga.print("BLUE WINS!");
      }
      vga.setFont(CodePage437_8x14);
      vga.setCursor(75, 150);
      vga.setTextColor(vga.RGB(255, 255, 255), vga.RGB(0, 0, 0));

        if (textBlinkState)
      {
        vga.print("Press Start to rematch");
      }
        
      if (millis() > textBlinkInterval)
      {
        textBlinkInterval = millis() + 800;
        textBlinkState = !textBlinkState;
      }

      if ( digitalRead(startButton) == 0 )
      {
        delay(200);
        gameState = 1;
        unsigned long fading = 0;
        while (true)
        {
            vga.fillRect(fading-20, 1, fading-15, 239, vga.RGB(0, 0, 0));
            fading++;
            vga.show();
  
          if (fading >= 320)
          {
            break;
          }
        }
        break;
      }
    }
    
    vga.show();
  }

  
}
