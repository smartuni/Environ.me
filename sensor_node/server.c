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
// led control header
#include "ledcontrol.h"

#ifndef THREAD_CREATE_STACKTEST 
#define THREAD_CREATE_STACKTEST CREATE_STACKTEST
#endif

#define BUFFER_SIZE       (256)
#define MSG_QUEUE_SIZE    (64)
#define PORT              (5683)
#define RSP_BUFFER_SIZE   (64)

static void *server(void *arg);
static int send_rsp(struct sockaddr_in6 *client_addr, uint8_t *rsp,
                    size_t rsp_len);
static int get_temperature_handle(coap_rw_buffer_t *scratch,
                                  const coap_packet_t *inpkt,
                                  coap_packet_t *outpkt,
                                  uint8_t id_hi, uint8_t id_lo);
static int get_humidity_handle(coap_rw_buffer_t *scratch,
                               const coap_packet_t *inpkt,
                               coap_packet_t *outpkt,
                               uint8_t id_hi, uint8_t id_lo);
static int get_illuminance_handle(coap_rw_buffer_t *scratch,
                                  const coap_packet_t *inpkt,
                                  coap_packet_t *outpkt,
                                  uint8_t id_hi, uint8_t id_lo);
static int get_all_handle(coap_rw_buffer_t *scratch,
                          const coap_packet_t *inpkt,
                          coap_packet_t *outpkt,
                          uint8_t id_hi, uint8_t id_lo);
static int put_led_handle(coap_rw_buffer_t *scratch,
                          const coap_packet_t *inpkt,
                          coap_packet_t *outpkt,
                          uint8_t id_hi, uint8_t id_lo);
static void dumpHeader(coap_header_t *hdr);
static void dump(const uint8_t *buf, size_t buflen, bool bare);
static void dumpOptions(coap_option_t *opts, size_t numopt);
static void dumpPacket(coap_packet_t *pkt);

static const coap_endpoint_path_t temperature_path = {1, {"temperature"}};
static const coap_endpoint_path_t humidity_path = {1, {"humidity"}};
static const coap_endpoint_path_t illuminance_path = {1, {"illuminance"}};
static const coap_endpoint_path_t all_path = {1, {"all"}};
static const coap_endpoint_path_t led_path = {1, {"led"}};
const coap_endpoint_t endpoints[] =
{
    {COAP_METHOD_GET, get_temperature_handle, &temperature_path, "ct=0"},
    {COAP_METHOD_GET, get_humidity_handle, &humidity_path, "ct=0"},
    {COAP_METHOD_GET, get_illuminance_handle, &illuminance_path, "ct=0"},
    {COAP_METHOD_GET, get_all_handle, &all_path, "ct=0"},
    {COAP_METHOD_PUT, put_led_handle, &led_path, "ct=0"},
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
                  THREAD_CREATE_STACKTEST, server, NULL, "coap_server");
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
        puts("[coap_server] ERROR: Invalid port specified");
        return NULL;
    }
    server_socket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    if (server_socket < 0) {
        puts("[coap_server] ERROR: Initializing server socket failed");
        server_socket = 0;
        return NULL;
    }
    server_addr.sin6_family = AF_INET6;
    memset(&server_addr.sin6_addr, 0, sizeof(server_addr.sin6_addr));
    server_addr.sin6_port = htons(port);
    if (bind(server_socket, (struct sockaddr *)&server_addr,
         sizeof(server_addr)) < 0) {
        server_socket = -1;
        puts("[coap_server] ERROR: Binding socket failed");
        return NULL;
    }
    printf("[coap_server] INFO: Started CoAP server on port %" PRIu16 "\n",
           port);
    while (1) {
        int recv_len;
        size_t buffer_size = sizeof(server_buffer);
        int rc;
        struct sockaddr_in6 client_addr;
        socklen_t client_addr_len = sizeof(struct sockaddr_in6);
        char client_addr_str[IPV6_ADDR_MAX_STR_LEN];
        // blocking receive, waiting for data
        if ((recv_len = recvfrom(server_socket, server_buffer, buffer_size, 0,
                                 (struct sockaddr *)&client_addr,
                                 &client_addr_len)) < 0) {
            puts("[coap_server] ERROR: Receive failed");
        }
        else if (recv_len == 0) {
            puts("[coap_server] INFO: Peer did shut down");
        }
        else { // CoAP part
            coap_packet_t inpkt;
            puts("[coap_server] INFO: Received packet: ");
            dump(server_buffer, recv_len, true);
            inet_ntop(AF_INET6, &(client_addr.sin6_addr), client_addr_str,
                      sizeof(client_addr_str));
            printf("\nFrom: [%s]:%u\n", client_addr_str, ntohs(client_addr.sin6_port));
            if (0 != (rc = coap_parse(&inpkt, server_buffer, recv_len))) {
                printf("[coap_server] ERROR: Bad packet rc=%d\n", rc);
            } else {
                coap_packet_t outpkt;
                puts("Content:");
                dumpPacket(&inpkt);
                coap_handle_req(&scratch_buffer, &inpkt, &outpkt);
                if (0 != (rc = coap_build(server_buffer, &buffer_size,
                                          &outpkt))) {
                    printf("[coap_server] ERROR: coap_build failed rc=%d\n",
                           rc);
                } else {
                    puts("[coap_server] INFO: Sending packet");
                    dump(server_buffer, buffer_size, true);
                    puts("\nContent:");
                    dumpPacket(&outpkt);
                    send_rsp(&client_addr, server_buffer, buffer_size);
                }
            }
        }
    }
    return NULL;
}

