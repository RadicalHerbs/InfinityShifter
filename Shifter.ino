/* Infinity Shifter Code
 *  Charles Baird
 */

#include <FastLED.h>
#include <Math.h>
#include <EEPROM.h>

// Number of LEDs in strip
#define NUM_LEDS 20

// Raw value limits for photoresistor
// To call changeColor()
//  rv exceeds LIGHTLIM
// Mapping values for raw values
// LOPHO - This and smaller
//  will map to LOBRI
// HIPHO - This and higher
//  will map to HIBRI
#define LIGHTLIM 300
#define LOPHO 20
#define HIPHO 150
#define LOBRI 25
#define HIBRI 255

// Data pin is middle pin on strip
// Clock pin for SPI chipsets only
#define DATA_PIN 7
// #define CLOCK_PIN 13

// Photoresistor analog pin
#define LIGHT_PIN 1

//  Array of leds
CRGB leds[NUM_LEDS];

/*
 * Settings contains the hue and drag
 *  of the colors.
 *  
 *  hue is the first parameter of CHSV
 *  drag is [0-4], values increase in
 *    proportion to speed and how far
 *    from base hue the colors stretch
 *  
 *  @hue: int value of hue
 *  @drag: [0-4] drag value
 */
 
struct Settings{
  int hue;
  int drag;
};

// Global Settings Object s
Settings s;

void setup() {
    delay( 1500 );

    // Add strip (GRB)
    FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);

    // Get Settings from address 0
    EEPROM.get(0, s);

    // If Settings are invalid, reset
    //  to default of (blue, low drag)
    if(s.hue < 0 || s.drag < 0 || s.hue > 255 || s.drag > 4) {
      s.hue = 135;
      s.drag = 0;

      EEPROM.put(0, s);
    }
}

// COME BACK AND CHANGE nscale to factor in drag
void fadeall() {for(int i = 0; i < NUM_LEDS; i++) { leds[i].nscale8(100 + s.drag * 25); }}

/*
 * changeColor runs when a light source is
 *  directly pointed at the photoresistor.
 *  
 */
void changeColor(){
  // count keeps track of color changes
  // If count reaches dragChangeNum,
  //  drag value changes after delay
  //
  int count = 0;
  int dragChangeNum = 5;

  // Small bounce
  delay(100);

  // Run while light source is applied
  while(analogRead(LIGHT_PIN) > LIGHTLIM){
    // Cycle through hues
    if(count < dragChangeNum){
      s.hue = (s.hue + 20) % 255;
    }

    // Configure with new hue
    for(int i = 0; i < NUM_LEDS; i++){
      leds[i] = CHSV(s.hue, 255, 255);
    }

    // Blackout to indicate
    // no changes (hue adjusted later)
    // to Settings
    if(count == dragChangeNum){
      for(int j = 0; j < NUM_LEDS; j++){
        leds[j] = CRGB::Black;
      }
    }

    // As drag value increases,
    // lights will brighten
    // Drag will change by one and
    // hue will be adjusted back
    if(count > dragChangeNum){
      s.drag = (s.drag + 1) % 5;
      for(int k = 0; k < NUM_LEDS; k++){
        leds[k] = CHSV(s.hue, 255, 127 + s.drag * 32);
      }
    }
   
    // Show changed settings
    FastLED.show();
    delay(1000);

    count++;
  }

  // If the drag changes, change
  // the hue back to original
  if(count > dragChangeNum){
    s.hue = (s.hue + 155) % 255;
  }
  // Write changed Settings
  // object to address 0
  EEPROM.put(0, s);
}

void loop() {
    // Control from photoresistor analog input
    uint8_t BRT = map(constrain(analogRead(LIGHT_PIN),
      LOPHO, HIPHO), LOPHO, HIPHO, LOBRI, HIBRI);

    // If external light is applied to sensor,
    // start changeColor
    if(analogRead(LIGHT_PIN) > LIGHTLIM) {
      changeColor();
    }

    // Go halfway out, making the colors warmer
    // by the drag amount with each step
    // and then come back
    for(int i = 0; i < NUM_LEDS; i++){
      if(i < NUM_LEDS / 2) {
        leds[i] = CHSV(s.hue = (s.hue - s.drag) % 255, 255, BRT);
      }
      if(i >= NUM_LEDS / 2) {
        leds[i] = CHSV(s.hue = (s.hue + s.drag) % 255, 255, BRT);
      }

      FastLED.show();

      fadeall();

      // Slower speeds will have smaller tails
      delay(50 - (s.drag * 10));
    }
}
