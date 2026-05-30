#include "librfa/rfa_internal.h"

#include "minilzo.h"

int RFA_Decompress(const uint8_t* in, size_t in_size, uint8_t* out, size_t out_size)
{
    lzo_uint written = out_size;
    int result = lzo1x_decompress_safe(in, in_size, out, &written, NULL);
    if (result != LZO_E_OK || written != out_size)
    {
        fprintf(stderr, "RFA_Decompress: Decompression failed: %d (got %lu bytes, expected %lu)", result, (uint64_t)written, (uint64_t)out_size);
        return -1;
    }
    
    return 0;
}
