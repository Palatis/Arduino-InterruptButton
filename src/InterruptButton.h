/*
  InterruptButton.h - Interrupt based single/double/longpress button handler
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

#ifndef _INTERRUPT_BUTTON_H_
#define _INTERRUPT_BUTTON_H_

namespace Palatis {

#ifndef ICACHE_RAM_ATTR
#define ICACHE_RAM_ATTR
#endif

typedef void (*interrupt_button_cb_t)(void);

template <
  uint32_t DEBOUNCE_TIMEOUT = 20, // interrupt between 20ms to be considered a bounce
  uint32_t DOUBLE_TIMEOUT = 200, // clicks between 200ms to be considered a double click
  uint32_t LONGPRESS_TIMEOUT = 1000 // key held longer than 1000ms to be considered a long press
  > class InterruptButton {
  public:
    /**
       @param pin
          pin for the button
       @param level
          active level for the pin. LOW means pressing the button would pull the pin
          LOW, HIGH means pressing the button would pull the pin HIGH, respectively.
    */
    InterruptButton(uint8_t pin, uint8_t level = LOW):
      _pin(pin), _level(level),
      _up_millis_0(0), _up_millis_1(0), _down_millis(0),
      _processed(true),
      _single_cb(NULL), _double_cb(NULL), _long_cb(NULL)
    {
    }

    bool begin(void (*isr_func)(void)) {
      uint8_t interrupt = digitalPinToInterrupt(_pin);
      if (interrupt == NOT_AN_INTERRUPT)
        return false;

      pinMode(_pin, INPUT_PULLUP);
      attachInterrupt(interrupt, isr_func, CHANGE);

      return true;
    }
    
    void end() {
      uint8_t interrupt = digitalPinToInterrupt(_pin);
      if (interrupt == NOT_AN_INTERRUPT)
        return;
      detachInterrupt(interrupt);
      _processed = true;
    }

    void ICACHE_RAM_ATTR handleInterrupt() {
      uint32_t now = millis();
      uint8_t state = digitalRead(_pin);

      if (now - _down_millis < DEBOUNCE_TIMEOUT)
        return;

      if (state == _level) // down
      {
        _down_millis = now;
      }
      else // up
      {
        _up_millis_0 = _up_millis_1;
        _up_millis_1 = now;
      }
      _processed = false;
    }

    /**
     *  @return true if the button has to be processed again in a near future,
     *          false otherwise.
     *          when this function returns false, you can safely put the MCU to
     *          sleep without breaking functionality.
     *          however you should still let it to wake up by the pin interrupt.
     */
    bool tick() {
      // the event has been processed
      if (_processed)
        return false;
      
      // we're probably from a long-press reboot
      if (_down_millis == 0)
        return true;

      uint32_t now = millis();

      // haven't got enough time to confirm an action
      if (now - _up_millis_1 < DOUBLE_TIMEOUT)
        return true;

      // key is held for a long time, and not released yet
      if (_up_millis_1 - _down_millis >= 0x80000000) // unsigned negative value
      {
        if (now - _down_millis > LONGPRESS_TIMEOUT)
        {
          _processed = true;
          if (_long_cb != NULL)
            _long_cb();
          return true;
        }
        return false;
      }

      _processed = true;

      // it was a long press, should have been processed.
      if (_up_millis_1 - _down_millis > LONGPRESS_TIMEOUT)
        return false;

      // two clicks within a short time, should be double click
      if (_down_millis - _up_millis_0 < DOUBLE_TIMEOUT) {
        if (_double_cb != NULL)
          _double_cb();
        return false;
      }

      // else it's a short click
      if (_single_cb != NULL)
        _single_cb();
      return false;
    }

    void attachOnSingleCallback(interrupt_button_cb_t callback) {
      _single_cb = callback;
    }

    void attachOnDoubleCallback(interrupt_button_cb_t callback) {
      _double_cb = callback;
    }

    void attachOnLongPressCallback(interrupt_button_cb_t callback) {
      _long_cb = callback;
    }

  private:
    uint8_t const _pin;
    uint8_t const _level;

    volatile uint32_t mutable _up_millis_0;
    volatile uint32_t mutable _up_millis_1;
    volatile uint32_t mutable _down_millis;

    volatile bool _processed;

    interrupt_button_cb_t _single_cb;
    interrupt_button_cb_t _double_cb;
    interrupt_button_cb_t _long_cb;
};

};

#endif