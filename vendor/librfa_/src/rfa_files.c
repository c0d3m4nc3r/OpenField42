#include "librfa/rfa_internal.h"

#include <string.h>

size_t RFA_GetFileCount(RFA_Archive* archive)
{
    if (!archive)
    {
        fprintf(stderr, "RFA_GetFileCount: archive is NULL!\n");
        return 0;
    }

    return archive->entries_count;
}

RFA_FileEntry* RFA_GetEntryByName(RFA_Archive* archive, const char* name)
{
    if (!archive || !name)
    {
        fprintf(stderr, "RFA_GetEntryByName: Invalid params! archive or name is NULL!");
        return NULL;
    }

    for (uint32_t i = 0; i < archive->entries_count; i++)
    {
        RFA_FileEntry* entry = &archive->entries[i];

        if (!entry->name) continue;

        if (RFA_strcmp(entry->name, name) == 0)
            return entry;
    }

    return NULL;
}

RFA_FileEntry* RFA_GetEntryByIndex(RFA_Archive* archive, size_t index)
{
    if (!archive)
    {
        fprintf(stderr, "RFA_GetEntryByIndex: Invalid params! archive is NULL!\n");
        return NULL;
    }

    if (index > archive->entries_count - 1)
    {
        fprintf(stderr, "RFA_GetEntryByIndex: Index %lu out of bounds (max %u)\n", index, archive->entries_count - 1);
        return NULL;
    }

    return &archive->entries[index];
}

bool RFA_FileExists(RFA_Archive* archive, const char* name)
{
    if (!archive || !name)
    {
        fprintf(stderr, "RFA_FileExists: Invalid params! archive or name is NULL!\n");
        return false;
    }

    return RFA_GetEntryByName(archive, name) != NULL;
}
