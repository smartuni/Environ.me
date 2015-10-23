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

#define DEBUG
#define BUFFER_SIZE       (256)
#define MSG_QUEUE_SIZE    (64)
#define PORT              (5683)
#define MAX_RESPONSE_SIZE (64)

static void start_server(void);
static void *server(void *arg);
static int get_temperature_handle(coap_rw_buffer_t *scratch,
				  const coap_packet_t *inpkt,
				  coap_packet_t *outpkt,
				  uint8_t id_hi, uint8_t id_lo);
static int request_temperature(int argc, char **argv);

static const shell_command_t shell_commands[] =
{
    {"temperature", "request temperature from server", request_temperature},
    {NULL, NULL, NULL}
};
static const coap_endpoint_path_t temperature_path = {1, {"temperature"}};
const coap_endpoint_t endpoints[] =
{
    {COAP_METHOD_GET, get_temperature_handle, &temperature_path, "ct=0"},
    {(coap_method_t)0, NULL, NULL, NULL}
};

// static vars
static char thread_stack[THREAD_STACKSIZE_DEFAULT];
static int server_socket = -1;
static uint8_t buffer[BUFFER_SIZE];
static uint8_t client_buffer[BUFFER_SIZE];
static msg_t msg_queue[MSG_QUEUE_SIZE];
static uint8_t scratch_raw[BUFFER_SIZE];
static coap_rw_buffer_t scratch_buffer = {scratch_raw,  sizeof(scratch_raw)};
static uint8_t response[MAX_RESPONSE_SIZE] = "";
static uint16_t message_id = 1;

/**
 * @brief the main programm loop
 *
 * @return non zero on error
 */
int main(void) {
    // some initial infos
    puts("Envron.me - CoAP test!");
    puts("================");
    printf("You are running RIOT on a(n) %s board.\n", RIOT_BOARD);
    printf("This board features a(n) %s MCU.\n", RIOT_MCU);
    puts("================");

    // start CoAP server
    start_server();

    // start shell
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}

/**
 * @brief start CoAP server thread
 */
static void start_server(void) {
    thread_create(thread_stack, sizeof(thread_stack), THREAD_PRIORITY_MAIN,
		  CREATE_STACKTEST, server, NULL, "coap_server");
}

/**
 * @brief CoAP server thread
 *
 * @param[in] arg   unused
 */
static void *server(void *arg) {
    struct sockaddr_in6 server_addr;
    uint16_t port;
    arg = arg;
    msg_init_queue(msg_queue, MSG_QUEUE_SIZE);
    server_socket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    /* parse port */
    port = (uint16_t)PORT;
    if (port == 0) {
        puts("ERROR: invalid port specified");
        return NULL;
    }
    server_addr.sin6_family = AF_INET6;
    memset(&server_addr.sin6_addr, 0, sizeof(server_addr.sin6_addr));
    server_addr.sin6_port = htons(port);
    if (server_socket < 0) {
        puts("ERROR: initializing socket");
        server_socket = 0;
        return NULL;
    }
    if (bind(server_socket, (struct sockaddr *)&server_addr,
	     sizeof(server_addr)) < 0) {
        server_socket = -1;
        puts("ERROR: binding socket");
        return NULL;
    }
    printf("INFO:  started CoAP server on port %" PRIu16 "\n", port);
    while (1) {
        int recv_len;
	int rc;
        struct sockaddr_in6 client_addr;
        socklen_t client_addr_len = sizeof(struct sockaddr_in6);
	coap_packet_t inpkt;
        // blocking receive, waiting for data
        if ((recv_len = recvfrom(server_socket, buffer, sizeof(buffer), 0,
				 (struct sockaddr *)&client_addr,
				 &client_addr_len)) < 0) {
            puts("ERROR: on receive");
        }
        else if (recv_len == 0) {
            puts("INFO:  peer did shut down");
        }
        else { // CoAP part
	    puts("INFO:  redeived packet: ");
	    coap_dump(buffer, recv_len, true);
	    puts("\n");
	    if (0 != (rc = coap_parse(&inpkt, buffer, recv_len))) {
		printf("Bad packet rc=%d\n", rc);
	    }
	    else {
		size_t send_len = sizeof(buffer);
		coap_packet_t outpkt;
		puts("content:");
		coap_dumpPacket(&inpkt);
		coap_handle_req(&scratch_buffer, &inpkt, &outpkt);
		if (0 != (rc = coap_build(buffer, &send_len, &outpkt))) {
		    printf("ERROR: coap_build failed rc=%d\n", rc);
		}
		else {
		    puts("INFO:  sending packet: ");
		    coap_dump(buffer, send_len, true);
		    puts("\ncontent:");
		    coap_dumpPacket(&outpkt);
		    if (sendto(server_socket, buffer, send_len, 0,
			       (struct sockaddr *)&client_addr,
			       sizeof(client_addr)) < 0) {
			puts("ERROR: sending data");
		    }
		}
	    }
        }
    }
    return NULL;
}

