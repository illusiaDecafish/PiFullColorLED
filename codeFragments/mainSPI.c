//
//  main.c
//  PiFullColorLED
//
//  Created by illusia decafish on 2018/12/17.
//  Copyright © 2018 decafish. All rights reserved.
//

#include <stdio.h>
#include <unistd.h>
#include <pigpio.h>
#include "PFCLastError.h"
#include "PFCRGBColor.h"
#include "PFCSPIDriver.h"

int main(int argc, const char * argv[])
{
    if (gpioInitialise() < 0)
        return -1;
    
    PFCSpiDriver    spid = openSpiForPFCDriver(0, 0, activeHigh);
    if (spid < 0) {
        gpioTerminate();
        return -1;
    }
    
    transferSpiAllOff(spid, 1, true);   //  clock is unstable on this transfer. I don't know why
    gpioDelay(1000 * 1000);
    
    //  send lights out
    transferSpiAllOff(spid, 4, true);
    gpioDelay(1000 * 1000);
    
    //  4 LEDs
    SpiSequence  *sb = createHollowSpiSequence(4);
    
    //  set each color
    setLEDColorToSpiSequence(sb, 0, &redColor);
    setLEDColorToSpiSequence(sb, 1, &greenColor);
    setLEDColorToSpiSequence(sb, 2, &blueColor);
    setLEDColorToSpiSequence(sb, 3, &whiteColor);
    
    //  then tranfer to LED chain
    transferSpiSequence(spid, sb);
    sleep(1);
    
    //  lights out again
    transferSpiAllOff(spid, 4, true);
    
    //  finish
    disposePFCSpiDriver(spid);
    gpioTerminate();
    return 0;
}
