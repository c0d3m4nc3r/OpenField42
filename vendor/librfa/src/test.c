#include "librfa/rfa.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

int create_directories(const char* path)
{
    char* temp = strdup(path);
    if (!temp) {
        fprintf(stderr, "Failed to allocate memory for path\n");
        return -1;
    }

    char* p = temp;
    if (strlen(p) > 1 && p[1] == ':') p += 2;
    while (*p == '/' || *p == '\\') p++;

    for (; *p; p++)
    {
        if (*p == '/' || *p == '\\')
        {
            *p = '\0';
            #ifdef _WIN32
                if (mkdir(temp) != 0 && errno != EEXIST) {
            #else
                if (mkdir(temp, 0755) != 0 && errno != EEXIST) {
            #endif
                fprintf(stderr, "Failed to create directory '%s': %s\n", temp, strerror(errno));
                free(temp);
                return -1;
            }
            *p = '/';
        }
    }

    free(temp);
    return 0;
}

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <path_to_archive>\n", argv[0]);
        return -1;
    }
    
    RFA_Archive* archive = RFA_Open(argv[1]);
    if (!archive) return -2;
    
    printf("Archive '%s' info: \n", argv[1]);
    printf(" Entries count: %d\n", archive->entries_count);
    printf(" Entries:\n");
    for (uint32_t i = 0; i < archive->entries_count; i++)
    {
        RFA_FileEntry* entry = &archive->entries[i];
        printf(" Entry %3u: Name: '%s'; Compressed: %u bytes; Uncompressed: %u bytes; Offset: 0x%08" PRIX32 "\n",
               i, entry->name, entry->csize, entry->ucsize, entry->offset);
    }
    
    RFA_FileEntry* entry0 = &archive->entries[0];
    void* data = NULL;
    size_t size = 0;
    
    if (RFA_ExtractFile(archive, entry0->name, &data, &size) != 0)
    {
        fprintf(stderr, "Failed to extract entry '%s' from '%s'!\n", entry0->name, argv[1]);
        return -3;
    }
    
    if (create_directories(entry0->name) != 0)
    {
        fprintf(stderr, "Failed to create directories for '%s'\n", entry0->name);
        return -4;
    }
    
    FILE* output_fp = fopen(entry0->name, "wb");
    if (!output_fp)
    {
        fprintf(stderr, "Failed to open file '%s' to output entry data!\n", entry0->name);
        return -4;
    }
    
    fwrite(data, size, 1, output_fp);
    fclose(output_fp);
    free(data);
    RFA_Close(archive);
    return 0;
}
