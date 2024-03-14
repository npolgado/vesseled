#include <FastLED.h>

FASTLED_USING_NAMESPACE

#define ARRAY_SIZE(A)           (sizeof(A) / sizeof((A)[0]))

#define DATA_PIN_TOP            4
#define DATA_PIN_BOTTOM         7

#define LED_TYPE                WS2812B
#define COLOR_ORDER             GRB

#define NUM_LEDS_TOP            144
#define NUM_LEDS_BOTTOM         192
#define NUM_LEDS                (NUM_LEDS_TOP + NUM_LEDS_BOTTOM)

#define TOP_STRIP_MIDPOINT      80
#define BOTTOM_STRIP_MIDPOINT   (NUM_LEDS_TOP + 54)

#define BRIGHTNESS              10
#define FRAMES_PER_SECOND       240

#define BPM                     76
#define NUM_JUNGLE              3
#define SPARKING                120
#define COOLING                 55
#define HAPTIC_TIMEOUT_SECONDS  1

void rainbow();
void rainbowWithGlitter();
void addGlitter( fract8 chanceOfGlitter);
void confetti();
void sinelon();
void juggle();
void bpm();
void nextPattern();
void solid_fill();
void solid_fill_range(int start, int end);
void fire();
void confetti_range(int start, int end);
void sinelon_range(int start, int end);
void bpm_range(int start, int end);
void juggle_bpm_range(int start, int end);
void rainbox_range(int start, int end);
bool gReverseDirection = false;

CRGBPalette16 gPal;
CRGB leds[NUM_LEDS];

// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
typedef void (*SimplePatternList2[])(int start , int end);
SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm, fire};
SimplePatternList2 gPatternsRange = {bpm_range, rainbox_range, juggle_bpm_range, sinelon_range, confetti_range, solid_fill_range};

uint8_t gCurrentPatternNumber = 0;
uint8_t gCurrentPatternNumberRange = 0;

uint8_t gHue = 0; // rotating "base color" used by many of the patterns

unsigned long lastSerialMessageTime = 0; // Variable to store the last time a serial message was sent
char incomingByte;
int incomingInt;

void setup() {
  delay(3000);

  // tell FastLED there's leds on pin 4, starting at index 0 in the led array
  FastLED.addLeds<LED_TYPE, DATA_PIN_TOP>(leds, 0, NUM_LEDS_TOP);
  // tell FastLED there's leds on pin 7, starting at index 144 in the led array
  FastLED.addLeds<LED_TYPE, DATA_PIN_BOTTOM>(leds, NUM_LEDS_TOP, NUM_LEDS_BOTTOM);
  // 0-144 = TOP
  // 144-336 = BOTTOM
  FastLED.setBrightness(BRIGHTNESS);
  gPal = HeatColors_p;

  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Red;
  }
  FastLED.show();
  delay(500);
  FastLED.clear();
  FastLED.show();
  delay(500);

  Serial.begin(115200);
  while(!Serial) { ; }
  Serial.setTimeout(1000);
}

void loop()
{
  /////////
  // DEMO IMPLEMENTATION
  /////////
  // gPatterns[gCurrentPatternNumber]();

  /////////
  // RANGED IMPLEMENTATION
  /////////
  // sinelon_range(0, TOP_STRIP_MIDPOINT-1);
  // bpm_range(TOP_STRIP_MIDPOINT, NUM_LEDS_TOP-1);
  // rainbox_range(NUM_LEDS_TOP, BOTTOM_STRIP_MIDPOINT-1);
  // juggle_bpm_range(BOTTOM_STRIP_MIDPOINT, NUM_LEDS);
  // addGlitter(80);

  gPatternsRange[gCurrentPatternNumberRange](NUM_LEDS_TOP+1, NUM_LEDS);
  gPatternsRange[gCurrentPatternNumber](0, NUM_LEDS_TOP-1);

  // Check if serial message to take
  if (Serial.available() > 0) {
    incomingByte = Serial.read();
    Serial.print("Arduino Recieved: ");

    // incomingInt = aincomingInt);

    // Update the lastSerialMessageTime with the current time
    lastSerialMessageTime = millis();
  }
  else
  {
    incomingInt = TOP_STRIP_MIDPOINT;
  }

  // light up strip @ led[incomingByte] with a color
  // leds[incomingInt] = CRGB::Red;

  FastLED.show();
  FastLED.delay(1000/FRAMES_PER_SECOND); 

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 )
  {
    //change gHue to be a random number from 0-255
    // gHue = random8();

    gHue++;
    gHue = gHue % 255;
  }

  EVERY_N_SECONDS( 10 )
  { 
    fadeToBlackBy( leds, NUM_LEDS, 1000);
    // add one to the current pattern number, and wrap around at the end

    // change the index to a random number from 0-6
    gCurrentPatternNumber = random8(0, 6);
    gCurrentPatternNumberRange = random8(0, 5);

    // gCurrentPatternNumber = (gCurrentPatternNumber + 1) % 6; // ARRAY_SIZE( gPatterns);
  } 
}

void solid_fill()
{
  // use fast LED to fill the strip with a solid color
  // use the hue shift to change the color
  fill_solid(leds, NUM_LEDS, CHSV(gHue, 255, 255));
}

void solid_fill_range(int start, int end)
{
  // use fast LED to fill the strip with a solid color
  // use the hue shift to change the color
  for( int i = start; i < end; ++i)
  {
    leds[i] = CRGB(gHue, 255, 255);
  }
}

void fire()
{
  random16_add_entropy( random());
  // Array of temperature readings at each simulation cell
  static uint8_t heat[NUM_LEDS];

    // Step 1.  Cool down every cell a little
    for( int i = 0; i < NUM_LEDS; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
    }
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= NUM_LEDS - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < SPARKING ) {
      int y = random8(7);
      heat[y] = qadd8( heat[y], random8(160,255) );
    }

    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < NUM_LEDS; j++) {
      // Scale the heat value from 0-255 down to 0-240
      // for best results with color palettes.
      uint8_t colorindex = scale8( heat[j], 240);
      CRGB color = ColorFromPalette( gPal, colorindex);
      int pixelnumber;
      if( gReverseDirection ) {
        pixelnumber = (NUM_LEDS-1) - j;
      } else {
        pixelnumber = j;
      }
      leds[pixelnumber] = color;
    } 
}

void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void rainbox_range(int start, int end)
{
  CHSV hsv;
  hsv.hue = gHue;
  hsv.val = 255;
  hsv.sat = 240;
  for( int i = start; i < end; ++i) {
      leds[i] = hsv;
      hsv.hue += 7;
  }
}

void rainbowWithGlitter() 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(50);
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti() 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void confetti_range(int start, int end) 
{
  // pointer to LED at MONITOR_MIDPOINT+1 --> NUM_LEDS
  int total = abs(end - start);
  CRGB* ledPtr = &leds[start];

  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy(ledPtr, total, 10);
  int pos = random16(start, end);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS-1 );
  leds[pos] += CHSV( gHue, 255, 192);
}

void sinelon_range(int start, int end)
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, start, end-1 );
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void bpm_range(int start, int end)
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BPM, 64, 255);
  for( int i = start; i < end; i++) {
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  uint8_t dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

void juggle_bpm_range(int start, int end) {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  uint8_t dothue = 0;
  for( int i = 0; i < NUM_JUNGLE; i++) {
    leds[beatsin16( i+7, start, end-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}
