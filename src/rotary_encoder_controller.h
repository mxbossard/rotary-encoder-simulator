//#include <Arduino.h>

#define SPI_MISO 19
#define SPI_MOSI 23
#define SPI_SCLK 18
#define SPI_CS   5
#define SPI_FREQUENCY 100000
#define SPI_WORD_SIZE 128

#define SENSOR_1_PIN_A 36
#define SENSOR_1_PIN_B 39
#define SENSOR_1_PIN_INDEX 34

#define SENSOR_2_PIN_A 25
#define SENSOR_2_PIN_B 26
#define SENSOR_2_PIN_INDEX 27

#define LED_PIN 21

#include <lib_rotary_encoder_controller.h>
#include <lib_rotary_encoder_controller_spi_slave_2.h>

void setup() {
    //Serial.begin(115200);

    controllerSetup();
    spiSlaveSetup();

    pinMode(LED_PIN, OUTPUT);
}

int counter = 0;
void loop() {
    spiSlaveProcess();

    // Blink led
    counter ++;
    digitalWrite(LED_PIN, counter % 2);
}