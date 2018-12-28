//
//  PFCSPIDriver.c
//  PiFullColorLED
//
//  Created by illusia decafish on 2018/12/17.
//  Copyright © 2018 decafish. All rights reserved.
//

#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <pigpio.h>
#include <string.h>
#include "PFCSPIDriver.h"

struct _SpiSequence {
    size_t          length;
    uint8_t         bytes[];
};


static unsigned         fclbaudrate = 4600000;

static size_t           bitsInByte  = 8;
static size_t           bytesInRGB  = 3;

static unsigned char    patternOne  = 0xFE; //  11111110
static unsigned char    patternZero = 0x80; //  10000000

static uint8_t          *writeByteInSequence(uint8_t *bPointer, unsigned char byte);


PFCSpiDriver    openSpiForPFCDriver(int spiBusNumber, int chipSelect, bool csPolarity)
{
    unsigned    csres = 0;
    if (chipSelect < 0) {
        csres = 1 << 5;
        chipSelect = 0;
    }
    unsigned    bus = spiBusNumber << 8;
    unsigned    pol = (csPolarity ? 1 : 0) << (2 + chipSelect);
    unsigned    flag = bus | pol | csres;
    
    int handle = spiOpen(chipSelect, fclbaudrate, flag);
    return handle;
}

void    disposePFCSpiDriver(PFCSpiDriver driver)
{
    spiClose(driver);
}

SpiSequence *createHollowSpiSequence(size_t countOfLED)
{
    size_t      len = countOfLED * bytesInRGB * bitsInByte;
    SpiSequence *ret = (SpiSequence *)malloc(sizeof(SpiSequence) + len);
    ret->length = len;
    uint8_t zero = patternZero;
    memset(ret->bytes, zero, len);
    return ret;
}

void    disposeSpiSequence(SpiSequence *bitSeq)
{
    free(bitSeq);
}

bool    setLEDColorToSpiSequence(SpiSequence *bitSeq, size_t indexAt, PFCRGBColor *rgb)
{
    size_t  pos = indexAt * bytesInRGB * bitsInByte;
    if (pos > bitSeq->length)
        return false;
    uint8_t *bp = bitSeq->bytes + pos;
    bp = writeByteInSequence(bp, rgb->r);
    bp = writeByteInSequence(bp, rgb->g);
    bp = writeByteInSequence(bp, rgb->b);
    return true;
}

size_t  numberOfLEDInSpiSequence(SpiSequence *bitSeq)
{
    return bitSeq->length / (bytesInRGB * bitsInByte);
}

bool    transferSpiSequence(PFCSpiDriver driver, SpiSequence *bits)
{
    return spiWrite(driver, (char *)bits->bytes, (unsigned)bits->length) == (int)bits->length;
}

bool    transferSpiAllOff(PFCSpiDriver driver, size_t countOfLED, bool polarity)
{
    SpiSequence *bs = createHollowSpiSequence(countOfLED);
    bool    yn = transferSpiSequence(driver, bs);
    disposeSpiSequence(bs);
    return yn;
}


static uint8_t *writeByteInSequence(uint8_t *bPointer, unsigned char byte)
{
    uint8_t one = patternOne;
    uint8_t zero = patternZero;
    for (uint8_t mask = 0x80 ; mask != 0 ; mask /= 2)
        *(bPointer ++) = ((byte & mask) != 0) ? one : zero;
    return bPointer;
}

