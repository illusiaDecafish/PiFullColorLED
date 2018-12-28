//
//  PFCSPIDriver.h
//  PiFullColorLED
//
//  Created by illusia decafish on 2018/12/17.
//  Copyright © 2018 decafish. All rights reserved.
//

#ifndef PFCSPIDriver_h
#define PFCSPIDriver_h


//  Raspberry pi driver for full color LED PL9823-F5/PL9823-F8
//

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "PFCRGBColor.h"


typedef int PFCSpiDriver;

typedef struct _SpiSequence  SpiSequence;

#define activeHigh  true
#define activeLow   false

//  assuming pigpio library already initialised
//  chip select signal will not be produced if chipSelect < 0
//  chip select logic will be active high if csPolarity true
PFCSpiDriver    openSpiForPFCDriver(int spiBusNumber, int chipSelect, bool csPolarity);
void            disposePFCSpiDriver(PFCSpiDriver driver);

SpiSequence     *createHollowSpiSequence(size_t countOfLED);
void            disposeSpiSequence(SpiSequence *bitSeq);
size_t          numberOfLEDInSpiSequence(SpiSequence *bitSeq);
bool            setLEDColorToSpiSequence(SpiSequence *bitSeq, size_t indexAt, PFCRGBColor *rgb);

bool            transferSpiSequence(PFCSpiDriver driver, SpiSequence *bytes);
bool            transferSpiAllOff(PFCSpiDriver driver, size_t countOfLED, bool polarity);

#endif /* PFCSPIDriver_h */
