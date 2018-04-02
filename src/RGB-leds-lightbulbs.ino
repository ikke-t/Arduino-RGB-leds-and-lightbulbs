/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * RGB-leds-lightbulbs.ino commands RGB-led strip and one chain of lights bulbs.
 *
 * modified from https://forum.mysensors.org/topic/6765/rgb-led-strip
 *
 * The RGB converter needed at OpenHAB in
 *
 * PINS connected to RADIO NRF24:
 * 7  - CE
 * 8  - CNS/CS
 * 11 - MOSI
 * 12 - MISO
 * 13 - SCK
 *
 * PINS Connected to RGB led strip N-MOSFET gate:
 * Red -5, Green - 6, Blue - 9
 * Pin connected to Light string of 3V light bulbs N-MOSFET gate:
 * 10
 */

// Prepare radio with NRF24L01+
#define MY_RADIO_NRF24
#define MY_BAUD_RATE   9600

#define MY_NODE_ID     5
#define MY_RF24_CE_PIN 7
#define MY_RF24_CS_PIN 8


#define CHILD_ID_RGB        1
#define CHILD_ID_LIGHTBULBS 2

#include <SPI.h>
#include <MySensors.h>

MyMessage rgbLightMsg(CHILD_ID_RGB, V_STATUS);
MyMessage rgbRgbMsg(CHILD_ID_RGB, V_RGB);
MyMessage rgbDimmerMsg(CHILD_ID_RGB, V_PERCENTAGE);
MyMessage rgbFadeMsg(CHILD_ID_RGB, V_VAR1);

MyMessage lbLightMsg(CHILD_ID_LIGHTBULBS, V_STATUS);
MyMessage lbDimmerMsg(CHILD_ID_LIGHTBULBS, V_PERCENTAGE);
MyMessage lbFadeMsg(CHILD_ID_LIGHTBULBS, V_VAR1);

byte white = 255;
byte red = 255;
byte green = 255;
byte blue = 255;
byte w0 = 255;
byte r0 = 255;
byte g0 = 255;
byte b0 = 255;
char rgbstring[] = "ffffff";

int lb_on_off_status = 1;
int lb_dimmerlevel = 100;
int lb_fadespeed = 10;

int rgb_on_off_status = 1;
int rgb_dimmerlevel = 100;
int rgb_fadespeed = 10;

// RGB strip
#define RED_PIN   5
#define GREEN_PIN 6
#define BLUE_PIN  9

// Ball lights control pin
#define LIGHTBULBS_PIN 10

void setup() {
  Serial.begin(MY_BAUD_RATE);
  Serial.println(F("Init done"));
  // Fix the PWM timer. Without this the LEDs will flicker.
  //TCCR0A = _BV(COM0A1) | _BV(COM0B1) | _BV(WGM00);

  // Output pins
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(LIGHTBULBS_PIN, OUTPUT);

}

void presentation()
{
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Alli's light controller", "1.0");
  // Register all sensors to gw (they will be created as child devices)
  present(CHILD_ID_RGB, S_RGB_LIGHT);
  present(CHILD_ID_LIGHTBULBS, S_DIMMER);
}

void loop()
{
  static bool first_message_sent = false;
  if ( first_message_sent == false ) {
    Serial.println( "Sending initial state..." );
    set_rgb_status();
    send_rgb_status();
    set_lb_status();
    send_lb_status();
    first_message_sent = true;
  }
}

void receive(const MyMessage &message)
{
  Serial.print( "sensor id: " );
  Serial.println( message.sensor );

  if (message.sensor == CHILD_ID_RGB)
    handle_rgb(message);
  else if (message.sensor == CHILD_ID_LIGHTBULBS)
    handle_lightbulbs(message);
}

