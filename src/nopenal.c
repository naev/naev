/*
 * See Licensing and Copyright notice in naev.h
 */
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


#ifdef DEBUGGING
/**
 * @brief Converts an OpenAL error to a string.
 *
 *    @param err Error to convert to string.
 *    @return String corresponding to the error.
 */
void al_checkHandleError( const char *func )
{
   ALenum err;
   const char *errstr;

   /* Get the possible error. */
   err = alGetError();

   /* No error. */
   if (err == AL_NO_ERROR)
      return;

   /* Get the message. */
   switch (err) {
      case AL_INVALID_NAME:
         errstr = _("a bad name (ID) was passed to an OpenAL function");
         break;
      case AL_INVALID_ENUM:
         errstr = _("an invalid enum value was passed to an OpenAL function");
         break;
      case AL_INVALID_VALUE:
         errstr = _("an invalid value was passed to an OpenAL function");
         break;
      case AL_INVALID_OPERATION:
         errstr = _("the requested operation is not valid");
         break;
      case AL_OUT_OF_MEMORY:
         errstr = _("the requested operation resulted in OpenAL running out of memory");
         break;

      default:
         errstr = _("unknown error");
         break;
   }
   WARN(_("OpenAL error [%s]: %s"), func, errstr);
}
#endif /* DEBUGGING */
