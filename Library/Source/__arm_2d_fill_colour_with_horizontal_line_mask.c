/*
 * Copyright (c) 2009-2024 Arm Limited. All rights reserved.
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

 /* ----------------------------------------------------------------------
 * Project:      Arm-2D Library
 * Title:        __arm_2d_fill_colour_with_horizontal_line_mask.c
 * Description:  APIs for colour-filling-with-horizontal-line-mask
 *
 * $Date:        20. April 2024
 * $Revision:    V.0.3.0
 *
 * Target Processor:  Cortex-M cores
 *
 * -------------------------------------------------------------------- */

/*============================ INCLUDES ======================================*/

#ifdef __ARM_2D_COMPILATION_UNIT
#undef __ARM_2D_COMPILATION_UNIT

#define __ARM_2D_IMPL__

#include "arm_2d.h"
#include "__arm_2d_impl.h"

#ifdef   __cplusplus
extern "C" {
#endif

#if defined(__clang__)
#   pragma clang diagnostic ignored "-Wunknown-warning-option"
#   pragma clang diagnostic ignored "-Wreserved-identifier"
#   pragma clang diagnostic ignored "-Wincompatible-pointer-types-discards-qualifiers"
#   pragma clang diagnostic ignored "-Wmissing-variable-declarations"
#   pragma clang diagnostic ignored "-Wcast-qual"
#   pragma clang diagnostic ignored "-Wcast-align"
#   pragma clang diagnostic ignored "-Wextra-semi-stmt"
#   pragma clang diagnostic ignored "-Wsign-conversion"
#   pragma clang diagnostic ignored "-Wunused-function"
#   pragma clang diagnostic ignored "-Wimplicit-int-float-conversion"
#   pragma clang diagnostic ignored "-Wdouble-promotion"
#   pragma clang diagnostic ignored "-Wunused-parameter"
#   pragma clang diagnostic ignored "-Wimplicit-float-conversion"
#   pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#   pragma clang diagnostic ignored "-Wtautological-pointer-compare"
#   pragma clang diagnostic ignored "-Wsign-compare"
#   pragma clang diagnostic ignored "-Wmissing-prototypes"
#   pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#   pragma clang diagnostic ignored "-Wswitch-enum"
#   pragma clang diagnostic ignored "-Wswitch"
#   pragma clang diagnostic ignored "-Wdeclaration-after-statement"
#elif defined(__IS_COMPILER_ARM_COMPILER_5__)
#   pragma diag_suppress 174,177,188,68,513,144
#endif

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ PROTOTYPES ====================================*/
extern
void __arm_2d_impl_cccn888_fill_colour_with_horizontal_line_mask(
                            uint32_t *__RESTRICT pSource,
                            int16_t iSourceStride,
                            uint32_t *__RESTRICT pTarget,
                            int16_t iTargetStride,
                            arm_2d_region_t *__RESTRICT ptValidRegionOnVirtualScreen,
                            arm_2d_region_t *ptTargetRegionOnVirtualScreen,
                            arm_2d_size_t *__RESTRICT ptCopySize);

/*============================ LOCAL VARIABLES ===============================*/
/*============================ IMPLEMENTATION ================================*/

/*
 * the Frontend API
 */

ARM_NONNULL(2,4)
arm_fsm_rt_t arm_2dp_cccn888_fill_colour_with_horizontal_line_mask(
                                        arm_2d_op_fill_cl_msk_t *ptOP,
                                        const arm_2d_tile_t *ptTarget,
                                        const arm_2d_region_t *ptRegion,
                                        const arm_2d_tile_t *ptLineMask,
                                        arm_2d_color_cccn888_t tColour)
{
    assert(NULL != ptTarget);
    assert(NULL != ptLineMask);

    ARM_2D_IMPL(arm_2d_op_fill_cl_msk_t, ptOP);

    //! valid alpha mask tile
    if (!__arm_2d_valid_mask(ptLineMask, 
                                __ARM_2D_MASK_ALLOW_A8
                        #if __ARM_2D_CFG_SUPPORT_COLOUR_CHANNEL_ACCESS__
                            |   __ARM_2D_MASK_ALLOW_8in32
                        #endif
                            )) {
        return (arm_fsm_rt_t)ARM_2D_ERR_INVALID_PARAM;
    }

    if (!__arm_2d_op_acquire((arm_2d_op_core_t *)ptThis)) {
        return arm_fsm_rt_on_going;
    }

    OP_CORE.ptOp = &ARM_2D_OP_FILL_COLOUR_WITH_HORIZONTAL_LINE_MASK_CCCN888;

    this.Target.ptTile = ptTarget;
    this.Target.ptRegion = ptRegion;
    this.Mask.ptTile = ptLineMask;
    this.wMode = 0;
    this.wColour = tColour.tValue;

    return __arm_2d_op_invoke((arm_2d_op_core_t *)ptThis);

}


/*
 * The backend entry
 */
arm_fsm_rt_t __arm_2d_cccn888_sw_fill_colour_with_horizontal_line_mask( __arm_2d_sub_task_t *ptTask)
{
    ARM_2D_IMPL(arm_2d_op_fill_cl_msk_t, ptTask->ptOP);

    assert(ARM_2D_COLOUR_SZ_32BIT == OP_CORE.ptOp->Info.Colour.u3ColourSZ);

    arm_2d_region_t tTargetRegion = *(((arm_2d_op_t *)ptThis)->Target.ptRegion);

    tTargetRegion.tLocation 
        = arm_2d_get_absolute_location( ((arm_2d_op_t *)ptThis)->Target.ptTile,
                                        tTargetRegion.tLocation,
                                        true);

    __arm_2d_impl_cccn888_fill_colour_with_horizontal_line_mask(   ptTask->Param.tCopy.tSource.pBuffer,
                                                    ptTask->Param.tCopy.tSource.iStride,
                                                    ptTask->Param.tCopy.tTarget.pBuffer,
                                                    ptTask->Param.tCopy.tTarget.iStride,
                                                    &(ptTask->Param.tCopy.tTarget.tValidRegionInVirtualScreen),
                                                    &tTargetRegion,
                                                    &ptTask->Param.tCopy.tCopySize);

    return arm_fsm_rt_cpl;
}


/* default low level implementation */
__WEAK
void __arm_2d_impl_cccn888_fill_colour_with_horizontal_line_mask(
                                    uint32_t *__RESTRICT pwSource,
                                    int16_t iSourceStride,
                                    uint32_t *__RESTRICT pwTarget,
                                    int16_t iTargetStride,
                                    arm_2d_region_t *__RESTRICT ptValidRegionOnVirtualScreen,
                                    arm_2d_region_t *ptTargetRegionOnVirtualScreen,
                                    arm_2d_size_t *__RESTRICT ptCopySize)
{

    /* calculate the offset between the target region and the valid region */
    arm_2d_location_t tOffset = {
        .iX = ptValidRegionOnVirtualScreen->tLocation.iX - ptTargetRegionOnVirtualScreen->tLocation.iX,
        .iY = ptValidRegionOnVirtualScreen->tLocation.iY - ptTargetRegionOnVirtualScreen->tLocation.iY,
    };
    ARM_2D_UNUSED(tOffset);
    /*
         Virtual Screen
         +--------------------------------------------------------------+
         |                                                              |
         |        Target Region                                         |
         |       +-------------------------------------------+          |
         |       |                                           |          |
         |       |                  +-------------------+    |          |
         |       |                  | Valid Region      |    |          |
         |       |                  |                   |    |          |
         |       |                  +-------------------+    |          |     
         |       |                                           |          |
         |       |                                           |          |     
         |       +-------------------------------------------+          |
         +--------------------------------------------------------------+     
     
         NOTE: 1. Both the Target Region and the Valid Region are relative
                  regions of the virtual Screen in this function.
               2. The Valid region is always inside the Target Region.
               3. tOffset is the relative location between the Valid Region
                  and the Target Region.
               4. The Valid Region marks the location and size of the current
                  working buffer on the virtual screen. Only the valid region
                  contains a valid buffer.
     */

    int_fast16_t iWidth = ptCopySize->iWidth;
    int_fast16_t iHeight = ptCopySize->iHeight;

    uint_fast8_t chTargetChannel = 0;

    for (int_fast16_t iY = 0; iY < ptCopySize->iHeight; iY++) {

        uint32_t *pwSourceLine = pwSource;
        uint32_t *pwTargetLine = pwTarget;

        for (int_fast16_t iX = 0; iX < iWidth; iX++) {

            arm_2d_color_ccca8888_t tSourcePixel = {.tValue = *pwSourceLine++};
            arm_2d_color_ccca8888_t tTargetPixel = {0};

            tTargetPixel.u8C[chTargetChannel] = tSourcePixel.u8C[chTargetChannel];

            *pwTargetLine++ = tTargetPixel.tValue;

        }

        pwSource += iSourceStride;
        pwTarget += iTargetStride;
    }
}

/*
 * OPCODE Low Level Implementation Entries
 */
__WEAK
def_low_lv_io(  __ARM_2D_IO_FILL_COLOUR_WITH_HORIZONTAL_LINE_MASK_CCCN888,
                __arm_2d_cccn888_sw_fill_colour_with_horizontal_line_mask);      /* Default SW Implementation */


/*
 * OPCODE
 */
const __arm_2d_op_info_t ARM_2D_OP_FILL_COLOUR_WITH_HORIZONTAL_LINE_MASK_CCCN888 = {
    .Info = {
        .Colour = {
            .chScheme   = ARM_2D_COLOUR_CCCN888,
        },
        .Param = {
            .bHasSource     = true,
            .bHasTarget     = true,
        },
        .chOpIndex      = __ARM_2D_OP_IDX_FILL_COLOUR_WITH_HORIZONTAL_LINE_MASK,
        
        .LowLevelIO = {
            .ptCopyLike = ref_low_lv_io(__ARM_2D_IO_FILL_COLOUR_WITH_HORIZONTAL_LINE_MASK_CCCN888),
            .ptFillLike = NULL,
        },
    },
};


#ifdef   __cplusplus
}
#endif

#endif
