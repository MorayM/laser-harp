#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>

#define MIDI_SERIAL Serial1
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

constexpr uint8_t LCD_BACKLIGHT_OFF = 0x0;
constexpr uint8_t LCD_BACKLIGHT_RED = 0x1;
constexpr uint8_t LCD_BACKLIGHT_GREEN = 0x2;
constexpr uint8_t LCD_BACKLIGHT_BLUE = 0x4;
constexpr uint8_t LCD_BACKLIGHT_YELLOW = LCD_BACKLIGHT_RED | LCD_BACKLIGHT_GREEN;
constexpr uint8_t LCD_BACKLIGHT_CYAN = LCD_BACKLIGHT_GREEN | LCD_BACKLIGHT_BLUE;
constexpr uint8_t LCD_BACKLIGHT_MAGENTA = LCD_BACKLIGHT_RED | LCD_BACKLIGHT_BLUE;
constexpr uint8_t LCD_BACKLIGHT_WHITE = LCD_BACKLIGHT_RED | LCD_BACKLIGHT_GREEN | LCD_BACKLIGHT_BLUE;

constexpr uint8_t HARP_MODE_SETUP = 0;
constexpr uint8_t HARP_MODE_PLAY = 1;

constexpr uint8_t SENSOR_COUNT = 10;
constexpr uint8_t SENSOR_PINS[SENSOR_COUNT] = {22, 23, 24, 25, 26, 27, 28, 29, 30, 31};
// Notes per string (MIDI note numbers). Adjust to taste.
//                                      A3  Bb3 B3  C4  D4  E4  F4  F#4 G4  A4
uint8_t tomStringNotes[SENSOR_COUNT] = {57, 58, 59, 60, 62, 64, 65, 66, 67, 69};
//                                      C4  G4  C5  F5  G5  Ab5 Bb5 C5  Db5 Eb5
uint8_t ox7StringNotes[SENSOR_COUNT] = {60, 67, 72, 77, 79, 80, 82, 84, 85, 87};

// State
uint8_t harpMode = HARP_MODE_SETUP;
uint8_t midiChannel = 0;
uint8_t midiVelocity = 100;

// -------------------------
// MIDI helpers
// -------------------------

inline void midiWriteByte(uint8_t b)
{
  MIDI_SERIAL.write(b);
}

// MIDI Note On (3 bytes)
void midiNoteOn(uint8_t ch, uint8_t note, uint8_t vel)
{
  midiWriteByte(0x90 | (ch & 0x0F));
  midiWriteByte(note & 0x7F);
  midiWriteByte(vel & 0x7F);
}

// MIDI Note Off (3 bytes) — can also use Note On with vel=0, but this is explicit
void midiNoteOff(uint8_t ch, uint8_t note)
{
  midiWriteByte(0x80 | (ch & 0x0F));
  midiWriteByte(note & 0x7F);
  midiWriteByte(0);
}

void setup()
{
  Serial.begin(9600);       // Debug serial
  MIDI_SERIAL.begin(31250); // MIDI baud rate

  // Inputs
  for (uint8_t i = 0; i < SENSOR_COUNT; i++)
  {
    // Use INPUT_PULLUP if your relay/contact pulls to GND when closed.
    // Otherwise use INPUT and provide external pull-ups/pull-downs appropriately.
    pinMode(SENSOR_PINS[i], INPUT_PULLUP);
  }

  lcd.begin(16, 2);
  lcd.setBacklight(LCD_BACKLIGHT_GREEN); // Start with backlight off
  Serial.println("Harp started");
}

void loop()
{
  /**
   * If in setup mode, see a menu structure on the LCD.
   * Navigate with up and down arrows, change values with left and right arrows, and select with the "Select" button.
   * Menu options:
   * - String mode: Tom or Ox7 (or more in the future)
   * - MIDI channel: 1-16
   * - MIDI velocity: 1-127
   * - Switch to play mode
   *
   *
   * If in play mode, read the sensors and send MIDI notes accordingly.
   * Pressing the "Select" button switches back to setup mode to allow changing the string note mapping.
   */
}
