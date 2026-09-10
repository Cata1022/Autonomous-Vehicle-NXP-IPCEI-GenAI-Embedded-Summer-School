#include "ble_interface.h"
#include "vehicle_control.h"
#include <NimBLEDevice.h>
#include "pid_controller.h"

// Custom UUIDs for the BLE Service and Characteristic
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

class MyCharacteristicCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
        String command = String(pCharacteristic->getValue().c_str());
        
        // Remove whitespace, carriage returns, and newlines
        command.trim();

        if (command == "l") {
            next_turn_direction = TURN_LEFT;
        } 
        else if (command == "r") {
            next_turn_direction = TURN_RIGHT;
        } 
        else if (command == "go") {
            emergency_stop = false;
        } 
        else if (command == "stop") {
            emergency_stop = true;
        } 
        else if (command == "f") {
            next_turn_direction = TURN_FORWARD;
        }
    }
};

void initBLE() {
    NimBLEDevice::init("ESP32_AutoCam");
    
    // Create the BLE Server
    NimBLEServer *pServer = NimBLEDevice::createServer();
    
    // Create the BLE Service
    NimBLEService *pService = pServer->createService(SERVICE_UUID);
    
    // Create a BLE Characteristic
    NimBLECharacteristic *pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR
    );
    
    // Set callbacks for the characteristic
    pCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
    
    // Start the server (this automatically starts all registered services)
    pServer->start();
    
    // Start advertising
    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->start();
}
