#include "ml_inference.h"
#include "vision_processing.h"
#include "pid_controller.h"
#include "autonomous_ml.h"

#define ML_TASK_STACK_SIZE 16384
#define ROI_X_START 205
#define ROI_Y_START 60
#define ROI_SIZE 96
#define TURN_THRESHOLD 0.85

uint8_t* ml_processed_roi = NULL;
char debug_ml_prediction[128] = "none";

extern bool is_turning;
extern bool intersection_parked;

// Edge Impulse Callback: Feeds the 96x96 ROI to the model
int raw_feature_get_data(size_t offset, size_t length, float *out_ptr) {
    for (size_t i = 0; i < length; i++) {
        size_t pixel_index = offset + i;
        int y = pixel_index / ROI_SIZE;
        int x = pixel_index % ROI_SIZE;
        uint8_t pixel = full_frame_copy[(ROI_Y_START + y) * 320 + (ROI_X_START + x)];
        
        ml_processed_roi[pixel_index] = pixel; 
        
        out_ptr[i] = (pixel << 16) | (pixel << 8) | pixel;
    }
    return 0;
}

void mlInferenceTask(void *pvParameters) {
    static float highest_turn_confidence = 0.0;
    static TurnDirection last_known_direction = TURN_NONE;

    while (true) {
        if (!frame_ready || is_turning || intersection_parked) {
            vTaskDelay(pdMS_TO_TICKS(1));
            continue;
        }

        if (next_turn_direction == TURN_NONE && last_known_direction != TURN_NONE) {
            highest_turn_confidence = 0.0;
        }

        signal_t features_signal;
        features_signal.total_length = EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;
        features_signal.get_data = &raw_feature_get_data;

        ei_impulse_result_t result = { 0 };
        EI_IMPULSE_ERROR res = run_classifier(&features_signal, &result, false);

        if (res == EI_IMPULSE_OK) {
            int offset = 0;
            float current_best_turn_score = 0.0;
            TurnDirection current_best_turn = TURN_NONE;

            for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
                offset += snprintf(debug_ml_prediction + offset, sizeof(debug_ml_prediction) - offset, 
                                   "%s: %.2f\n", result.classification[ix].label, result.classification[ix].value);
                                   
                if (strcmp(result.classification[ix].label, "none") != 0) {
                    if (result.classification[ix].value > current_best_turn_score) {
                        current_best_turn_score = result.classification[ix].value;
                        if (strcmp(result.classification[ix].label, "left") == 0) current_best_turn = TURN_LEFT;
                        else if (strcmp(result.classification[ix].label, "right") == 0) current_best_turn = TURN_RIGHT;
                        else if (strcmp(result.classification[ix].label, "forward") == 0) current_best_turn = TURN_FORWARD;
                    }
                }
            }
            
            if (next_turn_direction == TURN_NONE) {
                if (current_best_turn_score >= TURN_THRESHOLD && current_best_turn != TURN_NONE) {
                    next_turn_direction = current_best_turn;
                }
            }
            
            last_known_direction = next_turn_direction;
        }
        
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void startMLTask() {
    xTaskCreatePinnedToCore(mlInferenceTask, "MLTask", ML_TASK_STACK_SIZE, NULL, 1, NULL, 0); 
}
