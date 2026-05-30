#include "librfa/rfa_internal.h"

#include <stdlib.h>
#include <ctype.h>

int RFA_ReadSegmentHeaders(FILE* file, uint32_t offset, uint32_t* segment_count, RFA_SegmentHeader** segments)
{
    fseek(file, offset, SEEK_SET);
    if (fread(segment_count, sizeof(uint32_t), 1, file) != 1)
    {
        fprintf(stderr, "RFA_ReadSegmentHeaders: Failed to read segment count\n");
        return -1;
    }

    *segments = (RFA_SegmentHeader*)malloc(sizeof(RFA_SegmentHeader) * *segment_count);
    if (!*segments)
    {
        fprintf(stderr, "RFA_ReadSegmentHeaders: Failed to allocate memory for segments!\n");
        return -1;
    }

    for (uint32_t i = 0; i < *segment_count; i++)
    {
        if (fread(&(*segments)[i], sizeof(RFA_SegmentHeader), 1, file) != 1)
        {
            fprintf(stderr, "RFA_ReadSegmentHeaders: Failed to read segment header %u\n", i);
            free(*segments);
            *segments = NULL;
            return -1;
        }
    }

    return 0;
}

int RFA_strcmp(const char *a, const char *b)
{
    while (*a && *b)
    {
        char ca = (char)tolower((unsigned char)*a);
        char cb = (char)tolower((unsigned char)*b);
        if (ca != cb) return (unsigned char)ca - (unsigned char)cb;
        a++;
        b++;
    }
    return (unsigned char)tolower((unsigned char)*a) - (unsigned char)tolower((unsigned char)*b);
}
