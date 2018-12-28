//
//  PFCRGBColor.c
//  PiFullColorLED
//
//  Created by illusia decafish on 2018/12/17.
//  Copyright © 2018 decafish. All rights reserved.
//

#include "PFCRGBColor.h"


#include <math.h>
#include "PFCRGBColor.h"


PFCRGBColor  whiteColor      = {0xFF, 0xFF, 0xFF};
PFCRGBColor  blackColor      = {0x00, 0x00, 0x00};
PFCRGBColor  redColor        = {0xFF, 0x00, 0x00};
PFCRGBColor  greenColor      = {0x00, 0xFF, 0x00};
PFCRGBColor  blueColor       = {0x00, 0x00, 0xFF};
PFCRGBColor  cyanColor       = {0x00, 0xFF, 0xFF};
PFCRGBColor  yellowColor     = {0xFF, 0xFF, 0x00};
PFCRGBColor  magentaColor    = {0xFF, 0x00, 0xFF};

PFCRGBColor     colorWithRGB(uint8_t r, uint8_t g, uint8_t b)
{
    PFCRGBColor col;
    col.r = r;
    col.g = g;
    col.b = b;
    return col;
}


PFCRGBColor     rgbFromHSV(float hue, float sat, float val)
{
    static const float  piby3 = (float)(M_PI / 3.0);
    static const float  fullScale = 255.0f;
    float               modhue = fmodf(hue, (float)(M_PI * 2.0));
    
    if (sat < 0.0f)
        sat = 0.0f;
    else if (sat > 1.0f)
        sat = 1.0f;
    if (val < 0.0f)
        val = 0.0f;
    else if (val > 1.0f)
        val = 1.0f;
    
    float               fsat = val * sat;
    float               isat = val * (1.0f - sat);
    float               fr, fg, fb;
    PFCRGBColor         rgb;
    
    if (modhue < piby3) {
        fr = val;
        fg = (isat + fsat * modhue / piby3);
        fb = isat;
    }
    else if (modhue < piby3 * 2.0f) {
        fr = isat + fsat * (piby3 * 2.0f - modhue) / piby3;
        fg = val;
        fb = isat;
    }
    else if (modhue < piby3 * 3.0f) {
        fg = val;
        fb = isat + fsat * (modhue - piby3 * 2.0f) / piby3;
        fr = isat;
    }
    else if (modhue < piby3 * 4.0f) {
        fg = isat + fsat * (piby3 * 4.0f - modhue) / piby3;
        fb = val;
        fr = isat;
    }
    else if (modhue < piby3 * 5.0f) {
        fb = val;
        fr = isat + fsat * (modhue - piby3 * 4.0f) / piby3;
        fg = isat;
    }
    else {
        fb = isat + fsat * (piby3 * 6.0f - modhue) / piby3;
        fr = val;
        fg = isat;
    }
    rgb.r = (uint8_t)(fr * fullScale);
    rgb.g = (uint8_t)(fg * fullScale);
    rgb.b = (uint8_t)(fb * fullScale);
    return rgb;
}


void    dim(PFCRGBColor *color, float ratio)
{
    if ((0.0f <= ratio) && (ratio  < 1.0f)) {
        color->r = (uint8_t)roundf(color->r * ratio);
        color->g = (uint8_t)roundf(color->g * ratio);
        color->b = (uint8_t)roundf(color->b * ratio);
    }
}

