#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "shell.h"

#include "server.h"
#include "sensors.h"
#include "ledcontrol.h"

static int measure_temperature(int argc, char **argv);
static int measure_humidity(int argc, char **argv);
static int measure_illuminance(int argc, char **argv);
static int control_led(int argc, char **argv);

static const shell_command_t shell_commands[] =
{
    {"temperature", "measure the temperature", measure_temperature},
    {"humidity",    "measure the humidity",    measure_humidity},
    {"illuminance", "measure the illuminance", measure_illuminance},
    {"led",         "control the led band",    control_led},
    {NULL, NULL, NULL}
};

/**
 * @brief the main programm loop
 *
 * @return non zero on error
 */
int main(void) {
    // some initial infos
    puts("[main] Environ.me - Sensor node!");
    puts("[main] ================");
    printf("[main] You are running RIOT on a(n) %s board.\n", RIOT_BOARD);
    printf("[main] This board features a(n) %s MCU.\n", RIOT_MCU);
    puts("[main] ================");

    // Initializse sensors
    if (!init_sensors()) {
        puts("[main] ERROR: Sensor initializition failed");
        return -1;
    }

    // start LED control
    start_led_control();

    // start CoAP server
    start_server();

    // start shell
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}

static int measure_temperature(int argc, char **argv) {
    printf("[main] INFO:  Temperature = %d / 100 degree Celsius\n",
           get_temperature());
    return 0;
}

static int measure_humidity(int argc, char **argv) {
    printf("[main] INFO:  Humidity = %d / 100 Percent\n",
           get_humidity());
    return 0;
}

static int measure_illuminance(int argc, char **argv) {
    printf("[main] INFO:  Illuminance = %ld lux\n",
           get_illuminance());
    return 0;
}

static int control_led(int argc, char **argv) {
    if (argc != 2) {
        puts("[main] INFO:  Usage: led <mode>");
        puts("[main] INFO:         mode = 0 (off), 1 (left), 2 (right)");
        return 1;
    }
    switch (argv[1][0]) {
        case '1':
            set_led(LED_MODE_LEFT);
            break;
        case '2':
            set_led(LED_MODE_RIGHT);
            break;
        default:
            set_led(LED_MODE_OFF);
    }
    return 0;
}
