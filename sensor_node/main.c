#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "shell.h"

#include "server.h"
#include "sensors.h"

static const shell_command_t shell_commands[] =
{
    {NULL, NULL, NULL}
};

/**
 * @brief the main programm loop
 *
 * @return non zero on error
 */
int main(void) {
    // some initial infos
    puts("[main] Envron.me - Sensor node!");
    puts("[main] ================");
    printf("[main] You are running RIOT on a(n) %s board.\n", RIOT_BOARD);
    printf("[main] This board features a(n) %s MCU.\n", RIOT_MCU);
    puts("[main] ================");

    // Initializse sensors
    if (!init_sensors()) {
	puts("[main] ERROR: Sensor initializition failed");
	return -1;
    }

    get_temperature();
    get_illuminance();

    // start CoAP server
    start_server();

    // start shell
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
