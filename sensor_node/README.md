# Environ.me/sensor_node #

This is the sensor node application for the Envirom.me project.
It can measure the temperature, humidity and illmuninance and control an LED band.

The sensor node starts a CoAP server to receive requests from a client to measure a paramter
or to change the mode of the LED band. It also provides shell commands for the measurments and LED control.

The measurments are only done on a GET request from the client
and the result of the measurment is send to the client in the CoAP response as a plain text integer value.
The returned units are `°C * 100` for the temperature, `% * 100` for the humidity and `lux` for the illuminance.

The LED band supports 3 different modes defined by a number send in the payload of the PUT request:
```
0 = LED band off
1 = Moving light from right to left
2 = Moving light from left to right
```

## Requirements ##

### Hardware ###

* <b>Phytec phyWAVE KW22</b><br />
  The sensor node was only tested with the Phytec board,
  for other platforms supported by RIOT and with the needed sensores it might be required to modify the LED control module.
* <b>RGB LED band</b>

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

### Supported CoAP requests ###

The CoAP sever supports these CoAP request:

* `GET /tempature` Request to measure and return the current temperature
* `GET /humidity` Request to measure and return the current humidity
* `GET /illuminance` Request to measure and return the current illuminance
* `GET /all` Request to measure and return all current paramters
* `PUT /led <number>` Request to change the mode of the LED band (`<number>`: 0 = off, 1 = left, 2 = right)

### Shell commands ###

The shell supports these commands:

* `temperature` To measure and print the temperature
* `humidity` To measure and print the humidity
* `illuminance` To measure and print the illuminance
* `led <number>` To set the mode of the LED band

## Known Issues and Workarounds ##

### LED control module ###

The LED band is controled via a GPIO, but the RIOT timers are not precise enough for the timing of the LED band.
Thats why the timing is done with simple waiting loops. But these don't provide reliable timings,
which are highly depending on the used hardware and RIOT version.

Possible solutions:

* Check the timing with an oscilloscope and adjust the LED control module.
* Use a second microcontroller for the control of the LED band.
* Use the SPI MOSI line instead of the GPIO.
