/* Minimal API for the viewer: only these symbols are exported from the shared library. */
#include "generator.h"
#include "layers.h"
#include <stdlib.h>
#include <string.h>

void *bedrock_api_create(int mc)
{
    LayerStack *g = calloc(1, sizeof(LayerStack));
    if (!g) return NULL;
    if (mc < MC_1_0 || mc > MC_1_17)
        mc = MC_1_17;
    setupOverworldGenerator(g, mc);
    return g;
}

void bedrock_api_destroy(void *s)
{
    free(s);
}

void bedrock_api_apply_seed(void *s, uint64_t seed)
{
    applySeed((LayerStack *)s, seed);
}

int bedrock_api_gen_biomes(void *s, int *out, int areaX, int areaZ, int areaWidth, int areaHeight)
{
    LayerStack *g = (LayerStack *)s;
    Layer *entry_4 = g->entry_4;
    if (!entry_4) return -1;
    int *cache = allocCache(entry_4, areaWidth, areaHeight);
    if (!cache) return -1;
    int err = genArea(entry_4, cache, areaX, areaZ, areaWidth, areaHeight);
    if (err == 0)
        memcpy(out, cache, (size_t)areaWidth * areaHeight * sizeof(int));
    free(cache);
    return err;
}
