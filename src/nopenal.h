/*
 * Copyright 2006-2012 Edgar Simo Serra
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#if USE_OPENAL


#ifndef NOPENAL_H
#  define NOPENAL_H

#include "ncompat.h"

#include OPENAL_ALC_H
#include OPENAL_AL_H

/*
 * EFX stuff.
 */
#ifndef ALC_EXT_EFX
/* Auxiliary Effect Slot. */
#define AL_EFFECTSLOT_NULL                                 0x0000
#define AL_EFFECTSLOT_EFFECT                               0x0001
#define AL_EFFECTSLOT_GAIN                                 0x0002
#define AL_EFFECTSLOT_AUXILIARY_SEND_AUTO                  0x0003
/* Filters. */
#define AL_FILTER_TYPE                                     0x8001
#define AL_FILTER_NULL                                     0x0000
#define AL_FILTER_LOWPASS                                  0x0001
#define AL_FILTER_HIGHPASS                                 0x0002
#define AL_FILTER_BANDPASS                                 0x0003
/* Effects. */
#define AL_EFFECT_TYPE                                     0x8001
#define AL_EFFECT_NULL                                     0x0000
#define AL_EFFECT_EAXREVERB                                0x8000
#define AL_EFFECT_REVERB                                   0x0001
#define AL_EFFECT_CHORUS                                   0x0002
#define AL_EFFECT_DISTORTION                               0x0003
#define AL_EFFECT_ECHO                                     0x0004
#define AL_EFFECT_FLANGER                                  0x0005
#define AL_EFFECT_FREQUENCY_SHIFTER                        0x0006
#define AL_EFFECT_VOCAL_MORPHER                            0x0007
#define AL_EFFECT_PITCH_SHIFTER                            0x0008
#define AL_EFFECT_RING_MODULATOR                           0x0009
#define AL_EFFECT_AUTOWAH                                  0x000A
#define AL_EFFECT_COMPRESSOR                               0x000B
#define AL_EFFECT_EQUALIZER                                0x000C
/* Reverb Effect. */
#define AL_REVERB_DENSITY                                  0x0001
#define AL_REVERB_DIFFUSION                                0x0002
#define AL_REVERB_GAIN                                     0x0003
#define AL_REVERB_GAINHF                                   0x0004
#define AL_REVERB_DECAY_TIME                               0x0005
#define AL_REVERB_DECAY_HFRATIO                            0x0006
#define AL_REVERB_REFLECTIONS_GAIN                         0x0007
#define AL_REVERB_REFLECTIONS_DELAY                        0x0008
#define AL_REVERB_LATE_REVERB_GAIN                         0x0009
#define AL_REVERB_LATE_REVERB_DELAY                        0x000A
#define AL_REVERB_AIR_ABSORPTION_GAINHF                    0x000B
#define AL_REVERB_ROOM_ROLLOFF_FACTOR                      0x000C
#define AL_REVERB_DECAY_HFLIMIT                            0x000D
/* Echo Effect. */
#define AL_ECHO_DELAY                                      0x0001
#define AL_ECHO_LRDELAY                                    0x0002
#define AL_ECHO_DAMPING                                    0x0003
#define AL_ECHO_FEEDBACK                                   0x0004
#define AL_ECHO_SPREAD                                     0x0005
/* Listener Object Extensions. */
#define AL_METERS_PER_UNIT                                 0x20004
/* Source Object Extensions. */
#define AL_DIRECT_FILTER                                   0x20005
#define AL_AUXILIARY_SEND_FILTER                           0x20006
#define AL_AIR_ABSORPTION_FACTOR                           0x20007
#define AL_ROOM_ROLLOFF_FACTOR                             0x20008
#define AL_CONE_OUTER_GAINHF                               0x20009
#define AL_DIRECT_FILTER_GAINHF_AUTO                       0x2000A
#define AL_AUXILIARY_SEND_FILTER_GAIN_AUTO                 0x2000B
#define AL_AUXILIARY_SEND_FILTER_GAINHF_AUTO               0x2000C
/* Context Object Extensions. */
#define ALC_EFX_MAJOR_VERSION                              0x20001
#define ALC_EFX_MINOR_VERSION                              0x20002
#define ALC_MAX_AUXILIARY_SENDS                            0x20003
#endif
/* Auxiliary Effect Slot. */
ALvoid (AL_APIENTRY *nalGenAuxiliaryEffectSlots)(ALsizei,ALuint*);
ALvoid (AL_APIENTRY *nalDeleteAuxiliaryEffectSlots)(ALsizei,ALuint*);
ALboolean (AL_APIENTRY *nalIsAuxiliaryEffectSlot)(ALuint);
ALvoid (AL_APIENTRY *nalAuxiliaryEffectSloti)(ALuint,ALenum,ALint);
ALvoid (AL_APIENTRY *nalAuxiliaryEffectSlotiv)(ALuint,ALenum,ALint*);
ALvoid (AL_APIENTRY *nalAuxiliaryEffectSlotf)(ALuint,ALenum,ALfloat);
ALvoid (AL_APIENTRY *nalAuxiliaryEffectSlotfv)(ALuint,ALenum,ALfloat*);
ALvoid (AL_APIENTRY *nalGetAuxiliaryEffectSloti)(ALuint,ALenum,ALint*);
ALvoid (AL_APIENTRY *nalGetAuxiliaryEffectSlotiv)(ALuint,ALenum,ALint*);
ALvoid (AL_APIENTRY *nalGetAuxiliaryEffectSlotf)(ALuint,ALenum,ALfloat*);
ALvoid (AL_APIENTRY *nalGetAuxiliaryEffectSlotfv)(ALuint,ALenum,ALfloat*);
/* Filter. */
ALvoid (AL_APIENTRY *nalGenFilters)(ALsizei,ALuint*);
ALvoid (AL_APIENTRY *nalDeleteFilters)(ALsizei,ALuint*);
ALvoid (AL_APIENTRY *nalFilteri)(ALuint,ALenum,ALint);
ALvoid (AL_APIENTRY *nalFilteriv)(ALuint,ALenum,ALint*);
ALvoid (AL_APIENTRY *nalFilterf)(ALuint,ALenum,ALfloat);
ALvoid (AL_APIENTRY *nalFilterfv)(ALuint,ALenum,ALfloat*);
/* Effect. */
ALvoid (AL_APIENTRY *nalGenEffects)(ALsizei,ALuint*);
ALvoid (AL_APIENTRY *nalDeleteEffects)(ALsizei,ALuint*);
ALvoid (AL_APIENTRY *nalEffecti)(ALuint,ALenum,ALint);
ALvoid (AL_APIENTRY *nalEffectiv)(ALuint,ALenum,ALint*);
ALvoid (AL_APIENTRY *nalEffectf)(ALuint,ALenum,ALfloat);
ALvoid (AL_APIENTRY *nalEffectfv)(ALuint,ALenum,ALfloat*);


#endif /* NOPENAL_H */

#endif /* USE_OPENAL */


