
/*******************************************************************************
 *
 * Filename:
 * ---------
 * audio_acf_default.h
 *
 * Project:
 * --------
 *   ALPS
 *
 * Description:
 * ------------
 * This file is the header of audio customization related parameters or definition.
 *
 * Author:
 * -------
 * Tina Tsai
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 *
 *
 *
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
#ifndef AUDIO_ACF_DEFAULT_H
#define AUDIO_ACF_DEFAULT_H

   /* Compensation Filter HSF coeffs: default all pass filter       */
    /* BesLoudness also uses this coeffs    */ 
    #define BES_LOUDNESS_HSF_COEFF \
0x0,   0x0,   0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,   0x0,   0x0,     \
    \
0x0,   0x0,   0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,   0x0,   0x0

#define BES_LOUDNESS_BPF_COEFF \
0x4000825c,   0x3e587da3,   0xc1a70000,     \
0x400082a2,   0x3e337d5d,   0xc1cc0000,     \
0x4000840a,   0x3d887bf5,   0xc2770000,     \
0x4000860b,   0x3cbb79f4,   0xc3440000,     \
0x400086d4,   0x3c73792b,   0xc38c0000,     \
0x40008afc,   0x3b297503,   0xc4d60000,     \
    \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
    \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
    \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
    \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
    \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
\    
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
\
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0

#define BES_LOUDNESS_LPF_COEFF \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0
#define BES_LOUDNESS_WS_GAIN_MAX  0x0
#define BES_LOUDNESS_WS_GAIN_MIN  0x0
#define BES_LOUDNESS_FILTER_FIRST  0x0
#define BES_LOUDNESS_ATT_TIME  0x0
#define BES_LOUDNESS_REL_TIME  0x0
    #define BES_LOUDNESS_GAIN_MAP_IN \
0xd3, 0xda, 0xed, 0xee, 0x0
#define BES_LOUDNESS_GAIN_MAP_OUT \
0x0, 0x0, 0x0, 0x0, 0x0
#endif
