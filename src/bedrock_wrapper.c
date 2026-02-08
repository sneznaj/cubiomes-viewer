/* Load cubiomes-bedrock at runtime to avoid symbol clashes with Java cubiomes. */
#include "bedrock_wrapper.h"
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#define DLOPEN(path) LoadLibraryA(path)
#define DLSYM(h, name) GetProcAddress((HMODULE)(h), name)
#define DLCLOSE(h) FreeLibrary((HMODULE)(h))
#define SO_NAME "libcubiomes-bedrock.dll"
#else
#include <dlfcn.h>
#include <stdio.h>
#include <libgen.h>
#define DLOPEN(path) dlopen(path, RTLD_NOW | RTLD_LOCAL)
#define DLSYM(h, name) dlsym(h, name)
#define DLCLOSE(h) dlclose(h)
#if defined(__APPLE__)
#include <mach-o/dyld.h>
#define SO_NAME "libcubiomes-bedrock.dylib"
#else
#define SO_NAME "libcubiomes-bedrock.so"
#endif
#endif

static void *bedrock_handle;
static void *(*bedrock_api_create)(int);
static void (*bedrock_api_destroy)(void *);
static void (*bedrock_api_apply_seed)(void *, uint64_t);
static int (*bedrock_api_gen_biomes)(void *, int *, int, int, int, int);

static int load_bedrock(void)
{
    if (bedrock_handle)
        return 0;
#if defined(_WIN32) || defined(_WIN64)
    bedrock_handle = DLOPEN(SO_NAME);
#elif defined(__APPLE__)
    /* Prefer next to executable (for .app bundle), then env, then current dir */
    char exe_path[4096];
    uint32_t size = sizeof(exe_path);
    if (_NSGetExecutablePath(exe_path, &size) == 0)
    {
        char exe_copy[4096];
        snprintf(exe_copy, sizeof(exe_copy), "%s", exe_path);
        char *dir = dirname(exe_copy);
        char path[4096];
        snprintf(path, sizeof(path), "%s/" SO_NAME, dir);
        bedrock_handle = DLOPEN(path);
    }
    if (!bedrock_handle)
        bedrock_handle = DLOPEN(getenv("CUBIOMES_BEDROCK_LIB"));
    if (!bedrock_handle)
        bedrock_handle = DLOPEN(SO_NAME);
#else
    if (getenv("CUBIOMES_BEDROCK_LIB"))
        bedrock_handle = DLOPEN(getenv("CUBIOMES_BEDROCK_LIB"));
    if (!bedrock_handle)
        bedrock_handle = DLOPEN(SO_NAME);
    if (!bedrock_handle)
        bedrock_handle = DLOPEN("../../cubiomes-bedrock/" SO_NAME);
#endif
    if (!bedrock_handle)
        return -1;
    bedrock_api_create = (void *(*)(int))DLSYM(bedrock_handle, "bedrock_api_create");
    bedrock_api_destroy = (void (*)(void *))DLSYM(bedrock_handle, "bedrock_api_destroy");
    bedrock_api_apply_seed = (void (*)(void *, uint64_t))DLSYM(bedrock_handle, "bedrock_api_apply_seed");
    bedrock_api_gen_biomes = (int (*)(void *, int *, int, int, int, int))DLSYM(bedrock_handle, "bedrock_api_gen_biomes");
    if (!bedrock_api_create || !bedrock_api_destroy || !bedrock_api_apply_seed || !bedrock_api_gen_biomes)
    {
        DLCLOSE(bedrock_handle);
        bedrock_handle = NULL;
        return -1;
    }
    return 0;
}

bedrock_state_t bedrock_create(int version)
{
    if (load_bedrock() != 0)
        return NULL;
    return bedrock_api_create(version);
}

void bedrock_destroy(bedrock_state_t s)
{
    if (bedrock_api_destroy && s)
        bedrock_api_destroy(s);
}

void bedrock_apply_seed(bedrock_state_t s, uint64_t seed)
{
    if (bedrock_api_apply_seed && s)
        bedrock_api_apply_seed(s, seed);
}

int bedrock_gen_biomes(bedrock_state_t s, int *out, int areaX, int areaZ, int areaWidth, int areaHeight)
{
    if (!bedrock_api_gen_biomes || !s)
        return -1;
    return bedrock_api_gen_biomes(s, out, areaX, areaZ, areaWidth, areaHeight);
}
