#include "gltf.h"

#include "glad.h"
#include <libgen.h>

/*
 * EV:Nova
 * -> key light is top left, slight elevation, pointed at ship. directional
 * light
 * -> 1 to 3 fill lights. Depends on ship. Point lights.
 */
const Lighting L_default_const = {
   .ambient_r = 0.,
   .ambient_g = 0.,
   .ambient_b = 0.,
   .intensity = 1.,
   .nlights   = 2,
   .lights =
      {
         {
            /* Key Light. */
            .colour = { .v = { 1., 1., 1. } },
            // Endless Sky (point) power: 150, pos: -12.339, 10.559, -11.787
            /*
             */
            .sun       = 0,
            .pos       = { .v = { -3., 2.75, -3. } },
            .intensity = 80.,
            // Sharky (directional) power: 5, direction: 10.75, -12.272, 7.463
            /*
               .sun = 1,
               .pos = { .v = { 12., 10.5, -12. } },
               .intensity = 2.5,
               */
         },
         {
            /* Fill light. */
            .colour = { .v = { 1., 1., 1. } },
            // Endless Sky (directional) power: 1.5,
            // direction: 9.772, 11.602, 6.988
            /*
             */
            .sun       = 1,
            .pos       = { .v = { 10., 11.5, 7. } },
            .intensity = 1.,
            // Sharky (point) power: 2000., position: -12.339, 10.559, 11.787
            /*
               .sun = 0,
               .pos = { .v = { -12.5, 10.5, 12. } },
               .intensity = 2000.,
               */
         },
         { 0 },
      },
};
const Lighting L_store_const = {
   .ambient_r = 0.1,
   .ambient_g = 0.1,
   .ambient_b = 0.1,
   .nlights   = 2,
   .intensity = 1.5,
   .lights =
      {
         {
            /* Key Light. */
            .colour    = { .v = { 1., 1., 1. } },
            .sun       = 0,
            .pos       = { .v = { -3., 2.75, -3. } },
            .intensity = 100.,
         },
         {
            /* Fill light. */
            .colour    = { .v = { 1., 1., 1. } },
            .sun       = 1,
            .pos       = { .v = { 10., 11.5, 7. } },
            .intensity = 1.,
         },
         { 0 },
      },
};
Lighting L_default;
