# RGB-leds-and-lightbulbs

Remote control for both RGB led strip and string light bulbs using MySensors to drive it remotely over MQTT and radio.

The node runs on Arduino Nano connected with NRF24L01+ radio. There is also four PWM pins used to drive N-Channel Mosfets to control lights.

All the further logic and GUI is behind MySensors MQTT GW, and handled and visualized in OpenHab and Node-Red. The OpenHAB rules converter for RGB values is found from (rgb.rules)[src/rgb.rules] file.

The program uses [MySensors] (https://github.com/mysensors/MySensors), and code is modified version of the code fit for the purpose here: https://forum.mysensors.org/topic/6765/rgb-led-strip/ by @maghac.

Author: Ilkka Tengvall

LICENCE: GPLv2

# Pics

!(lightbulbs, leds and arduino)[pics/kuva1.jpg]
!(arduino, radio and mosfets)[pics/kuva2.jpg]
!(housing for the stuff)[pics/kuva3.jpg]
!(first trial)[pics/kuva4.jpg]
!(OpenHAB android 1)[pics/kuva5.jpg]
!(OpenHAB android 2)[pics/kuva6.jpg]
