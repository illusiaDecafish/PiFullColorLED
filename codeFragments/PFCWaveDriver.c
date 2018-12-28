//
//  PFCWaveDriver.c
//  PiFullColorLED
//
//  Created by illusia decafish on 2018/12/17.
//  Copyright © 2018 decafish. All rights reserved.
//

#include <stdlib.h>
#include <pigpio.h>
#include "PFCWaveDriver.h"
#include "PFCLastError.h"

PFCRGBColor  lightsOutColor  = {0x00, 0x00, 0x00};
/*
static const int    rPos = 2;
static const int    gPos = 1;
static const int    bPos = 0;
*/

#define rPos    2
#define gPos    1
#define bPos    0

typedef struct _bitPattern {
    uint32_t    onPat;
    uint32_t    offPat;
} bitPattern;

struct _PFCLEDChain {
    uint8_t         occupiedPins[2];
    bitPattern      transitToHigh;
    bitPattern      transitToLow;
    bitPattern      transitToMiddle;
    size_t          ledCount;
    PFCRGBColor     colors[];
};
static void     lightsOut(PFCLEDChain *chain);
static uint8_t  componentValueAt(PFCRGBColor *color, size_t cPos);
static bool     isBitOn(PFCLEDChain *chain, ssize_t ledIndex, size_t component, int32_t bitpos);
static bool     isValidIndex(PFCLEDChain *chain, ssize_t ledIndex);

static PFCLEDChain  *createHollowChain(size_t countOfLED);
static void         setTransitionsFromAlternative(PFCLEDChain *chain, pinSetting high, pinSetting low);
static void         setTransitionsFromTristate(PFCLEDChain *chain, pinSetting data, pinSetting middle);

static bool         setPinsToWrite(PFCLEDChainConverter *conv);
static bool         resetPinsFromWrite(PFCLEDChainConverter *conv);
static int          createWaveAtLEDIndex(PFCLEDChainConverter *conv, size_t index);

static bool         setLatchLevel(PFCLEDChainConverter *conv);
static void         buildMiddlePulse(PFCLEDChainConverter *conv);


PFCLEDChain     *createChainAlternativeLogic(size_t countOfLED, pinSetting high, pinSetting low)
{
    PFCLEDChain *chain = createHollowChain(countOfLED);
    chain->occupiedPins[0] = high.pinNumber;
    chain->occupiedPins[1] = low.pinNumber;
    setTransitionsFromAlternative(chain, high, low);
    return chain;
}

PFCLEDChain     *createChainTristateLogic(size_t countOfLED, pinSetting data, pinSetting middle)
{
    PFCLEDChain *chain = createHollowChain(countOfLED);
    chain->occupiedPins[0] = data.pinNumber;
    chain->occupiedPins[1] = middle.pinNumber;
    setTransitionsFromTristate(chain, data, middle);
    return chain;
}

void    disposeLEDChain(PFCLEDChain *ledChain)
{
    free(ledChain);
}

size_t  numberOfLED(PFCLEDChain *ledChain)
{
    return ledChain->ledCount;
}

bool    setLEDColorToChain(PFCLEDChain *ledChain, size_t indexAt, PFCRGBColor *rgb)
{
    if (indexAt >= ledChain->ledCount)
        return false;
    
    ledChain->colors[indexAt] = *rgb;
    return true;
}

static bool     isValidIndex(PFCLEDChain *chain, ssize_t ledIndex)
{
    return ((ledIndex >= 0) && (ledIndex < chain->ledCount));
}

static void lightsOut(PFCLEDChain *chain)
{
    for (size_t i = 0 ; i < chain->ledCount ; i ++)
        chain->colors[i] = lightsOutColor;
}


static PFCLEDChain  *createHollowChain(size_t countOfLED)
{
    PFCLEDChain *chain = (PFCLEDChain *)malloc(sizeof(PFCLEDChain) + countOfLED * sizeof(PFCRGBColor));
    chain->ledCount = countOfLED;
    lightsOut(chain);
    return chain;
}

static void setTransitionsFromAlternative(PFCLEDChain *chain, pinSetting high, pinSetting low)
{
    bitPattern  nullbp = {0, 0};
    chain->transitToHigh = nullbp;
    chain->transitToLow = nullbp;
    chain->transitToMiddle = nullbp;
    
    uint32_t    hipat = 1 << high.pinNumber;
    uint32_t    lopat = 1 << low.pinNumber;
    
    if (high.polarity) {
        chain->transitToHigh.onPat |= hipat;
        chain->transitToMiddle.offPat |= hipat;
        chain->transitToLow.offPat |= hipat;
    }
    else {
        chain->transitToHigh.offPat |= hipat;
        chain->transitToMiddle.onPat |= hipat;
        chain->transitToLow.onPat |= hipat;
    }
    if (low.polarity) {
        chain->transitToHigh.offPat |= lopat;
        chain->transitToMiddle.offPat |= lopat;
        chain->transitToLow.onPat |= lopat;
    }
    else {
        chain->transitToHigh.onPat |= lopat;
        chain->transitToMiddle.onPat |= lopat;
        chain->transitToLow.offPat |= lopat;
    }
}

