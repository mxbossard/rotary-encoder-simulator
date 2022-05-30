#ifndef lib_rotary_encoder_controller_h
#define lib_rotary_encoder_controller_h

#include <Arduino.h>
#include <inttypes.h>

#include <lib_utils.h>
#include <lib_circular_buffer.h>
#include <lib_model.h>

#define SPEEDS_COUNT 10

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

CircularBuffer timings1;
CircularBuffer timings2;

AngleSensor sensor1 = {SENSOR_1_PIN_A, SENSOR_1_PIN_B, SENSOR_1_PIN_INDEX, 4000, 0, 0, "sensor1"};
AngleSensor sensor2 = {SENSOR_2_PIN_A, SENSOR_2_PIN_B, SENSOR_2_PIN_INDEX, 1000, 0, 0, "sensor2"};

void IRAM_ATTR moveSensor1A() {
    bool aLevel = digitalRead(sensor1.pinA);
    bool bLevel = digitalRead(sensor1.pinB);

    portENTER_CRITICAL(&mux);
    if (aLevel == bLevel) {
        // Increment counter
        pushCircularBuffer(&timings1, esp_timer_get_time());
        sensor1.counter ++;
    } else {
        // Decrement counter
        pushCircularBuffer(&timings1, -esp_timer_get_time());
        sensor1.counter --;
    }
    sensor1.position = absMod16(sensor1.counter, sensor1.maxPosition);
    portEXIT_CRITICAL(&mux);
}

void IRAM_ATTR moveSensor1B() {
    bool aLevel = digitalRead(sensor1.pinA);
    bool bLevel = digitalRead(sensor1.pinB);

    portENTER_CRITICAL(&mux);
    if (aLevel != bLevel) {
        // Increment counter
        pushCircularBuffer(&timings1, esp_timer_get_time());
        sensor1.counter ++;
    } else {
        // Decrement counter
        pushCircularBuffer(&timings1, -esp_timer_get_time());
        sensor1.counter --;
    }
    sensor1.position = absMod16(sensor1.counter, sensor1.maxPosition);
    portEXIT_CRITICAL(&mux);
}

void IRAM_ATTR resetSensor1() {
    portENTER_CRITICAL(&mux);
    sensor1.counter = 0;
    sensor1.position = 0;
    portEXIT_CRITICAL(&mux);
}

void IRAM_ATTR moveSensor2A() {
    bool aLevel = digitalRead(sensor2.pinA);
    bool bLevel = digitalRead(sensor2.pinB);

    portENTER_CRITICAL(&mux);
    if (aLevel == bLevel) {
        // Increment position
        pushCircularBuffer(&timings2, esp_timer_get_time());
        sensor2.counter ++;
    } else {
        // Decrement position
        pushCircularBuffer(&timings2, -esp_timer_get_time());
        sensor2.counter --;
    }
    sensor2.position = absMod16(sensor2.counter, sensor2.maxPosition);
    portEXIT_CRITICAL(&mux);
}

void IRAM_ATTR moveSensor2B() {
    bool aLevel = digitalRead(sensor2.pinA);
    bool bLevel = digitalRead(sensor2.pinB);

    portENTER_CRITICAL(&mux);
    if (aLevel != bLevel) {
        // Increment position
        pushCircularBuffer(&timings2, esp_timer_get_time());
        sensor2.counter ++;
    } else {
        // Decrement position
        pushCircularBuffer(&timings2, -esp_timer_get_time());
        sensor2.counter --;
    }
    sensor2.position = absMod16(sensor2.counter, sensor2.maxPosition);
    portEXIT_CRITICAL(&mux);
}

void IRAM_ATTR resetSensor2() {
    portENTER_CRITICAL(&mux);
    sensor2.counter = 0;
    sensor2.position = 0;
    portEXIT_CRITICAL(&mux);
}

int16_t sensorDatagram(AngleSensor sensor, CircularBuffer* cb, uint8_t* buffer, int16_t position) {
    // Sensor Datagram :
    // CRC8 on 1 byte
    // Header on 1 byte (Parity, Speed counts, redondancy, ...)
    // sensor position on 2 bytes
    // Each speed on 2 bytes

    int16_t p = position;

    // Room for CRC8
    p += 1;

    // Header
    buffer[p] = (uint16_t) 0;
    p += 1;
    
    portENTER_CRITICAL(&mux);

    // Position
    buffer[p] = sensor.position >> 8;
    buffer[p+1] = sensor.position;
    p += 2;

    // Speeds
    int32_t* speeds = getDataArrayCircularBuffer(cb);
    for (int16_t k = 0; k < sizeCircularBuffer(*cb); k++) {
        buffer[p] = speeds[k] >> 8;
        buffer[p+1] = speeds[k];
        p += 2;
    }

    portEXIT_CRITICAL(&mux);

    // CRC8 at first position
    buffer[position] = crc8(&buffer[position], position - p);

    // Debug
    Serial.printf("sensor [%s] datagram: ", sensor.name);
    for (int16_t k = position; k < p; k++) {
        Serial.printf("%d ", buffer[k]);
    }
    Serial.println();

    return p - position;
}

