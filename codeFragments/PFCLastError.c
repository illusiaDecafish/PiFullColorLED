//
//  PFCLastError.c
//  PiFullColorLED
//
//  Created by illusia decafish on 2018/12/17.
//  Copyright © 2018 decafish. All rights reserved.
//

#include <pigpio.h>
#include "PFCLastError.h"

int         PFCLastErrorNo  = 0; //  0 means no error for almost functions in pigpio library.
//const int   PFCErrorNoBound = PI_CUSTOM_ERR_999 - 1;

typedef struct _PFCLastErrorMessage {
    int     no;
    char    *msg;
} PFCLastErrorMessage;

static PFCLastErrorMessage  errorMessages[] = {
    {PI_INIT_FAILED, "pigpio library initialization failed"},
    {PI_BAD_USER_GPIO, "GPIO pin number should be between 0 to 31"},
    {PI_BAD_MODE, "mode not 0-7"},
    {PI_BAD_PULSEWIDTH, "pulsewidth not 0 or 500-2500"},
    {PI_TOO_MANY_PULSES, "waveform has too many pulses"},
    {PI_CHAIN_NESTING, "chain counters nested too deeply"},
    {PI_CHAIN_LOOP_CNT, "bad chain loop count"},
    {PI_BAD_CHAIN_LOOP, "empty chain loop"},
    {PI_BAD_CHAIN_CMD, "bad chain command"},
    {PI_CHAIN_COUNTER, "too many chain counters"},
    {PI_BAD_CHAIN_DELAY, "bad chain delay micros"},
    {PI_CHAIN_TOO_BIG, "chain is too long"},
    {PI_BAD_WAVE_ID, "non existent wave id"},
    {PI_EMPTY_WAVEFORM, "attempt to create an empty waveform"},
    {PI_TOO_MANY_CBS, "No more CBs for waveform"},
    {PI_TOO_MANY_OOL, "No more OOL for waveform"},
    {PI_EMPTY_WAVEFORM,"attempt to create an empty waveform"},
    {PI_NO_WAVEFORM_ID, "no more waveforms"},
    
    {PFCError_SC_SINGLETION, "PFCSequenceConverter should be a singleton"},
    {PFCError_ALREADY_TRANSFER, "waveform already transfering"},
    {PFCError_CANNOT_SET_MODE, "can not set mode"}
      
};

static const size_t numberOfErrorMessages   = sizeof(errorMessages) / sizeof(PFCLastErrorMessage);


char    *PFCLastErrorDescription(void)
{
    PFCLastErrorMessage *em = errorMessages;
    for (size_t i = 0 ; i < numberOfErrorMessages ; i ++)
        if (em[i].no == PFCLastErrorNo)
            return em[i].msg;
    return NULL;
}

void    setPFCLastErrorNo(int errorNo)
{
    PFCLastErrorNo = errorNo;
}

void    resetPFCLastErrorNo(void)
{
    PFCLastErrorNo = 0;
}
