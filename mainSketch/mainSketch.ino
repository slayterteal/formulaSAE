/*
  Display code for the MSP3218 SPI Screen for use by the OSU Formula SAE Team
  Designed by Slayter Teal
  TFT_eSPI Library is created by Bodmer {https://github.com/Bodmer/TFT_eSPI) 
*/
#include <CAN.h>
#include <Adafruit_NeoPixel.h>

//Pin Definitions Constants
#define rxr_can 4 //GPIO4
#define txt_can 5 //GPIO5
#define NeoPIN 22 //GPIO 22

// screen (X,Y) pairs
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

//NeoPixel Constants
#define BRIGHTNESS 10 //sets brightness
#define flashDELAY 50 //sets flash speed in ms
const double NeoPIXELS = 8; //8 for one strip, 16 for 2
const double _1_3pixels = (NeoPIXELS/3)+1;
const double _2_3pixels = (2*NeoPIXELS/3)+1;

void CAN_Handler( void * parameter);
void set_LEDs(int PixelsON);
TaskHandle_t CAN_Bus;
Adafruit_NeoPixel pixels(NeoPIXELS, NeoPIN, NEO_GRB + NEO_KHZ800);

//Colors
const uint32_t off = pixels.Color(0, 0, 0);
const uint32_t green = pixels.Color(0, 150, 0);
const uint32_t yellow = pixels.Color(255, 255, 0);
const uint32_t red = pixels.Color(150, 0, 0);
const uint32_t blue = pixels.Color(0, 0, 150);

//NeoPixel Global variables
unsigned long flashTime;
int flash = 0;
int PixelsON = 0;

//Global variables to be filled by CAN messages and then displayed
//message: 0x01F0A000
double engine_speed = -1;               // 0-25,599.94 rpm
double throttle = -1;                   // 0-990998 %
double intake_air_temp = -1;            //-198.4-260.6 Deg F  #2c#
double coolant_temp = -1;               //-198.4-260.6 Deg F  #2c#

//message: 0x01F0A003
double afr_1 = -1;                      // 7.325-21.916 AFR
double afr_2 = -1;                      // 7.325-21.916 AFR
double vehicle_speed = -1;              // 0-255.996 mph
double gear = -1;                       // 0-255 unitless
double ign_timing = -1;                 //-17-72.65 Deg
double battery_voltage = -1;            // 0-16.089 volts

//message: 0x01F0A004
double manifold_absolute_pressure = -1; //-14.696-935.81 PSI(g)
double ve = -1;                         // 0-255 %
double fuel_pressure = -1;              // 0-147.939 PSI(g)
double oil_pressure = -1;               // 0-147.939 PSI(g)
double afr_target = -1;                 // 7.325-21.916 AFR

//Modifiers
//message: 0x01F0A000
double m_engine_speed = 0.39063;     // rpm/bit 
double m_throttle = 0.0015259;       // %/bit
double m_temp = 1.8;                 // Deg F/bit
double fahrenheit_offset = 32;       // Deg F

//message: 0x01F0A003
double m_afr = 0.057227;             // AFR/bit
double afr_offset = 7.325;           // AFR
double m_vehicle_speed = 0.00390625; // mph/bit
double m_gear = 1;                   // unitless
double m_ign_timing = 0.35156;       // Deg/bit
double ign_offset = -17;             // Deg
double m_battery_voltage = 0.0002455;// Volts/bit

//message: 0x01F0A004
double m_map = 0.014504;             // PSI/bit 
double map_offset = -14.6960;        // PSI(g)
double m_ve = 1;                     // %/bit
double m_pressure = 0.580151;        // PSIg/bit

