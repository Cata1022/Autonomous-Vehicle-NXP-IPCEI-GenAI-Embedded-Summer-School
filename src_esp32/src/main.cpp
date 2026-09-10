#include <Arduino.h>
#include "ble_interface.h"
#include "vehicle_control.h"
#include "pid_controller.h"
#include "camera_hardware.h"
#include "vision_processing.h"
#include "ml_inference.h"
#include "web_server.h"

#if ENABLE_WIFI_SERVER
#include <WiFi.h>
#endif

void setup() {
  Serial.begin(115200);

#if ENABLE_WIFI_SERVER
  WiFi.setTxPower(WIFI_POWER_8_5dBm);
  WiFi.begin("ssid", "password"); 
#endif

  initBLE();
  initCamera();
  initPID();
  startVisionTask();
  startMLTask();
  
#if ENABLE_WIFI_SERVER
  startWebServer();
#endif
}

void loop() {}
