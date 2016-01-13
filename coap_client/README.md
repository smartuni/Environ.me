# Environ.me/coap_client #

This is a small CoAP client for RIOT. It can be used to test the CoAP server of the [Environ.me/sensor_node](/sensor_node).

## Requirements ##

### Hardware ###

* <b>Phytec phyWAVE KW22</b><br />
  The CoAP client was only tested with the Phytec board,
  but it should also work with every other platform supported by RIOT.

### Software ###

* <b>RIOT (2015.09-branch)</b><br />
  At this time (2016-01-08) newer RIOT versions are not working with the Phytec board.

## Usage ##

### Make ###

The `Makefile` uses the default environment variables:
```
RIOTBASE ?= $(CURDIR)/../../RIOT
BOARD ?= pba-d-01-kw2x
```
These can be changed in the `Makefile` or by setting these variables manually.

To build, flash and start the terminal:
```
make flash term
```

### Sending CoAP requests ###

The CoAP client only supports the CoAP request:
```
GET /tempature
```
To send the request to the server with the ip `fe80::f8e3:4e62:71ba:699a` use the shell command:
```
temperature fe80::f8e3:4e62:71ba:699a
```

## Known Issues and Workarounds ##

### Response Port ###

The CoAP client listens for the response on port 5683, but sends the request on another one.
A correct response to the clients port that was used to send the request will not be received.

As a workaround uncomment the line:
```
client_addr->sin6_port = htons((uint16_t)PORT);
```
in [Environ.me//sensor_node/server.c](/sensor_node/server.c#L197) of the sensor_node to force the CoAP server
to send all responses to the clients port 5683.
