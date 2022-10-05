/*
 * Copyright (c) 2009-2021 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*============================ INCLUDES ======================================*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#include "arm_extra_lcd_printf.h"
#include "arm_2d.h"
#include "./__common.h"

#if defined(__clang__)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wunknown-warning-option"
#   pragma clang diagnostic ignored "-Wreserved-identifier"
#   pragma clang diagnostic ignored "-Wdeclaration-after-statement"
#   pragma clang diagnostic ignored "-Wformat-nonliteral"
#   pragma clang diagnostic ignored "-Wsign-compare"
#   pragma clang diagnostic ignored "-Wmissing-prototypes"
#   pragma clang diagnostic ignored "-Wcast-qual"
#   pragma clang diagnostic ignored "-Wsign-conversion"
#   pragma clang diagnostic ignored "-Wpedantic"
#   pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#   pragma clang diagnostic ignored "-Wpadded"
#endif

/*============================ MACROS ========================================*/

#ifndef __LCD_PRINTF_CFG_TEXT_BUFFER_SIZE__
#   define __LCD_PRINTF_CFG_TEXT_BUFFER_SIZE__              256
#endif

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
extern
const uint8_t Font_6x8_h[(144-32)*8];


/*============================ PROTOTYPES ====================================*/
/*============================ LOCAL VARIABLES ===============================*/

static struct {
    arm_2d_region_t tRegion;
    struct {
        uint8_t         chX;
        uint8_t         chY;
    } tTextLocation;
    
    struct {
        COLOUR_INT_TYPE     tForeground;
        COLOUR_INT_TYPE     tBackground;
    } tColour;
    
    arm_2d_tile_t *ptTargetFB;
    uint32_t       wMode;
    
    arm_2d_font_t *ptFont;
    arm_2d_tile_t tileFont;
    arm_2d_tile_t tileChar;
    
} s_tLCDTextControl = {
    .tRegion = { 
        .tSize = {
            .iWidth = __GLCD_CFG_SCEEN_WIDTH__,
            .iHeight = __GLCD_CFG_SCEEN_HEIGHT__,
        },
    },
    .tColour = {
        .tForeground = GLCD_COLOR_GREEN,
        .tBackground = GLCD_COLOR_BLACK,
    },
    .ptTargetFB = (arm_2d_tile_t *)(-1),
    .wMode = ARM_2D_DRW_PATN_MODE_COPY,
            //| ARM_2D_DRW_PATN_MODE_NO_FG_COLOR    
            //| ARM_2D_DRW_PATH_MODE_COMP_FG_COLOUR 
    .ptFont = &ARM_2D_FONT_6x8,
    
    .tileFont = {
        .tRegion = {
            .tSize = {
                .iWidth = 6,
                .iHeight = 8 * 112,
            },
        },
        .tInfo = {
            .bIsRoot = true,
            .bHasEnforcedColour = true,
            .tColourInfo = {
                .chScheme = ARM_2D_COLOUR_BIN,
            },
        },
        .pchBuffer = (uint8_t *)Font_6x8_h,
    },
    
    .tileChar = {
        .tRegion = {
            .tSize = {
                .iWidth = 6,
                .iHeight = 8,
            },
        },
        .ptParent = (arm_2d_tile_t *)&s_tLCDTextControl.tileFont,
    }
};

/*============================ IMPLEMENTATION ================================*/

void arm_lcd_text_set_colour(COLOUR_INT_TYPE wForeground, COLOUR_INT_TYPE wBackground)
{
    s_tLCDTextControl.tColour.tForeground = wForeground;
    s_tLCDTextControl.tColour.tBackground = wBackground;
}

void arm_lcd_text_set_target_framebuffer(arm_2d_tile_t *ptFrameBuffer)
{
    s_tLCDTextControl.ptTargetFB = ptFrameBuffer;

    if (NULL == ptFrameBuffer) {
        // use default framebuffer
        s_tLCDTextControl.ptTargetFB = (arm_2d_tile_t *)(-1);
    }
}

void arm_lcd_text_set_display_mode(uint32_t wMode)
{
    s_tLCDTextControl.wMode = wMode;
}

void arm_lcd_text_set_draw_region(arm_2d_region_t *ptRegion)
{
    if (NULL == ptRegion) {
        ptRegion =  (arm_2d_region_t []) {
                    {
                        .tSize = {
                            .iWidth = __GLCD_CFG_SCEEN_WIDTH__,
                            .iHeight = __GLCD_CFG_SCEEN_HEIGHT__,
                        },
                    }};
    }
    
    s_tLCDTextControl.tRegion = *ptRegion;
}

void arm_lcd_text_location(uint8_t chY, uint8_t chX)
{
    s_tLCDTextControl.tTextLocation.chX = chX;
    s_tLCDTextControl.tTextLocation.chY = chY;
    
    if (    s_tLCDTextControl.tTextLocation.chX *  s_tLCDTextControl.ptFont->tSize.iWidth
        >=  s_tLCDTextControl.tRegion.tSize.iWidth ) {
        s_tLCDTextControl.tTextLocation.chX = 0;
        s_tLCDTextControl.tTextLocation.chY++;
        if (    s_tLCDTextControl.tTextLocation.chY * s_tLCDTextControl.ptFont->tSize.iHeight 
            >= s_tLCDTextControl.tRegion.tSize.iHeight) {
            s_tLCDTextControl.tTextLocation.chY = 0;
        }
    }
}

