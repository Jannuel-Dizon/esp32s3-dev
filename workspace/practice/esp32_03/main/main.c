#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"

// Kconfig definitions
#define SLPIN_R CONFIG_SLPIN_R
#define SLPIN_Y CONFIG_SLPIN_Y
#define SLPIN_G CONFIG_SLPIN_G
// Time for each color
#define SLTIME_R CONFIG_SLTIME_R
#define SLTIME_Y CONFIG_SLTIME_Y
#define SLTIME_G CONFIG_SLTIME_G
#define SLPIN_RESET CONFIG_SLPIN_RESET

#define LOG_MAX_LEN 64

static const int stoplight_pins[] = {
    SLPIN_R,
    SLPIN_Y,
    SLPIN_G
};

static const char *TAG = "Stoplight";

// RTOS handlers
SemaphoreHandle_t rst_sem;
SemaphoreHandle_t rst_logic;
QueueHandle_t LOG_queue;

// ISR handler
static void IRAM_ATTR gpio_isr_handler(void *arg) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(rst_sem, &xHigherPriorityTaskWoken);
    if(xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}

// FreeRTOS tasks
void LOG_task(void *pvParameter) {
    char temp_message[LOG_MAX_LEN] = "LOG handler task started";
    xQueueSend(LOG_queue, &temp_message, portMAX_DELAY);
    char data_log[LOG_MAX_LEN];
    while(1) {
        if(xQueueReceive(LOG_queue, &data_log, portMAX_DELAY) == pdTRUE) {
            ESP_LOGI("DATA_LOG", "%s", data_log);
        }
    }
}

void button_handler_task(void *pvParameter) {
    char temp_message[LOG_MAX_LEN] = "Reset handler task started";
    xQueueSend(LOG_queue, &temp_message, portMAX_DELAY);
    while(1) {
        if(xSemaphoreTake(rst_sem, portMAX_DELAY) == pdTRUE) {
            // send log reset button pressed
            snprintf(temp_message, LOG_MAX_LEN, "Stoplight has been reset");
            xQueueSend(LOG_queue, &temp_message, portMAX_DELAY);

            xSemaphoreGive(rst_logic);
        }
    }
}

void stoplight_state(void *pvParameter) {
    char temp_message[LOG_MAX_LEN] = "Reset handler task started";
    xQueueSend(LOG_queue, &temp_message, portMAX_DELAY);

    typedef enum {R_STATE, Y_STATE, G_STATE} sl_state_t;
    sl_state_t currState = R_STATE;
    int counter = SLTIME_R;
    while(1) {
        bool rst_detected = false;

        switch(currState) {
            case R_STATE:
                gpio_set_level(stoplight_pins[0], 1);
                gpio_set_level(stoplight_pins[1], 0);
                gpio_set_level(stoplight_pins[2], 0);

                if(xSemaphoreTake(rst_logic, pdMS_TO_TICKS(1000)) == pdTRUE) {
                    rst_detected = true;
                }
                counter-=1000;

                snprintf(temp_message, LOG_MAX_LEN, "Red: %d", counter/1000);
                xQueueSend(LOG_queue, &temp_message, portMAX_DELAY);

                currState = rst_detected || counter > 0 ?  R_STATE :
                                                           G_STATE;
                counter = rst_detected ?         SLTIME_R :
                          currState == G_STATE ? SLTIME_G :
                                                 counter;

                break;
            case Y_STATE:
                gpio_set_level(stoplight_pins[0], 0);
                gpio_set_level(stoplight_pins[1], 1);
                gpio_set_level(stoplight_pins[2], 0);

                if(xSemaphoreTake(rst_logic, pdMS_TO_TICKS(1000)) == pdTRUE) {
                    rst_detected = true;
                }
                counter-=1000;

                snprintf(temp_message, LOG_MAX_LEN, "Yellow: %d", counter/1000);
                xQueueSend(LOG_queue, &temp_message, portMAX_DELAY);

                currState = rst_detected || counter == 0 ?  R_STATE :
                                                            Y_STATE;
                counter = rst_detected || currState == R_STATE ? SLTIME_R :
                                                                 counter;

                break;
            case G_STATE:
                gpio_set_level(stoplight_pins[0], 0);
                gpio_set_level(stoplight_pins[1], 0);
                gpio_set_level(stoplight_pins[2], 1);

                if(xSemaphoreTake(rst_logic, pdMS_TO_TICKS(1000)) == pdTRUE) {
                    rst_detected = true;
                }
                counter-=1000;

                snprintf(temp_message, LOG_MAX_LEN, "Green: %d", counter/1000);
                xQueueSend(LOG_queue, &temp_message, portMAX_DELAY);

                currState = rst_detected ?  R_STATE :
                            counter > 0 ?   G_STATE :
                                            Y_STATE;
                counter = rst_detected ?         SLTIME_R :
                          currState == Y_STATE ? SLTIME_Y :
                                                 counter;

                break;
            default: break;
        }
    }
}


// Setup
void app_main(void) {
    // RTOS handler setup
    rst_sem = xSemaphoreCreateBinary();
    rst_logic = xSemaphoreCreateBinary();
    LOG_queue = xQueueCreate(10, LOG_MAX_LEN);

    // LED pin setup
    for(int i = 0; i < 3; i++) {
        gpio_reset_pin(stoplight_pins[i]);
        if(gpio_set_direction(stoplight_pins[i], GPIO_MODE_OUTPUT) != ESP_OK) {
            ESP_LOGE(TAG, "Couldnt set GPIO direction for: %d", stoplight_pins[i]);
            return;
        }
    }

    // Button setup
    gpio_reset_pin(SLPIN_RESET);
    gpio_set_direction(SLPIN_RESET, GPIO_MODE_INPUT);
    gpio_set_pull_mode(SLPIN_RESET, GPIO_PULLUP_ONLY);
    gpio_set_intr_type(SLPIN_RESET, GPIO_INTR_NEGEDGE);

    // ISR service setup
    gpio_install_isr_service(0);
    gpio_isr_handler_add(SLPIN_RESET, gpio_isr_handler, NULL);

    // FreeRTOS Tasks
    xTaskCreate(button_handler_task,
                "Butthon handler",
                2048,
                NULL,
                12,
                NULL);

    xTaskCreate(stoplight_state,
                "Stoplight state machine",
                2048,
                NULL,
                10,
                NULL);

    xTaskCreate(LOG_task,
                "LOG handler",
                4096,
                NULL,
                5,
                NULL);
}