void decodeDatagram(uint8_t* buffer, uint16_t wordSize) {
    Serial.printf("Decoded Datagram: ");
    uint8_t crc = buffer[0];
    uint8_t header = buffer[1];
    uint16_t position = (buffer[2] << 8) + buffer[3];
    int16_t speed0 = (buffer[4] << 8) + buffer[5];
    Serial.printf("CRC: %d Header: %d Position: %d Speed0: %d\n", crc, header, position, speed0);
}

void printSensorInputs() {
    Serial.printf("INPUTS  [%s] pinA(%d): %d pinB(%d): %d pinIndex(%d): %d", sensor1.name, sensor1.pinA, digitalRead(sensor1.pinA), sensor1.pinB, digitalRead(sensor1.pinB), sensor1.pinIndex, digitalRead(sensor1.pinIndex));
    Serial.printf(" ; [%s] pinA(%d): %d pinB(%d): %d pinIndex(%d): %d\n", sensor2.name, sensor2.pinA, digitalRead(sensor2.pinA), sensor2.pinB, digitalRead(sensor2.pinB), sensor2.pinIndex, digitalRead(sensor2.pinIndex));
}

int32_t* calculateSpeeds(CircularBuffer* b, int64_t now) {
    int32_t* data = getDataArrayCircularBuffer(b);
    int32_t size = sizeCircularBuffer(*b);

    for (int32_t k = size - 1 ; k > 0 ; k --) {
        // Calculate instantaneous speed between timings
        if (data[k] > 0) {
            data[k] = abs(data[k - 1]) - data[k];
        } else {
            data[k] = - abs(data[k - 1]) - data[k];
        }
    }

    // Last speed is calculated with current time
    if (data[0] > 0) {
        data[0] = now - data[0];
    } else {
        data[0] = - now - data[0];
    }

    return data;
}

const char* positionMessage(AngleSensor sensor) {
    char* buf = (char*) malloc(sizeof(char) * 16);
    sprintf(buf, "%d", sensor.position);
    return buf;
}

int32_t speedMessage(char* buf, CircularBuffer* b, int64_t now) {
    int32_t size = sizeCircularBuffer(*b);
    int32_t* speeds = calculateSpeeds(b, now);
    return printArray(buf, speeds, size);
}

const char* sensorMessage(AngleSensor sensor, CircularBuffer* timings, int64_t now = esp_timer_get_time()) {
    char* buf = (char*) malloc(sizeof(char) * 128);
    int32_t n = sprintf(buf, "[%s] position: %d speeds: ", sensor.name, sensor.position);
    n += speedMessage(buf, timings, now);
    
    return buf;
}

int32_t sensorsMessage(char* buf) {
    // FIXME: probleme sur les overflow des timings.
    int64_t now = esp_timer_get_time();
    int32_t n = sprintf(buf, "[%s] position: %d speeds: ", sensor1.name, sensor1.position);
    n += speedMessage(&buf[n], &timings1, now);
    n += sprintf(&buf[n], " ; [%s] position: %d speeds: ", sensor2.name, sensor2.position);
    n += speedMessage(&buf[n], &timings2, now);
    return n;
}
/*
void printSpeeds(CircularBuffer* b, bool newLine = true) {
    String message = "speeds: %s";
    if (newLine) {
        message.concat("\n");
    }

    int32_t size = sizeCircularBuffer(*b);
    int32_t* speeds = calculateSpeeds(b);

    const char* timings = printArray(speeds, size);
    Serial.printf(message.c_str(), timings);
}
*/

void printSensor(AngleSensor sensor, CircularBuffer* timings, bool newLine = true) {
    const char* msg = sensorMessage(sensor, timings);
    Serial.printf("%s", msg);
    if (newLine) {
        Serial.println();
    }
}

char* sensorsMessageBuffer = (char*) malloc(sizeof(char) * 256);
void printSensors() {
    sensorsMessage(sensorsMessageBuffer);
    Serial.printf("%s\n", sensorsMessageBuffer);
}

void controllerSetup() {
    initCircularBuffer(&timings1, SPEEDS_COUNT);
    initCircularBuffer(&timings2, SPEEDS_COUNT);

    // Sensor 1
    pinMode(sensor1.pinA, INPUT_PULLUP);
    pinMode(sensor1.pinB, INPUT_PULLUP);
    pinMode(sensor1.pinIndex, INPUT_PULLUP);
    attachInterrupt(sensor1.pinA, moveSensor1A, CHANGE);
    attachInterrupt(sensor1.pinB, moveSensor1B, CHANGE);
    attachInterrupt(sensor1.pinIndex, resetSensor1, RISING);

    // Sensor 2
    pinMode(sensor2.pinA, INPUT_PULLUP);
    pinMode(sensor2.pinB, INPUT_PULLUP);
    pinMode(sensor2.pinIndex, INPUT_PULLUP);
    attachInterrupt(sensor2.pinA, moveSensor2A, CHANGE);
    attachInterrupt(sensor2.pinB, moveSensor2B, CHANGE);
    attachInterrupt(sensor2.pinIndex, resetSensor2, RISING);
}

#endif