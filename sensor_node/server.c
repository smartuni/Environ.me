#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include "thread.h"
#include "coap.h"
// own header
#include "server.h"
// sensors header
#include "sensors.h"

static void *server(void *arg);
static int get_temperature_handle(coap_rw_buffer_t *scratch,
				  const coap_packet_t *inpkt,
				  coap_packet_t *outpkt,
				  uint8_t id_hi, uint8_t id_lo);
static void dumpHeader(coap_header_t *hdr);
static void dump(const uint8_t *buf, size_t buflen, bool bare);
static void dumpOptions(coap_option_t *opts, size_t numopt);
static void dumpPacket(coap_packet_t *pkt);

static const coap_endpoint_path_t temperature_path = {1, {"temperature"}};
const coap_endpoint_t endpoints[] =
{
    {COAP_METHOD_GET, get_temperature_handle, &temperature_path, "ct=0"},
    {(coap_method_t)0, NULL, NULL, NULL}
};

static char thread_stack[THREAD_STACKSIZE_DEFAULT];
static msg_t msg_queue[MSG_QUEUE_SIZE];
static uint8_t server_buffer[BUFFER_SIZE];
static uint8_t scratch_raw[BUFFER_SIZE];
static coap_rw_buffer_t scratch_buffer = {scratch_raw,  sizeof(scratch_raw)};
static char response[RSP_BUFFER_SIZE];

/**
 * @brief start CoAP server thread
 */
void start_server(void) {
    thread_create(thread_stack, sizeof(thread_stack), THREAD_PRIORITY_MAIN,
		  CREATE_STACKTEST, server, NULL, "coap_server");
}

/**
 * @brief CoAP server thread
 *
 * @param[in] arg   unused
 */
static void *server(void *arg) {
    uint16_t port;
    int server_socket;
    struct sockaddr_in6 server_addr;
    (void) arg;
    msg_init_queue(msg_queue, MSG_QUEUE_SIZE);
    port = (uint16_t)PORT;
    if (port == 0) {
        puts("[coap_server] ERROR: invalid port specified");
        return NULL;
    }
    server_socket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    if (server_socket < 0) {
        puts("[coap_server] ERROR: initializing socket");
        server_socket = 0;
        return NULL;
    }
    server_addr.sin6_family = AF_INET6;
    memset(&server_addr.sin6_addr, 0, sizeof(server_addr.sin6_addr));
    server_addr.sin6_port = htons(port);
    if (bind(server_socket, (struct sockaddr *)&server_addr,
	     sizeof(server_addr)) < 0) {
        server_socket = -1;
        puts("[coap_server] ERROR: binding socket");
        return NULL;
    }
    printf("[coap_server] INFO:  started CoAP server on port %" PRIu16 "\n",
	   port);
    while (1) {
        int recv_len;
	size_t buffer_size = sizeof(server_buffer);
	int rc;
        struct sockaddr_in6 client_addr;
        socklen_t client_addr_len = sizeof(struct sockaddr_in6);
        // blocking receive, waiting for data
        if ((recv_len = recvfrom(server_socket, server_buffer, buffer_size, 0,
				 (struct sockaddr *)&client_addr,
				 &client_addr_len)) < 0) {
            puts("[coap_server] ERROR: on receive");
        }
        else if (recv_len == 0) {
            puts("[coap_server] INFO:  peer did shut down");
        }
        else { // CoAP part
	    coap_packet_t inpkt;
	    puts("[coap_server] INFO:  received packet: ");
	    dump(server_buffer, recv_len, true);
	    puts("\n");
	    if (0 != (rc = coap_parse(&inpkt, server_buffer, recv_len))) {
		printf("[coap_server] ERROR: bad packet rc=%d\n", rc);
	    }
	    else {
		coap_packet_t outpkt;
		puts("content:");
		dumpPacket(&inpkt);
		coap_handle_req(&scratch_buffer, &inpkt, &outpkt);
		if (0 != (rc = coap_build(server_buffer, &buffer_size,
					  &outpkt))) {
		    printf("[coap_server] ERROR: coap_build failed rc=%d\n",
			   rc);
		}
		else {
		    puts("[coap_server] INFO:  sending packet");
		    dump(server_buffer, buffer_size, true);
		    puts("\ncontent:");
		    dumpPacket(&outpkt);
		    if (sendto(server_socket, server_buffer, buffer_size, 0,
			       (struct sockaddr *)&client_addr,
			       sizeof(client_addr)) < 0) {
			puts("[coap_server] ERROR: sending data");
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
    int temperature;
    puts("[coap_server] INFO:  handling temperature response");
    temperature = get_temperature();
    sprintf(response, "%d", temperature);
    return coap_make_response(scratch, outpkt, (const uint8_t *)response,
			      strlen(response), id_hi, id_lo,
			      &inpkt->tok, COAP_RSPCODE_CONTENT,
			      COAP_CONTENTTYPE_TEXT_PLAIN);
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