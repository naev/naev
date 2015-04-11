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


#ifndef NFILE_H
#  define NFILE_H


const char* nfile_dataPath (void);
const char* nfile_configPath (void);
const char* nfile_cachePath (void);
char* nfile_dirname( char *path );
int nfile_dirMakeExist( const char* path, ... ); /* Creates if doesn't exist, 0 success */
int nfile_dirExists( const char* path, ... ); /* Returns 1 on exists. */
int nfile_fileExists( const char* path, ... ); /* Returns 1 on exists */
int nfile_backupIfExists( const char* path, ... );
char** nfile_readDir( int* nfiles, const char* path, ... );
char** nfile_readDirRecursive( int* nfiles, const char* path, ... );
char* nfile_readFile( int* filesize, const char* path, ... );
int nfile_touch( const char* path, ... );
int nfile_writeFile( const char* data, int len, const char* path, ... );
int nfile_delete( const char* file );
int nfile_rename( const char* oldname, const char* newname );


#endif /* NFILE_H */

