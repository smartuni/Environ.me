included files implement the usage of the temperature, humidity, light, gas sensor and the usage of the WS2812 led stripe:

Hardware:
Phytec Phywave kw2x, MQ-7 gas sensor, Ws2812 Led stripe.

Software:
RIOT OS branch 2015.09

compile and flash via terminal command:
(sudo) BOARD=pba-d-01-kw2x make flash

for displaying data use terminal, open via terminal command:
(sudo) BOARD=pba-d-01-kw2x make term



writeLed(...) writes GRB-values for one led in corresponding array,

resetArray(...) writes all values in led array to zero,

sendZero() gives a pulse on led pin which is interpreted as low bit, only needed in sendArray(),

sendOne() gives a pulse on led pin which is interpreted as high bit, only needed in sendArray(),

sendArray(...) sends whole array data through led pin.

adcCoCalc(...) gas sensor is read via adc and its value is calculated by this function

temperature, humidity and light sensors are already implemented in RIOT, take a look at included tests for usage of hdc1000 and tcs37727.
