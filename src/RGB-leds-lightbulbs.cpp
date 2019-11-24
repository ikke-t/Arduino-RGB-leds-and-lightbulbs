/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * RGB-leds-lightbulbs.ino commands RGB-led strip and one chain of lights bulbs.
 *
 * Modified from https://forum.mysensors.org/topic/6765/rgb-led-strip
 *
 * The RGB converter needed at OpenHAB in rgb.rules in same directory.
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
 *
 * Author: Ilkka Tengvall <ilkka.tengvall@iki.fi> 2018
 * Original Author Credits to: @maghac at forum.mysensors.org
 */

// Prepare radio with NRF24L01+
#define MY_RADIO_RF24
#define MY_BAUD_RATE   9600

#define MY_NODE_ID     5
#define MY_RF24_CE_PIN 7
#define MY_RF24_CS_PIN 8

#define CHILD_ID_RGB        1
#define CHILD_ID_LIGHTBULBS 2

#include <Arduino.h>
#include <SPI.h>
#include <MySensors.h>

MyMessage rgbLightMsg(CHILD_ID_RGB, V_STATUS);
MyMessage rgbRgbMsg(CHILD_ID_RGB, V_RGB);
MyMessage rgbDimmerMsg(CHILD_ID_RGB, V_PERCENTAGE);
MyMessage rgbFadeMsg(CHILD_ID_RGB, V_VAR1);
MyMessage rgbProgramMsg(CHILD_ID_RGB, V_VAR2);

MyMessage lbLightMsg(CHILD_ID_LIGHTBULBS, V_STATUS);
MyMessage lbDimmerMsg(CHILD_ID_LIGHTBULBS, V_PERCENTAGE);
MyMessage lbFadeMsg(CHILD_ID_LIGHTBULBS, V_VAR1);

#define RR 0
#define GG 1
#define BB 2

byte current[] = {255, 255, 255};
byte target[] = {255, 255, 255};
byte save[] = {0, 0, 0};
byte temp[] = {0, 0, 0};

float delta[] = {0.0, 0.0, 0.0};

byte white = 255;
byte w0 = 255;
char rgbstring[] = "ffffff";

int lb_on_off_status = 1;
int lb_dimmerlevel = 100;
int lb_fadespeed = 40;

int rgb_on_off_status = 1;
int rgb_dimmerlevel = 100;
int rgb_prgspeed = 20;

unsigned long rgb_last_update = 0;
const unsigned long rgb_tick_length = 5;
int rgb_fade_step = 0;

int rgb_program_timer;
int rgb_program_cycle;
int rgb_program_step;

// RGB strip
#define RED_PIN   5
#define GREEN_PIN 6
#define BLUE_PIN  9

// Ball lights control pin
#define LIGHTBULBS_PIN 10

#define LIGHT_NORMAL 0
#define LIGHT_FADING 1

#define PROGRAM_NOP 0

int light_mode = LIGHT_NORMAL;
int program_mode = PROGRAM_NOP;

#define SET 0
#define SET_AND_WAIT 1
#define SET_RANDOM 2
#define SET_RANDOM_AND_WAIT 3
#define FADE 4
#define FADE_RANDOM 5
#define WAIT 6

typedef struct rgb_cmd {
  byte cmd;
  int p;
  byte rgb[3];
} rgb_cmd;

rgb_cmd program_ALARM[] = {
  {SET_AND_WAIT, 25, {255, 255, 255}},
  {SET_AND_WAIT, 25, {0, 0, 0}},
  {SET_AND_WAIT, 25, {0, 0, 0}},
  {SET_AND_WAIT, 25, {0, 0, 0}}
};

rgb_cmd program_RELAX[] = {
  {FADE, 1000, {255, 32, 0}},
  {FADE, 1000, {255, 32, 16}},
  {FADE, 1000, {255, 16, 32}},
  {FADE, 1000, {255, 128, 0}},
  {FADE, 1000, {255, 32, 0}},
  {FADE, 1000, {255, 32, 32}},
  {FADE, 1000, {255, 0, 32}}
};

