#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>

Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

// MIDI constants
const int MIDI_CHANNEL = 1;
const int MIDI_VELOCITY = 100;
const int MIDI_NOTES[] = {60, 62, 64, 65, 67, 69, 71, 72}; // C4-C5
const int MIDI_NOTE_COUNT = 8;

// Timing
unsigned long lastMidiTime = 0;
unsigned long lastDisplayTime = 0;
unsigned long lastColorTime = 0;
const unsigned long MIDI_INTERVAL = 200;  // ms per note
const unsigned long DISPLAY_INTERVAL = 500;  // ms
const unsigned long COLOR_INTERVAL = 1000;  // ms

// State
int currentMidiNote = 0;
bool noteOn = false;
int currentChar = 32;  // Start at space
int currentColor = 0;
uint8_t lastButtons = 0;

// Color definitions for RGB backlight
const uint8_t COLORS[] = {
  0x1,  // RED
  0x2,  // GREEN
  0x4,  // BLUE
  0x3,  // YELLOW (RED + GREEN)
  0x6,  // CYAN (GREEN + BLUE)
  0x5,  // MAGENTA (RED + BLUE)
  0x7,  // WHITE (RED + GREEN + BLUE)
  0x0   // OFF
};
const int COLOR_COUNT = 8;

void setup() {
  Serial.begin(9600);
  Serial1.begin(31250);  // MIDI baud rate
  lcd.begin(16, 2);
  lcd.setBacklight(0x0);  // Start with backlight off
  Serial.println("Hardware test started");
}

void loop() {
  unsigned long now = millis();
  
  // Handle MIDI output
  if (now - lastMidiTime >= MIDI_INTERVAL) {
    sendMidiNote();
    lastMidiTime = now;
  }
  
  // Handle display updates
  if (now - lastDisplayTime >= DISPLAY_INTERVAL) {
    updateDisplay();
    lastDisplayTime = now;
  }
  
  // Handle color changes
  if (now - lastColorTime >= COLOR_INTERVAL) {
    changeColor();
    lastColorTime = now;
  }
  
  // Handle button presses
  checkButtons();
}

void sendMidiNote() {
  if (!noteOn) {
    // Send Note On
    int note = MIDI_NOTES[currentMidiNote];
    Serial1.write(0x90 | (MIDI_CHANNEL - 1));  // Note On, channel 1
    Serial1.write(note);
    Serial1.write(MIDI_VELOCITY);
    noteOn = true;
  } else {
    // Send Note Off
    int note = MIDI_NOTES[currentMidiNote];
    Serial1.write(0x80 | (MIDI_CHANNEL - 1));  // Note Off, channel 1
    Serial1.write(note);
    Serial1.write(0);
    noteOn = false;
    // Move to next note
    currentMidiNote = (currentMidiNote + 1) % MIDI_NOTE_COUNT;
  }
}

void updateDisplay() {
  lcd.clear();
  lcd.setCursor(0, 0);
  
  // Display current ASCII character and its code
  char displayChar = (char)currentChar;
  lcd.print("Char: ");
  if (currentChar >= 32 && currentChar <= 126) {
    lcd.print(displayChar);
  } else {
    lcd.print("?");
  }
  
  lcd.setCursor(0, 1);
  lcd.print("Code: ");
  lcd.print(currentChar);
  
  // Move to next character
  currentChar++;
  if (currentChar > 126) {
    currentChar = 32;  // Wrap back to space
  }
}

void changeColor() {
  lcd.setBacklight(COLORS[currentColor]);
  currentColor = (currentColor + 1) % COLOR_COUNT;
}

void checkButtons() {
  uint8_t buttons = lcd.readButtons();
  
  // Detect button press events (edge detection)
  if (buttons != lastButtons) {
    if (buttons & BUTTON_UP) {
      Serial.println("Button UP pressed");
    }
    if (buttons & BUTTON_DOWN) {
      Serial.println("Button DOWN pressed");
    }
    if (buttons & BUTTON_LEFT) {
      Serial.println("Button LEFT pressed");
    }
    if (buttons & BUTTON_RIGHT) {
      Serial.println("Button RIGHT pressed");
    }
    if (buttons & BUTTON_SELECT) {
      Serial.println("Button SELECT pressed");
    }
    lastButtons = buttons;
  }
}
