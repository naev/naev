// clang-format off
/**
 * SDL_PhysFS.h v3.0.0 - PhysFS virtual file system support for SDL.
 *
 * https://github.com/RobLoach/SDL_PhysFS
 *
 * @license zlib
 *
 * Copyright (c) 2024 Rob Loach <https://robloach.net>
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * Modified for naev by:
 * 1. Exposing SDL_PhysFS_OpenIO as part of the API.
 */
#ifndef SDL_PHYSFS_H__
#define SDL_PHYSFS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <SDL3/SDL.h>
#include "physfs.h"

#ifndef SDL_PHYSFS_DEF
#ifdef SDL_PHYSFS_STATIC
#define SDL_PHYSFS_DEF static
#else
#define SDL_PHYSFS_DEF extern
#endif
#endif

SDL_PHYSFS_DEF bool SDL_PhysFS_Init(const char* argv);
SDL_PHYSFS_DEF bool SDL_PhysFS_InitEx(const char* argv, const char* org, const char* app);
SDL_PHYSFS_DEF bool SDL_PhysFS_Quit();
SDL_PHYSFS_DEF bool SDL_PhysFS_Mount(const char* newDir, const char* mountPoint);
SDL_PHYSFS_DEF bool SDL_PhysFS_MountFromMemory(const unsigned char *fileData, int dataSize, const char* newDir, const char* mountPoint);
SDL_PHYSFS_DEF bool SDL_PhysFS_Unmount(const char* oldDir);
SDL_PHYSFS_DEF SDL_IOStream* SDL_PhysFS_IOFromFile(const char* filename);
SDL_PHYSFS_DEF SDL_Surface* SDL_PhysFS_LoadBMP(const char* filename);
SDL_PHYSFS_DEF bool SDL_PhysFS_LoadWAV(const char* filename, SDL_AudioSpec * spec, Uint8 ** audio_buf, Uint32 * audio_len);
SDL_PHYSFS_DEF void* SDL_PhysFS_LoadFile(const char* filename, size_t *datasize);
SDL_PHYSFS_DEF size_t SDL_PhysFS_Write(const char* file, const void* buffer, size_t size);
SDL_PHYSFS_DEF bool SDL_PhysFS_SetWriteDir(const char* path);
SDL_PHYSFS_DEF char** SDL_PhysFS_LoadDirectoryFiles(const char *directory);
SDL_PHYSFS_DEF void SDL_PhysFS_FreeDirectoryFiles(char** files);
SDL_PHYSFS_DEF bool SDL_PhysFS_Exists(const char* file);
SDL_PHYSFS_DEF SDL_IOStatus SDL_PhysFS_IOStatus(int error);

/* Our modifications. */
SDL_IOStream *SDL_PhysFS_OpenIO(PHYSFS_File *handle);

#ifndef SDL_PhysFS_IMG_Load
/**
 * Load an image through PhysFS with SDL_image.
 *
 * @param filename A "const char*" representing the file to load from "PhysFS".
 *
 * @return SDL_Surface*, or NULL on failure. Use "SDL_GetError()" to get more information.
 */
#define SDL_PhysFS_IMG_Load(filename) (IMG_Load_RW(SDL_PhysFS_IOFromFile(filename), true))
#endif  // SDL_PhysFS_IMG_Load

#ifndef SDL_PhysFS_STBIMG_Load
/**
 * Integration with SDL_stbimage.h for image loading support with stb_image.h.
 *
 * Be sure to include both SDL_PhysFS.h and SDL_stbimage.h for this to function properly.
 *
 * @param filename A "const char*" representing the file to load from "PhysFS".
 *
 * @return SDL_Surface*, or NULL on failure. Use "SDL_GetError()" to get more information.
 *
 * @see https://github.com/DanielGibson/Snippets/blob/master/SDL_stbimage.h
 */
#define SDL_PhysFS_STBIMG_Load(filename) (STBIMG_Load_RW(SDL_PhysFS_IOFromFile(filename), 1))
#endif  // SDL_PhysFS_STBIMG_Load

#ifndef SDL_PhysFS_Mix_LoadMUS
/**
 * Load a supported audio format with SDL_mixer into a music object through PhysFS.
 *
 * @param filename A "const char*" representing the file to load from "PhysFS".
 *
 * @return Mix_Music*, or NULL on failure. Use "SDL_GetError()" to get more information.
 *
 * @see https://github.com/libsdl-org/SDL_mixer
 */
