#ifndef __LIBRFA_RFA_H__
#define __LIBRFA_RFA_H__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Structs */

#pragma pack(push, 1)

typedef struct {
    uint32_t entries_ofs;             // Offset to the file table (points to the entries count)
    uint32_t compressed;              // 0 = uncompressed archive, 1 = compressed with lzo1x
    uint8_t  magic_checksum[148];     // Possibly a hash or signature; exact purpose unknown
} RFA_Header;

typedef struct {
    char*    name;                    // Null-terminated ASCII string, file path inside the archive
    uint32_t csize;                   // Compressed size, includes 16-byte segment header block
    uint32_t ucsize;                  // Uncompressed size of the file data
    uint32_t offset;                  // Offset to the segment headers of the file
} RFA_FileEntry;

#pragma pack(pop) 

typedef struct {
    FILE* file;                       // Pointer to the opened .rfa file
    RFA_Header header;                // Archive header structure
    uint32_t entries_count;           // Number of file entries in the archive
    RFA_FileEntry* entries;           // Array of file entry structures
} RFA_Archive;

/* Functions */

bool RFA_Init(void);
void RFA_Shutdown(void);

RFA_Archive* RFA_Open(const char* path);
void RFA_Close(RFA_Archive* archive);

size_t RFA_GetFileCount(RFA_Archive* archive);

RFA_FileEntry* RFA_GetEntryByName(RFA_Archive* archive, const char* name);
RFA_FileEntry* RFA_GetEntryByIndex(RFA_Archive* archive, size_t index);

bool RFA_FileExists(RFA_Archive* archive, const char* name);

int RFA_ExtractFile(RFA_Archive* archive, const char* name, void** data, size_t* size);
int RFA_ExtractFileByIndex(RFA_Archive* archive, size_t index, void** data, size_t* size);

/* Will be implemented later */

// RFA_Archive* RFA_Create(void);
// int RFA_Save(RFA_Archive* archive, const char* path);
// int RFA_AddFile(RFA_Archive* archive, const char* name, const void* data, size_t size);
// int RFA_RemoveFile(RFA_Archive* archive, const char* name);

#ifdef __cplusplus
}
#endif

#endif // __LIBRFA_RFA_H__