rgb_cmd program_PARTY[] = {
  {SET_AND_WAIT, 10, {255, 0, 0}},
  {SET_AND_WAIT, 10, {0, 0, 0}},
  {SET_AND_WAIT, 10, {255, 0, 0}},
  {SET_AND_WAIT, 10, {0, 0, 0}},
  {SET_AND_WAIT, 10, {255, 0, 0}},
  {SET_AND_WAIT, 10, {0, 0, 0}},
  {SET_AND_WAIT, 10, {255, 0, 0}},
  {SET_AND_WAIT, 10, {0, 0, 0}},
  {SET_AND_WAIT, 10, {255, 0, 0}},
  {SET_AND_WAIT, 10, {0, 0, 0}},
  {FADE_RANDOM, 50, {255, 255, 255}},
  {FADE_RANDOM, 50, {255, 255, 255}},
  {FADE_RANDOM, 50, {255, 255, 255}},
  {FADE_RANDOM, 50, {255, 255, 255}},
  {SET_AND_WAIT, 50, {0, 0, 255}},
  {SET_AND_WAIT, 50, {0, 255, 255}},
  {SET_AND_WAIT, 50, {255, 255, 0}},
  {SET_AND_WAIT, 50, {0, 255, 0}},
  {FADE_RANDOM, 50, {255, 255, 255}},
  {FADE_RANDOM, 50, {255, 255, 255}},
  {FADE_RANDOM, 50, {255, 255, 255}},
  {FADE_RANDOM, 50, {255, 255, 255}},
  {FADE_RANDOM, 50, {255, 255, 255}}
};

rgb_cmd* programs[] = {
  &program_ALARM[0], &program_RELAX[0], &program_PARTY[0]
};

const int rgb_program_steps[] = {
  sizeof(program_ALARM)/sizeof(rgb_cmd),
  7,
  22
};

void selftest();
void set_rgb_status();
void set_lb_status();
void send_lb_status();
void send_rgb_status();
void calc_fade();
void handle_program();
void handle_rgb(const MyMessage &);
void handle_lightbulbs(const MyMessage &);
byte hextoint (byte);
void init_fade(int, byte rgb[]);
void stop_program();
void save_state();
void restore_state();
void init_program(int);
void set_rgb (byte rgb[]);
void set_rgb_random (byte rgb[]);
void init_fade_random(int, byte rgb[]);

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

  selftest();
}

void presentation()
{
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Alli's light controller", "1.2");
  // Register all sensors to gw (they will be created as child devices)
  present(CHILD_ID_RGB, S_RGB_LIGHT);
  present(CHILD_ID_LIGHTBULBS, S_DIMMER);
}

void selftest() {
  rgb_on_off_status = 1;
  current[RR] = 255;
  current[GG] = 0;
  current[BB] = 0;
  set_rgb_status();
  analogWrite(LIGHTBULBS_PIN, 255);
  wait(200);
  current[RR] = 0;
  current[GG] = 255;
  set_rgb_status();
  wait(200);
  current[GG] = 0;
  current[BB] = 255;
  set_rgb_status();
  wait(200);
  current[BB] = 0;
  set_rgb_status();
  wait(200);
  current[RR] = 0;
  current[GG] = 0;
  current[BB] = 0;
  set_rgb_status();
  wait(200);
  rgb_on_off_status = 0;
  analogWrite(LIGHTBULBS_PIN, 0);
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

  unsigned long now = millis();
  // Maybe we wrapped around? Then reset rgb_last_update to 0.
  if (now < rgb_last_update) {
    rgb_last_update = 0;
  }

  if (now - rgb_last_update > rgb_tick_length) {
    rgb_last_update = now;

    // If we're fading, finish that before we do anything else
    if (light_mode == LIGHT_FADING) {
       calc_fade();
    } else {
      if (program_mode > PROGRAM_NOP) {
        handle_program();
      }
    }
  }

  set_rgb_status();
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
    // Save old value
    strcpy(rgbstring, message.data);

    for (int i=0; i<=3; i++) {
      temp[i] = hextoint(message.data[i*2]) * 16 +
                hextoint(message.data[i*2+1]);
    }

    init_fade(rgb_prgspeed, temp);
  } else if (message.type ==  V_STATUS) {
    Serial.println( "V_STATUS command: " );
    Serial.println(message.data);
    val = atoi(message.data);
    if (val == 0 or val == 1) {
      rgb_on_off_status = val;
    }
  } else if (message.type == V_PERCENTAGE){
    Serial.print( "V_PERCENTAGE command: " );
    Serial.println(message.data);
    val = atoi(message.data);
    if (val >= 0 and val <=100) {
      rgb_dimmerlevel = val;
    }
  } else if (message.type == V_VAR1) {
    Serial.print( "V_VAR1 command: " );
    Serial.println(message.data);
    val = atoi(message.data);
    if (val >= 0 and val <= 2000) {
      rgb_prgspeed = val;
    }
  } else if (message.type == V_VAR2 ) {
    val = atoi(message.data);
    if (val == PROGRAM_NOP) {
      stop_program();
    } else {
      init_program(val);
    }
  } else {
      Serial.println( "Invalid command received..." );
      return;
  }
  send_rgb_status();
}

