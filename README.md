# Willkommen auf der Packet Autobahn
##A visual representation of the Wifi Data traffic in the air surrounding you
compile with ```g++ main.cpp -o packet_autobahn -O3 -std=c++11 -lpthread -ltins -lncurses```
###to do
- add either resize before [enter any key to start] or responsive resizing during runtime
- allow wifi hoping with ```sudo aiport -z``` to disassociate from any network before the start and ```sudo aiport -c [+channel number]``` to hope channels and allow to hop channels. >>> maybe this should be done through keypresses as the app 'follows' individual devices traffic over time, hoping would interrupt that and therefore be a choice (e.g. when there is not much going on on a current channel). <br> resolve: whats the difference between ```sudo aiport -z``` and ```networksetup -setairportpower <device name> <on off>```
- alternative mode: line always filled with pipe symbols from left to right and colors representing the share of each individual device.

![screenshot](https://github.com/leoneckert/packet-autobahn/blob/master/in_progres_screenshot_1.png)

.