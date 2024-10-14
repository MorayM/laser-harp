#include <SPI.h>

const int DAC_CS = 53; // Chip Select pin for MCP4822

int positions[] = {0, 1024, 2048, 3072, 4095};
const int positionCount = 5;

void setup() {
  // Set CS pin as output
  pinMode(DAC_CS, OUTPUT);
  digitalWrite(DAC_CS, HIGH); // Keep CS high to start

  // Initialize SPI
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV2); // Fast communication
}

/**
 * Write a 12-bit value to the MCP4822 DAC
 * @param int channel 0 for Channel A, 1 for Channel B
 * @param value 12-bit value (0-4095) to write
 */
void writeDAC(int channel, int value) {
  // Ensure value is 12-bit
  value = value & 0x0FFF;

  // Build the 16-bit command: Channel and value
  uint16_t command = 0x3000; // Channel A, gain=1, active
  if (channel == 1) {
    command = 0xB000; // Channel B, gain=1, active
  }

  command |= value;

  // Transmit the 16-bit command to DAC
  digitalWrite(DAC_CS, LOW);
  SPI.transfer16(command);
  digitalWrite(DAC_CS, HIGH);
}

void loop() {
  // int positionX = 2048;  // Example position
  // int positionY = 2048;  // Example position
  
  // writeDAC(0, positionX);  // Write to Channel A
  // writeDAC(1, positionY);  // Write to Channel B

  for (int step = 0; step <= positionCount; step++) {
    writeDAC(0, positions[step]);
    writeDAC(1, positions[step]);
    delay(20);
  }
}
