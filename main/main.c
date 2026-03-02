#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "esp_timer.h"   //Using esp timer for trigger and echo


#define TRIG GPIO_NUM_11
#define ECHO GPIO_NUM_12
#define LOOP_DELAY_MS 1000
#define OUT_OF_RANGE_SHORT 116 // around 2 cm
#define OUT_OF_RANGE_LONG 23200  // around 4 cm


//----- Global variables -----//
esp_timer_handle_t oneshot_timer;   // One-shot timer handle
uint64_t echo_pulse_time = 0;       // Pulse time calculated in echo ISR
uint64_t rising_edge_time = 0; 
uint64_t falling_edge_time = 0; 

// ISR for the trigger pulse
void IRAM_ATTR oneshot_timer_handler(void* arg)
{
    gpio_set_level(TRIG, 0);
}

/*************************/
/* 3. Echo ISR goes here */
/*************************/
void IRAM_ATTR echo_isr_handler(void* arg) {
    if (gpio_get_level(ECHO)) { //rising
        rising_edge_time = esp_timer_get_time();
    }
    else { //falling edge
        falling_edge_time = esp_timer_get_time();
        echo_pulse_time = rising_edge_time-falling_edge_time;
    }
}


// Initialize pins and timer
void hc_sr04_init() {
    //Trigger is an output, initially 0
    gpio_reset_pin(TRIG);
    gpio_set_direction(TRIG, GPIO_MODE_OUTPUT);
    gpio_set_level(TRIG, 0); // Ensure trig is low initially
   
   
    // Configure echo to interrupt on both edges.
    gpio_reset_pin(ECHO);
    gpio_set_direction(ECHO, GPIO_MODE_INPUT);
    gpio_set_intr_type(ECHO, GPIO_INTR_ANYEDGE);
    gpio_intr_enable(ECHO);  // Enable interrupts on ECHO
    gpio_install_isr_service(0);  // Creates global ISR for all GPIO interrupts
   
    //Dispatch pin handler for ECHO
    gpio_isr_handler_add(ECHO, echo_isr_handler, NULL);
   
    // Create one-shot esp timer for trigger
    const esp_timer_create_args_t oneshot_timer_args = {
        .callback = &oneshot_timer_handler,
        .name = "one-shot"
    };
   
    esp_timer_create(&oneshot_timer_args, &oneshot_timer);
   
}


/***************************/
/* 4. app_main() goes here */
/***************************/
void app_main() {
    hc_sr04_init();
}