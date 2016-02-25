/*
  InterruptButton.ino - Example for the Palatis::InterruptButton class
  Copyright (c) 2016 Victor Tseng <palatis@gmail.com>
  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <InterruptButton.h>

Palatis::InterruptButton<> button(D3, LOW); // NodeMCU onboard FLASH key

void button_isr() {
  button.handleInterrupt();
}

void button_single() {
  Serial.println("Single");
}

void button_double() {
  Serial.println("Double");
}

void button_longpress() {
  Serial.println("Long Press");
}

void setup() {
  Serial.begin(115200);

  // attach the callbacks
  button.attachOnSingleCallback(button_single);
  button.attachOnDoubleCallback(button_double);
  button.attachOnLongPressCallback(button_longpress);

  // begin the button
  button.begin(button_isr);
}

void loop() {
  button.tick();
}
