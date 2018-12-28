//
//  mainWave.c
//  PiFullColorLED
//
//  Created by illusia decafish on 2018/12/17.
//  Copyright © 2018 decafish. All rights reserved.
//

#include <stdio.h>
#include <unistd.h>
#include <pigpio.h>
#include "PFCWaveDriver.h"
#include "PFCLastError.h"
#include "PFCRGBColor.h"

int main(int argc, const char * argv[])
{
    if (gpioInitialise() < 0)
        return -1;
    
    //  assuming 2 chains of LEDs
    //  fist chain
    pinSetting    mid0;
    pinSetting    dat0;
    mid0.pinNumber = 23;
    mid0.polarity = activeHigh;
    dat0.pinNumber = 24;
    dat0.polarity = activeHigh;
    int    count0 = 6;
    PFCLEDChain *seq0 = createChainTristateLogic(count0, dat0, mid0);
    
    //  and second chain
    pinSetting    mid1;
    pinSetting    dat1;
    mid1.pinNumber = 21;
    mid1.polarity = activeHigh;
    dat1.pinNumber = 20;
    dat1.polarity = activeHigh;
    int    count1 = 4;
    PFCLEDChain *seq1 = createChainTristateLogic(count1, dat1, mid1);
    
    //    first of all, lights out.
    PFCRGBColor col = blackColor;
    for (int i = 0 ; i < count0 ; i ++)
        setLEDColorToChain(seq0, i, &col);
    for (int i = 0 ; i < count1 ; i ++)
        setLEDColorToChain(seq1, i, &col);
    
    //  create array of chains
    PFCLEDChain *chains[] = {seq0, seq1};
    
    //  create converter from chains to waveform
    PFCLEDChainConverter    *conv = createLEDChainConverter(3, chains, 2);
    
    //  kick transfer to turn of all LEDs
    if (! transferColors(conv)) {
        fprintf(stderr, "%s\n", PFCLastErrorDescription());
        resetPFCLastErrorNo();
    }
    
    //  wait for finish to transfer
    while (isNowTransferring(conv))
        gpioDelay(10000);
    sleep(1);
    
    //  then, turn on LED one after another
    for (int i = 0 ; i < 256 ; i ++) {
        uint8_t    c = (i * 6) % 256;
        col = colorWithRGB(c, c, c);
        setLEDColorToChain(seq0, (i * 6) / 256, &col);
        
        c = (i * 4) % 255;
        col = colorWithRGB(c, c, c);
        setLEDColorToChain(seq1, i / 64, &col);
        
        if (! transferColors(conv)) {
            fprintf(stderr, "%s\n", PFCLastErrorDescription());
            resetPFCLastErrorNo();
        }
        gpioDelay(10000);
        while (isNowTransferring(conv))
            gpioDelay(10000);
    }
    
    for (int i = 0 ; i < 512 ; i ++) {
        for (int l = 0 ; l < count0 ; l ++) {
            col = rgbFromHSV(3.141592f * (i + 16 * l) / 64.0f, 1.0f, 1.0f);
            setLEDColorToChain(seq0, l, &col);
        }
        if (i % 32 == 0) {
            int j = i / 32;
            for (int l = 0 ; l < count1 ; l ++) {
                col = rgbFromHSV(2.0f * 3.141592f * (j + l) / 3.0f, 1.0f, 1.0f);
                setLEDColorToChain(seq1, l, &col);
            }
        }
        if (! transferColors(conv)) {
            fprintf(stderr, "%s\n", PFCLastErrorDescription());
            resetPFCLastErrorNo();
        }
        gpioDelay(10000);
        while (isNowTransferring(conv))
            gpioDelay(10000);
    }
    
    for (int i = 0 ; i < count0 ; i ++)
        setLEDColorToChain(seq0, i, &blackColor);
    for (int i = 0 ; i < count1 ; i ++)
        setLEDColorToChain(seq1, i, &blackColor);
    if (! transferColors(conv)) {
        fprintf(stderr, "%s\n", PFCLastErrorDescription());
        resetPFCLastErrorNo();
    }
    while (isNowTransferring(conv))
        gpioDelay(10000);

    disposeLEDChainConveter(conv);
    disposeLEDChain(seq0);
    disposeLEDChain(seq1);
    gpioTerminate();
    return 0;
}
