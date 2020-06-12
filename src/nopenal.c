/*
 * See Licensing and Copyright notice in naev.h
 */
#if USE_OPENAL

#include "nopenal.h"

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

#endif /* USE_OPENAL */