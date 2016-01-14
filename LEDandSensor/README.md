included files implement the usage of the temperature, humidity, light, gas sensor and the usage of the WS2812 led stripe:

writeLed(...) writes GRB-values for one led in corresponding array,

resetArray(...) writes all values in led array to zero,

sendZero() gives a pulse on led pin which is interpreted as low bit, only needed in sendArray(),

sendOne() gives a pulse on led pin which is interpreted as high bit, only needed in sendArray(),

sendArray(...) sends whole array data through led pin.

adcCoCalc(...) gas sensor is read via adc and its value is calculated by this function

temperature, humidity and light sensors are already implemented in RIOT, take a look at included tests for usage of hdc1000 and tcs37727.
