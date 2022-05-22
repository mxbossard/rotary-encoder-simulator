#include <Arduino.h>

#include <lib_rotary_encoder_controller.h>
#include <lib_rotary_encoder_simulator.h>
#include <lib_controller_test.h>

#define LED_PIN 2

void setup() {
    Serial.begin(115200);

    controllerSetup();
    simulatorSetup();

    pinMode(LED_PIN, OUTPUT);
}

void loop() {
    //simul1.enabled = false;
    //simul2.enabled = false;

    delay(100);

    Serial.println("Running simulation ...");
    // testModulo();

    // printSimulators();
    // printSensors();

    uint32_t periodInUs = 1;

    indexSimul(simul1, periodInUs);
    assertCount("Reseting index", sensor1, simul1);
    indexSimul(simul2, periodInUs);
    assertCount("Reseting index", sensor2, simul2);
    // printSimulators();
    // printSensors();

    moveBothSimulators(true, 0, false, 1, periodInUs);
    assertCount("Turning 0 steps", sensor1, simul1);
    assertCount("Turning 1 step left", sensor2, simul2);

    moveBothSimulators(true, 1, false, 0, periodInUs);
    assertCount("Turning 1 steps right", sensor1, simul1);
    assertCount("Turning 0 step", sensor2, simul2);

    moveBothSimulators(true, 1, false, 1, periodInUs);
    assertCount("Turning 1 steps right", sensor1, simul1);
    assertCount("Turning 1 step left", sensor2, simul2);

    //printSensors();

    moveBothSimulators(false, 2, true, 3, periodInUs);
    assertCount("Turning 2 steps left", sensor1, simul1);
    assertCount("Turning 3 step right", sensor2, simul2);

    //printSensors();

    moveBothSimulators(true, 3, false, 2, periodInUs);
    printSensors();
    assertCount("Turning 3 steps right", sensor1, simul1);
    assertCount("Turning 2 step left", sensor2, simul2);

    moveBothSimulators(true, 11, false, 7, periodInUs);
    assertCount("Turning 11 steps right", sensor1, simul1);
    assertCount("Turning 7 steps left", sensor2, simul2);
    
    moveBothSimulators(true, 101, true, 101, periodInUs);
    assertCount("Turning 101 steps right", sensor1, simul1);
    assertCount("Turning 101 steps right", sensor2, simul2);

    moveBothSimulators(true, 3997, false, 1001, periodInUs);
    assertCount("Turning 3997 steps right", sensor1, simul1);
    assertCount("Turning 1001 steps left", sensor2, simul2);
   
    // Turn 1 round one side
    moveBothSimulators(true, sensor1.maxPosition + 3, false, sensor2.maxPosition - 5, periodInUs);
    assertCount("Turning 1 round + 3 steps right", sensor1, simul1);
    assertCount("Turning 1 round - 5 steps right", sensor2, simul2);
    // printSimulators();
    // printSensors();

    testPendulum(sensor1.maxPosition/2, sensor2.maxPosition/2, 12, periodInUs);

    // Turn 50 round one side
    //moveBothSimulators(true, sensor1.maxPosition * 50, false, sensor2.maxPosition * 50, periodInUs);
    //assertPosition("Turning 50 round right", sensor1, simul1);
    //assertPosition("Turning 50 round left", sensor2, simul2);

    // Reset index
    indexSimul(simul1, periodInUs);
    assertCount("Reseting index", sensor1, simul1);
    indexSimul(simul2, periodInUs);
    assertCount("Reseting index", sensor2, simul2);

    // Turn 20 round the other side
    moveBothSimulators(false, sensor1.maxPosition * 10 + 42, true, sensor2.maxPosition * 10 + 21, periodInUs);
    assertCount("Turning 10 round + 42 steps left", sensor1, simul1);
    assertCount("Turning 10 round + 21 steps right", sensor2, simul2);

    Serial.println("Simulation finished.");
}