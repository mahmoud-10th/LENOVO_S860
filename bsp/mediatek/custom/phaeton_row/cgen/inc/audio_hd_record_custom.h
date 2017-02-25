
/*******************************************************************************
 *
 * Filename:
 * ---------
 * aud_hd_record_custom.h
 *
 * Project:
 * --------
 *   ALPS
 *
 * Description:
 * ------------
 * This file is the header of audio customization related function or definition.
 *
 * Author:
 * -------
 * Charlie Lu.
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
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
#ifndef AUDIO_HD_RECORD_CUSTOM_H
#define AUDIO_HD_RECORD_CUSTOM_H

//hd_rec_fir
/* 0: Input FIR coefficients for Normal mode */
/* 1: Input FIR coefficients for Headset mode */
/* 2: Input FIR coefficients for Handfree mode */
/* 3: Input FIR coefficients for BT mode */
/* 4: Input FIR coefficients for Normal mode */
/* 5: Input FIR coefficients for Handfree mode */
/* 6: Input FIR coefficients for Normal mode 2nd Mic*/
/* 7: Input FIR coefficients for Voice Recognition*/
/* 8: Input FIR coefficients for Voice Unlock Main Mic */
/* 9: Input FIR coefficients for Voice Unlock 2nd Mic */
/* 10: Input FIR coefficients for 1st customization Main Mic*/
/* 11: Input FIR coefficients for 1st customization 2nd Mic*/
/* 12: Input FIR coefficients for 2nd customization Main Mic*/
/* 13: Input FIR coefficients for 2nd customization 2nd Mic*/
/* 14: Input FIR coefficients for 3rd customization Main Mic*/
/* 15: Input FIR coefficients for 3rd customization Ref Mic*/

#define DEFAULT_HD_RECORD_FIR_Coeff \
    32767,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,  \
\
    32767,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,  \
\
    32767,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,  \
\
    32767,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,  \
\
    32767,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,  \
\
    32767,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
\
    32767,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
\
	32767,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0,  \
\
	32767,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0,  \
\
	32767,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0,  \
\
	32767,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0,  \
\
	32767,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
\
	32767,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0,  \
\
   -146,    209,    345,    272,     -5,    -76,   -279,   -150,     64,    303, \
    200,    104,     88,   -127,   -301,    316,    127,   -503,   -602,    637, \
    -89,   -877,   -396,   -916,   -121,    968,   1111,   1219,    473,  -1561, \
   -589,  -2906,  -2175,    473,   2757,   2896,   4696,   2203,   3283,  -6236, \
  -2626,  -4041,    477,  -4814,  22367,  22367,  -4814,    477,  -4041,  -2626, \
  -6236,   3283,   2203,   4696,   2896,   2757,    473,  -2175,  -2906,   -589, \
  -1561,    473,   1219,   1111,    968,   -121,   -916,   -396,   -877,    -89, \
    637,   -602,   -503,    127,    316,   -301,   -127,     88,    104,    200, \
    303,     64,   -150,   -279,    -76,     -5,    272,    345,    209,   -146, \
\
	32767,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
		0,	   0,	  0,	 0, 	0,	   0,	  0,	 0, 	0,	   0, \
\
    32767,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0




#define DEFAULT_HD_RECORD_MODE_FIR_MAPPING_CH1 \
    7, 1, 3, \
    0, 1, 3, \
    0, 1, 3, \
    0, 1, 3, \
    0, 1, 3, \
    0, 1, 3, \
    8, 1, 3, \
    10, 1, 3, \
    12, 1, 3, \
    14, 1, 3

#define DEFAULT_HD_RECORD_MODE_FIR_MAPPING_CH2 \
    6, 6, 6, \
    6, 6, 6, \
    6, 6, 6, \
    6, 6, 6, \
    6, 6, 6, \
    6, 6, 6, \
    9, 6, 6, \
    11, 6, 6, \
    13, 6, 6, \
    15, 6, 6

#endif