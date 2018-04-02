# RGB-leds-and-lightbulbs

Remote control for both RGB led strip and string light bulbs using MySensors to drive it remotely over MQTT and radio.

The node runs on Arduino Nano connected with NRF24L01+ radio. There is also four PWM pins used to drive N-Channel Mosfets to control lights.

All the further logic and GUI is behind MySensors MQTT GW, and handled and visualized in OpenHab and Node-Red. The OpenHAB rules converter for RGB values is found from (rgb.rules)[rgb.rules] file.

The program uses [MySensors] (https://github.com/mysensors/MySensors), and code is modified version of the code fit for the purpose here: https://forum.mysensors.org/topic/6765/rgb-led-strip/ by @maghac.

Author: Ilkka Tengvall
LICENCE: GPLv2
