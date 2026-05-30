#ifndef __LIBRFA_RFA_INTERNAL_H__
#define __LIBRFA_RFA_INTERNAL_H__

#include "librfa/rfa.h"

/* Structs */

typedef struct {
    uint32_t csize;     // Compressed data size
    uint32_t ucsize;    // Uncompressed data size
    uint32_t data_ofs;  // Offset to data
} RFA_SegmentHeader;

/* Functions */

int RFA_Decompress(const uint8_t* in, size_t in_size, uint8_t* out, size_t out_size);
int RFA_ExtractEntry(RFA_Archive* archive, RFA_FileEntry* entry, void** data, size_t* size);
int RFA_ReadSegmentHeaders(FILE* file, uint32_t offset, uint32_t* segment_count, RFA_SegmentHeader** segments);
int RFA_strcmp(const char *a, const char *b);

#endif // __LIBRFA_RFA_INTERNAL_H__
