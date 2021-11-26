
#include "AL/efx.h"
#include "AL/efx-presets.h"

#include <math.h>
#include <stdio.h>

void print_p( const char *name, float value, float def )
{
   if (fabs(value-def) < 1e-5)
      return;

   printf( "      %s=%g,\n", name, value );
}

void print_params( const char *name, const EFXEAXREVERBPROPERTIES *p )
{
   printf( "function efx_preset.%s ()\n"
           "   return {\n"
           "      type=\"reverb\",\n", name );
   print_p( "gain", p->flGain, 0.3162 );
   print_p( "highgain", p->flGainHF, 0.8913 );
   print_p( "density", p->flDensity, 1.0 );
   print_p( "diffusion", p->flDiffusion, 1.0  );
   print_p( "decaytime", p->flDecayTime, 1.49 );
   print_p( "decayhighratio", p->flDecayHFRatio, 0.83 );
   print_p( "earlygain", p->flReflectionsGain, 0.05 );
   print_p( "earlydelay", p->flReflectionsDelay, 0.007 );
   print_p( "lategain", p->flLateReverbGain, 1.2589 );
   print_p( "latedelay", p->flLateReverbDelay, 0.011 );
   print_p( "roomrolloff", p->flRoomRolloffFactor, 0.0 );
   print_p( "airabsorption", p->flAirAbsorptionGainHF, 0.9943 );
   if (p->iDecayHFLimit != AL_TRUE)
      printf( "      %s=%d,\n", "highlimit", p->iDecayHFLimit );
   printf( "   }\n"
           "end\n" );
}
