
#include <Adafruit_NeoPixel.h>

//NeoPixel Control pin
#define NeoPIN 15 //GPIO 15
#define BRIGHTNESS 10 //sets brightness
#define flashDELAY 50 //sets flash speed in ms
const double NeoPIXELS = 8; //8 for one strip, 16 for 2

Adafruit_NeoPixel pixels(NeoPIXELS, NeoPIN, NEO_GRB + NEO_KHZ800);

const uint32_t off = pixels.Color(0, 0, 0);
const uint32_t green = pixels.Color(0, 150, 0);
const uint32_t yellow = pixels.Color(255, 255, 0);
const uint32_t red = pixels.Color(150, 0, 0);
const uint32_t blue = pixels.Color(0, 0, 150);

double _1_3pixels = (NeoPIXELS/3)+1;
double _2_3pixels = (2*NeoPIXELS/3)+1;

unsigned long flashTime;
int flash = 0;
int PixelsON = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial);
  pixels.begin();
  pixels.setBrightness(BRIGHTNESS);
  flashTime = millis();
  
}

void loop() {
  //testing only
  Serial.print("PixelsON: ");
  Serial.println(PixelsON);
  Serial.print("1/3: ");
  Serial.println(_1_3pixels);
  Serial.print("/3: ");
  Serial.println(_2_3pixels);
  if (PixelsON > (NeoPIXELS + 1)) PixelsON = 0;
  if (PixelsON == (NeoPIXELS + 1)) {
    for (int i = 0; i<1000;i++){
      set_LEDs(PixelsON);
      delay(1);
    }
  } else {
    set_LEDs(PixelsON);
  }
  delay(1000);
  PixelsON++;
}

//input ranges from 0 -> (NeoPIXELS + 1)
void set_LEDs(int PixelsON){
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
