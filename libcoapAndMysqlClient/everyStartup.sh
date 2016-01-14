##############################################################
#
# Nicolas Albers
# 2016
#
##############################################################

# it is necessary to make the settigs after every startup
# recomended is to put this into a cronjob that it will be called after every startup 

# configure 6LoWPAN interface for channel 23 and PAN ID 0x23
sudo kill -s KILL `pgrep ifplugd`
sudo modprobe ipv6
sudo ip link set wpan0 down
sudo ip link set lowpan0 down
sudo iwpan phy phy0 set channel 0 26
sudo ip link add link wpan0 name lowpan0 type lowpan
sudo iwpan dev wpan0 set pan_id 0x23
sudo ip link set wpan0 up
sudo ip link set lowpan0 up
#sudo reboot


## node configuration
sudo ifconfig lowpan0 inet6 add fd2d:0388:6a7b:0001:0000:0000:0000:0001/120
sudo rmmod nhc_udp

