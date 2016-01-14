##############################################################
#
# Nicolas Albers
# 2016
#
##############################################################


# this script is nessesary to get data periodicaly

while true
do
	echo "try"
    ./libcoap/examples/client -o '' -m get coap://[fe80::5bb3:4e48:6fdc:6002%lowpan0]/temperature


done