static void     setTransitionsFromTristate(PFCLEDChain *chain, pinSetting data, pinSetting middle)
{
    bitPattern  nullbp = {0, 0};
    chain->transitToHigh = nullbp;
    chain->transitToLow = nullbp;
    chain->transitToMiddle = nullbp;
    
    uint32_t    dtapat = 1 << data.pinNumber;
    uint32_t    midpat = 1 << middle.pinNumber;
    if (data.polarity) {
        chain->transitToHigh.onPat |= dtapat;
        chain->transitToLow.offPat |= dtapat;
    }
    else {
        chain->transitToHigh.offPat |= dtapat;
        chain->transitToLow.onPat |= dtapat;
    }
    if (middle.polarity) {
        chain->transitToHigh.offPat |= midpat;
        chain->transitToMiddle.onPat |= midpat;
        chain->transitToLow.offPat |= midpat;
    }
    else {
        chain->transitToHigh.onPat |= midpat;
        chain->transitToMiddle.offPat |= midpat;
        chain->transitToLow.onPat |= midpat;
    }
}




#pragma mark    LEDSeqenceConverter

static const size_t     pulseCountForOneLED     = 16 * 3;   //  8bits x 2 states x 3 colors
static const uint32_t   latchDurationInMicroSec = 3000;     //  3msec as latch time of LED

struct _PFCLEDChainConverter {
    uint32_t    pwidth;
    PFCLEDChain **chains;
    size_t      seqCount;
    char        *wids;
    uint32_t    widCount;
    uint32_t    inactivePat;
    uint32_t    endTick;
    gpioPulse_t middle;
    gpioPulse_t *pulseBuf;
};

static PFCLEDChainConverter *sequenceConveterSingleton  = NULL;


PFCLEDChainConverter    *createLEDChainConverter(uint32_t pulseWidthInMicroSec, PFCLEDChain **chains, size_t numberOfChains)
{
    if (sequenceConveterSingleton != NULL) {
        setPFCLastErrorNo(PFCError_SC_SINGLETION);
        return NULL;
    }
    
    PFCLEDChainConverter    *conv = (PFCLEDChainConverter *)malloc(sizeof(PFCLEDChainConverter));
    conv->chains = (PFCLEDChain **)malloc(sizeof(PFCLEDChain *) * numberOfChains);
    conv->pwidth = pulseWidthInMicroSec;
    conv->seqCount = numberOfChains;
    uint32_t    inactive = 0;
    uint32_t    max = 0;
    for (size_t i = 0 ; i < numberOfChains ; i ++) {
        conv->chains[i] = chains[i];
        uint32_t    cnt = (uint32_t)(chains[i]->ledCount);
        if (max < cnt)
            max = cnt;
    }
    conv->widCount = max;
    conv->wids = (char *)malloc(sizeof(char) * max);
    for (uint32_t i = 0 ; i < max ; i ++)
        conv->wids[i] = -1;
    conv->inactivePat = inactive;
    conv->endTick = 0;
    conv->pulseBuf = NULL;
    buildMiddlePulse(conv);
    sequenceConveterSingleton = conv;
    return conv;
}

void    disposeLEDChainConveter(PFCLEDChainConverter *conv)
{
    resetPinsFromWrite(conv);
    if (conv->pulseBuf != NULL)
        free(conv->pulseBuf);
    free(conv->wids);
    free(conv->chains);
    free(conv);
    sequenceConveterSingleton = NULL;
}

