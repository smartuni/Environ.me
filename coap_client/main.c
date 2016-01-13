/**
 * @brief  Small test client for the CoAP server in the sensor node.
 * 
 * @author Ruediger Bartz
 * @author Timo Gerken
 */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include "thread.h"
#include "shell.h"
#include "coap.h"

#define BUFFER_SIZE       (256)
#define MSG_QUEUE_SIZE    (64)
#define PORT              (5683)    // UDP port for the clients receiver
#define MAX_RESPONSE_SIZE (64)

// RIOT changed the constants name during development
#ifndef THREAD_CREATE_STACKTEST 
#define THREAD_CREATE_STACKTEST CREATE_STACKTEST
#endif

static void *client_recv(void *arg);
static int request_temperature(int argc, char **argv);
static void dumpHeader(coap_header_t *hdr);
static void dump(const uint8_t *buf, size_t buflen, bool bare);
static void dumpOptions(coap_option_t *opts, size_t numopt);
static void dumpPacket(coap_packet_t *pkt);

// definition of shell commands
static const shell_command_t shell_commands[] =
{
    {"temperature", "request temperature from server", request_temperature},
    {NULL, NULL, NULL}
};
// definition of CoAP paths
static const coap_endpoint_path_t temperature_path = {1, {"temperature"}};

static char thread_stack[THREAD_STACKSIZE_DEFAULT];
static msg_t msg_queue[MSG_QUEUE_SIZE];
static uint8_t client_buffer[BUFFER_SIZE];
static uint16_t message_id = 1;

/**
 * @brief the main programm loop
 *
 * @return non zero on error
 */
int main(void) {
    // some initial infos
    puts("[main] Envron.me - CoAP client test!");
    puts("[main] ================");
    printf("[main] You are running RIOT on a(n) %s board.\n", RIOT_BOARD);
    printf("[main] This board features a(n) %s MCU.\n", RIOT_MCU);
    puts("[main] ================");

    // start clients receiver
    thread_create(thread_stack, sizeof(thread_stack), THREAD_PRIORITY_MAIN,
          THREAD_CREATE_STACKTEST, client_recv, NULL, "client_receiver");
    

    // start shell
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}

/**
 * @brief This thread listens on the UDP port 5683 for any CoAP packages
 *        and prints any received CoAP package.
 *
 * @param[in] arg unused
 * @return NULL on error
 */
static void *client_recv(void *arg) {
    int client_socket = -1;
    struct sockaddr_in6 client_addr;
    size_t client_addr_len = sizeof(client_addr);
    uint16_t port;
    (void) arg;
    msg_init_queue(msg_queue, MSG_QUEUE_SIZE);
    port = (uint16_t)PORT;
    if (port == 0) {
        puts("[client_recv] ERROR: invalid port specified");
        return NULL;
    }
    // create posix IPv6 UDP socket to receive data
    client_socket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    if (client_socket < 0) {
        puts("[client_recv] ERROR: initializing socket");
        client_socket = 0;
        return NULL;
    }
    // configure the clients IPv6 address stucture
    client_addr.sin6_family = AF_INET6;
    memset(&client_addr.sin6_addr, 0, sizeof(client_addr.sin6_addr));
    client_addr.sin6_port = htons(port);
    // bind the socket to the UDP port
    if (bind(client_socket, (struct sockaddr *)&client_addr,
             client_addr_len) < 0) {
        puts("[client_recv] ERROR: binding socket");
        return NULL;
    }
    printf("[client_recv] INFO: started client receiver on port %" PRIu16 "\n",
           port);
    // infinit loop to receive CoAP packages
    while (1) {
        struct sockaddr_in6 server_addr;
        size_t server_addr_len = sizeof(server_addr);
        size_t buffer_size = sizeof(client_buffer);
        int rc;
        int recv_len;
        // blocking receive, waiting for data
        if ((recv_len = recvfrom(client_socket, client_buffer, buffer_size, 0,
                                 (struct sockaddr *)&server_addr,
                                 &server_addr_len)) < 0) {
            puts("[client_recv] ERROR: on receive");
        } else if (recv_len == 0) {
            puts("[client_recv] INFO:  peer did shut down");
        } else {
            // CoAP part
            coap_packet_t inpkt;
            // check if the received data is a CoAP package
            if (0 != (rc = coap_parse(&inpkt, client_buffer, recv_len))) {
                printf("[client_recv] INFO:  bad packet rc=%d\n", rc);
            } else {
                // print the received CoAP package
                puts("[client_recv] INFO:  redeived packet");
                dump(client_buffer, recv_len, true);
                puts("\ncontent:");
                dumpPacket(&inpkt);
            }
        }
    }
    return NULL;    // should never be reached
}

/**
 * @brief Handler for the shell command to send a CoAP GET /temperature request.
 * 
 * @param[in] argc number of argument strings
 * @param[in] argv array of argument strings
 * @return non 0 on error
 */
