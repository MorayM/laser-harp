#include <MCP48xx.h>

const int DAC_CS = 53; // Chip Select pin for MCP4822

MCP4822 dac(DAC_CS);

int positions[] = {0, 455, 910, 1365, 1820, 2275, 2730, 3185, 3640, 4095};
const int positionCount = 10;

void setup() {
  // We call the init() method to initialize the instance
    dac.init();

    // The channels are turned off at startup so we need to turn the channel we need on
    dac.turnOnChannelA();
    dac.turnOnChannelB();

    // We configure the channels in High gain
    // It is also the default value so it is not really needed
    dac.setGainA(MCP4822::High);
    dac.setGainB(MCP4822::High);
}

/**
 * Write a 12-bit value to the MCP4822 DAC
 * @param int channel 0 for Channel A, 1 for Channel B
 * @param value 12-bit value (0-4095) to write
 */
// void writeDAC(int channel, int value) {
//   // Ensure value is 12-bit
//   value = value & 0x0FFF;

//   // Build the 16-bit command: Channel and value
//   uint16_t command = 0x3000; // Channel A, gain=1, active
//   if (channel == 1) {
//     command = 0xB000; // Channel B, gain=1, active
//   }

//   command |= value;

//   // Transmit the 16-bit command to DAC
//   digitalWrite(DAC_CS, LOW);
//   SPI.transfer16(command);
//   digitalWrite(DAC_CS, HIGH);
// }

void loop() {
  // int positionX = 2048;  // Example position
  // int positionY = 2048;  // Example position
  
  // writeDAC(0, positionX);  // Write to Channel A
  // writeDAC(1, positionY);  // Write to Channel B
  dac.setVoltageA(2048);
  dac.updateDAC();

  for (int step = 0; step <= positionCount; step++) {
    dac.setVoltageB(positions[step]);
    dac.updateDAC();
    delay(10);
  }
}