bool    transferColors(PFCLEDChainConverter *conv)
{
    int err = 0;
    
    if (conv->pulseBuf == NULL) {
        if (! setPinsToWrite(conv)) {
            setPFCLastErrorNo(PFCError_CANNOT_SET_MODE);
            return false;
        }
        conv->pulseBuf = (gpioPulse_t *)malloc(sizeof(gpioPulse_t) * pulseCountForOneLED);
        setLatchLevel(conv);
        gpioDelay(latchDurationInMicroSec);
    }
    else {
        if (gpioWaveTxBusy() != 0) {
            setPFCLastErrorNo(PFCError_ALREADY_TRANSFER);
            return false;
        }
        
        int32_t wait = (conv->endTick + latchDurationInMicroSec) - gpioTick();
        if (wait > 0)           //  wait LEDS to latch mid level for new data sequence
            gpioDelay(wait);    //  perhaps not too long
    }
    gpioWaveClear();
    gpioWaveAddNew();
    for (size_t i = 0 ; i < conv->widCount ; i ++) {
        int id = createWaveAtLEDIndex(conv, i);
        if (id < 0) {
            setPFCLastErrorNo(id);
            return false;
        }
        conv->wids[i] = (char)id;
    }
    err = gpioWaveChain(conv->wids, (unsigned int)(conv->widCount));
    if (err != 0) {
        setPFCLastErrorNo(err);
        return false;
    }
    
    conv->endTick = gpioTick() + conv->pwidth * pulseCountForOneLED * conv->widCount;
    
    return true;
}

bool    isNowTransferring(PFCLEDChainConverter *conv)
{
    return gpioWaveTxBusy() != 0;
}

static bool setPinsToWrite(PFCLEDChainConverter *conv)
{
    for (size_t i = 0 ; i < conv->seqCount ; i ++) {
        PFCLEDChain *chain = conv->chains[i];
        if (gpioSetMode((unsigned int)(chain->occupiedPins[0]), PI_OUTPUT) != 0)
            return false;
        if (gpioSetMode((unsigned int)(chain->occupiedPins[1]), PI_OUTPUT) != 0)
            return false;
    }
    return true;
}

static bool resetPinsFromWrite(PFCLEDChainConverter *conv)
{
    for (size_t i = 0 ; i < conv->seqCount ; i ++) {
        PFCLEDChain *chain = conv->chains[i];
        if (gpioSetMode((unsigned int)(chain->occupiedPins[0]), PI_INPUT) != 0)
            return false;
        if (gpioSetMode((unsigned int)(chain->occupiedPins[1]), PI_INPUT) != 0)
            return false;
    }
    return true;
}

static bool setLatchLevel(PFCLEDChainConverter *conv)
{
    return gpioWrite_Bits_0_31_Set(conv->middle.gpioOn) == 0;
}

static void buildMiddlePulse(PFCLEDChainConverter *conv)
{
    gpioPulse_t mid = {0, 0, conv->pwidth};
    for (size_t sc = 0 ; sc < conv->seqCount ; sc ++) {
        PFCLEDChain *chain = conv->chains[sc];
        mid.gpioOn |= chain->transitToMiddle.onPat;
        mid.gpioOff |= chain->transitToMiddle.offPat;
    }
    conv->middle = mid;
}

static int  createWaveAtLEDIndex(PFCLEDChainConverter *conv, size_t index)
{
    gpioPulse_t *pt = conv->pulseBuf;
    for (size_t cmp = 0 ; cmp < 3 ; cmp ++) {       //  3 is number of color components
        for (int32_t bp = 7 ; bp >= 0 ; bp --) {    //  8 is number of bits in byte, MSB first
            pt->gpioOn = 0;
            pt->gpioOff = 0;
            pt->usDelay = conv->pwidth;
            for (size_t sc = 0 ; sc < conv->seqCount ; sc ++) {
                PFCLEDChain *chain = conv->chains[sc];
                if (isValidIndex(chain, index)) {
                    if (isBitOn(chain, index, cmp, bp)) {
                        pt->gpioOn |= chain->transitToHigh.onPat;
                        pt->gpioOff |= chain->transitToHigh.offPat;
                    }
                    else {
                        pt->gpioOn |= chain->transitToLow.onPat;
                        pt->gpioOff |= chain->transitToLow.offPat;
                    }
                }
            }
            pt ++;
            *(pt ++) = conv->middle;
        }
    }
    int cnt = gpioWaveAddGeneric(pulseCountForOneLED, conv->pulseBuf);
    if (cnt < 0) {
        setPFCLastErrorNo(cnt);
        return -1;
    }
    return gpioWaveCreate();
}


static bool isBitOn(PFCLEDChain *chain, ssize_t ledIndex, size_t component, int32_t bitpos)
{
    PFCRGBColor  *col = &lightsOutColor;
    if ((0 <= ledIndex) && (ledIndex < chain->ledCount))
        col = chain->colors + ledIndex;
    return (componentValueAt(col, component) & (1 << bitpos)) != 0;
}

static uint8_t  componentValueAt(PFCRGBColor *color, size_t cPos)
{
    uint8_t val = 0;
    switch(cPos) {
        case rPos:
            val = color->r;     break;
        case gPos:
            val = color->g;     break;
        case bPos:
            val = color->b;     break;
    }
    return val;
}


