#include "LUFAConfig.h"
#include <LUFA.h>
#include "XS_HID.h"
#include "PSXPad.h"
#include <EEPROM.h>
#include <inttypes.h>

//delay in ms for start+select to become HOME
#define HOME_DELAY 1000

/* Buttons declarations */
unsigned long startAndSelTime = 0;
unsigned long currTime = 0;

byte internalButtonStatus[4];

/* MODE DECLARATIONS */
typedef enum {
  RIGHT_ANALOG_MODE,
  ANALOG_MODE,
  DIGITAL,
} State_t;
State_t state;

byte lbAttPinNos[] = {2};
PSXPad_KeyState_t tKeyState;

/* mode selectors */
bool xinput;
bool modeChanged;

void checkModeChange(){
    if (buttonStatus[BUTTONSTART] && buttonStatus[BUTTONSELECT])
    {
      if ( !modeChanged )
      {
        bool need_update = true;
        if (internalButtonStatus[BUTTONLEFT])
          state = ANALOG_MODE;
        else if (internalButtonStatus[BUTTONRIGHT])
          state = RIGHT_ANALOG_MODE;
        else if (internalButtonStatus[BUTTONUP])
          state = DIGITAL;
        else need_update = false;
        
        if (need_update) EEPROM.put(0, state);
        modeChanged = true;
      }
    }
    else 
    {
      modeChanged = false;
    }
}

void setup() {
  
  PSXPads.begin(1, lbAttPinNos);
  modeChanged = false;
  EEPROM.get(0, state);
  EEPROM.get(2, xinput);
  delay(500);
// if select is held on boot, NSWitch mode
  PSXPads.pool();
  PSXPads.lpcPads[0]->getKeyState(&tKeyState);
  
  if(tKeyState.bSel)
  {
    xinput = false;
    EEPROM.put(2, xinput);
  }
// if start is held on boot, XInput mode
  else if(tKeyState.bStt)
      {
        xinput = true;
        EEPROM.put(2, xinput);
      }
  
  
  SetupHardware(xinput);
  GlobalInterruptEnable();
}


void loop() {
    currTime = millis();
    buttonRead();
    checkModeChange();    
    convert_dpad();
    send_pad_state();
}

