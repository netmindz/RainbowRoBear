#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <WiFiClient.h>
  #include <ESP8266WebServer.h>
#elif defined(ESP32)
  #include <WiFi.h>
  #include <WiFiClient.h>
  #include <WebServer.h>
#endif

#include "wifi.h"
// Create file with the following
// *************************************************************************
// #define SECRET_SSID "";  /* Replace with your SSID */
// #define SECRET_PSK "";   /* Replace with your WPA2 passphrase */
// *************************************************************************
const char ssid[] = SECRET_SSID;
const char passphrase[] = SECRET_PSK;


#include <ElegantOTA.h>

#include <FastLED.h>
#include <MIDI.h>

MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, MIDI); // Feather32 uses Serial2

FASTLED_USING_NAMESPACE

#define DATA_PIN    23 // https://learn.adafruit.com/assets/111179
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    40
CRGB leds[NUM_LEDS];

#define BRIGHTNESS         100
#define FRAMES_PER_SECOND  120

WebServer server(80);

void setup() {
  Serial.begin(115200);
  delay(3000); // 3 second delay for recovery

  WiFi.mode(WIFI_STA);
//  Serial.printf("Connecting to %s ", SECRET_SSID);

  if (passphrase != NULL)
    WiFi.begin(ssid, passphrase);
  else
    WiFi.begin(ssid);
  Serial.println("");

  // Wait for connection
  int sanity = 0;
//  while (WiFi.status() != WL_CONNECTED) {
//    delay(500);
//    Serial.print(".");
//      sanity++;
//      if(sanity > 20) break;
//  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  delay(1000);

  server.on("/", []() {
    server.send(200, "text/plain", "Rainbow RoBear, try going to /update");
  });
  ElegantOTA.begin(&server);    // Start ElegantOTA
  server.begin();
  Serial.println("HTTP server started");
  
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);

  Serial.println("Setting up MIDI");

  // Connect the handleNoteOn function to the library,
  // so it is called upon reception of a NoteOn.
  MIDI.setHandleNoteOn(handleNoteOn);  // Put only the name of the function

  // Do the same for NoteOffs
  MIDI.setHandleNoteOff(handleNoteOff);

  // Initiate MIDI communications, listen to all channels
  MIDI.begin(MIDI_CHANNEL_OMNI);

  Serial.println("End Setup");
}



uint8_t gHue = 0; // rotating "base color" used by many of the patterns
  
void loop()
{

  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  // insert a delay to keep the framerate modest
//  FastLED.delay(1000/FRAMES_PER_SECOND); 


  // Call MIDI.read the fastest you can for real-time performance.
  MIDI.read();

  // There is no need to check if there are messages incoming
  // if they are bound to a Callback function.
  // The attached method will be called automatically
  // when the corresponding message has been received.

  EVERY_N_MILLISECONDS( 5 ) {
    fadeToBlackBy(leds, NUM_LEDS, 1);
  }
}

// -----------------------------------------------------------------------------

// This function will be automatically called when a NoteOn is received.
// It must be a void-returning function with the correct parameters,
// see documentation here:
// https://github.com/FortySevenEffects/arduino_midi_library/wiki/Using-Callbacks

void handleNoteOn(byte channel, byte pitch, byte velocity)
{
    // Do whatever you want when a note is pressed.

    // Try to keep your callbacks short (no delays ect)
    // otherwise it would slow down the loop() and have a bad impact
    // on real-time performance.
    Serial.printf("handleNoteOn channel=%s pitch=%s velocity=%s\n", (String) channel, (String) pitch, (String) velocity);

    // Turn on the first LED Red when any note pressed
//    leds[0] = CRGB::Red;

    // Light up all the LEDs using pitch=hue,value=velocity
//    fill_solid(leds, NUM_LEDS, CHSV(map(pitch, 21, 108, 0, 255), 255, map(velocity, 0, 127, 0, 255)));

    // Use map of the range of pitch values of piano to the which LED
    // Create a colour using HueSaturationValue (aka brightness) using pitch=hue,value=velocity
    leds[map(pitch, 21, 108, 0, (NUM_LEDS - 1))] = CHSV(map(pitch, 0, 127, 0, 255), 255, map(velocity, 0, 127, 0, 255));
}

void handleNoteOff(byte channel, byte pitch, byte velocity)
{
    // Do something when the note is released.
    // Note that NoteOn messages with 0 velocity are interpreted as NoteOffs.
   Serial.printf("handleNoteOff channel=%s pitch=%s velocity=%s\n", (String) channel, (String) pitch, (String) velocity);
//   leds[0] = CRGB::Black;
  // Turn the note you released black
//   leds[map(pitch, 21, 108, 0, (NUM_LEDS - 1))] = CRGB::Black;
}

// -----------------------------------------------------------------------------

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}


void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}
