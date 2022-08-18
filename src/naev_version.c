/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file naev_version.c
 *
 * @brief Implements naev_version() in a separate compilation unit, so that a "git pull" only forces a recompile of this tiny piece plus a relink.
 */
/** @cond */
#include "naev.h"
#include "naev_build_version.h"
/** @endcond */

#include "gettext.h"
#include "start.h"

static char version_human[STRMAX_SHORT]; /**< Human readable version. */

/**
 * @brief Returns the version in a human readable string.
 *
 *    @param long_version Returns the long version if it's long.
 *    @return The human readable version string.
 */
const char *naev_version( int long_version )
{
   /* Set up the long version. */
   if (long_version) {
      if (version_human[0] == '\0')
         snprintf( version_human, sizeof(version_human),
               " "APPNAME" v%s%s - %s", VERSION,
#ifdef DEBUGGING
               _(" debug"),
#else /* DEBUGGING */
               "",
#endif /* DEBUGGING */
               start_name() );
      return version_human;
   }

   return VERSION;
}
