/**
 * @brief  Sensor node module that controlles the LED band.
 * 
 * @author Ruediger Bartz
 * @author Timo Gerken
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "periph/gpio.h"
#include "thread.h"
#include "msg.h"
#include "xtimer.h"
#include "ledcontrol.h"
#include "board.h"

// RIOT changed the constants name during development
#ifndef THREAD_CREATE_STACKTEST 
#define THREAD_CREATE_STACKTEST CREATE_STACKTEST
#endif

#define MSG_QUEUE_SIZE (64)
#define LED_ARRAY_SIZE (30)

static void *led_control(void *arg);
static void send_zero(void);
static void send_one(void);
static void send_array(uint32_t array[]);
static void write_led(uint32_t array[], int led, uint8_t green, uint8_t red,
                      uint8_t blue);
static void reset_array(uint32_t array[]);

static kernel_pid_t led_control_pid;
static char thread_stack[THREAD_STACKSIZE_DEFAULT];
static msg_t msg_queue[MSG_QUEUE_SIZE];
// array for the RGB values of the LEDs
static uint32_t led_array[LED_ARRAY_SIZE];
// global mode variable (local mode will cause RIOT kernel panic)
static uint32_t mode = LED_MODE_OFF;
/**
 * @brief Start led control thread
 */
void start_led_control(void) {
    led_control_pid = thread_create(thread_stack, sizeof(thread_stack),
                                    THREAD_PRIORITY_MAIN, THREAD_CREATE_STACKTEST,
                                    led_control, NULL, "led_control");
}

/**
 * @brief LED control thread. The mode is changed via messages.
 *
 * @param[in] arg   unused
 */
static void *led_control(void *arg) {
    (void) arg;
    msg_t msg;
    int index = 0;    // position of the active LED
    msg_init_queue(msg_queue, MSG_QUEUE_SIZE);
    printf("[led_control] INFO:  Thread started, pid: %" PRIkernel_pid "\n",
           led_control_pid);
    // infinit loop that updates the LED band to create the desired pattern
    while(1) {
        reset_array(led_array);
        // check if a message is availible that changes the mode
        // (non blocking so the LED band will be updated regular)
        if (msg_try_receive(&msg) == 1) {
            mode = msg.content.value;
            printf("[led_control] INFO:  New mode %u\n",
                   (unsigned int) mode);
        }
        if (mode == LED_MODE_LEFT) {
            // moving red light from left to right
            if (index > (LED_ARRAY_SIZE - 3)) {
                index = 0;
            }
            // set RGB values in the LED array
            write_led(led_array, index,       0, 10, 0);
            write_led(led_array, (index + 1), 0, 50, 0);
            write_led(led_array, (index + 2), 0, 10, 0);
            index++;
        } else if (mode == LED_MODE_RIGHT) {
            // moving red light from right to left
            if (index < 2) {
                index = LED_ARRAY_SIZE - 1;
            }
            // set RGB values in the LED array
            write_led(led_array, index,       0, 10, 0);
            write_led(led_array, (index - 1), 0, 50, 0);
            write_led(led_array, (index - 2), 0, 10, 0);
            index--;
        } else {
            // disable LED band
            send_array(led_array);
            index = 0;
        }
        // tansmitt the calculated RGB values
        send_array(led_array);
        xtimer_usleep(71428);    // 2 seconds to run through all 30 LEDs
    }
    return NULL;    // should never be reached
}

/**
 * @brief Funktion to send a message to the LED control thread to change the mode.
 * 
 * @param[in] new_mode the new mode for the LED band
 */
void set_led(uint32_t new_mode) {
    msg_t msg;
    int result;
    msg.content.value = new_mode;
    // send message non blocking so that the calling thread will not be blocked in case of an error
    result =  msg_try_send(&msg, led_control_pid);
    if (result == 0) {
        puts("[led_control] ERROR: Failed to set LED mode, receiver is not waiting or has a full message queue");
    } else if (result == -1) {
        puts("[led_control] ERROR: Failed to set LED mode, invalid PID");
    }
}

/**
 * @brief Sends a 0 bit to the LED band via the GPIO of the red board LED.
 */
static void send_zero(void) {
    LED_R_OFF;
    LED_R_OFF;
    LED_R_OFF;
    LED_R_OFF;
    LED_R_OFF;
    LED_R_OFF;
    LED_R_OFF;
    LED_R_OFF;
    LED_R_OFF;
    
    
    for(int i = 0; i < 16; i++) {
        LED_R_ON;
    }
}

/**
 * @brief Sends a 1 bit to the LED band via the GPIO of the red board LED
 */
static void send_one(void) {
    LED_R_OFF;
    LED_R_OFF;
    LED_R_OFF;
    LED_R_OFF;
    LED_R_OFF;
    LED_R_OFF;
    LED_R_OFF;
    LED_R_OFF;
    LED_R_OFF;
    LED_R_OFF;
    LED_R_OFF;
    LED_R_OFF;
    LED_R_OFF;
    LED_R_OFF;
    LED_R_OFF;
    LED_R_OFF;
    
    LED_R_OFF;
    LED_R_OFF;
    LED_R_OFF;
    LED_R_OFF;
    LED_R_OFF;
    LED_R_OFF;
    LED_R_OFF;
    LED_R_OFF;
    LED_R_OFF;
    LED_R_OFF;
    LED_R_OFF;
    LED_R_OFF;
    LED_R_OFF;
    LED_R_OFF;
    LED_R_OFF;
    LED_R_OFF;
    
    LED_R_OFF;
    LED_R_OFF;
    
    for(int i = 0; i < 16; i++) {
        LED_R_ON;
    }
}

/**
 * @brief Send all the RGB bits in the LED array to the LED band.
 * 
 * @param[in] array the LED array that will be send
 */
static void send_array(uint32_t array[]) {
    for(int led_position = 0; led_position < LED_ARRAY_SIZE; led_position++) {
        for(int bit_position = 0; bit_position < 24; bit_position++) {
            if(((array[led_position] >> (23 - bit_position)) & 1) == 1) {
                send_one();
            } else {
                send_zero();
            }
        }
    }
}

/**
 * @brief Sets the RGB bits in the LED array for a LED.
 * 
 * @param[in] array the LED array to write the RGB values to
 * @param[in] led the number of the LED to set
 * @param[in] green the green RGB value that will be set
 * @param[in] red the red RGB value that will be set
 * @param[in] blue the blue RGB value that will be set
 */
static void write_led(uint32_t array[], int led, uint8_t green, uint8_t red,
                      uint8_t blue) {
    array[led] = ((green << 16) | (red << 8) | blue);
}

/**
 * @brief reset the RGB values of all LEDs to R=0 G=0 B=0.
 */
static void reset_array(uint32_t array[]) {
    for(int i = 0; i < LED_ARRAY_SIZE; i++) {
        array[i] = 0;
    }
}