static int request_temperature(int argc, char **argv) {
    int client_socket = -1;
    struct sockaddr_in6 client_addr;
    struct sockaddr_in6 server_addr;
    size_t server_addr_len = sizeof(server_addr);
    size_t buffer_size = sizeof(client_buffer);
    uint16_t port;
    int rc;
    // check if the number of arguments is correct
    if (argc != 2) {
        puts("[main] INFO:  usage: temperature <addr>");
        return 1;
    }
    port = (uint16_t)PORT;
    if (port == 0) {
        puts("[main] ERROR: invalid port specified");
        return 1;
    }
    // create posix IPv6 UDP socket to send CoAP requests
    client_socket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    if (client_socket < 0) {
        puts("[main] ERROR: initializing socket");
        client_socket = 0;
        return 1;
    }
    // configure the clients and serves IPv6 address structure
    client_addr.sin6_family = AF_INET6;
    server_addr.sin6_family = AF_INET6;
    memset(&client_addr.sin6_addr, 0, sizeof(client_addr.sin6_addr));
    // parse the servers IPv6 address from the given argument
    if (inet_pton(AF_INET6, argv[1], &server_addr.sin6_addr) != 1) {
        puts("[main] ERROR: unable to parse destination address");
        return 1;
    }
    client_addr.sin6_port = htons(port);
    server_addr.sin6_port = htons(port);
    puts("[main] INFO:  creating CoAP GET request");
    // create CoAP GET /temperature request package
    coap_header_t request_hdr = {
        .ver = 1,
        .t = COAP_TYPE_CON,
        .tkl = 0,
        .code = MAKE_RSPCODE(0, COAP_METHOD_GET),
        .id = {(message_id & 0xFF), ((message_id >> 8) & 0xFF)}
    };
    coap_buffer_t payload_buf = {
        .p = (const uint8_t *)NULL,
        .len = 0
    };
    coap_buffer_t token_buf = {
        .p = (const uint8_t *)NULL,
        .len = 0
    };
    coap_packet_t request_pkt = {
        .hdr = request_hdr,
        .tok = token_buf,
        .numopts = temperature_path.count,
        .payload = payload_buf
    };
    for (int i = 0; i < temperature_path.count; i++) {
        puts("[main] INFO:  creating path option");
        coap_option_t path_option = {
            .num = COAP_OPTION_URI_PATH,
            .buf = {.p = (const uint8_t *)temperature_path.elems[i],
                    .len = strlen(temperature_path.elems[i])}
    };
    request_pkt.opts[i] = path_option;
    }
    puts("[main] INFO:  building coap package");
    if (0 != (rc = coap_build(client_buffer, &buffer_size, &request_pkt))) {
        printf("[main] ERROR: coap_build failed rc=%d\n", rc);
    return 1;
    }
    puts("[main] INFO:  sending packet");
    dump(client_buffer, buffer_size, true);
    puts("\ncontent:");
    dumpPacket(&request_pkt);
    // send CoAP request package to the server
    if (sendto(client_socket, client_buffer, buffer_size, 0,
               (struct sockaddr *)&server_addr, server_addr_len) < 0) {
        puts("[main] ERROR: sending data");
    }
    // cleanup
    close(client_socket);
    return 0;
}

/**
 * @brief Print the content of a CoAP header.
 *
 * @param[in] hdr CoAP header to print
 */
void dumpHeader(coap_header_t *hdr) {
    printf("Header:\n");
    printf("  ver  0x%02X\n", hdr->ver);
    printf("  t    0x%02X\n", hdr->t);
    printf("  tkl  0x%02X\n", hdr->tkl);
    printf("  code 0x%02X\n", hdr->code);
    printf("  id   0x%02X%02X\n", hdr->id[0], hdr->id[1]);
}

/**
 * @brief Print a buffer.
 * 
 * @param[in] buf the buffer to print
 * @param[in] buflen number of bytes to print from the buffer
 * @param[in] bare only print the buffers raw data
 */
void dump(const uint8_t *buf, size_t buflen, bool bare) {
    if (bare) {
        while(buflen--)
            printf("%02X%s", *buf++, (buflen > 0) ? " " : "");
    } else {
        printf("Dump: ");
        while(buflen--) {
            printf("%02X%s", *buf++, (buflen > 0) ? " " : "");
        }
        printf("\n");
    }
}

/**
 * @brief Print CoAP options.
 * 
 * @param[in] opts CoAP options to print
 * @param[in] numopt number of CoAp options to print
 */
void dumpOptions(coap_option_t *opts, size_t numopt) {
    size_t i;
    printf(" Options:\n");
    for (i=0;i<numopt;i++) {
        printf("  0x%02X [ ", opts[i].num);
        dump(opts[i].buf.p, opts[i].buf.len, true);
        printf(" ]\n");
    }
}

/**
 * @brief Print CoAP package.
 * 
 * @param[in] pkt CoAP package to print
 */
void dumpPacket(coap_packet_t *pkt) {
    dumpHeader(&pkt->hdr);
    dumpOptions(pkt->opts, pkt->numopts);
    printf("Payload: ");
    dump(pkt->payload.p, pkt->payload.len, true);
    printf("\n");
}