void setup() {
  Serial.begin(115200);
  while (!Serial);
  
//==========================TFT Initilization==================================// 
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(TFT_WHITE);

  // uint16_t bg_color = tft.readPixel(120, 120); // Get colour from dial centre
  uint16_t bg_color = TFT_WHITE;
  uint16_t circlecolor = TFT_BLACK;

  // text back for the speedometer
  tft.drawCircle(SPEED_CENTER_X+30, SPEED_CENTER_Y+15, 40, circlecolor);
  tft.setTextColor(TFT_BLACK, bg_color);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("(MPH)", SPEED_CENTER_X+30, SPEED_CENTER_Y + 50, 2);

  // text box for the Revs
  tft.drawCircle(REV_CENTER_X+30, REV_CENTER_Y+15, 50, circlecolor);
  tft.setTextColor(TFT_BLACK, bg_color);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("(REVS)", REV_CENTER_X+30, REV_CENTER_Y + 60, 2);

  // text box for the gear
  tft.drawCircle(GEAR_CENTRE_X, GEAR_CENTRE_Y, 55, circlecolor);
  tft.setTextColor(TFT_BLACK, bg_color);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("(GEAR)", GEAR_CENTRE_X, GEAR_CENTRE_Y + 48, 4);

  //text box for the temp
  tft.drawCircle(TEMP_CENTER_X, TEMP_CENTER_Y, 55, circlecolor);
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
//======================END OF TFT SETUP=================================================//

  //Set CAN to core 0
  xTaskCreatePinnedToCore(CAN_Handler, "CAN_Bus", 10000, NULL, 0, &CAN_Bus, 0);
  CAN.setPins(rxr_can, txt_can);
  
  // start the CAN bus at 500 kbps
  if (!CAN.begin(500E3)) {
    //make sure to display this failure onscreen
    Serial.println("Starting CAN failed!");
    while (1);
  }
  Serial.println("Starting CAN success");
  
  //Initialize NeoPixels
  pixels.begin();
  pixels.setBrightness(BRIGHTNESS);
  pixels.clear();
  flashTime = millis();
}

uint16_t i = 0;
double rev_segment = 15000 / (NeoPIXELS+1);
// =======================================================================================
// Loop
// =======================================================================================
void loop() {
//uint16_t angle = random(241); // random speed in range 0 to 240
//  Testing code 
//  uint16_t testgear = random(7);
//double testrev = random(900);
//  //uint16_t i = 0;
//  if(i == 600){
//    i = 0;
//  };
//  i = i+1;

  //================LED LOOP=======================//
  if (engine_speed == 0 ) {
    set_LEDs(0);
  } else if( engine_speed <= rev_segment ){
    set_LEDs(1);
  }
  else if( engine_speed <= (2*rev_segment)){
    set_LEDs(2);
  }
  else if( engine_speed <= (3*rev_segment)){
    set_LEDs(3);
  }
  else if( engine_speed <= (4*rev_segment)){
    set_LEDs(4);
  }
  else if( engine_speed <= (5*rev_segment)){
    set_LEDs(5);
  }
  else if( engine_speed <= (6*rev_segment)){
    set_LEDs(6);
  }
  else if( engine_speed <= (7*rev_segment)){
    set_LEDs(7);
  }
  else if( engine_speed <= (8*rev_segment)){
    set_LEDs(8);
  }
  else{
    set_LEDs(9);
  }

  // draw a number to the speed
  spr_width = spr.textWidth("277");
  spr.createSprite(spr_width, spr.fontHeight());
  spr.setTextPadding(spr_width);
  spr.drawNumber(vehicle_speed, spr_width/2, spr.fontHeight()/2);
  spr.pushSprite(SPEED_CENTER_X, SPEED_CENTER_Y);

  // draw a number to the revs
  spr_width = spr.textWidth("277");
  spr.createSprite(spr_width, spr.fontHeight());
  spr.setTextPadding(spr_width);
  spr.drawNumber(engine_speed, spr_width/2, spr.fontHeight()/2);
  spr.pushSprite(REV_CENTER_X, REV_CENTER_Y);
  
  // draw a number to the gear
  spr_width = spr.textWidth("277");
  spr.createSprite(spr_width, spr.fontHeight());
  spr.setTextPadding(spr_width);
  spr.drawNumber(gear, spr_width/2, spr.fontHeight()/2, 6);
  spr.pushSprite(GEAR_CENTRE_X - spr_width / 2, GEAR_CENTRE_Y - spr.fontHeight() / 2);

  // draw a number to the temp
  spr_width = spr.textWidth("277");
  spr.createSprite(spr_width, spr.fontHeight());
  spr.setTextPadding(spr_width);
  spr.drawNumber(coolant_temp, spr_width/2, spr.fontHeight()/2, 6);
  spr.pushSprite(TEMP_CENTER_X - spr_width / 2, TEMP_CENTER_Y - spr.fontHeight() / 2);

  delay(60);
}

