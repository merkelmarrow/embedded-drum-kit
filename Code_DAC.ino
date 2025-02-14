#include <SPI.h>

#define CS 10  // Chip Select for MCP4922
#define LDAC 9
 // Default state HIGH

void setup() {
    pinMode(CS, OUTPUT);
    pinMode(LDAC, OUTPUT);
    SPI.begin();
    digitalWrite(CS, HIGH);
    digitalWrite(LDAC, HIGH); 
    
}

void loop() {
    for (int i = 0; i < 4096; i += 50) {  // Step in increments of 50
      sendToDAC(i, 0);
      delayMicroseconds(1000);
    }
}

// Function to send data to MCP4922
void sendToDAC(int value, int channel) {
  digitalWrite(CS, LOW);
    byte highByte = (channel << 7) | (0b0011 << 4) | ((value >> 8) & 0x0F);
    byte lowByte = value & 0xFF;
    SPI.transfer(highByte);
    SPI.transfer(lowByte);
    digitalWrite(CS, HIGH);

    // Manually toggle LDAC to update the output
    digitalWrite(LDAC, LOW);
    delayMicroseconds(1);
    digitalWrite(LDAC, HIGH);
}