arm_2d_err_t arm_lcd_text_set_font(arm_2d_font_t *ptFont)
{
    if (NULL == ptFont) {
        ptFont = &ARM_2D_FONT_6x8;   /* use default font */
    }

    if (s_tLCDTextControl.ptFont == ptFont) {
        return ARM_2D_ERR_NONE;
    }

    do {
        if (NULL == ptFont->chBitmap) {
            break;
        }
        if (    (0 == ptFont->tSize.iHeight) 
            ||  (0 == ptFont->tSize.iWidth)
            ||  (0 == ptFont->nCount)) {
            break;
        }
        
        s_tLCDTextControl.ptFont = ptFont;
        
        /* update tileFont */
        s_tLCDTextControl.tileFont.tRegion.tSize.iWidth = ptFont->tSize.iWidth;
        s_tLCDTextControl.tileFont.tRegion.tSize.iHeight = ptFont->tSize.iHeight * ptFont->nCount;
        s_tLCDTextControl.tileFont.pchBuffer = (uint8_t *)ptFont->chBitmap;
        
        /* update tileChar */
        s_tLCDTextControl.tileChar.tRegion.tSize = ptFont->tSize;

        /* reset draw pointer */
        arm_lcd_text_location(0,0);
        
        return ARM_2D_ERR_NONE;
    } while(0);
    
    return ARM_2D_ERR_INVALID_PARAM;
}

void lcd_draw_char(int16_t iX, int16_t iY, char chChar)
{
    s_tLCDTextControl.tileChar.tRegion.tLocation.iY = 
        (chChar - (int16_t)s_tLCDTextControl.ptFont->nOffset) * s_tLCDTextControl.ptFont->tSize.iHeight;
    
    arm_2d_region_t tDrawRegion = {
        .tLocation = {.iX = iX, .iY = iY},
        .tSize = s_tLCDTextControl.tileChar.tRegion.tSize,
    };
    
    arm_2d_draw_pattern(&s_tLCDTextControl.tileChar, 
                        s_tLCDTextControl.ptTargetFB, 
                        &tDrawRegion,
                        s_tLCDTextControl.wMode,
                        s_tLCDTextControl.tColour.tForeground,
                        s_tLCDTextControl.tColour.tBackground);
    arm_2d_op_wait_async(NULL);
}

void arm_lcd_puts(const char *str)
{
    while(*str) {
        if (*str == '\r') {
            s_tLCDTextControl.tTextLocation.chX = 0;
        } else if (*str == '\n') {
            s_tLCDTextControl.tTextLocation.chX = 0;
            s_tLCDTextControl.tTextLocation.chY++;
        } else if (*str == '\t') { 
            s_tLCDTextControl.tTextLocation.chX += 8;
            s_tLCDTextControl.tTextLocation.chX &= ~(_BV(3)-1);

            if (    s_tLCDTextControl.tTextLocation.chX * s_tLCDTextControl.ptFont->tSize.iWidth 
                >=  s_tLCDTextControl.tRegion.tSize.iWidth ) {
                s_tLCDTextControl.tTextLocation.chX = 0;
                s_tLCDTextControl.tTextLocation.chY++;
            }
        }else if (*str == '\b') {
            if (s_tLCDTextControl.tTextLocation.chX) {
                s_tLCDTextControl.tTextLocation.chX--;
            }
        } else {
        
            int16_t iX = s_tLCDTextControl.tTextLocation.chX * s_tLCDTextControl.ptFont->tSize.iWidth;
            int16_t iY = s_tLCDTextControl.tTextLocation.chY * s_tLCDTextControl.ptFont->tSize.iHeight;
        
            lcd_draw_char(  s_tLCDTextControl.tRegion.tLocation.iX + iX, 
                            s_tLCDTextControl.tRegion.tLocation.iY + iY, 
                            *str);
                            
            s_tLCDTextControl.tTextLocation.chX++;
            if (    s_tLCDTextControl.tTextLocation.chX * s_tLCDTextControl.ptFont->tSize.iWidth 
                >=  s_tLCDTextControl.tRegion.tSize.iWidth ) {
                s_tLCDTextControl.tTextLocation.chX = 0;
                s_tLCDTextControl.tTextLocation.chY++;
                if (    s_tLCDTextControl.tTextLocation.chY * s_tLCDTextControl.ptFont->tSize.iHeight 
                    >= s_tLCDTextControl.tRegion.tSize.iHeight) {
                    s_tLCDTextControl.tTextLocation.chY = 0;
                }
            }
        }
        
        str++;
    }
}

#if defined(__IS_COMPILER_IAR__) && __IS_COMPILER_IAR__
#define __va_list    va_list

#endif

int arm_lcd_printf(const char *format, ...)
{
    int real_size;
    static char s_chBuffer[__LCD_PRINTF_CFG_TEXT_BUFFER_SIZE__ + 1];
    __va_list ap;
    va_start(ap, format);
        real_size = vsnprintf(s_chBuffer, sizeof(s_chBuffer)-1, format, ap);
    va_end(ap);
    real_size = MIN(sizeof(s_chBuffer)-1, real_size);
    s_chBuffer[real_size] = '\0';
    arm_lcd_puts(s_chBuffer);
    return real_size;
}

#if defined(__clang__)
#   pragma clang diagnostic pop
#endif
