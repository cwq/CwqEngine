/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2013 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
 */

/* This provides the default mixing callback for the SDL audio routines */

#include "audio_mixer.h"
#include <stdlib.h>

void AUDIO_MixAudioFormat(uint8_t * dst, const uint8_t * src,
		uint32_t len, int volume)
{
    if (volume == 0 || dst == NULL || src == NULL) {
        return;
    }

    short* pDest = (short*)dst;
    short* pSrc = (short*)src;
    
	int bitOffset = 16;
    long bitMax = 65536;
    long  bitMid = 32768;
	long nDiv = (32767);
    
    int n;
    long nLength = len/2;
	for ( n=0;n<nLength;n++)
	{
		int value1 = pDest[n];
		int value2 = pSrc[n];
		long sValue =0;
        
		/*
         if( value1 < 0 && value2 < 0)
         sValue = value1+value2 + (value1 * value2 / nDiv);
         else
         sValue = value1+value2 - (value1 * value2 / nDiv);
         */
        //		/*
		int sign1 = (value1 == 0)? 0 : abs(value1)/value1;
		int sign2 = (value2 == 0)? 0 : abs(value2)/value2;
		
		if (sign1 == sign2)
		{
			if( value1 < 0 && value2 < 0)
				sValue = value1+value2 + (value1 * value2 / nDiv);
			else
				sValue = value1+value2 - (value1 * value2 / nDiv);
		}
		else
		{
			long tmpValue1 = value1 + bitMid;
			long tmpValue2 = value2 + bitMid;
			
			long tmp = ((tmpValue1 * tmpValue2) >> (bitOffset -1));
			
			sValue = 2 * (tmpValue1  + tmpValue2 ) - tmp - bitMax - bitMid;
		}
		long nTemp = sValue>0?sValue:-sValue;
		if (nTemp >= bitMid)
		{
			long sign = nTemp/sValue;
			
			sValue = sign * (bitMid -  1);
		}
		/**/
        
		pDest[n] = sValue;
	}
/*
    int16_t src1, src2;
    int dst_sample;
    const int max_audioval = ((1 << (16 - 1)) - 1);
    const int min_audioval = -(1 << (16 - 1));

    len /= 2;
    while (len--) {
        src1 = ((src[1]) << 8 | src[0]);
        ADJUST_VOLUME(src1, volume);
        src2 = ((dst[1]) << 8 | dst[0]);
        src += 2;
        dst_sample = src1 + src2;
        if (dst_sample > max_audioval) {
            dst_sample = max_audioval;
        } else if (dst_sample < min_audioval) {
            dst_sample = min_audioval;
        }
        dst[0] = dst_sample & 0xFF;
        dst_sample >>= 8;
        dst[1] = dst_sample & 0xFF;
        dst += 2;
    }
 */
}

void AUDIO_AudioVolume(uint8_t * src, uint32_t len, int volume)
{
    if (volume<0 || volume>AUDIO_MIX_MAXVOLUME) {
        return;
    }
	else if (volume ==0)
	{
//		int16_t src1;

		len /= 2;
		short* dest = (short*)src;
		while (len--)
		{
			dest[0] = 0;
			dest++;
		}
	}
	else
	{
//		int16_t src1;

		len /= 2;
		short* dest = (short*)src;
		while (len--)
		{
			dest[0] = dest[0] * volume/AUDIO_MIX_MAXVOLUME;
			dest++;
		}

	}
	

	
	/*
    while (len--) {
        src1 = ((src[1]) << 8 | src[0]);
        ADJUST_VOLUME(src1, volume);

        src[0] = src1 & 0xFF;
        src1 >>= 8;
        src[1] = src1 & 0xFF;

        src += 2;
    }
	*/
}
