#include "librfa/rfa_internal.h"

#include <stdlib.h>

int RFA_ExtractEntry(RFA_Archive* archive, RFA_FileEntry* entry, void** data, size_t* size)
{
    RFA_SegmentHeader* segments = NULL;
    uint8_t* output_data = NULL;
    uint8_t* compressed_data = NULL;
    int result = -1;
    uint32_t segment_count = 0;
    uint32_t total_output_size = 0;
    uint32_t data_base = 0;
    uint32_t write_offset = 0;

    if (!entry || !archive)
    {
        fprintf(stderr, "RFA_ExtractEntry: Invalid params! Archive or file entry is NULL\n");
        return -1;
    }

    if (archive->header.compressed == 0)
    {
        output_data = (uint8_t*)malloc(entry->ucsize);
        if (!output_data)
        {
            fprintf(stderr, "RFA_ExtractEntry: Failed to allocate memory for output data!");
            return -2;
        }

        fseek(archive->file, entry->offset, SEEK_SET);
        if (fread(output_data, 1, entry->ucsize, archive->file) != entry->ucsize)
        {
            fprintf(stderr, "RFA_ExtractEntry: Failed to read uncompressed data");
            free(output_data);
            return -3;
        }

        if (data) *data = output_data;
        if (size) *size = entry->ucsize;
        
        return 0;
    }
    
    segment_count = 0;
    if (RFA_ReadSegmentHeaders(archive->file, entry->offset, &segment_count, &segments) != 0)
    {
        goto cleanup;
    }

    total_output_size = 0;
    for (uint32_t i = 0; i < segment_count; i++)
        total_output_size += segments[i].ucsize;

    if (size) *size = total_output_size;

    output_data = (uint8_t*)malloc(total_output_size);
    if (!output_data)
    {
        fprintf(stderr, "RFA_ExtractEntry: Failed to allocate memory for output data!");
        goto cleanup;
    }

    data_base = entry->offset + 4 + segment_count * sizeof(RFA_SegmentHeader);
    write_offset = 0;

    for (uint32_t i = 0; i < segment_count; i++)
    {
        RFA_SegmentHeader* seg = &segments[i];
        
        compressed_data = (uint8_t*)malloc(seg->csize);
        if (!compressed_data)
        {
            fprintf(stderr, "RFA_ExtractEntry: Failed to allocate memory for compressed segment %u", i);
            goto cleanup;
        }

        fseek(archive->file, data_base + seg->data_ofs, SEEK_SET);
        if (fread(compressed_data, 1, seg->csize, archive->file) != seg->csize)
        {
            fprintf(stderr, "RFA_ExtractEntry: Failed to read compressed segment %u", i);
            goto cleanup;
        }

        if (RFA_Decompress(compressed_data, seg->csize, 
                          output_data + write_offset, seg->ucsize) != 0)
        {
            fprintf(stderr, "RFA_ExtractEntry: Decompression failed for segment %u", i);
            goto cleanup;
        }

        free(compressed_data);
        compressed_data = NULL;
        write_offset += seg->ucsize;
    }

    if (data) *data = output_data;
    if (size) *size = total_output_size;

    output_data = NULL;
    result = 0;

cleanup:
    free(segments);
    free(output_data);
    free(compressed_data);
    
    return result;
}

int RFA_ExtractFile(RFA_Archive* archive, const char* name, void** data, size_t* size)
{
    if (!archive || !name)
    {
        fprintf(stderr, "RFA_ExtractFile: Invalid params! archive or name is NULL!\n");
        return -1;
    }

    RFA_FileEntry* entry = RFA_GetEntryByName(archive, name);
    if (!entry)
    {
        fprintf(stderr, "RFA_ExtractFile: Can't find file entry with name \"%s\"\n", name);
        return -2;
    }

    return RFA_ExtractEntry(archive, entry, data, size);
}

int RFA_ExtractFileByIndex(RFA_Archive* archive, size_t index, void** data, size_t* size)
{
    if (!archive)
    {
        fprintf(stderr, "RFA_ExtractFileByIndex: Invalid params! archive is NULL!\n");
        return -1;
    }

    RFA_FileEntry* entry = RFA_GetEntryByIndex(archive, index);
    if (!entry)
    {
        fprintf(stderr, "RFA_ExtractFileByIndex: Can't find file entry with index %lu\n", index);
        return -2;
    }

    return RFA_ExtractEntry(archive, entry, data, size);
}
