#include "web_server.h"

#if ENABLE_WIFI_SERVER
#include <Arduino.h>
#include <WebServer.h>
#include "esp_camera.h"
#include "img_converters.h"
#include "vision_processing.h"
#include "ml_inference.h"

#define HTTP_TASK_STACK_SIZE 8192
#define ROI_WIDTH 96
#define ROI_HEIGHT 96
#define JPEG_QUALITY 12

WebServer server(80);

void handleCaptureROI() {
    if (!frame_ready) {
        server.send(503, "text/plain", "Frame not ready");
        return;
    }
    
    uint8_t* roi_buf = (uint8_t*)malloc(ROI_WIDTH * ROI_HEIGHT);
    if (!roi_buf) {
        server.send(500, "text/plain", "Out of memory");
        return;
    }
    
    memcpy(roi_buf, ml_processed_roi, ROI_WIDTH * ROI_HEIGHT);

    camera_fb_t fake_fb;
    fake_fb.width = ROI_WIDTH;
    fake_fb.height = ROI_HEIGHT;
    fake_fb.format = PIXFORMAT_GRAYSCALE;
    fake_fb.buf = roi_buf;
    fake_fb.len = ROI_WIDTH * ROI_HEIGHT;
    
    uint8_t *jpg_buf = NULL;
    size_t jpg_len = 0;
    
    if (frame2jpg(&fake_fb, JPEG_QUALITY, &jpg_buf, &jpg_len)) {
        server.send_P(200, "image/jpeg", (const char*)jpg_buf, jpg_len);
        free(jpg_buf);
    } else {
        server.send(500, "text/plain", "JPEG compression failed");
    }
    free(roi_buf);
}

void httpServerTask(void *pvParameters) {
    server.on("/roi", handleCaptureROI);
    server.begin();
    
    while (true) {
        server.handleClient();
        vTaskDelay(pdMS_TO_TICKS(10)); // Yield to watchdog
    }
}

void startWebServer() {
    xTaskCreatePinnedToCore(httpServerTask, "HttpTask", HTTP_TASK_STACK_SIZE, NULL, 1, NULL, 0); 
}
#endif
