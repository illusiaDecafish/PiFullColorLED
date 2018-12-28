//
//  PFCLastError.h
//  PiFullColorLED
//
//  Created by illusia decafish on 2018/12/17.
//  Copyright © 2018 decafish. All rights reserved.
//

#ifndef PFCLastError_h
#define PFCLastError_h

#include <pigpio.h>
#include <stdlib.h>

extern int          PFCLastErrorNo;
#define PFCErrorNumberBound (PI_CUSTOM_ERR_999 - 1)

char    *PFCLastErrorDescription(void);

void    setPFCLastErrorNo(int errorNo);
void    resetPFCLastErrorNo(void);



//  append other internal errors
#define PFCError_SC_SINGLETION      (PFCErrorNumberBound - 1)
#define PFCError_ALREADY_TRANSFER   (PFCErrorNumberBound - 2)
#define PFCError_CANNOT_SET_MODE    (PFCErrorNumberBound - 3)


#endif /* PFCLastError_h */
