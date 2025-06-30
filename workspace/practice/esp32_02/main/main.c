#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

#define BLINKY_PIN CONFIG_BLINKY_PIN
#define BLINKY_PERIOD CONFIG_BLINKY_PERIOD

static uint8_t led_state = 0;

void app_main(void) {

    // Setup
    gpio_reset_pin(BLINKY_PIN);

    if(gpio_set_direction(BLINKY_PIN, GPIO_MODE_OUTPUT) != ESP_OK) {
        printf("Error setting GPIO %d", BLINKY_PIN);
        return;
    }

    while(1) {
        gpio_set_level(BLINKY_PIN, led_state);

        led_state = !led_state;
        vTaskDelay(BLINKY_PERIOD/portTICK_PERIOD_MS);
    }
}