void convert_dpad(){
  
  switch (state)
  {
    case DIGITAL:
    buttonStatus[AXISLX] = 128;
    buttonStatus[AXISLY] = 128;
    buttonStatus[AXISRX] = 128;
    buttonStatus[AXISRY] = 128;
    buttonStatus[BUTTONUP] = internalButtonStatus[BUTTONUP];
    buttonStatus[BUTTONDOWN] = internalButtonStatus[BUTTONDOWN];
    buttonStatus[BUTTONLEFT] = internalButtonStatus[BUTTONLEFT];
    buttonStatus[BUTTONRIGHT] = internalButtonStatus[BUTTONRIGHT];
    break;

    case ANALOG_MODE:   
    buttonStatus[AXISRX] = 128;
    buttonStatus[AXISRY] = 128;
    buttonStatus[BUTTONUP] = 0;
    buttonStatus[BUTTONDOWN] = 0;
    buttonStatus[BUTTONLEFT] = 0;
    buttonStatus[BUTTONRIGHT] = 0;
    
    if ((internalButtonStatus[BUTTONUP]) && (internalButtonStatus[BUTTONRIGHT])){buttonStatus[AXISLY] = 0;buttonStatus[AXISLX] = 255;}
    else if ((internalButtonStatus[BUTTONDOWN]) && (internalButtonStatus[BUTTONRIGHT])) {buttonStatus[AXISLY] = 255;buttonStatus[AXISLX] = 255;}
    else if ((internalButtonStatus[BUTTONDOWN]) && (internalButtonStatus[BUTTONLEFT])) {buttonStatus[AXISLY] = 255;buttonStatus[AXISLX] = 0;}
    else if ((internalButtonStatus[BUTTONUP]) && (internalButtonStatus[BUTTONLEFT])){buttonStatus[AXISLY] = 0;buttonStatus[AXISLX] = 0;}
    else if (internalButtonStatus[BUTTONUP]) {buttonStatus[AXISLY] = 0;buttonStatus[AXISLX] = 128;}
    else if (internalButtonStatus[BUTTONDOWN]) {buttonStatus[AXISLY] = 255;buttonStatus[AXISLX] = 128;}
    else if (internalButtonStatus[BUTTONLEFT]) {buttonStatus[AXISLX] = 0;buttonStatus[AXISLY] = 128;}
    else if (internalButtonStatus[BUTTONRIGHT]) {buttonStatus[AXISLX] = 255;buttonStatus[AXISLY] = 128;}
    else {buttonStatus[AXISLX] = 128;buttonStatus[AXISLY] = 128;}

    break;
    
    case RIGHT_ANALOG_MODE:   
    buttonStatus[AXISLX] = 128;
    buttonStatus[AXISLY] = 128;
    buttonStatus[BUTTONUP] = 0;
    buttonStatus[BUTTONDOWN] = 0;
    buttonStatus[BUTTONLEFT] = 0;
    buttonStatus[BUTTONRIGHT] = 0;
    
    if ((internalButtonStatus[BUTTONUP]) && (internalButtonStatus[BUTTONRIGHT])){buttonStatus[AXISRY] = 0;buttonStatus[AXISRX] = 255;}
    else if ((internalButtonStatus[BUTTONUP]) && (internalButtonStatus[BUTTONLEFT])){buttonStatus[AXISRY] = 0;buttonStatus[AXISRX] = 0;}
    else if ((internalButtonStatus[BUTTONDOWN]) && (internalButtonStatus[BUTTONRIGHT])) {buttonStatus[AXISRY] = 255;buttonStatus[AXISRX] = 255;}
    else if ((internalButtonStatus[BUTTONDOWN]) && (internalButtonStatus[BUTTONLEFT])) {buttonStatus[AXISRY] = 255;buttonStatus[AXISRX] = 0;}
    else if (internalButtonStatus[BUTTONUP]) {buttonStatus[AXISRY] = 0;buttonStatus[AXISRX] = 128;}
    else if (internalButtonStatus[BUTTONDOWN]) {buttonStatus[AXISRY] = 255;buttonStatus[AXISRX] = 128;}
    else if (internalButtonStatus[BUTTONLEFT]) {buttonStatus[AXISRX] = 0;buttonStatus[AXISRY] = 128;}
    else if (internalButtonStatus[BUTTONRIGHT]) {buttonStatus[AXISRX] = 255;buttonStatus[AXISRY] = 128;}
    else {buttonStatus[AXISRX] = 128;buttonStatus[AXISRY] = 128;}

    break;
  }
}

void buttonRead()
{    
  PSXPads.pool();
  PSXPads.lpcPads[0]->getKeyState(&tKeyState);
  buttonStatus[BUTTONSELECT] = tKeyState.bSel;
  buttonStatus[BUTTONL3] = tKeyState.bL3;
  buttonStatus[BUTTONR3] = tKeyState.bR3;
  buttonStatus[BUTTONSTART] = tKeyState.bStt;
  internalButtonStatus[BUTTONUP] = tKeyState.bU;
  internalButtonStatus[BUTTONRIGHT] = tKeyState.bR;
  internalButtonStatus[BUTTONDOWN] = tKeyState.bD;
  internalButtonStatus[BUTTONLEFT] = tKeyState.bL;
  buttonStatus[BUTTONLT] = tKeyState.bL2;
  buttonStatus[BUTTONRT] = tKeyState.bR2;
  buttonStatus[BUTTONLB] = tKeyState.bL1;
  buttonStatus[BUTTONRB] = tKeyState.bR1;
  buttonStatus[BUTTONX] = tKeyState.bTri;
  buttonStatus[BUTTONA] = tKeyState.bCir;
  buttonStatus[BUTTONY] = tKeyState.bSqr;
  buttonStatus[BUTTONB] = tKeyState.bCrs;

#define HOME_HOTKEY
#ifdef HOME_HOTKEY  
  if(buttonStatus[BUTTONSTART] && buttonStatus[BUTTONSELECT]) {
   if (startAndSelTime == 0)
    startAndSelTime = millis();
   else if (currTime - startAndSelTime > HOME_DELAY)
   {
      buttonStatus[BUTTONHOME] = 1;
   }
 } else {
  startAndSelTime = 0;
  buttonStatus[BUTTONHOME] = 0;
 }
#endif
  
}
