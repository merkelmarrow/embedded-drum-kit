// drum-kit.c

#include "pico/stdlib.h"
#include "hardware/spi.h"

// SPI configuration
#define SPI_PORT    spi0
#define PIN_MISO    16   // Not used here since we're only transmitting
#define PIN_MOSI    19
#define PIN_SCK     18
#define PIN_CS      17   // Chip Select for the MCP4922

int main(void) {
    stdio_init_all();  // Initialize USB stdio (optional)

    // Initialize SPI port at 1 MHz (adjust if needed)
    spi_init(SPI_PORT, 1000000);

    // Set SPI pin functions
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);

    // Manually control the chip select (CS) pin
    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);  // CS inactive high

    uint16_t dac_values[4] = { 1000, 1800, 2500, 3400 };
    uint16_t state = 0;


    while (true) {
        // about 1.65V (4096 / 2)
        uint16_t dac_value = dac_values[state++];  // 12-bit value
        state %= 4;

        // Build the 16-bit command word for MCP4922.
        // Command word format:
        //   Bit 15: Channel Select (0 = Channel A)
        //   Bit 14: Buffer bit (1 = Buffered)
        //   Bit 13: Gain (1 = 1×, i.e., Vout = (DAC/4095)*Vref)
        //   Bit 12: Shutdown (1 = Active mode)
        //   Bits 11-0: 12-bit DAC value.
        uint16_t command = (0 << 15)     // Channel A
            | (1 << 14)     // Buffered output
            | (1 << 13)     // Gain = 1× (Vref)
            | (1 << 12)     // Active mode (not shutdown)
            | (dac_value & 0x0FFF); // 12-bit data

        // Split the 16-bit command into two 8-bit bytes (MSB first)
        uint8_t data[2];
        data[0] = command >> 8;       // Most Significant Byte
        data[1] = command & 0xFF;     // Least Significant Byte

        // Transmit the command to the DAC via SPI
        gpio_put(PIN_CS, 0);          // Assert chip select (active low)
        spi_write_blocking(SPI_PORT, data, 2);
        gpio_put(PIN_CS, 1);          // Deassert chip select


        sleep_ms(1);

    }

    return 0;
}
