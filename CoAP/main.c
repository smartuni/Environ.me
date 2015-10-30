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
#define SERVER_PORT       (5683)
#define CLIENT_PORT       (5684)
#define MAX_RESPONSE_SIZE (64)

static void start_server(void);
static void *server(void *arg);
static int get_temperature_handle(coap_rw_buffer_t *scratch,
				  const coap_packet_t *inpkt,
				  coap_packet_t *outpkt,
				  uint8_t id_hi, uint8_t id_lo);
static int request_temperature(int argc, char **argv);
static void dumpHeader(coap_header_t *hdr);
static void dump(const uint8_t *buf, size_t buflen, bool bare);
static void dumpOptions(coap_option_t *opts, size_t numopt);
static void dumpPacket(coap_packet_t *pkt);

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
static int client_socket = -1;
static uint8_t server_buffer[BUFFER_SIZE];
static uint8_t client_buffer[BUFFER_SIZE];
static msg_t msg_queue[MSG_QUEUE_SIZE];
static uint8_t scratch_raw[BUFFER_SIZE];
static coap_rw_buffer_t scratch_buffer = {scratch_raw,  sizeof(scratch_raw)};
static char *response = "22.5 C";
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
    uint16_t server_port = (uint16_t)SERVER_PORT;
    uint16_t client_port = (uint16_t)CLIENT_PORT;
    arg = arg;
    msg_init_queue(msg_queue, MSG_QUEUE_SIZE);
    server_socket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    if (server_port == 0 || client_port == 0) {
        puts("ERROR: invalid port specified");
        return NULL;
    }
    server_addr.sin6_family = AF_INET6;
    memset(&server_addr.sin6_addr, 0, sizeof(server_addr.sin6_addr));
    server_addr.sin6_port = htons(server_port);
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
    printf("INFO:  started CoAP server on port %" PRIu16 "\n", server_port);
    while (1) {
        int recv_len;
	size_t buffer_size = sizeof(server_buffer);
	int rc;
        struct sockaddr_in6 client_addr;
        socklen_t client_addr_len = sizeof(struct sockaddr_in6);
	coap_packet_t inpkt;
        // blocking receive, waiting for data
        if ((recv_len = recvfrom(server_socket, server_buffer, buffer_size, 0,
				 (struct sockaddr *)&client_addr,
				 &client_addr_len)) < 0) {
            puts("ERROR: on receive");
        }
        else if (recv_len == 0) {
            puts("INFO:  peer did shut down");
        }
        else { // CoAP part
	    puts("INFO:  redeived packet: ");
	    dump(server_buffer, recv_len, true);
	    puts("\n");
	    if (0 != (rc = coap_parse(&inpkt, server_buffer, recv_len))) {
		printf("Bad packet rc=%d\n", rc);
	    }
	    else {
		coap_packet_t outpkt;
		puts("content:");
		dumpPacket(&inpkt);
		coap_handle_req(&scratch_buffer, &inpkt, &outpkt);
		puts("outpkt before build:");
		dumpPacket(&outpkt);
		if (0 != (rc = coap_build(server_buffer, &buffer_size,
					  &outpkt))) {
		    printf("ERROR: coap_build failed rc=%d\n", rc);
		}
		else {
		    puts("INFO:  sending packet: ");
		    dump(server_buffer, buffer_size, true);
		    puts("\ncontent:");
		    dumpPacket(&outpkt);
		    // send response on different port
		    client_addr.sin6_port = htons(client_port);
		    if (sendto(server_socket, server_buffer, buffer_size, 0,
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
    puts("INFO:  handling temperature response");
    return coap_make_response(scratch, outpkt, (const uint8_t *)response,
			      strlen(response), id_hi, id_lo,
			      &inpkt->tok, COAP_RSPCODE_CONTENT,
			      COAP_CONTENTTYPE_TEXT_PLAIN);
}

static int request_temperature(int argc, char **argv) {
    struct sockaddr_in6 server_addr;
    size_t server_addr_len = sizeof(server_addr);
    struct sockaddr_in6 client_addr;
    size_t client_addr_len = sizeof(client_addr);
    uint16_t server_port = (uint16_t)SERVER_PORT;
    uint16_t client_port = (uint16_t)CLIENT_PORT;
    size_t buffer_size = sizeof(client_buffer);
    int rc;
    int recv_len;
    coap_packet_t inpkt;
    if (argc != 2) {
	puts("INFO:  usage: temperature <addr>");
	return 1;
    }
    if (server_port == 0 || client_port == 0) {
        puts("ERROR: invalid port specified");
        return 1;
    }
    client_socket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    if (client_socket < 0) {
        puts("ERROR: initializing socket");
        server_socket = 0;
        return 1;
    }
    server_addr.sin6_family = AF_INET6;
    if (inet_pton(AF_INET6, argv[1], &server_addr.sin6_addr) != 1) {
	puts("ERROR: unable to parse destination address");
	return 1;
    }
    server_addr.sin6_port = htons(server_port);
    client_addr.sin6_family = AF_INET6;
    memset(&client_addr.sin6_addr, 0, sizeof(client_addr.sin6_addr));
    client_addr.sin6_port = htons(client_port);
    if (bind(client_socket, (struct sockaddr *)&client_addr,
	     client_addr_len) < 0) {
        client_socket = -1;
        puts("ERROR: binding socket");
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
	puts("DEBUG: creating path option");
	coap_option_t path_option = {
	    .num = COAP_OPTION_URI_PATH,
	    .buf = {.p = (const uint8_t *)temperature_path.elems[i],
	            .len = strlen(temperature_path.elems[i])}
	};
	request_pkt.opts[i] = path_option;
    }
    puts("DEBUG: building coap package");
    if (0 != (rc = coap_build(client_buffer, &buffer_size, &request_pkt))) {
	printf("ERROR: coap_build failed rc=%d\n", rc);
	return 1;
    }
    puts("INFO:  sending packet: ");
    dump(client_buffer, buffer_size, true);
    puts("\ncontent:");
    dumpPacket(&request_pkt);
    if (sendto(client_socket, client_buffer, buffer_size, 0,
	       (struct sockaddr *)&server_addr, server_addr_len) < 0) {
	puts("ERROR: sending data");
    }
    // blocking receive, waiting for data
    if ((recv_len = recvfrom(client_socket, client_buffer, buffer_size, 0,
			     (struct sockaddr *)&server_addr,
			     &server_addr_len)) < 0) {
        puts("ERROR: on receive");
    } else if (recv_len == 0) {
        puts("INFO:  peer did shut down");
    } else { // CoAP part
	puts("INFO:  redeived packet: ");
	dump(client_buffer, recv_len, true);
	puts("\n");
	if (0 != (rc = coap_parse(&inpkt, client_buffer, recv_len))) {
	    printf("Bad packet rc=%d\n", rc);
        } else {
	    puts("\ncontent:");
	    dumpPacket(&inpkt);
	}
    }
    return 0;
}

void dumpHeader(coap_header_t *hdr)
{
    printf("Header:\n");
    printf("  ver  0x%02X\n", hdr->ver);
    printf("  t    0x%02X\n", hdr->t);
    printf("  tkl  0x%02X\n", hdr->tkl);
    printf("  code 0x%02X\n", hdr->code);
    printf("  id   0x%02X%02X\n", hdr->id[0], hdr->id[1]);
}


void dump(const uint8_t *buf, size_t buflen, bool bare)
{
    if (bare)
    {
        while(buflen--)
            printf("%02X%s", *buf++, (buflen > 0) ? " " : "");
    }
    else
    {
        printf("Dump: ");
        while(buflen--)
            printf("%02X%s", *buf++, (buflen > 0) ? " " : "");
        printf("\n");
    }
}

void dumpOptions(coap_option_t *opts, size_t numopt)
{
    size_t i;
    printf(" Options:\n");
    for (i=0;i<numopt;i++)
    {
        printf("  0x%02X [ ", opts[i].num);
        dump(opts[i].buf.p, opts[i].buf.len, true);
        printf(" ]\n");
    }
}

void dumpPacket(coap_packet_t *pkt)
{
    dumpHeader(&pkt->hdr);
    dumpOptions(pkt->opts, pkt->numopts);
    printf("Payload: ");
    dump(pkt->payload.p, pkt->payload.len, true);
    printf("\n");
}
