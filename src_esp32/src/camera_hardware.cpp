#include "camera_hardware.h"
#include "esp_camera.h"
#include <Arduino.h>

// Standard ESP32-S3 WROOM CAM Pins
#define PWDN_GPIO_NUM  -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM  15
#define SIOD_GPIO_NUM  4
#define SIOC_GPIO_NUM  5
#define Y9_GPIO_NUM    16
#define Y8_GPIO_NUM    17
#define Y7_GPIO_NUM    18
#define Y6_GPIO_NUM    12
#define Y5_GPIO_NUM    10
#define Y4_GPIO_NUM    8
#define Y3_GPIO_NUM    9
#define Y2_GPIO_NUM    11
#define VSYNC_GPIO_NUM 6
#define HREF_GPIO_NUM  7
#define PCLK_GPIO_NUM  13

#define JPEG_QUALITY 12
#define CAMERA_FRAME_SIZE FRAMESIZE_QVGA
#define CAMERA_PIXEL_FORMAT PIXFORMAT_GRAYSCALE

extern uint8_t* full_frame_copy;
extern uint8_t* ml_processed_roi;

void initCamera() {
    full_frame_copy = (uint8_t*)ps_malloc(320 * 240);
    ml_processed_roi = (uint8_t*)ps_malloc(96 * 96);
    
    if (!full_frame_copy) full_frame_copy = (uint8_t*)malloc(320 * 240);
    if (!ml_processed_roi) ml_processed_roi = (uint8_t*)malloc(96 * 96);
    
    if (ml_processed_roi) memset(ml_processed_roi, 0, 96 * 96);

    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    
    config.frame_size = CAMERA_FRAME_SIZE; 
    config.pixel_format = CAMERA_PIXEL_FORMAT; 
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.jpeg_quality = JPEG_QUALITY;
    config.fb_count = 2;
    
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        return;
    }
    
    sensor_t * s = esp_camera_sensor_get();
    s->set_vflip(s, 1);
    s->set_hmirror(s, 0);
}
