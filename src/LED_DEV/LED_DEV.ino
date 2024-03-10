
#include <FastLED.h>

FASTLED_USING_NAMESPACE

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GEN CONTROL
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define DATA_PIN    7
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    114 // 192 - 114 = 78 LED's left
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

#define BRIGHTNESS          25
#define FRAMES_PER_SECOND  120

#define COLOR_SHIFT_DELAY   15
#define SECONDS_PER_PATTERN 30

#define MONITOR_MIDPOINT 53

// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 55, suggested range 20-100 
#define COOLING  100

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKING 200

#define BPM 62

#define NUM_JUNGLE 16

#define HAPTIC_TIMEOUT_SECONDS 1

bool gReverseDirection = false;
bool is_daytime = false;

CRGB leds[NUM_LEDS];
CRGBPalette16 gPal;

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current

uint8_t gCurrentPatternNumberL = 0; // Index number of which pattern is current
uint8_t gCurrentPatternNumberR = 0; // Index number of which pattern is current

uint8_t gHue = 0; // rotating "base color" used by many of the patterns

unsigned long lastSerialMessageTime = 0; // Variable to store the last time a serial message was sent

int random_num = 30;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SETUP
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() {
  delay(500);
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
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

typedef void (*SimplePatternList[])();
typedef void (*SimplePatternList2[])(int start , int end);
// List of patterns to cycle through.  Each is defined as a separate function below.
SimplePatternList gPatterns = {sinelon, rainbowWithGlitter, juggle, bpm, confetti, fire};

SimplePatternList2 gPatternsL = {bpm_range, rainbox_range, juggle_bpm_range, sinelon_range, confetti_range, solid_fill_range};
SimplePatternList2 gPatternsR = {confetti_range, solid_fill_range, rainbox_range, juggle_bpm_range, bpm_range, sinelon_range};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MAIN
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop()
{

  // Check if serial message to take
  if (Serial.available() > 0) {
    // Read the incoming byte:
    char incomingByte = Serial.read();
    // Say what you got:
    Serial.print("Arduino Recieved: ");
    Serial.println(incomingByte);

    // Update the lastSerialMessageTime with the current time
    lastSerialMessageTime = millis();

    // Rest of your code...
    // if the incoming byte decoded in utf-8 is 1, then set the pattern to solid fill
    if (incomingByte == '1') {
      gCurrentPatternNumberL = 5;
    }
    else {
      gCurrentPatternNumberL = gCurrentPatternNumber;
    }

    if (incomingByte == '2') {
      gCurrentPatternNumberR = 1;
    }
    else {
      gCurrentPatternNumberR = gCurrentPatternNumber;
    }
  }
  
  int calc_time_seconds = (millis() - lastSerialMessageTime) / 1000;
  
  // if the calculated time is greater than the haptic timeout, then set the pattern to their parent index
  if (calc_time_seconds > HAPTIC_TIMEOUT_SECONDS) {
    gCurrentPatternNumberL = gCurrentPatternNumber;
    gCurrentPatternNumberR = gCurrentPatternNumber;
  }


  // random split
  // gPatternsL[gCurrentPatternNumber](0, random_num);
  // gPatternsR[gCurrentPatternNumber](random_num+1, NUM_LEDS);

  // monitor split
  gPatternsL[gCurrentPatternNumberL](0, MONITOR_MIDPOINT);
  gPatternsR[gCurrentPatternNumberR](MONITOR_MIDPOINT+1, NUM_LEDS);

  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 

  // do some periodic updates
  EVERY_N_MILLISECONDS( COLOR_SHIFT_DELAY ) { gHue++; } // slowly cycle the "base color" through the rainbow
  EVERY_N_SECONDS( SECONDS_PER_PATTERN )
  { 
    random_num = random(0, NUM_LEDS); 
    // random_num2 = random(random_num, NUM_LEDS);
    nextPattern();
  }
  EVERY_N_SECONDS( 600 ) { gReverseDirection = !gReverseDirection; } 
  EVERY_N_HOURS( 12 ) { is_daytime = !is_daytime; } 
}


void nextPattern()
{
  fadeToBlackBy( leds, NUM_LEDS, 1000);
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
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