static int get_temperature_handle(coap_rw_buffer_t *scratch,
				  const coap_packet_t *inpkt,
				  coap_packet_t *outpkt,
				  uint8_t id_hi, uint8_t id_lo) {
    char *temperature = "20";
    memcpy((void*)response, temperature, strlen((char*)response));
    return coap_make_response(scratch, outpkt, response,
			      strlen((char*)response), id_hi, id_lo,
			      &inpkt->tok, COAP_RSPCODE_CONTENT,
			      COAP_CONTENTTYPE_TEXT_PLAIN);
}

static int request_temperature(int argc, char **argv) {
    struct sockaddr_in6 dst_addr;
    size_t dst_addr_len = sizeof(dst_addr);
    uint16_t port;
    int client_socket;
    size_t buffer_len = sizeof(client_buffer);
    size_t request_len;
    int rc;
    int recv_len;
    coap_packet_t inpkt;
    if (argc != 2) {
	puts("INFO:  usage: temperature <addr>");
	return 1;
    }
    dst_addr.sin6_family = AF_INET6;
    if (inet_pton(AF_INET6, argv[1], &dst_addr.sin6_addr) != 1) {
	puts("ERROR: unable to parse destination address");
	return 1;
    }
    port = (uint16_t)PORT;
    dst_addr.sin6_port = htons(port);
    client_socket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    if (client_socket < 0) {
	puts("ERROR: socket initialization failed");
	return 1;
    }
    puts("INFO:  creating CoAP GET request");
    coap_header_t request_hdr = {
	.ver = 1,
	.t = COAP_TYPE_CON,
	.tkl = 0,
	.code = MAKE_RSPCODE(0, COAP_METHOD_GET),
	.id = {(message_id & 0xFF), ((message_id >> 8) & 0xFF)}
    };
    coap_buffer_t payload_buf = {
	.p = (const uint8_t *){0},
	.len = 0
    };
    coap_buffer_t token_buf = {
	.p = (const uint8_t *){0},
	.len = 0
    };
    coap_packet_t request_pkt = {
	.hdr = request_hdr,
	.tok = token_buf,
	.numopts = temperature_path.count,
	.payload = payload_buf
    };
    for (int i = 0; i < temperature_path.count; i++) {
	puts("DEBUG: creating path option");
	coap_option_t path_option = {
	    .num = COAP_OPTION_URI_PATH,
	    .buf = {.p = (const uint8_t *)temperature_path.elems[i],
	            .len = strlen(temperature_path.elems[i])}
	};
	request_pkt.opts[i] = path_option;
    }
    request_len = sizeof(request_pkt);
    if (request_len > buffer_len) {
	printf("ERROR: buffer to small. request_len = %d, buffer_len = %d\n",
	       (int)request_len, (int)buffer_len);
	return 1;
    }
    puts("DEBUG: building coap package");
    if (0 != (rc = coap_build(client_buffer, &buffer_len, &request_pkt))) {
	printf("ERROR: coap_build failed rc=%d\n", rc);
	return 1;
    }
    puts("INFO:  sending packet: ");
    coap_dump(client_buffer, request_len, true);
    puts("\ncontent:");
    coap_dumpPacket(&request_pkt);
    if (sendto(client_socket, client_buffer, buffer_len, 0,
	       (struct sockaddr *)&dst_addr, dst_addr_len) < 0) {
	puts("ERROR: sending data");
    }
    // blocking receive, waiting for data
    if ((recv_len = recvfrom(client_socket, client_buffer, buffer_len, 0,
			     (struct sockaddr *)&dst_addr,
			     &dst_addr_len)) < 0) {
        puts("ERROR: on receive");
    } else if (recv_len == 0) {
        puts("INFO:  peer did shut down");
    } else { // CoAP part
	puts("INFO:  redeived packet: ");
	coap_dump(client_buffer, recv_len, true);
	puts("\n");
	if (0 != (rc = coap_parse(&inpkt, client_buffer, recv_len))) {
	    printf("Bad packet rc=%d\n", rc);
        } else {
	    puts("\ncontent:");
	    coap_dumpPacket(&inpkt);
	}
    }
    return 0;
}