static int send_rsp(struct sockaddr_in6 *client_addr, uint8_t *rsp, size_t rsp_len) {
    size_t client_addr_len = sizeof(*client_addr);
    int rsp_socket = -1;
    
    char addr_str[IPV6_ADDR_MAX_STR_LEN+4];
    int send_len = -2;
    
    rsp_socket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    if (rsp_socket < 0) {
        puts("[coap_server] ERROR: Initializing rsp socket failed");
        rsp_socket = 0;
        return 1;
    }
    
    //client_addr->sin6_port = htons((uint16_t)PORT);
    
    inet_ntop(AF_INET6, &(client_addr->sin6_addr), addr_str, sizeof(addr_str));
    puts("[coap_server]  INFO: Sending response:");
    printf("[coap_server]  INFO:     Data = ");
    dump(rsp, rsp_len, true);
    printf("\n[coap_server]  INFO:   Client = [%s]:%u\n", addr_str, ntohs(client_addr->sin6_port));
    
    if((send_len = sendto(rsp_socket, rsp, rsp_len, 0, (struct sockaddr *)client_addr,
              client_addr_len)) < 0) {
        puts("[coap_server] ERROR: Sending response failed");
    }
    
    printf("[coap_server]  INFO: Send %d bytes\n", send_len);
    
    close(rsp_socket);
    return 0;
}

static int get_temperature_handle(coap_rw_buffer_t *scratch,
                                  const coap_packet_t *inpkt,
                                  coap_packet_t *outpkt,
                                  uint8_t id_hi, uint8_t id_lo) {
    int temperature;
    puts("[coap_server] INFO: Handling get temperature response");
    temperature = get_temperature();
    sprintf(response, "%d", temperature);
    return coap_make_response(scratch, outpkt, (const uint8_t *)response,
                              strlen(response), id_hi, id_lo,
                              &inpkt->tok, COAP_RSPCODE_CONTENT,
                              COAP_CONTENTTYPE_TEXT_PLAIN);
}

static int get_humidity_handle(coap_rw_buffer_t *scratch,
                               const coap_packet_t *inpkt,
                               coap_packet_t *outpkt,
                               uint8_t id_hi, uint8_t id_lo) {
    int humidity;
    puts("[coap_server] INFO: Handling get humidity response");
    humidity = get_humidity();
    sprintf(response, "%d", humidity);
    return coap_make_response(scratch, outpkt, (const uint8_t *)response,
                              strlen(response), id_hi, id_lo,
                              &inpkt->tok, COAP_RSPCODE_CONTENT,
                              COAP_CONTENTTYPE_TEXT_PLAIN);
}

