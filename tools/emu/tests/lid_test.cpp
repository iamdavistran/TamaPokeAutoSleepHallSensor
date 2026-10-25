// The lid: closing the ball deep-sleeps it, opening it is a fresh boot.
//
// What is pinned here is the behaviour around the sleep, since the sleep itself
// (ESP-IDF ext0 on GPIO16) only exists on the board:
//
//   * no reed fitted (pin reads open) never sleeps -- stock behaviour
//   * a short close, a peek or the reed bouncing as the halves meet, does not
//   * a real close sleeps exactly once, with the clock saved for syncClock()
//   * opening applies the time it was shut, like any power-off
//   * closing at night puts the creature to bed as PWR does, and opening wakes
//     it -- but one the player sent to bed with the light STAYS asleep, because
//     setScreenOff() is the one place that rule lives and the lid goes through it
#include "Arduino.h"
#include "Arduino_GFX_Library.h"
#include "Preferences.h"
#include "pet.h"
#include "party.h"
#include <cstdio>
#include <chrono>
#include <thread>
uint32_t g_seed=1234; FakeSerial Serial; FakeESP ESP; FakeWire Wire;
volatile int g_touchX=0,g_touchY=0; volatile bool g_touchDown=false;
void FakeESP::restart(){exit(0);}
int FakeSerial::available(){return 0;}
String FakeSerial::readStringUntil(char){return String("");}
void setup(); void loop();
extern Pet pet;
extern Arduino_CO5300 *panel;
extern bool g_lidClosed, g_lidWoke;
extern int g_lidSleeps;
extern bool screenOff;
uint32_t rtcEpoch(); void rtcSetEpoch(uint32_t e);

static int bad=0;
static void ck(bool ok,const char*w){printf("%s  %s\n",ok?"PASS":"FAIL",w); if(!ok)bad++;}

// the lid is debounced off millis(), so real time has to pass
static void pumpMs(int ms){
  for(int t=0;t<ms;t+=30){ std::this_thread::sleep_for(std::chrono::milliseconds(30)); loop(); }
}
static uint32_t savedSeen(){ Preferences p; p.begin("tamapoke", true); return p.getUInt("seen", 0); }

#define DAY0 1767225600UL   // 2026-01-01 00:00, as the RTC counts it
#define HOUR 3600UL

// the board opening: wake cause is the lid, and setup() runs from the top
static void openLid(uint32_t epoch){
  g_lidClosed=false; g_lidWoke=true;
  rtcSetEpoch(epoch);
  setup();
  g_lidWoke=false;
}

int main(){
  rtcSetEpoch(DAY0 + 12*HOUR);
  setup();
  if (pet.awaitingStarter()) pet.chooseStarter(4);
  if (pet.isEgg()) pet.dbgHatchAs(4,false);
  pet.fullness=pet.joy=pet.energy=pet.hygiene=90;
  pet.saveNow();

  pumpMs(2400);
  ck(g_lidSleeps==0, "no reed fitted: the pin reads open and it never sleeps");

  g_lidClosed=true;  pumpMs(600);
  g_lidClosed=false; pumpMs(1500);
  ck(g_lidSleeps==0, "a short close does not sleep, and the wait restarts after it");

  // a peek with the panel off (PWR) never reaches the sleep, so nothing reboots
  // the screen: opening has to light it itself, or it stays dark
  screenOff=true; pet.setScreenOff(true); pumpMs(300);
  g_lidClosed=true;  pumpMs(600);
  ck(screenOff, "(the panel is still off while the lid is shut)");
  g_lidClosed=false; pumpMs(300);
  ck(!screenOff && g_lidSleeps==0, "opening after a peek lights a panel PWR had turned off");

  uint32_t age0 = pet.ageMinutes;
  g_lidClosed=true; pumpMs(2100);
  ck(g_lidSleeps==1, "a held close sleeps, once");
  ck(!panel->on, "the panel is switched off before sleeping");
  uint32_t seen = savedSeen();
  ck(seen + 5 >= rtcEpoch() && seen <= rtcEpoch(), "the clock is saved at the moment of closing");
  ck(!pet.sleeping, "closed at noon: the creature is not sent to bed");

  openLid(rtcEpoch() + 3*HOUR);
  ck(pet.ageMinutes >= age0 + 179, "opening applies the hours it was shut");
  ck(pet.fullness < 90, "and its needs drained while it was");

  // night: the lid is the device being put down, so it does what PWR does
  rtcSetEpoch(DAY0 + 25*HOUR);          // 01:00 the next day
  g_lidClosed=true; pumpMs(2100);
  ck(g_lidSleeps==2, "closed at night: sleeps");
  ck(pet.sleeping && pet.sleepAuto==SLEEP_AUTO, "and the creature went to bed with it");

  openLid(DAY0 + 31*HOUR);              // 07:00
  ck(!pet.sleeping, "opening the lid wakes a creature the dark put to bed");

  // one the player put to bed with the light stays there
  rtcSetEpoch(DAY0 + 50*HOUR);          // 02:00 the day after
  pumpMs(100);
  pet.lastSeenEpoch = rtcEpoch();
  if (!pet.sleeping) pet.toggleLight();
  ck(pet.sleeping && pet.sleepAuto==SLEEP_PLAYER, "put to bed by the player");
  g_lidClosed=true; pumpMs(2100);
  ck(g_lidSleeps==3, "closed again: sleeps");
  openLid(DAY0 + 52*HOUR);
  ck(pet.sleeping, "opening does NOT wake a creature the player sent to bed");

  printf(bad?"\n%d FAILED\n":"\nall passed\n", bad);
  return bad?1:0;
}
