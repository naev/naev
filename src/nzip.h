/*
 * Copyright 2006-2012 nloewen
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


#ifndef NZIP_H
#define NZIP_H

#include <zip.h>
#include <SDL.h>

int nzip_isZip ( const char* filename );
struct zip* nzip_open ( const char* filename );
void nzip_close ( struct zip* arc );

int nzip_hasFile ( struct zip* arc, const char* filename );
void* nzip_readFile ( struct zip* arc, const char* filename, uint32_t* size );
char** nzip_listFiles ( struct zip* arc, uint32_t* nfiles );

SDL_RWops* nzip_rwops ( struct zip* arc, const char* filename );

#endif
