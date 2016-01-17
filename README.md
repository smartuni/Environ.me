# Environ.me #

Environ.me is part of the smartuni bachelor project with [RIOT-OS](https://github.com/RIOT-OS/RIOT)
at Hamburg University of Applied Sciences.

It shall provide an environment surveillance system for rooms people living, learning or working in
via permanently measuring the luminance, the air's temperature, humidity and quality (ppm gas proportions like carbon monoxide)
and displaying those data through a (web) server for monitoring.
Also in case of a fire the system is able to automatically show the nearest safe emergency exit to be used
via animated coloured light stripes.

## Hardware ##

##### phyNODE Eval-Board #####

The Eval-Board of the [Phytec phyWAVE KW22](https://github.com/RIOT-OS/RIOT/wiki/Board%3A-Phytec-phyWAVE-KW22)
is the sensor node measuring the environmental paramters, sending the measured values to the Raspberry Pi
and controlling the LED stripe.

It provides 6 onboard sensors:

* Humidity Sensor HDC1000
* Pressure Sensor MPL3115A2
* Tri-axis Accelerometer MMA8652FC
* Magnetometer MAG3110FCR1
* Light Sensor TCS3772
* IR-Termopile Sensor TMP006

Environ.me uses the HDC1000 for measuring the humidity and temperature and the TCS3772 for measuring the illuminace.

The phyWAVE also has an integrated IEEE 802.15.4 compliant radio used for the communication with the Raspberry Pi.

##### MQ-7 gas sensor #####

The MQ-7 in combination with the ADC of the phyWAVE is used to measure the air quality in the room.

##### WS2812 LED stripe #####

The LED stripe is used to signal the nearest safe emergency exit with a moving light to the exit in case of a fire.
It is controlled via one GPIO by the phyWAVE.

##### Raspberry Pi #####

The Raspberry Pi request the the sensor nodes measurment results and presents these to the user via a webpage.
