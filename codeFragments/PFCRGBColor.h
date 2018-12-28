//
//  PFCRGBColor.h
//  PiFullColorLED
//
//  Created by illusia decafish on 2018/12/17.
//  Copyright © 2018 decafish. All rights reserved.
//

#ifndef PFCRGBColor_h
#define PFCRGBColor_h

#include <stdint.h>


typedef struct _PFCRGBColor {
    uint8_t     r;
    uint8_t     g;
    uint8_t     b;
} PFCRGBColor;

extern PFCRGBColor   whiteColor;
extern PFCRGBColor   blackColor;
extern PFCRGBColor   redColor;
extern PFCRGBColor   greenColor;
extern PFCRGBColor   blueColor;
extern PFCRGBColor   cyanColor;
extern PFCRGBColor   yellowColor;
extern PFCRGBColor   magentaColor;

PFCRGBColor     colorWithRGB(uint8_t r, uint8_t g, uint8_t b);

//  create RGB color from HSV color values
//  Hue: 0 to 2 pi
//  saturation: 0 to 1
//  value:  0 to 1
PFCRGBColor     rgbFromHSV(float hue, float sat, float val);

//  dim color by ratio, from 0 to 1
void            dim(PFCRGBColor *color, float ratio);


#endif /* PFCRGBColor_h */