static int get_illuminance_handle(coap_rw_buffer_t *scratch,
                                  const coap_packet_t *inpkt,
                                  coap_packet_t *outpkt,
                                  uint8_t id_hi, uint8_t id_lo) {
    long illuminance;
    puts("[coap_server] INFO: Handling get illuminance response");
    illuminance = get_illuminance();
    sprintf(response, "%ld", illuminance);
    return coap_make_response(scratch, outpkt, (const uint8_t *)response,
                              strlen(response), id_hi, id_lo,
                              &inpkt->tok, COAP_RSPCODE_CONTENT,
                              COAP_CONTENTTYPE_TEXT_PLAIN);
}


static int get_all_handle(coap_rw_buffer_t *scratch,
                          const coap_packet_t *inpkt,
                          coap_packet_t *outpkt,
                          uint8_t id_hi, uint8_t id_lo) {
    puts("[coap_server] INFO: Handling get all response");
    int temp;
    int hum;
    long lux;
    get_all(&temp, &hum, &lux);
    sprintf(response, "%d,%d,%ld", temp, hum, lux);
    return coap_make_response(scratch, outpkt, (const uint8_t *)response,
                              strlen(response), id_hi, id_lo,
                              &inpkt->tok, COAP_RSPCODE_CONTENT,
                              COAP_CONTENTTYPE_TEXT_PLAIN);
}

static int put_led_handle(coap_rw_buffer_t *scratch,
                          const coap_packet_t *inpkt,
                          coap_packet_t *outpkt,
                          uint8_t id_hi, uint8_t id_lo) {
    puts("[coap_server] INFO: Handling put led response");
    if (inpkt->payload.len > 0) {
        if ((char)*(inpkt->payload.p) == '1') {
            set_led(LED_MODE_LEFT);
        } else if ((char)*(inpkt->payload.p) == '2') {
            set_led(LED_MODE_RIGHT);
        } else {
            set_led(LED_MODE_OFF);
        }
    } else {
        set_led(LED_MODE_OFF);
    }
    return coap_make_response(scratch, outpkt, (const uint8_t *)response,
                              0, id_hi, id_lo,
                              &inpkt->tok, COAP_RSPCODE_CHANGED,
                              COAP_CONTENTTYPE_TEXT_PLAIN);
}

void dumpHeader(coap_header_t *hdr) {
    printf("Header:\n");
    printf("  ver  0x%02X\n", hdr->ver);
    printf("  t    0x%02X\n", hdr->t);
    printf("  tkl  0x%02X\n", hdr->tkl);
    printf("  code 0x%02X\n", hdr->code);
    printf("  id   0x%02X%02X\n", hdr->id[0], hdr->id[1]);
}


void dump(const uint8_t *buf, size_t buflen, bool bare) {
    if (bare) {
        while(buflen--) {
            printf("%02X%s", *buf++, (buflen > 0) ? " " : "");
        }
    } else {
        printf("Dump: ");
        while(buflen--) {
            printf("%02X%s", *buf++, (buflen > 0) ? " " : "");
    }
        printf("\n");
    }
}

void dumpOptions(coap_option_t *opts, size_t numopt) {
    size_t i;
    printf(" Options:\n");
    for (i=0;i<numopt;i++) {
        printf("  0x%02X [ ", opts[i].num);
        dump(opts[i].buf.p, opts[i].buf.len, true);
        printf(" ]\n");
    }
}

void dumpPacket(coap_packet_t *pkt) {
    dumpHeader(&pkt->hdr);
    dumpOptions(pkt->opts, pkt->numopts);
    printf("Payload: ");
    dump(pkt->payload.p, pkt->payload.len, true);
    printf("\n");
}
