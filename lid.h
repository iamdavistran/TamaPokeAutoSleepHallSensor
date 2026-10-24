#pragma once
#include <stdint.h>

// Lid sensor: the ball sleeps when it is closed and wakes when it is opened.
//
// Hardware: an Adafruit ScoutMakes DRV5032 Hall breakout (DRV5032FB, #6051) in the
// lid tray, wired over the hinge to the 8-pin expansion header: VIN to 3V3, GND to
// GND, OUT to the pin marked GPIO16 (go by the board's silkscreen, not a pin count).
// A 6x3 mm magnet in the screen tray's deck sits under it when the ball is shut.
// The header's 3V3 is the AXP2101's DCDC1 (the chip's own supply), so the sensor
// stays powered through deep sleep; pmuDisablePanel() only cuts the panel's BLDO1.
// The FB is push-pull and omnipolar: it drives OUT low whenever either pole is
// near, high otherwise, sampling at ~5 Hz on ~0.7 uA. So:
//   no magnet   (lid up)    -> HIGH
//   magnet near (lid down)  -> LOW
//
// The internal pull-up is ON while awake and OFF in deep sleep. Awake, it is what
// keeps a board with NO sensor fitted reading "open" -- a floating pin would
// read low at random and put the ball to sleep -- and its current is nothing
// next to the panel. Asleep, the sensor drives the pin itself, and a pull-up
// fighting its low output would waste ~70 uA for the whole time the ball is
// shut: a hundred times what the sensor draws. A plain normally-open reed
// switch to GND still works on awake, but would need the pull-up kept on asleep
// (LID_SLEEP_PULLUP 1 in pin_config.h).

// Kept apart from the sketch the same way linknow.cpp is: everything here
// touches ESP-IDF sleep and RTC-GPIO APIs, and the emulator stubs it in
// tools/emu/host_impl.cpp.

void lidBegin();      // first thing in setup(): undoes the deep-sleep pin setup
bool lidClosed();     // the switch reads closed right now (no debouncing)
bool lidWokeUs();     // this boot is the lid being opened
// Amplifier held off, then deep sleep until the lid opens. The caller saves
// and powers down the panel and touch first. Never returns: opening the lid is
// a fresh boot through setup(), and syncClock() applies the time it was shut.
void lidSleepNow();