void send_rgb_status() {
  send(rgbRgbMsg.set(rgbstring));
  send(rgbLightMsg.set(rgb_on_off_status));
  send(rgbDimmerMsg.set(rgb_dimmerlevel));
  send(rgbFadeMsg.set(rgb_prgspeed));
  send(rgbProgramMsg.set(rgb_prgspeed));
}

void execute_step(rgb_cmd cmd) {

  if (cmd.cmd == SET) {
    set_rgb(cmd.rgb);
  } else if (cmd.cmd == SET_AND_WAIT) {
    set_rgb(cmd.rgb);
    rgb_program_timer = cmd.p;
  } else if (cmd.cmd == SET_RANDOM) {
    set_rgb_random(cmd.rgb);
  } else if (cmd.cmd == SET_RANDOM_AND_WAIT) {
    set_rgb_random(cmd.rgb);
    rgb_program_timer = cmd.p;
  } else if (cmd.cmd == FADE) {
    init_fade(cmd.p, cmd.rgb);
  } else if (cmd.cmd == FADE_RANDOM) {
    init_fade_random(cmd.p, cmd.rgb);
  } else if (cmd.cmd == WAIT) {
    rgb_program_timer = cmd.p;
  }
}

void init_program(int program) {
  program_mode = program;
  rgb_program_step = 0;
  rgb_program_timer = 0;
  save_state();
  execute_step(programs[program_mode-1][0]);
}

void handle_program() {
  if (rgb_program_timer > 0) {
    rgb_program_timer--;
  }

  if (rgb_program_timer == 0) {
   rgb_program_step++;
    if (rgb_program_step == rgb_program_steps[program_mode-1]) {
     rgb_program_step = 0;
    }
    execute_step(programs[program_mode-1][rgb_program_step]);
  }
}

void stop_program() {
  restore_state();
  light_mode = LIGHT_NORMAL;
  program_mode = PROGRAM_NOP;
}

void save_state() {
  memcpy(save, current, 3 );
}

void restore_state() {
  memcpy(current, save, 3 );
}

void set_rgb (byte rgb[]) {
  light_mode = LIGHT_NORMAL;
  memcpy(current, rgb, 3);
}

void set_rgb_random (byte rgb[]) {
  light_mode = LIGHT_NORMAL;
  for (int i=0; i <= 2; i++){
    current[i] = random(rgb[i]);
  }
}

void init_fade(int t, byte rgb[]) {
  light_mode = LIGHT_FADING;
  rgb_fade_step = t;
  memcpy(target, rgb, 3);
  for (int i=0; i<=2; i++) {
    delta[i] = (target[i] - current[i]) / float(rgb_fade_step);
  }
}

void init_fade_random(int t, byte rgb[]) {
  light_mode = LIGHT_FADING;
  rgb_fade_step = t;
  for (int i=0; i<=2; i++) {
    target[i] = random(rgb[i]);
    delta[i] = (target[i] - current[i]) / float(rgb_fade_step);
  }
}

void calc_fade() {
  if (rgb_fade_step > 0) {
    rgb_fade_step--;
    for (int i=0; i<=2; i++) {
     current[i] = target[i] - delta[i] * rgb_fade_step;
    }
  } else {
    light_mode = LIGHT_NORMAL;
  }
}

void set_rgb_status() {
  analogWrite(RED_PIN,
    rgb_on_off_status * (int)(current[RR] * rgb_dimmerlevel/100.0));
  analogWrite(GREEN_PIN,
    rgb_on_off_status * (int)(current[GG] * rgb_dimmerlevel/100.0));
  analogWrite(BLUE_PIN,
    rgb_on_off_status * (int)(current[BB] * rgb_dimmerlevel/100.0));
}

byte hextoint (byte c) {
   if ((c >= '0') && (c <= '9')) return c - '0';
   if ((c >= 'A') && (c <= 'F')) return c - 'A' + 10;
   if ((c >= 'a') && (c <= 'f')) return c - 'a' + 10;
   return 0;
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
