#ifndef BEDROCK_WRAPPER_H
#define BEDROCK_WRAPPER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Opaque Bedrock generator state (LayerStack inside). */
typedef void* bedrock_state_t;

/* Create/destroy state. version = Bedrock MC version (e.g. 6..17 for 1.6–1.17). */
bedrock_state_t bedrock_create(int version);
void bedrock_destroy(bedrock_state_t s);

/* Set world seed (Bedrock uses full 64-bit; game often shows lower 32 bits). */
void bedrock_apply_seed(bedrock_state_t s, uint64_t seed);

/* Generate overworld biomes at scale 1:4 into out[x + z*areaWidth].
 * areaX, areaZ = position in 4-block coords; areaWidth, areaHeight = size. */
int bedrock_gen_biomes(bedrock_state_t s, int *out,
    int areaX, int areaZ, int areaWidth, int areaHeight);

#ifdef __cplusplus
}
#endif

#endif
