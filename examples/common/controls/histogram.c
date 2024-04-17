/*
 * Copyright (c) 2009-2022 Arm Limited. All rights reserved.
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
#define __HISTOGRAM_IMPLEMENT__

#include "./arm_extra_controls.h"
#include "./__common.h"
#include "arm_2d.h"
#include "arm_2d_helper.h"
#include "histogram.h"
#include <assert.h>
#include <string.h>

#if defined(__clang__)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wunknown-warning-option"
#   pragma clang diagnostic ignored "-Wreserved-identifier"
#   pragma clang diagnostic ignored "-Wdeclaration-after-statement"
#   pragma clang diagnostic ignored "-Wsign-conversion"
#   pragma clang diagnostic ignored "-Wpadded"
#   pragma clang diagnostic ignored "-Wcast-qual"
#   pragma clang diagnostic ignored "-Wcast-align"
#   pragma clang diagnostic ignored "-Wmissing-field-initializers"
#   pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#   pragma clang diagnostic ignored "-Wmissing-braces"
#   pragma clang diagnostic ignored "-Wunused-const-variable"
#   pragma clang diagnostic ignored "-Wmissing-declarations"
#   pragma clang diagnostic ignored "-Wmissing-variable-declarations"
#endif

/*============================ MACROS ========================================*/

#if __GLCD_CFG_COLOUR_DEPTH__ == 8


#elif __GLCD_CFG_COLOUR_DEPTH__ == 16


#elif __GLCD_CFG_COLOUR_DEPTH__ == 32

#else
#   error Unsupported colour depth!
#endif

#undef this
#define this    (*ptThis)

/*============================ MACROFIED FUNCTIONS ===========================*/

#define INT16_2_Q16(__VALUE)    ((int32_t)(__VALUE) << 16)

/*============================ TYPES =========================================*/
enum {
    HISTOGRAM_DR_START = 0,

};

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ LOCAL VARIABLES ===============================*/

const uint8_t c_chScanLineVerticalLineMask[]= {
    255, 255, 128, 0, 128,
};

const arm_2d_tile_t c_tileLineVerticalLineMask = {
    .tRegion = {
        .tSize = {
            .iWidth = 1,
            .iHeight = 5,
        },
    },
    .tInfo = {
        .bIsRoot = true,
        .bHasEnforcedColour = true,
        .tColourInfo = {
            .chScheme = ARM_2D_COLOUR_8BIT,
        },
    },
    .pchBuffer = (uint8_t *)c_chScanLineVerticalLineMask,
};


/*============================ IMPLEMENTATION ================================*/

ARM_NONNULL(1,2)
void histogram_init( histogram_t *ptThis,
                     histogram_cfg_t *ptCFG)
{
    assert(NULL != ptThis);
    assert(NULL != ptCFG);
    assert(NULL != ptCFG->Bin.ptItems);
    assert(ptCFG->Bin.hwCount > 0);

    memset(ptThis, 0, sizeof(histogram_t));

    this.tCFG = *ptCFG;


    if (this.tCFG.Bin.tSize.iWidth <= 0) {
        this.tCFG.Bin.tSize.iWidth = 1;
    }
    if (this.tCFG.Bin.tSize.iHeight <=0) {
        this.tCFG.Bin.tSize.iHeight = 64;   /* default value */
    }
    if (this.tCFG.Bin.u6BinsPerDirtyRegion == 0) {
        if (this.tCFG.Bin.tSize.iWidth >= 16) {
            this.tCFG.Bin.u6BinsPerDirtyRegion = 1;
        } else {
            this.tCFG.Bin.u6BinsPerDirtyRegion = 64 / this.tCFG.Bin.tSize.iWidth;
        }
    }

    if (this.tCFG.Bin.iMaxValue <= 0) {
        this.tCFG.Bin.iMaxValue = INT16_MAX;
    }

    if (this.tCFG.Bin.bSupportNegative) {
        this.q16Ratio = (   INT16_2_Q16((this.tCFG.Bin.tSize.iHeight >> 1))  
                        /   this.tCFG.Bin.iMaxValue);
    } else {
        this.q16Ratio = (   INT16_2_Q16(this.tCFG.Bin.tSize.iHeight)  
                        /   this.tCFG.Bin.iMaxValue);
    }

    if (this.tCFG.Colour.wFrom == this.tCFG.Colour.wTo) {
        /* same colour */
        arm_foreach(histogram_bin_item_t, 
                    this.tCFG.Bin.ptItems, 
                    this.tCFG.Bin.hwCount,
                    ptItem) {
            memset(ptItem, 0, sizeof(histogram_bin_item_t));

            /* initialize colour */
            ptItem->tColour = arm_2d_pixel_from_brga8888(this.tCFG.Colour.wFrom);
        }
    } else {
        uint32_t wFrom = this.tCFG.Colour.wFrom;
        uint32_t wTo = this.tCFG.Colour.wTo;

        histogram_bin_item_t *ptItem = this.tCFG.Bin.ptItems;
        for (uint_fast16_t n = 0; n < this.tCFG.Bin.hwCount; n++) {
            /* initialize colour */
            ptItem->tColour = arm_2d_pixel_from_brga8888( 
                                            __arm_2d_helper_colour_slider(
                                                wFrom, 
                                                wTo,
                                                this.tCFG.Bin.hwCount,
                                                n));
            ptItem++;
        }
    }

    /* calculate the histogram size */
    this.tHistogramSize.iHeight = this.tCFG.Bin.tSize.iHeight;
    this.tHistogramSize.iWidth =    (  this.tCFG.Bin.tSize.iWidth 
                                    +  this.tCFG.Bin.chPadding)
                               * this.tCFG.Bin.hwCount;

    if (NULL != this.tCFG.ptParent) {
        this.bUseDirtyRegion = true;

        arm_2d_scene_player_dynamic_dirty_region_init(  &this.tDirtyRegion,
                                                        this.tCFG.ptParent);
    }


}

