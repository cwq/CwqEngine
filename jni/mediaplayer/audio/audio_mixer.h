/*****************************************************************************
 * ijksdl_mixer.h
 *****************************************************************************
 *
 * copyright (c) 2013 Zhang Rui <bbcallen@gmail.com>
 *
 * This file is part of ijkPlayer.
 *
 * ijkPlayer is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * ijkPlayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with ijkPlayer; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef SDL_MIXER_H_
#define SDL_MIXER_H_

#include <stdint.h>

#define BG_VOLUME_MAX_LEVEL     32
#define BG_VOLUME_DEFAULT_LEVEL 4

#define AUDIO_MIX_MAXVOLUME 128

/* The volume ranges from 0 - 128 */
#define ADJUST_VOLUME(s, v) (s = (s*v)/AUDIO_MIX_MAXVOLUME)

void AUDIO_MixAudioFormat(uint8_t * dst, const uint8_t * src,
		uint32_t len, int volume);

void AUDIO_AudioVolume(uint8_t * src, uint32_t len, int volume);

#endif /* SDL_MIXER_H_ */