//Sets NeoPixels
void set_LEDs(int PixelsON){ //input ranges from 0 -> (NeoPIXELS + 1)
  if ((millis()-flashTime) > flashDELAY) {
    flash = abs(flash-1);
    flashTime = millis();
  }
  if (PixelsON == 0) pixels.clear();
  else {
    //Green
    if (PixelsON < _1_3pixels) {
      for (int i = 0; i<PixelsON; i++) pixels.setPixelColor(i, green); //Pixels Green
      
    //Red
    } else if ((PixelsON > _1_3pixels)&& (PixelsON < _2_3pixels)) {
      for (int i = 0; i<int(_1_3pixels); i++) pixels.setPixelColor(i, green); //Pixels Green
      for (int i = int(_1_3pixels); i<PixelsON; i++) pixels.setPixelColor(i, red); //Pixels Red
      
    //Blue
    } else if (PixelsON > _2_3pixels && PixelsON<(NeoPIXELS + 1)) {
      for (int i = 0; i<int(_1_3pixels); i++) pixels.setPixelColor(i, green); //Pixels Green
      for (int i = int(_1_3pixels); i<int(_2_3pixels); i++) pixels.setPixelColor(i, red); //Pixels Red
      for (int i = int(_2_3pixels); i<PixelsON; i++) pixels.setPixelColor(i, blue); //Pixels Red
      
    //Flashing Red
    } else if (PixelsON == (NeoPIXELS + 1)) {
      if (flash == 1) for (int i = 0; i<NeoPIXELS; i++)pixels.setPixelColor(i, red); //Pixels Red
      else if (flash == 0) pixels.clear();

    //Something went wrong
    } else {
      for (int i = 0; i<NeoPIXELS; i++)pixels.setPixelColor(i, yellow); //Pixels yellow
    }
    for (int i = PixelsON; i<NeoPIXELS; i++) pixels.setPixelColor(i, off); //Pixels off
  }
  
  pixels.show();
}

int _2c8bit(int num){
  if (num > 0x7F) num -= 0x100;
  return num;
}

void CAN_Handler( void * parameter){
  for(;;) {
    //Serial.print(xPortGetCoreID());
    int packet_size = CAN.parsePacket();
    if (packet_size) {
      // received a packet
      Serial.print("Received");
    
      if (CAN.packetExtended()) {
        Serial.print("extended ");
      }
    
      if (CAN.packetRtr()) {
        // Remote transmission request, packet contains no data
        Serial.print("RTR ");
      }
    
      //Serial.print("packet with id 0x");
      long ID = CAN.packetId();
      Serial.print(ID, HEX);
    
      if (CAN.packetRtr()) {
      } else {
        Serial.print(" and length ");
        Serial.println(packet_size);
    
        //Create array to hold message data
        int message[8];

        //only print packet data for non-RTR packets
        //assuming big endian
        int i = 0;
        while (CAN.available()) {
          int message_data = CAN.read();
          Serial.print(message_data);
          message[i++] = message_data;
        }
        //AEM Infinity Series 3 IDs
        switch(ID){
            //Engine Speed 0-1, Throttle 4-5, Intake Air Temp 6, Coolant Temp 7
            case 0x1F0A000:
                //template: variable = message * modifier + offset
                engine_speed = (message[0]*16*16 + message[1]) * m_engine_speed;
                throttle = (message[4]*16*16 + message[5]) * m_throttle;
                intake_air_temp = (_2c8bit(message[6]*1) * m_temp) + fahrenheit_offset;
                coolant_temp = (_2c8bit(message[7]*1) * m_temp) + fahrenheit_offset;
                break;
            //AFR #1 0, AFR #2 1, Vehicle Speed 2-3, Gear Calculated 4, Ign Timing 5, Battery Volts 6-7
            case 0x1F0A003:
                afr_1 = message[0] * m_afr + afr_offset;
                afr_2 = message[1] * m_afr + afr_offset;
                vehicle_speed = (message[2]*16*16 + message[3]) * m_vehicle_speed;
                gear = message[4] * m_gear;
                ign_timing = message[5] * m_ign_timing + ign_offset;
                battery_voltage = (message[6]*16*16 + message[7]) * m_battery_voltage;
                break;
            //MAP 0-1, VE 2, FuelPressure 3, OilPressure 4, AFRTarget 5, 6 and 7 are boolean variables that could be used as lamps
            case 0x1F0A004:
                manifold_absolute_pressure = (message[0]*16*16 + message[1]) * m_map + map_offset;
                ve = message[2] * m_ve;
                fuel_pressure = message[3] * m_pressure;
                oil_pressure = message[4] * m_pressure;
                afr_target = message[5] * m_afr + afr_offset;
                break;
            default:
                Serial.print(" message was not used");
                break;
          }
        Serial.println();
      }
    
      Serial.println();
    }
  }
}
