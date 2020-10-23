/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file glue_macos.m
 *
 * @brief Support code for macOS
 *
 * The functions here deal with the macOS parts that call into Objective-C land.
 */


#include "glue_macos.h"
#include <Foundation/Foundation.h>


/**
 * @brief Write an NSString to a C buffer
 */
static int macos_writeString ( NSString *str, char *res, size_t n )
{
   BOOL ok = [str getCString:res
                   maxLength:n
                    encoding:NSUTF8StringEncoding];
   return ok ? 0 : -1;
}


/**
 * @brief Determine if we're running from inside an app bundle
 */
int macos_isBundle ()
{
   NSString *path = [[NSBundle mainBundle] bundlePath];
   return [path hasSuffix:@".app"] ? 1 : 0;
}

/**
 * @brief Get the path to the bundle resources directory
 */
int macos_resourcesPath ( char *res, size_t n )
{
   NSString *path = [[NSBundle mainBundle] resourcePath];
   return macos_writeString( path, res, n );
}


/**
 * @brief Get the path to the specified user directory
 */
static int macos_userLibraryDir ( NSString *kind, char *res, size_t n )
{
   NSString *path = [@[
      NSHomeDirectory(),
      @"/Library/",
      kind,
      @"/org.naev.Naev/"
   ] componentsJoinedByString:@""];
   return macos_writeString( path, res, n );
}


/**
 * @brief Get the config directory path
 */
int macos_configPath ( char *res, size_t n )
{
   return macos_userLibraryDir( @"Preferences", res, n );
}


/**
 * @brief Get the data directory path
 */
int macos_dataPath ( char *res, size_t n )
{
   return macos_userLibraryDir( @"Application Support", res, n );
}


/**
 * @brief Get the cache directory path
 */
int macos_cachePath ( char *res, size_t n )
{
   return macos_userLibraryDir( @"Caches", res, n );
}
