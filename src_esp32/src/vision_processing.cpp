#include "vision_processing.h"
#include "esp_camera.h"
#include "pid_controller.h"

#define VISION_TASK_STACK_SIZE 8192
#define ROI_HEIGHT 40
#define BLACK_THRESHOLD 150
#define INTERSECTION_MIN_PIXELS 1250
#define INTERSECTION_MAX_PIXELS 6000

uint8_t* full_frame_copy = NULL;
bool frame_ready = false;
volatile int debug_centroid_x = -1; 
volatile int debug_intersection_pixels = 0;

void visionProcessingTask(void *pvParameters) {
    while (true) {
        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb) {
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }
        
        memcpy(full_frame_copy, fb->buf, fb->len);
        frame_ready = true;
        
        int roi_w = fb->width;
        int roi_h = ROI_HEIGHT;
        int roi_x = 0;
        int roi_y = fb->height - roi_h;
        
        uint8_t* crop_buf = (uint8_t*)malloc(roi_w * roi_h);
        if (!crop_buf) {
            esp_camera_fb_return(fb);
            continue;
        }
        
        for (int y = 0; y < roi_h; y++) {
            memcpy(&crop_buf[y * roi_w], &fb->buf[(roi_y + y) * fb->width + roi_x], roi_w);
        }
        
        int total_black_pixels = 0;
        int tripwire_start_y = fb->height - ROI_HEIGHT;
        for (int y = tripwire_start_y; y < fb->height; y++) {
            for (int x = 0; x < fb->width / 2; x++) {
                if (fb->buf[y * fb->width + x] < BLACK_THRESHOLD) {
                    total_black_pixels++;
                }
            }
        }
        
        bool intersection_stop = (total_black_pixels > INTERSECTION_MIN_PIXELS && total_black_pixels < INTERSECTION_MAX_PIXELS);
        debug_intersection_pixels = total_black_pixels;

        int total_x = 0;
        int valid_rows = 0;
        
        for (int y = 0; y < roi_h; y++) {
            for (int x = roi_w - 1; x >= 2; x--) {
                if (crop_buf[y * roi_w + x] < BLACK_THRESHOLD &&
                    crop_buf[y * roi_w + x - 1] < BLACK_THRESHOLD &&
                    crop_buf[y * roi_w + x - 2] < BLACK_THRESHOLD) {
                    
                    int start_x = x;
                    int end_x = x - 2;
                    while (end_x >= 0 && crop_buf[y * roi_w + end_x] < BLACK_THRESHOLD) {
                        end_x--;
                    }
                    
                    int center_x = (start_x + end_x) / 2;
                    total_x += center_x;
                    valid_rows++;
                    
                    for (int dx = start_x; dx > end_x; dx--) {
                        crop_buf[y * roi_w + dx] = 0;
                    }
                    crop_buf[y * roi_w + center_x] = 255;
                    break; 
                }
            }
        }
        
        int rightmost_centroid_x = -1;
        if (valid_rows > 5) { 
            rightmost_centroid_x = total_x / valid_rows;
            
            for (int y = 0; y < roi_h; y++) {
                if (rightmost_centroid_x >= 0 && rightmost_centroid_x < roi_w) {
                    crop_buf[y * roi_w + rightmost_centroid_x] = 255;
                    if (rightmost_centroid_x > 0) crop_buf[y * roi_w + rightmost_centroid_x - 1] = 255;
                    if (rightmost_centroid_x < roi_w - 1) crop_buf[y * roi_w + rightmost_centroid_x + 1] = 255;
                }
            }
        }

        bool line_lost = (valid_rows <= 5);
        debug_centroid_x = rightmost_centroid_x;
        updatePID(rightmost_centroid_x, line_lost, intersection_stop);
        
        free(crop_buf);
        esp_camera_fb_return(fb);
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void startVisionTask() {
    xTaskCreatePinnedToCore(
        visionProcessingTask,
        "VisionTask",
        VISION_TASK_STACK_SIZE, 
        NULL,
        1,
        NULL,
        1
    );
}