void handle_rgb(const MyMessage &message)
{
  int val;
  if (message.type == V_RGB){
    Serial.println( "V_RGB command: " );
    Serial.println(message.data);
    long number = (long) strtol( message.data, NULL, 16);
    // Save old value
    strcpy(rgbstring, message.data);
    // Split it up into r, g, b values
    red = number >> 16;
    green = number >> 8 & 0xFF;
    blue = number & 0xFF;

    send_rgb_status();
    set_rgb_status();
  } else if (message.type ==  V_STATUS) {
    Serial.println( "V_STATUS command: " );
    Serial.println(message.data);
    val = atoi(message.data);
    if (val == 0 or val == 1) {
      rgb_on_off_status = val;
      send_rgb_status();
      set_rgb_status();
    }
  } else if (message.type == V_PERCENTAGE){
    Serial.print( "V_PERCENTAGE command: " );
    Serial.println(message.data);
    val = atoi(message.data);
    if (val >= 0 and val <=100) {
      rgb_dimmerlevel = val;
      send_rgb_status();
      set_rgb_status();
    }
  } else if (message.type == V_VAR1) {
    Serial.print( "V_VAR1 command: " );
    Serial.println(message.data);
    val = atoi(message.data);
    if (val >= 0 and val <= 2000) {
      rgb_fadespeed = val;
    }
    send_rgb_status();
  } else {
      Serial.println( "Invalid command received..." );
      return;
  }
}

void set_rgb(int r, int g, int b) {
  analogWrite(RED_PIN, r);
  analogWrite(GREEN_PIN, g);
  analogWrite(BLUE_PIN, b);
}

void set_rgb_status() {
  int r = rgb_on_off_status * (int)(red * rgb_dimmerlevel/100.0);
  int g = rgb_on_off_status * (int)(green * rgb_dimmerlevel/100.0);
  int b = rgb_on_off_status * (int)(blue * rgb_dimmerlevel/100.0);

  if (rgb_fadespeed >0) {

    float dr = (r - r0) / float(rgb_fadespeed);
    float db = (b - b0) / float(rgb_fadespeed);
    float dg = (g - g0) / float(rgb_fadespeed);

    for (int x = 0;  x < rgb_fadespeed; x++) {
      set_rgb(r0 + dr*x, g0 + dg*x, b0 + db*x);
      delay(100);
    }
  }

  set_rgb(r, g, b);
  r0 = r;
  b0 = b;
  g0 = g;

}

void send_rgb_status() {
  send(rgbRgbMsg.set(rgbstring));
  send(rgbLightMsg.set(rgb_on_off_status));
  send(rgbDimmerMsg.set(rgb_dimmerlevel));
  send(rgbFadeMsg.set(rgb_fadespeed));
}

void handle_lightbulbs(const MyMessage &message)
{
  int val;
  // switch (message.type) {
	//  case V_STATUS :
  if (message.type == V_STATUS) {
    Serial.println( "Lightbulbs V_STATUS command: " );
    Serial.println(message.data);
    val = atoi(message.data);
    if (val == 0 or val == 1 ) {
      lb_on_off_status = val;
      set_lb_status();
      send_lb_status();
    }
  } else if (message.type == V_PERCENTAGE){
     Serial.println( "Lightbulbs dimmer V_PERCENTAGE command: " );
     Serial.println(message.data);
     val = atoi(message.data);
     if (val >= 0 and val <=100) {
       lb_dimmerlevel = val;
       set_lb_status();
       send_lb_status();
     }
   } else if (message.type == V_VAR1) {
     Serial.print( "Lightbulbs V_VAR1 command: " );
     Serial.println(message.data);
     val = atoi(message.data);
     if (val >= 0 and val <= 2000) {
       lb_fadespeed = val;
     }
   }
}

void set_lb_status() {
  int lb = lb_on_off_status * (int)(white * lb_dimmerlevel/100.0);

  if (lb_fadespeed >0) {
    float dlb = (lb - w0) / float(lb_fadespeed);

    for (int x = 0;  x < lb_fadespeed; x++) {
      analogWrite(LIGHTBULBS_PIN, w0 + dlb*x);
      delay(100);
    }
  }
  analogWrite(LIGHTBULBS_PIN, lb);
  w0=lb;
}

void send_lb_status() {
  send(lbLightMsg.set(lb_on_off_status));
  send(lbDimmerMsg.set(lb_dimmerlevel));
  send(lbFadeMsg.set(lb_fadespeed));
}
