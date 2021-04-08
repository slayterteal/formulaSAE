/*
  Display code for the MSP3218 SPI Screen for use by the OSU Formula SAE Team
  Designed by Slayter Teal
  TFT_eSPI Library is created by Bodmer {https://github.com/Bodmer/TFT_eSPI) 
*/

//#define NEEDLE_LENGTH 35  // Visible length
//#define NEEDLE_WIDTH   5  // Width of needle - make it an odd number
//#define NEEDLE_RADIUS 90  // Radius at tip
//#define NEEDLE_COLOR1 TFT_MAROON  // Needle periphery colour
//#define NEEDLE_COLOR2 TFT_RED     // Needle centre colour
#define GEAR_CENTRE_X 100
#define GEAR_CENTRE_Y 60 // for dial2.h
#define SPEED_CENTER_X 230
#define SPEED_CENTER_Y 50
#define REV_CENTER_X 230
#define REV_CENTER_Y 160
#define TEMP_CENTER_X 100
#define TEMP_CENTER_Y 180

// Font attached to this sketch
#include "NotoSansBold36.h"
#define AA_FONT_LARGE NotoSansBold36

#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite needle = TFT_eSprite(&tft); // Sprite object for needle
TFT_eSprite spr    = TFT_eSprite(&tft); // Sprite for meter reading

uint16_t* tft_buffer;
bool      buffer_loaded = false;
uint16_t  spr_width = 0;

// =======================================================================================
// Setup
// =======================================================================================
void setup()   {
  Serial.begin(115200); // Debug only

  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(TFT_WHITE);

  // uint16_t bg_color = tft.readPixel(120, 120); // Get colour from dial centre
  uint16_t bg_color = TFT_WHITE;

  // text back for the speedometer
  tft.drawCircle(SPEED_CENTER_X+30, SPEED_CENTER_Y+15, 40, TFT_DARKGREY);
  tft.setTextColor(TFT_BLACK, bg_color);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("(MPH)", SPEED_CENTER_X+30, SPEED_CENTER_Y + 50, 2);

  // text box for the Revs
  tft.drawCircle(REV_CENTER_X+30, REV_CENTER_Y+15, 50, TFT_DARKGREY);
  tft.setTextColor(TFT_BLACK, bg_color);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("(REVS x10)", REV_CENTER_X+30, REV_CENTER_Y + 60, 2);

  // text box for the gear
  tft.drawCircle(GEAR_CENTRE_X, GEAR_CENTRE_Y, 55, TFT_DARKGREY);
  tft.setTextColor(TFT_BLACK, bg_color);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("(GEAR)", GEAR_CENTRE_X, GEAR_CENTRE_Y + 48, 4);

  //text box for the temp
  tft.drawCircle(TEMP_CENTER_X, TEMP_CENTER_Y, 55, TFT_DARKGREY);
  tft.setTextColor(TFT_BLACK, bg_color);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("(TEMP `F)", TEMP_CENTER_X, TEMP_CENTER_Y + 48, 4);

  // create the spite for the engine values
  spr.loadFont(AA_FONT_LARGE);
  spr_width = spr.textWidth("277");
  //spr.createSprite(spr_width, spr.fontHeight());
  spr.fillSprite(bg_color);
  spr.setTextColor(TFT_BLACK, bg_color);
  spr.setTextDatum(MC_DATUM);
  spr.setTextPadding(spr_width);
  //spr.drawNumber(0, spr_width/2, spr.fontHeight()/2);
  //spr.pushSprite(DIAL_CENTRE_X - spr_width / 2, DIAL_CENTRE_Y - spr.fontHeight() / 2);

  delay(2000);
}
uint16_t i = 0;
// =======================================================================================
// Loop
// =======================================================================================
void loop() {
  uint16_t angle = random(241); // random speed in range 0 to 240
  
  uint16_t testgear = random(7);
  uint16_t testrev = random(900);
  //uint16_t i = 0;
  if(i == 300) i = 0;
  i = i+1;

  // draw a number to the speed
  spr_width = spr.textWidth("277");
  spr.createSprite(spr_width, spr.fontHeight());
  spr.setTextPadding(spr_width);
  spr.drawNumber(angle, spr_width/2, spr.fontHeight()/2);
  spr.pushSprite(SPEED_CENTER_X, SPEED_CENTER_Y);

  // draw a number to the revs
  spr_width = spr.textWidth("277");
  spr.createSprite(spr_width, spr.fontHeight());
  spr.setTextPadding(spr_width);
  spr.drawNumber(testrev, spr_width/2, spr.fontHeight()/2);
  spr.pushSprite(REV_CENTER_X, REV_CENTER_Y);
  
  // draw a number to the gear
  spr_width = spr.textWidth("277");
  spr.createSprite(spr_width, spr.fontHeight());
  spr.setTextPadding(spr_width);
  spr.drawNumber(testgear, spr_width/2, spr.fontHeight()/2, 6);
  spr.pushSprite(GEAR_CENTRE_X - spr_width / 2, GEAR_CENTRE_Y - spr.fontHeight() / 2);

  // draw a number to the temp
  spr_width = spr.textWidth("277");
  spr.createSprite(spr_width, spr.fontHeight());
  spr.setTextPadding(spr_width);
  spr.drawNumber(i, spr_width/2, spr.fontHeight()/2, 6);
  spr.pushSprite(TEMP_CENTER_X - spr_width / 2, TEMP_CENTER_Y - spr.fontHeight() / 2);

  delay(60);
}
// =======================================================================================
