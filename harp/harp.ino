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

// Menu
constexpr uint8_t MENU_COUNT = 4;
constexpr uint8_t MENU_STRING_MODE = 0;
constexpr uint8_t MENU_MIDI_CHANNEL = 1;
constexpr uint8_t MENU_MIDI_VELOCITY = 2;
constexpr uint8_t MENU_PLAY = 3;

const char *menuLabels[MENU_COUNT] = {
    "String mode",
    "MIDI channel",
    "MIDI velocity",
    ">> Play"};

constexpr uint8_t STRING_MODE_COUNT = 2;
const char *stringModeLabels[STRING_MODE_COUNT] = {"Tom", "Ox7"};
uint8_t *stringModeNotes[STRING_MODE_COUNT] = {tomStringNotes, ox7StringNotes};

// Sensor config
constexpr bool SENSOR_ACTIVE_LOW = false; // HIGH = beam broken (relay closes)
constexpr unsigned long DEBOUNCE_MS = 5;
constexpr unsigned long UI_UPDATE_MS = 50; // 20 Hz button/LCD poll in play mode

// State — menu
uint8_t harpMode = HARP_MODE_SETUP;
uint8_t menuIndex = 0;
uint8_t stringMode = 0;
uint8_t *stringNotes = tomStringNotes;
uint8_t midiChannel = 0;
uint8_t midiVelocity = 100;
uint8_t lastButtons = 0;
bool lcdDirty = true;

// State — play mode
uint16_t sensorState = 0;                  // debounced state (bit=1 = beam broken)
uint16_t prevSensorState = 0;              // previous debounced state for edge detect
uint16_t lastRawState = 0;                 // raw reading from previous scan
unsigned long debounceStart[SENSOR_COUNT]; // per-channel debounce timers
unsigned long lastUiUpdate = 0;

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

// -------------------------
// Sensor helpers
// -------------------------

// Read all sensor pins into a bitmask (bit=1 = beam broken)
uint16_t scanSensors()
{
  uint16_t bits = 0;
  for (uint8_t i = 0; i < SENSOR_COUNT; i++)
  {
    bool level = digitalRead(SENSOR_PINS[i]);
    if (SENSOR_ACTIVE_LOW)
      level = !level;
    if (level)
      bits |= (1 << i);
  }
  return bits;
}

// Send CC 123 (All Notes Off) then zero state
void allNotesOff()
{
  midiWriteByte(0xB0 | (midiChannel & 0x0F)); // Control Change
  midiWriteByte(123);                         // All Notes Off
  midiWriteByte(0);
  sensorState = 0;
  prevSensorState = 0;
}

// -------------------------
// LCD helpers
// -------------------------

// Draw the setup menu for the current menuIndex
void drawMenu()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(menuLabels[menuIndex]);

  lcd.setCursor(0, 1);
  switch (menuIndex)
  {
  case MENU_STRING_MODE:
    lcd.print("< ");
    lcd.print(stringModeLabels[stringMode]);
    lcd.setCursor(15, 1);
    lcd.print(">");
    break;
  case MENU_MIDI_CHANNEL:
    lcd.print("< ");
    lcd.print(midiChannel + 1); // display 1-16
    lcd.setCursor(15, 1);
    lcd.print(">");
    break;
  case MENU_MIDI_VELOCITY:
    lcd.print("< ");
    lcd.print(midiVelocity);
    lcd.setCursor(15, 1);
    lcd.print(">");
    break;
  case MENU_PLAY:
    lcd.print("SELECT to start");
    break;
  }
}

// Draw the play mode status screen
void drawPlayScreen()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Playing...");
  lcd.setCursor(0, 1);
  lcd.print("Ch:");
  if (midiChannel + 1 < 10)
    lcd.print("0");
  lcd.print(midiChannel + 1);
  lcd.print(" Vel:");
  lcd.print(midiVelocity);
}

// -------------------------
// Mode loops
// -------------------------

