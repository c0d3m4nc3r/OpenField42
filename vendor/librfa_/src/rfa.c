#include "librfa/rfa_internal.h"

#include "minilzo.h"

#include <stdlib.h>
#include <string.h>

bool RFA_Init(void)
{
    static bool initialized = false;
    if (initialized)
        return true;

    if (lzo_init() != LZO_E_OK)
    {
        fprintf(stderr, "RFA_Init: Failed to initialize LZO!\n");
        return false;
    }

    initialized = true;
    return true;
}

void RFA_Shutdown(void)
{
    // Does nothing for now
}

RFA_Archive* RFA_Open(const char* archive_path)
{
    RFA_Archive* archive = NULL;
    FILE* file = NULL;
    RFA_FileEntry* entries = NULL;
    uint32_t i;

    archive = (RFA_Archive*)malloc(sizeof(RFA_Archive));
    if (!archive)
    {
        fprintf(stderr, "RFA_Open: Failed to allocate memory for RFA archive!\n");
        goto cleanup;
    }

    file = fopen(archive_path, "rb");
    if (!file)
    {
        fprintf(stderr, "RFA_Open: Failed to open file: \"%s\"\n", archive_path);
        goto cleanup;
    }
    archive->file = file;

    if (fread(&archive->header, sizeof(RFA_Header), 1, file) != 1)
    {
        fprintf(stderr, "RFA_Open: Failed to read archive header!\n");
        goto cleanup;
    }

    fseek(file, archive->header.entries_ofs, SEEK_SET);
    if (fread(&archive->entries_count, sizeof(uint32_t), 1, file) != 1)
    {
        fprintf(stderr, "RFA_Open: Failed to read entries count!\n");
        goto cleanup;
    }

    entries = (RFA_FileEntry*)malloc(sizeof(RFA_FileEntry) * archive->entries_count);
    if (!entries)
    {
        fprintf(stderr, "RFA_Open: Failed to allocate memory for file entries!\n");
        goto cleanup;
    }
    archive->entries = entries;

    for (i = 0; i < archive->entries_count; i++)
    {
        RFA_FileEntry* entry = &entries[i];

        size_t name_len = 0;

        if (fread(&name_len, sizeof(uint32_t), 1, file) != 1)
        {
            fprintf(stderr, "RFA_Open: Failed to read name length for entry %u!\n", i);
            goto cleanup;
        }

        entry->name = (char*)calloc(name_len + 1, 1);
        if (!entry->name)
        {
            fprintf(stderr, "RFA_Open: Failed to allocate memory for entry name!\n");
            goto cleanup;
        }

        if (fread(entry->name, 1, name_len, file) != name_len)
        {
            fprintf(stderr, "RFA_Open: Failed to read name for entry %u!\n", i);
            goto cleanup;
        }

        entry->name[name_len] = '\0';

        if (fread(&entry->csize, sizeof(uint32_t), 1, file) != 1 ||
            fread(&entry->ucsize, sizeof(uint32_t), 1, file) != 1 ||
            fread(&entry->offset, sizeof(uint32_t), 1, file) != 1)
        {
            fprintf(stderr, "RFA_Open: Failed to read entry metadata for entry %u!\n", i);
            goto cleanup;
        }

        fseek(file, sizeof(uint32_t) * 3, SEEK_CUR); // skip 3 unknown values
    }

    return archive;

cleanup:
    if (entries)
    {
        for (uint32_t j = 0; j < i; j++)
            free(entries[j].name);

        free(entries);
    }
    
    if (file) fclose(file);
    if (archive) free(archive);

    return NULL;
}

void RFA_Close(RFA_Archive* archive)
{
    if (!archive)
    {
        fprintf(stderr, "RFA_Close: Invalid params! archive is NULL!\n");
        return;
    }

    if (archive->entries)
    {
        for (uint32_t i = 0; i < archive->entries_count; i++)
            free(archive->entries[i].name);
        
        free(archive->entries);
    }

    if (archive->file)
        fclose(archive->file);

    free(archive);
}