ARM_NONNULL(1)
void histogram_depose( histogram_t *ptThis)
{
    assert(NULL != ptThis);
    
    arm_2d_scene_player_dynamic_dirty_region_depose(&this.tDirtyRegion,
                                                    this.tCFG.ptParent);
}

ARM_NONNULL(1)
void histogram_on_frame_start( histogram_t *ptThis)
{
    assert(NULL != ptThis);

    arm_foreach(histogram_bin_item_t, 
                    this.tCFG.Bin.ptItems, 
                    this.tCFG.Bin.hwCount,
                    ptItem) {
        ptItem->iLastValue = ptItem->iCurrentValue;
        ptItem->iCurrentValue = ptItem->iNewValue;
    }
    arm_2d_dynamic_dirty_region_on_frame_start(&this.tDirtyRegion, HISTOGRAM_DR_START);
}

ARM_NONNULL(1)
void histogram_show(histogram_t *ptThis,
                    const arm_2d_tile_t *ptTile, 
                    const arm_2d_region_t *ptRegion,
                    uint8_t chOpacity)
{
    assert(NULL!= ptThis);

    arm_2d_container(ptTile, __histogram, ptRegion) {

        arm_2d_region_t tPanelRegion = {
            .tSize = this.tHistogramSize,
        };

        arm_2d_container(&__histogram, __panel, &tPanelRegion) {

            arm_2d_location_t tBaseLine = {0,0};
            int16_t iBinWidth = this.tCFG.Bin.tSize.iWidth;
            if (this.tCFG.Bin.bSupportNegative) {
                tBaseLine.iY = this.tHistogramSize.iHeight >> 1;

            } else {
                tBaseLine.iY = this.tHistogramSize.iHeight;

                arm_foreach(histogram_bin_item_t, 
                    this.tCFG.Bin.ptItems, 
                    this.tCFG.Bin.hwCount,
                    ptItem) {
                
                    int16_t iHeight = ((int64_t)this.q16Ratio * (int64_t)INT16_2_Q16(ptItem->iCurrentValue)) >> 32;

                    arm_2d_region_t tBinRegion = {
                        .tLocation = {
                            .iX = tBaseLine.iX,
                            .iY = tBaseLine.iY - iHeight,
                        },
                        .tSize = {
                            .iWidth = iBinWidth,
                            .iHeight = ABS(iHeight),
                        },
                    };

                    if (this.tCFG.Bin.bUseScanLine) {
                    
                        arm_2d_container(&__panel, __bin, &tBinRegion) {

                            int16_t iOffset = this.tHistogramSize.iHeight - iHeight;
                            arm_2d_region_t tOriginalRegion = __bin_canvas;
                            tOriginalRegion.tLocation.iY -= this.tHistogramSize.iHeight - iHeight;
                            tOriginalRegion.tSize.iHeight += iOffset;

                            arm_2d_fill_colour_with_vertical_line_mask_and_opacity(
                                &__bin,
                                &tOriginalRegion,
                                &c_tileLineVerticalLineMask,
                                (__arm_2d_color_t) {ptItem->tColour},
                                chOpacity);
                        }

                    } else {
                        arm_2d_fill_colour_with_opacity(
                            &__panel,
                            &tBinRegion,
                            (__arm_2d_color_t) {ptItem->tColour},
                            chOpacity
                        );
                    }
                    ARM_2D_OP_WAIT_ASYNC();

                    tBaseLine.iX += iBinWidth + this.tCFG.Bin.chPadding;
                }

            }


            
        }
    }

    ARM_2D_OP_WAIT_ASYNC();
}

#if defined(__clang__)
#   pragma clang diagnostic pop
#endif
