#include "SPI.h"
#include "LPD8806.h"

// Example to control LPD8806-based RGB LED Modules in a strip

/*****************************************************************************/

// Number of RGB LEDs in strand:
unsigned int nLEDs = 32;

// Chose 2 pins for output; can be any valid output pins:
int dataPin  = 2;
int clockPin = 3;

// First parameter is the number of LEDs in the strand.  The LED strips
// are 32 LEDs per meter but you can extend or cut the strip.  Next two
// parameters are SPI data and clock pins:
LPD8806 strip = LPD8806(nLEDs, dataPin, clockPin);

// You can optionally use hardware SPI for faster writes, just leave out
// the data and clock pin parameters.  But this does limit use to very
// specific pins on the Arduino.  For "classic" Arduinos (Uno, Duemilanove,
// etc.), data = pin 11, clock = pin 13.  For Arduino Mega, data = pin 51,
// clock = pin 52.  For 32u4 Breakout Board+ and Teensy, data = pin B2,
// clock = pin B1.  For Leonardo, this can ONLY be done on the ICSP pins.
//LPD8806 strip = LPD8806(nLEDs);

void setup() {
  // Start up the LED strip
  strip.begin();

  // Update the strip, to start they are all 'off'
  strip.show();

  Serial.begin(115200);
}

void rainbow(uint8_t wait);
void rainbowCycle(uint8_t wait);
void colorWipe(uint32_t c, uint8_t wait);
void colorChase(uint32_t c, uint8_t wait);
uint32_t Wheel(uint16_t WheelPos);

typedef struct {
  int x;
  int y;
  int z;
} accelerometer_values;

float read_accel(accelerometer_values &accel)
{
  static float total = 596.384;
  unsigned long x, y, z;
  x = analogRead(A3);
  y = analogRead(A4);
  z = analogRead(A5);

  accel.x = x;
  accel.y = y;
  accel.z = z;
  float normd = sqrt(x*x + y*y + z*z);
  if (normd < 600 && normd > 585)
    total = (total + normd) / 2;
  return normd/total;
  Serial.println(normd/total);
  //Serial.print(sqrt(x*x + y*y + z*z));
  Serial.print(" ");
  //Serial.println(x*x + y*y + z*z);
}

//bool freefall(accelerometer_values &accel)
bool freefall(accelerometer_values &accel)
{
  if (accel.x < 100 || accel.y < 100 || accel.z < 100){
    return true;
   }
  return false;
}

bool landed(accelerometer_values &accel)
{
  if (accel.x > 400 || accel.y > 400 || accel.z > 400) {
    return true;
  }
  return false;
}

void blank_strip()
{
  for (unsigned int i=0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, 0);
  }  
}

void simple_anim()
{
  static int i = 0;

  static int j = 1;
  static int k = 2;

  //dir = 1;
  blank_strip();
  strip.setPixelColor(i/10, strip.Color((i%3) * 40, (j%3) * 40, (k%3) * 40));
  //i = (i+dir) % (strip.numPixels()*10);
  //if (i == 0)
  ++j;
  ++k;
  strip.show();
}

void set_strip_color(uint32_t color)
{
  for (unsigned int i=0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
  }
}

void loop()
{
  rainbow(0);
return;
  static bool jump_flag = false;
  accelerometer_values accel;
  // read accelerometer
  read_accel(accel);
  //Serial.println("hi");
/**
  Serial.print(accel.x);
  Serial.print(" ");
  Serial.print(accel.y);
  Serial.print(" ");
  Serial.print(accel.z);
  Serial.println("");
**/
  if (freefall(accel)) {
    //zero g
    blank_strip();
    strip.show();
    jump_flag = true;
  } else if (landed(accel) && jump_flag) {
    unsigned int col, i;
    blank_strip();
    for (col=40; col >= 0; --col){
      for (i=0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, strip.Color(col,col,col));
      }  
      strip.show();
      delay(10);
    }
    jump_flag = false;
  }

  // Send a simple pixel chase in...
/**
  colorChase(strip.Color(127, 127, 127), 50); // White
  colorChase(strip.Color(127,   0,   0), 50); // Red
  colorChase(strip.Color(127, 127,   0), 50); // Yellow
  colorChase(strip.Color(  0, 127,   0), 50); // Green
  colorChase(strip.Color(  0, 127, 127), 50); // Cyan
  colorChase(strip.Color(  0,   0, 127), 50); // Blue
  colorChase(strip.Color(127,   0, 127), 50); // Violet
  // Fill the entire strip with...
  colorWipe(strip.Color(127,   0,   0), 50);  // Red
  colorWipe(strip.Color(  0, 127,   0), 50);  // Green
  colorWipe(strip.Color(  0,   0, 127), 50);  // Blue

**/
  if(!jump_flag){
    simple_anim();
    //Serial.println("done rainbow");
  }
  //rainbow(0);
  //rainbowCycle(0);  // make it go through the cycle fairly fast
}

void rainbow(uint8_t wait) {
  unsigned int i, j;
   
  for (j=0; j < 384; j++) {     // 3 cycles of all 384 colors in the wheel
    for (i=0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel( (i + j) % 384));
    }  
    strip.show();   // write all the pixels out
    delay(wait);
  }
}

// Slightly different, this one makes the rainbow wheel equally distributed 
// along the chain
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;
  
  for (j=0; j < 384 * 5; j++) {     // 5 cycles of all 384 colors in the wheel
    for (i=0; i < strip.numPixels(); i++) {
      // tricky math! we use each pixel as a fraction of the full 384-color wheel
      // (thats the i / strip.numPixels() part)
      // Then add in j which makes the colors go around per pixel
      // the % 384 is to make the wheel cycle around
      strip.setPixelColor(i, Wheel( ((i * 384 / strip.numPixels()) + j) % 384) );
    }  
    strip.show();   // write all the pixels out
    delay(wait);
  }
}

// Fill the dots progressively along the strip.
void colorWipe(uint32_t c, uint8_t wait) {
  unsigned int i;

  for (i=0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}

// Chase one dot down the full strip.
void colorChase(uint32_t c, uint8_t wait) {
  unsigned int i;

  // Start by turning all pixels off:
  for(i=0; i<strip.numPixels(); i++) strip.setPixelColor(i, 0);

  // Then display one pixel at a time:
  for(i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c); // Set new pixel 'on'
    strip.show();              // Refresh LED states
    strip.setPixelColor(i, 0); // Erase pixel, but don't refresh!
    delay(wait);
  }

  strip.show(); // Refresh to turn off last pixel
}

/* Helper functions */

//Input a value 0 to 384 to get a color value.
//The colours are a transition r - g -b - back to r

uint32_t Wheel(uint16_t WheelPos)
{
  byte r, g, b;
  switch(WheelPos / 128)
  {
    case 0:
      r = 127 - WheelPos % 128;   //Red down
      g = WheelPos % 128;      // Green up
      b = 0;                  //blue off
      break; 
    case 1:
      g = 127 - WheelPos % 128;  //green down
      b = WheelPos % 128;      //blue up
      r = 0;                  //red off
      break; 
    case 2:
      b = 127 - WheelPos % 128;  //blue down 
      r = WheelPos % 128;      //red up
      g = 0;                  //green off
      break; 
  }
  return(strip.Color(r,g,b));
}
