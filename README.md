# RGB-leds-and-lightbulbs

Remote control for both RGB led strip and string light bulbs using MySensors to drive it remotely over MQTT and radio.

The node runs on Arduino Nano connected with NRF24L01+ radio. There is also four PWM pins used to drive N-Channel Mosfets to control lights.

All the further logic and GUI is behind MySensors MQTT GW, and handled and visualized in OpenHab and Node-Red. The OpenHAB rules converter for RGB values is found from [rgb.rules](https://github.com/ikke-t/Arduino-RGB-leds-and-lightbulbs/raw/master/src/rgb.rules) file.

The program uses [MySensors] (https://github.com/mysensors/MySensors), and code is modified version of the code fit for the purpose here: https://forum.mysensors.org/topic/6765/rgb-led-strip/ by @maghac.

Author: Ilkka Tengvall

LICENCE: GPLv2

# Pics

![lightbulbs, leds and arduing](https://github.com/ikke-t/Arduino-RGB-leds-and-lightbulbs/raw/master/pics/kuva1.jpg)
![arduino, radio and mosfets](https://github.com/ikke-t/Arduino-RGB-leds-and-lightbulbs/raw/master/pics/kuva2.jpg)
![housing for the stuff](https://github.com/ikke-t/Arduino-RGB-leds-and-lightbulbs/raw/master/pics/kuva3.jpg)
![first trial](https://github.com/ikke-t/Arduino-RGB-leds-and-lightbulbs/raw/master/pics/kuva4.jpg)
![OpenHAB android 1](https://github.com/ikke-t/Arduino-RGB-leds-and-lightbulbs/raw/master/pics/kuva5.jpg)
![OpenHAB android 2](https://github.com/ikke-t/Arduino-RGB-leds-and-lightbulbs/raw/master/pics/kuva6.jpg)

# Video of the led effects

I was requested to add:

[LED effects video at youtube](https://youtu.be/chN6CxMVWm4)

I mention delay there, I meant fade, which is adjustable. Effects are created by @maghac. Please send PR for more :) I forgot to video the dimmer slider for lightbulbs, there is that too. The leds are still just laying on shelves, not attache to anything yet.