void setupModeLoop()
{
  uint8_t buttons = lcd.readButtons();
  uint8_t pressed = buttons & ~lastButtons; // rising edge only
  lastButtons = buttons;

  if (!pressed && !lcdDirty)
    return;

  if (pressed & BUTTON_UP)
  {
    if (menuIndex > 0)
      menuIndex--;
    lcdDirty = true;
  }
  if (pressed & BUTTON_DOWN)
  {
    if (menuIndex < MENU_COUNT - 1)
      menuIndex++;
    lcdDirty = true;
  }
  if (pressed & BUTTON_LEFT)
  {
    switch (menuIndex)
    {
    case MENU_STRING_MODE:
      if (stringMode > 0)
        stringMode--;
      stringNotes = stringModeNotes[stringMode];
      break;
    case MENU_MIDI_CHANNEL:
      if (midiChannel > 0)
        midiChannel--;
      break;
    case MENU_MIDI_VELOCITY:
      if (midiVelocity > 1)
        midiVelocity--;
      break;
    }
    lcdDirty = true;
  }
  if (pressed & BUTTON_RIGHT)
  {
    switch (menuIndex)
    {
    case MENU_STRING_MODE:
      if (stringMode < STRING_MODE_COUNT - 1)
        stringMode++;
      stringNotes = stringModeNotes[stringMode];
      break;
    case MENU_MIDI_CHANNEL:
      if (midiChannel < 15)
        midiChannel++;
      break;
    case MENU_MIDI_VELOCITY:
      if (midiVelocity < 127)
        midiVelocity++;
      break;
    }
    lcdDirty = true;
  }
  if (pressed & BUTTON_SELECT)
  {
    // Snapshot current sensors to avoid spurious edges on entry
    uint16_t snap = scanSensors();
    sensorState = snap;
    prevSensorState = snap;
    lastRawState = snap;
    unsigned long now = millis();
    for (uint8_t i = 0; i < SENSOR_COUNT; i++)
      debounceStart[i] = now;
    lastUiUpdate = now;

    harpMode = HARP_MODE_PLAY;
    lcdDirty = true;
    lcd.setBacklight(LCD_BACKLIGHT_RED);
    drawPlayScreen();
    Serial.println("Entering play mode");
    return;
  }

  if (lcdDirty)
  {
    drawMenu();
    lcdDirty = false;
  }
}

void playModeLoop()
{
  unsigned long now = millis();

  // 1. Scan sensors
  uint16_t raw = scanSensors();

  // 2. Per-channel debounce
  for (uint8_t i = 0; i < SENSOR_COUNT; i++)
  {
    bool rawBit = (raw >> i) & 1;
    bool lastRawBit = (lastRawState >> i) & 1;
    bool stableBit = (sensorState >> i) & 1;

    // Raw changed — reset debounce timer
    if (rawBit != lastRawBit)
      debounceStart[i] = now;

    // Raw differs from accepted and has been stable long enough — accept
    if (rawBit != stableBit && (now - debounceStart[i] >= DEBOUNCE_MS))
    {
      if (rawBit)
        sensorState |= (1 << i);
      else
        sensorState &= ~(1 << i);
    }
  }
  lastRawState = raw;

  // 3. Edge detection
  uint16_t edges = sensorState ^ prevSensorState;

  // 4. MIDI emission
  for (uint8_t i = 0; i < SENSOR_COUNT; i++)
  {
    if (!(edges & (1 << i)))
      continue;
    if (sensorState & (1 << i))
      midiNoteOn(midiChannel, stringNotes[i], midiVelocity);
    else
      midiNoteOff(midiChannel, stringNotes[i]);
  }

  // 5. Commit state
  prevSensorState = sensorState;

  // 6. Throttled UI — check SELECT to exit
  if (now - lastUiUpdate >= UI_UPDATE_MS)
  {
    lastUiUpdate = now;
    uint8_t buttons = lcd.readButtons();
    uint8_t pressed = buttons & ~lastButtons;
    lastButtons = buttons;

    if (pressed & BUTTON_SELECT)
    {
      allNotesOff();
      harpMode = HARP_MODE_SETUP;
      lcdDirty = true;
      lcd.setBacklight(LCD_BACKLIGHT_GREEN);
      Serial.println("Entering setup mode");
    }
  }
}

// -------------------------
// Setup & main loop
// -------------------------

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
  lcd.setBacklight(LCD_BACKLIGHT_GREEN);
  drawMenu();
  lcdDirty = false;
  Serial.println("Harp started");
}

void loop()
{
  if (harpMode == HARP_MODE_SETUP)
    setupModeLoop();
  else
    playModeLoop();
}
