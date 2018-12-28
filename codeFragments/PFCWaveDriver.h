//
//  PFCWaveDriver.h
//  PiFullColorLED
//
//  Created by illusia decafish on 2018/12/17.
//  Copyright © 2018 decafish. All rights reserved.
//

#ifndef PFCWaveDriver_h
#define PFCWaveDriver_h

//  for full color LED OST4ML5B32A/OST4ML8132A
//  two types of external circuit spported.
//
#include <stdbool.h>
#include <stdint.h>
#include "PFCRGBColor.h"


#define activeHigh  true
#define activeLow   false
typedef struct _pinSetting {
    uint8_t     pinNumber;
    bool        polarity;
} pinSetting;


//  PFCLEDChain can be resued by re-writing colors unless number of LEDs change
typedef struct _PFCLEDChain PFCLEDChain;

//  initially all black (all LEDs are turned off)
//  PFCLEDChain instances can be reused unless LED count or level settings are changed
PFCLEDChain     *createChainAlternativeLogic(size_t     countOfLED,
                                             pinSetting high,
                                             pinSetting low);
PFCLEDChain     *createChainTristateLogic(size_t        countOfLED,
                                          pinSetting    data,
                                          pinSetting    middle);

void            disposeLEDChain(PFCLEDChain *ledChain);
size_t          numberOfLED(PFCLEDChain *ledChain);
bool            setLEDColorToChain(PFCLEDChain  *ledChain,
                                   size_t       indexAt,
                                   PFCRGBColor  *rgb);


//  assuming pigpio library initialised before struct creation
//  PFCLEDChainConverter can be reused unless the sequences are disposed
//  after starting transfer, color settings in PFCLEDChain can be changed for next transfer
typedef struct _PFCLEDChainConverter    PFCLEDChainConverter;

PFCLEDChainConverter    *createLEDChainConverter(uint32_t       pulseWidthInUSec,
                                                 PFCLEDChain    **chains,
                                                 size_t         numberOfChains);
void                    disposeLEDChainConveter(PFCLEDChainConverter *conv);
bool                    transferColors(PFCLEDChainConverter *conv);
bool                    isNowTransferring(PFCLEDChainConverter *conv);

#endif /* PFCWaveDriver_h */
