#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "periph/gpio.h"
#include "thread.h"
#include "msg.h"
#include "xtimer.h"
#include "ledcontrol.h"

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
static uint32_t mode;
static uint32_t led_array[LED_ARRAY_SIZE];

/**
 * @brief start led control thread
 */
void start_led_control(void) {
    led_control_pid = thread_create(thread_stack, sizeof(thread_stack),
				    THREAD_PRIORITY_MAIN, CREATE_STACKTEST,
				    led_control, NULL, "led_control");
}

/**
 * @brief led control thread
 *
 * @param[in] arg   unused
 */
static void *led_control(void *arg) {
    (void) arg;
    //uint32_t mode = 0;
    msg_t msg;
    int index = 0;
    msg_init_queue(msg_queue, MSG_QUEUE_SIZE);
    printf("[led_control] INFO:  Thread started, pid: %" PRIkernel_pid "\n",
	   led_control_pid);
    while(1) {
	reset_array(led_array);
	if (mode == LED_MODE_LEFT) {
	    if (index > 28) {
		index = 0;
	    }
	    write_led(led_array, index,       0, 10, 0);
	    write_led(led_array, (index + 1), 0, 50, 0);
	    write_led(led_array, (index + 2), 0, 10, 0);
	    send_array(led_array);
	    index++;
	    while (msg_try_receive(&msg) > 0) {
		mode = msg.content.value;
		printf("[led_control] INFO:  Mode 1: new mode %u\n",
		       (unsigned int) mode);
	    }
	} else if (mode == LED_MODE_RIGHT) {
	    if (index < 2) {
		index = 30;
	    }
	    write_led(led_array, index,       0, 10, 0);
	    write_led(led_array, (index - 1), 0, 50, 0);
	    write_led(led_array, (index - 2), 0, 10, 0);
	    send_array(led_array);
	    index--;
	    while (msg_try_receive(&msg) > 0) {
		mode = msg.content.value;
		printf("[led_control] INFO:  Mode 2, new mode %u\n",
		       (unsigned int) mode);
	    }
	} else {
	    send_array(led_array);
	    index = 0;
	    msg_receive(&msg);
	    mode = msg.content.value;
	    printf("[led_control] INFO:  Mode 0, new mode %u\n",
		   (unsigned int) mode);
	}
	xtimer_usleep(71428);    // 2 seconds for one cycle
    }
    return NULL;
}

void set_led(uint32_t new_mode) {
    msg_t msg;
    int result;
    msg.content.value = new_mode;
    result =  msg_try_send(&msg, led_control_pid);
    if (result == 0) {
	puts("[led_control] ERROR: Failed to set LED mode, receiver is not waiting or has a full message queue");
    } else if (result == -1) {
	puts("[led_control] ERROR: Failed to set LED mode, invalid PID");
    }
}

static void send_zero(void) {
    gpio_set(1);
    gpio_clear(1);
    gpio_clear(1);
    gpio_clear(1);
}

static void send_one(void) {
    gpio_set(1);
    gpio_set(1);
    gpio_clear(1);
    gpio_clear(1);
}

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

static void write_led(uint32_t array[], int led, uint8_t green, uint8_t red,
		      uint8_t blue) {
    array[led] = ((green << 16) | (red << 8) | blue);
}

static void reset_array(uint32_t array[]) {
    for(int i = 0; i < LED_ARRAY_SIZE; i++) {
	array[i] = 0;
    }
}