#define SDL_PhysFS_Mix_LoadMUS(filename) (Mix_LoadMUS_RW(SDL_PhysFS_IOFromFile(filename), 1))
#endif  // SDL_PhysFS_Mix_LoadMUS

#ifndef SDL_PhysFS_TTF_OpenFont
/**
 * Load a font with SDL_ttf from PhysFS, at the given size.
 *
 * @param filename A const char* representing the file to load from PhysFS.
 * @param ptsize Integer point size to use for the newly-opened font
 *
 * @return TTF_Font*, or NULL on failure.
 *
 * @see https://wiki.libsdl.org/SDL2_ttf/TTF_OpenFontRW
 */
#define SDL_PhysFS_TTF_OpenFont(filename, ptsize) (TTF_OpenFontRW(SDL_PhysFS_IOFromFile(filename), 1, ptsize))
#endif  // SDL_PhysFS_TTF_OpenFont

#ifdef __cplusplus
}
#endif

#endif  // SDL_PHYSFS_H__

#ifdef SDL_PHYSFS_IMPLEMENTATION
#ifndef SDL_PHYSFS_IMPLEMENTATION_ONCE
#define SDL_PHYSFS_IMPLEMENTATION_ONCE

#ifdef __cplusplus
extern "C" {
#endif

#include <SDL3/SDL.h>

// physfs.h
#ifndef SDL_PHYSFS_PHYSFS_H
#define SDL_PHYSFS_PHYSFS_H "physfs.h"
#endif
#include SDL_PHYSFS_PHYSFS_H

#ifndef SDL_PhysFS_SetError
/**
 * Reports the latest PhysFS error to SDL.
 *
 * @param description A description of the error that occurred.
 */
#define SDL_PhysFS_SetError(description) do { SDL_SetError("SDL_PhysFS.h:%d: %s (%s)", __LINE__, description, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode())); } while(0)
#endif

void* SDL_PhysFS_AllocatorMalloc(PHYSFS_uint64 size) {
    SDL_malloc_func malloc_func;
    SDL_GetMemoryFunctions(&malloc_func, NULL, NULL, NULL);
    return malloc_func((size_t)size);
}

void* SDL_PhysFS_AllocatorRealloc(void* mem, PHYSFS_uint64 size) {
    SDL_realloc_func realloc_func;
    SDL_GetMemoryFunctions(NULL, NULL, &realloc_func, NULL);
    return realloc_func(mem, (size_t)size);
}

/**
 * Initialize the PhysFS virtual file system.
 *
 * @return true on success, false otherwise.
 *
 * @see SDL_PhysFS_Quit()
 */
bool SDL_PhysFS_Init(const char* argv) {
    // Set up the memory functions.
    PHYSFS_Allocator allocator;
    allocator.Init = NULL;
    allocator.Deinit = NULL;
    allocator.Malloc = &SDL_PhysFS_AllocatorMalloc;
    allocator.Realloc = &SDL_PhysFS_AllocatorRealloc;
    allocator.Free = &SDL_free;
    PHYSFS_setAllocator(&allocator);

    if (PHYSFS_init(argv) == 0) {
        SDL_PhysFS_SetError("Failed to initialize PhysFS");
        return false;
    }

    return true;
}

/**
 * Initializes the PhysFS virtual file system with access to SDL's preferences directory as /app.
 *
 * Mounts the writable preferences directory as "/app".
 *
 * @param org The name of your organization.
 * @param app The name of your application.
 *
 * @return true on success, false otherwise.
 *
 * @see SDL_PhysFS_Init()
 */
bool SDL_PhysFS_InitEx(const char* argv, const char* org, const char* app) {
    // Initialize
    if (SDL_PhysFS_Init(argv) == false) {
        return false;
    }

    // Find the preferences path.
    char* path = SDL_GetPrefPath(org, app);
    if (path == NULL) {
        SDL_PhysFS_Quit();
        SDL_PhysFS_SetError("Failed to find SDL's pref directory");
        return false;
    }

    // Set up the write directory as /app
    SDL_PhysFS_SetWriteDir(path);
    SDL_PhysFS_Mount(path, "app");

    return true;
}

/**
 * Close the PhysFS virtual file system.
 *
 * @return true on success, false otherwise.
 */
bool SDL_PhysFS_Quit() {
    if (PHYSFS_deinit() == 0) {
        SDL_PhysFS_SetError("Failed to deinitialize PhysFS");
        return false;
    }

    // Remove the SDL allocator.
    PHYSFS_setAllocator(NULL);

    return true;
}

SDL_IOStatus SDL_PhysFS_IOStatus(int error) {
    switch ((PHYSFS_ErrorCode)error) {
        case PHYSFS_ERR_OK: return SDL_IO_STATUS_READY;
        case PHYSFS_ERR_OTHER_ERROR: return SDL_IO_STATUS_ERROR;
        case PHYSFS_ERR_OUT_OF_MEMORY: return SDL_IO_STATUS_NOT_READY;
        case PHYSFS_ERR_NOT_INITIALIZED: return SDL_IO_STATUS_NOT_READY;
        case PHYSFS_ERR_IS_INITIALIZED: return SDL_IO_STATUS_ERROR;
        case PHYSFS_ERR_ARGV0_IS_NULL: return SDL_IO_STATUS_ERROR;
        case PHYSFS_ERR_UNSUPPORTED: return SDL_IO_STATUS_NOT_READY;
        case PHYSFS_ERR_PAST_EOF: return SDL_IO_STATUS_EOF;
        case PHYSFS_ERR_FILES_STILL_OPEN: return SDL_IO_STATUS_ERROR;
        case PHYSFS_ERR_INVALID_ARGUMENT: return SDL_IO_STATUS_ERROR;
        case PHYSFS_ERR_NOT_MOUNTED: return SDL_IO_STATUS_ERROR;
        case PHYSFS_ERR_NOT_FOUND: return SDL_IO_STATUS_ERROR;
        case PHYSFS_ERR_SYMLINK_FORBIDDEN: return SDL_IO_STATUS_ERROR;
        case PHYSFS_ERR_NO_WRITE_DIR: return SDL_IO_STATUS_ERROR;
        case PHYSFS_ERR_OPEN_FOR_READING: return SDL_IO_STATUS_ERROR;
        case PHYSFS_ERR_OPEN_FOR_WRITING: return SDL_IO_STATUS_ERROR;
        case PHYSFS_ERR_NOT_A_FILE: return SDL_IO_STATUS_ERROR;
        case PHYSFS_ERR_READ_ONLY: return SDL_IO_STATUS_ERROR;
        case PHYSFS_ERR_CORRUPT: return SDL_IO_STATUS_ERROR;
        case PHYSFS_ERR_SYMLINK_LOOP: return SDL_IO_STATUS_ERROR;
        case PHYSFS_ERR_IO: return SDL_IO_STATUS_ERROR;
        case PHYSFS_ERR_PERMISSION: return SDL_IO_STATUS_ERROR;
        case PHYSFS_ERR_NO_SPACE: return SDL_IO_STATUS_ERROR;
        case PHYSFS_ERR_BAD_FILENAME: return SDL_IO_STATUS_ERROR;
        case PHYSFS_ERR_BUSY: return SDL_IO_STATUS_ERROR;
        case PHYSFS_ERR_DIR_NOT_EMPTY: return SDL_IO_STATUS_ERROR;
        case PHYSFS_ERR_OS_ERROR: return SDL_IO_STATUS_ERROR;
        case PHYSFS_ERR_DUPLICATE: return SDL_IO_STATUS_ERROR;
        case PHYSFS_ERR_BAD_PASSWORD: return SDL_IO_STATUS_ERROR;
        case PHYSFS_ERR_APP_CALLBACK: return SDL_IO_STATUS_ERROR;
    }

    return SDL_IO_STATUS_READY;
}

/**
 * Mounts the given directory, at the given mount point.
 *
 * @param newDir Directory or archive to add to the path, in platform-dependent notation.
 * @param mountPoint Location in the interpolated tree that this archive will be "mounted", in platform-independent notation. NULL or "" is equivalent to "/".
 *
 * @return true on success, false otherwise.
 *
 * @see SDL_PhysFS_Unmount()
 */
bool SDL_PhysFS_Mount(const char* newDir, const char* mountPoint) {
    if (PHYSFS_mount(newDir, mountPoint, 1) == 0) {
        SDL_PhysFS_SetError("Failed to mount");
        return false;
    }

    return true;
}

/**
 * Mounts the given file data as a mount point in PhysFS.
 *
 * @param fileData The archive data as a file buffer.
 * @param dataSize The size of the file buffer.
 * @param newDir A filename that can represent the file data. Has to be unique. For example: data.zip
 * @param mountPoint The location in the tree that the archive will be mounted.
 *
 * @return true on success, false otherwise.
 *
 * @see SDL_PhysFS_Mount()
 */
bool SDL_PhysFS_MountFromMemory(const unsigned char *fileData, int dataSize, const char* newDir, const char* mountPoint) {
    if (dataSize <= 0) {
        SDL_SetError("SDL_PhysFS_MountFromMemory: Cannot mount a data size of 0");
        return false;
    }

    if (PHYSFS_mountMemory(fileData, (PHYSFS_uint64)dataSize, 0, newDir, mountPoint, 1) == 0) {
        SDL_PhysFS_SetError("Failed to mount internal memory");
        return false;
    }

    return true;
}

/**
 * Unmounts the given directory or archive.
 *
 * @param oldDir The directory that was supplied to "MountPhysFS's" "newDir".
 *
 * @return true on success, false otherwise.
 *
 * @see SDL_PhysFS_Mount()
 */
bool SDL_PhysFS_Unmount(const char* oldDir) {
    if (PHYSFS_unmount(oldDir) == 0) {
        SDL_PhysFS_SetError("Failed to unmount old directory");
        return false;
    }

    return true;
}

/**
 * SDL_IOStream callback: size.
 *
 * @internal
 */
Sint64 SDLCALL SDL_PhysFS_GetIOSize(void *userdata) {
    PHYSFS_File *handle = (PHYSFS_File *)userdata;
    if (handle == NULL) {
        return 0;
    }

    return (Sint64) PHYSFS_fileLength(handle);
}

/**
 * SDL_IOStream callback: seek.
 *
 * @internal
 */
Sint64 SDLCALL SDL_PhysFS_SeekIO(void *userdata, Sint64 offset, SDL_IOWhence whence) {
    PHYSFS_File *handle = (PHYSFS_File *)userdata;
    PHYSFS_sint64 pos = 0;

    if (whence == SDL_IO_SEEK_SET) {
        pos = (PHYSFS_sint64) offset;
    }
    else if (whence == SDL_IO_SEEK_CUR) {
        const PHYSFS_sint64 current = PHYSFS_tell(handle);
        if (current == -1) {
            SDL_PhysFS_SetError("Cannot find position in file");
            return -1;
        }

        if (offset == 0) {
            return (Sint64) current;
        }

        pos = current + ((PHYSFS_sint64) offset);
    }
    else if (whence == SDL_IO_SEEK_END) {
        const PHYSFS_sint64 len = PHYSFS_fileLength(handle);
        if (len == -1) {
            SDL_PhysFS_SetError("Cannot find end of file");
            return -1;
        }

        pos = len + ((PHYSFS_sint64) offset);
    }
    else {
        SDL_PhysFS_SetError("Invalid 'whence' parameter");
        return -1;
    }

    if (pos < 0) {
        SDL_PhysFS_SetError("Attempt to seek past start of file");
        return -1;
    }

    if (!PHYSFS_seek(handle, (PHYSFS_uint64)pos)) {
        SDL_PhysFS_SetError("Failed to seek in file");
        return -1;
    }

    return (Sint64) pos;
}

/**
 * SDL_IOStream callback: read.
 *
 * @internal
 */
size_t SDLCALL SDL_PhysFS_ReadIO(void *userdata, void *ptr, size_t size, SDL_IOStatus *status) {
    PHYSFS_File *handle = (PHYSFS_File *)userdata;
    PHYSFS_sint64 rc = PHYSFS_readBytes(handle, ptr, (PHYSFS_uint64)size);
    if (rc <= 0) {
        *status = SDL_PhysFS_IOStatus(PHYSFS_getLastErrorCode());
        rc = 0;
    }

    return (size_t)rc;
}

/**
 * SDL_IOStream callback: write.
 *
 * @internal
 */
size_t SDLCALL SDL_PhysFS_WriteIO(void *userdata, const void *ptr, size_t size, SDL_IOStatus *status) {
    PHYSFS_File *handle = (PHYSFS_File *)userdata;
    PHYSFS_sint64 wc;

    if (handle == NULL) {
        return 0;
    }

    wc = PHYSFS_writeBytes(handle, ptr, (PHYSFS_uint64)size);
    if (wc < 0) {
        SDL_PhysFS_SetError("Failed to write file");
        *status = SDL_PhysFS_IOStatus(PHYSFS_getLastErrorCode());
        wc = 0;
    }

    return (size_t)wc;
}

/**
 * SDL_IOStream callback: flush.
 *
 * @internal
 */
bool SDLCALL SDL_PhysFS_FlushIO(void *userdata, SDL_IOStatus *status) {
    PHYSFS_File* handle = (PHYSFS_File *)userdata;
    if (handle == NULL) {
        return false;
    }

    if (PHYSFS_flush(handle) != 0) {
        return true;
    }

    if (status != NULL) {
        *status = SDL_PhysFS_IOStatus(PHYSFS_getLastErrorCode());
    }

    return false;
}

/**
 * SDL_IOStream callback: close.
 *
 * @internal
 */
bool SDLCALL SDL_PhysFS_CloseIO(void *userdata) {
    PHYSFS_File *handle = (PHYSFS_File *)userdata;

    if (handle != NULL) {
        if (!PHYSFS_close(handle)) {
            SDL_PhysFS_SetError("Failed to close file");
            return false;
        }

        // TODO: Do we need to SDL_free() the handle?
        //SDL_free(handle);
        return true;
    }

    return false;
}

/**
 * Creates a SDL_IOStream based on the given PHYSFS_File.
 *
 * @internal
 */
SDL_IOStream *SDL_PhysFS_OpenIO(PHYSFS_File *handle) {
    SDL_IOStream *retval = NULL;

    if (handle != NULL) {
        SDL_IOStreamInterface iface;
        SDL_INIT_INTERFACE(&iface);
        iface.size = SDL_PhysFS_GetIOSize;
        iface.seek  = SDL_PhysFS_SeekIO;
        iface.read  = SDL_PhysFS_ReadIO;
        iface.write = SDL_PhysFS_WriteIO;
        iface.flush = SDL_PhysFS_FlushIO;
        iface.close = SDL_PhysFS_CloseIO;
        retval = SDL_OpenIO(&iface, (void*)handle);
    }

    return retval;
}

/**
 * Loads a SDL_IOStream from the given filename in PhysFS.
 *
 * @param filename The filename to load from PhysFS.
 *
 * @return The resulting "SDL_IOStream*", which must be freed with "SDL_CloseIO()" afterwards. NULL on failure, use "SDL_GetError()" to see details.
 */
SDL_IOStream* SDL_PhysFS_IOFromFile(const char* filename) {
    PHYSFS_File* handle = PHYSFS_openRead(filename);
    if (handle == NULL) {
        SDL_PhysFS_SetError("Failed to open file for reading");
        return NULL;
    }

    return SDL_PhysFS_OpenIO(handle);
}

/**
 * Loads a bitmap file from "PhysFS" into a "SDL_Surface".
 *
 * @param filename The filename to load.
 *
 * @return The SDL_Surface, or NULL on failure, use "SDL_GetError()" for details.
 */
SDL_Surface* SDL_PhysFS_LoadBMP(const char* filename) {
    SDL_IOStream* io = SDL_PhysFS_IOFromFile(filename);
    if (io == NULL) {
        return NULL;
    }

    return SDL_LoadBMP_IO(io, 1);
}

/**
 * Loads a wav file from PhysFS.
 *
 * @param filename The filename of the wav file to load.
 *
 * @return True or false depending on if loading was successful. Use "SDL_GetError()" for details of the failure.
 */
bool SDL_PhysFS_LoadWAV(const char* filename, SDL_AudioSpec * spec, Uint8 ** audio_buf, Uint32 * audio_len) {
    SDL_IOStream* io = SDL_PhysFS_IOFromFile(filename);
    if (io == NULL) {
        return NULL;
    }

    return SDL_LoadWAV_IO(io, 1, spec, audio_buf, audio_len);
}

/**
 * Loads all the file data from a given filename.
 *
 * @param filename The name of the file to load.
 * @param datasize Where to put the resulting size of the file.
 *
 * @return A new memory buffer containing all the data from the file. NULL on failure, use "SDL_GetError()" for details.
 */
void* SDL_PhysFS_LoadFile(const char* filename, size_t *datasize) {
    void* handle = PHYSFS_openRead(filename);
    if (handle == 0) {
        SDL_PhysFS_SetError("Failed to load file");
        *datasize = 0;
        return 0;
    }

    // Check to see how large the file is.
    PHYSFS_sint64 size = PHYSFS_fileLength(handle);
    if (size <= 0) {
        *datasize = 0;
        PHYSFS_close(handle);
        return 0;
    }

    // Read the file, return if it's empty.
    void* buffer = SDL_malloc((size_t)size);
    PHYSFS_sint64 read = PHYSFS_readBytes(handle, buffer, (PHYSFS_uint64)size);
    if (read < 0) {
        *datasize = 0;
        SDL_free(buffer);
        SDL_PhysFS_SetError("Failed to read bytes from file");
        PHYSFS_close(handle);
        return 0;
    }

    // Close the file handle, and return the bytes read and the buffer.
    PHYSFS_close(handle);
    *datasize = (size_t)read;

    return buffer;
}

/**
 * Writes a data buffer to the given file.
 *
 * @return The number of bytes written, or 0 on failure.
 *
 * @see SDL_PhysFS_SetWriteDir()
 */
size_t SDL_PhysFS_Write(const char* file, const void* buffer, size_t size) {
    if (size <= 0 || buffer == NULL) {
        return 0;
    }

    // Open the file.
    PHYSFS_File* handle = PHYSFS_openWrite(file);
    if (handle == NULL) {
        SDL_PhysFS_SetError("Failed to open file for writing");
        return 0;
    }

    // Write the data to the file.
    PHYSFS_sint64 bytesWritten = PHYSFS_writeBytes(handle, buffer, (PHYSFS_uint64)size);
    if (bytesWritten <= 0) {
        SDL_PhysFS_SetError("Failed to write data to file");
        PHYSFS_close(handle);
        return 0;
    }

    PHYSFS_close(handle);
    return (size_t)bytesWritten;
}

/**
 * Sets the directory where PhysFS will write files.
 *
 * @return true on success, false otherwise.
 *
 * @see SDL_PhysFS_Write()
 */
bool SDL_PhysFS_SetWriteDir(const char* path) {
    if (PHYSFS_setWriteDir(path) == 0) {
        SDL_PhysFS_SetError("Failed to set write directory");
        return false;
    }

    return true;
}

/**
 * Loads a list of files from the given directory.
 *
 * Make sure to unload the list by using SDL_PhysFS_FreeDirectoryFiles().
 *
 * @code
 * char** directoryFiles = SDL_PhysFS_LoadDirectoryFiles("res");
 * int count = 0;
 * for (char** file = directoryFiles; *file != NULL; file++) {
 *     count++;
 * }
 * SDL_PhysFS_FreeDirectoryFiles(directoryFiles);
 * @endcode
 *
 * @return A list of files that were found in the given search path. NULL otherwise.
 *
 * @see SDL_PhysFS_FreeDirectoryFiles()
 */
inline char** SDL_PhysFS_LoadDirectoryFiles(const char *directory) {
    return PHYSFS_enumerateFiles(directory);
}

/**
 * Unloads a list of directory files.
 *
 * @see SDL_PhysFS_LoadDirectoryFiles()
 */
inline void SDL_PhysFS_FreeDirectoryFiles(char** files) {
    PHYSFS_freeList(files);
}

/**
 * Determine if a file exists in the search path.
 *
 * @return true if it exists, false otherwise.
 */
inline bool SDL_PhysFS_Exists(const char* file) {
    return PHYSFS_exists(file) != 0;
}

#ifdef __cplusplus
}
#endif

#endif  // SDL_PHYSFS_IMPLEMENTATION_ONCE
#endif  // SDL_PHYSFS_IMPLEMENTATION
