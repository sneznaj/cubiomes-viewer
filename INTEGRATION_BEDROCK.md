# Integrating a Cubiomes Library with Bedrock Support

This document describes how to stitch a cubiomes library that supports Minecraft Bedrock Edition into cubiomes-viewer.

## Option A: Single library with `MC_BEDROCK` (recommended)

Use a cubiomes fork or build that adds Bedrock as another “MC version” so the viewer can keep using the same API.

### What the library must provide

1. **Version enum**  
   In `cubiomes/biomes.h` (or equivalent), add a Bedrock entry to the MC version enum, for example:
   ```c
   // After MC_NEWEST, add:
   MC_BEDROCK,   // Minecraft Bedrock Edition (current)
   ```
   The viewer will pass this as `wi.mc` when the user selects Bedrock Edition. Implement **`mc2str(MC_BEDROCK)`** (in `util.c` or equivalent) so the version combo can display e.g. `"Bedrock"` or `"1.20"` when Bedrock is selected.

2. **Generator**  
   Implement Bedrock world generation behind the existing API:
   - `setupGenerator(Generator *g, int mc, uint32_t flags)`  
     When `mc == MC_BEDROCK`, initialize generator state for Bedrock (e.g. Bedrock biome layers/noise).
   - `applySeed(Generator *g, int dim, uint64_t seed)`  
     When the generator is in Bedrock mode, use Bedrock’s seed rules (typically lower 32 bits of `seed`).
   - `genBiomes(const Generator *g, int *cache, Range r)`  
     Fill `cache` with Bedrock biome IDs for the given range.
   - `getBiomeAt(const Generator *g, int scale, int x, int y, int z)`  
     Return the Bedrock biome ID at the given position.

3. **Structures (optional but useful)**  
   So that the map and seed finder can show Bedrock structures:
   - `getStructureConfig(int structureType, int mc, StructureConfig *sconf)`  
     For `mc == MC_BEDROCK`, return config for Bedrock structure types (village, temple, stronghold, etc.).
   - `getStructurePos()`, `getVariant()`, and any other finder APIs used by the viewer for the structures you want to support.

4. **Biome names (optional)**  
   Either:
   - Export a function the viewer can call for Bedrock biome names (e.g. `biome2str(MC_BEDROCK, id)`), or  
   - Rely on the viewer’s fallback table (see below).

### Viewer-side integration (already wired)

- **WorldInfo** has an `edition` field; when the user selects “Bedrock Edition”, `getSeed()` sets `wi.edition = EDITION_BEDROCK` and, **if** the cubiomes header defines `MC_BEDROCK`, sets `wi.mc = MC_BEDROCK`.
- Map and search use `wi.mc` and `wi.seed` as usual, so once the library handles `MC_BEDROCK` in `setupGenerator` / `applySeed` / `genBiomes` / `getBiomeAt` (and optionally finders), Bedrock maps and seed search work without further viewer changes.
- **Banner**: the map shows the “Bedrock Edition selected — map shows Java generation…” banner only when Bedrock is selected **and** the current world is not using Bedrock (i.e. `wi.mc != MC_BEDROCK`). So once the library defines `MC_BEDROCK` and you set `wi.mc = MC_BEDROCK`, the banner disappears for that view.
- **Biome names**: if the library does not provide Bedrock biome strings, the viewer uses a small fallback table for common Bedrock biome IDs; you can extend it in `util.cpp` (`getBiomeDisplayBedrock()`).

### Build

- Replace (or update) the `cubiomes` submodule with the Bedrock-capable version that defines `MC_BEDROCK` and implements the APIs above.
- Rebuild the viewer; no extra defines are required. The viewer uses `#ifdef MC_BEDROCK` so it still builds with a cubiomes that does **not** define `MC_BEDROCK` (current upstream); in that case, selecting Bedrock only shows the info banner and keeps using Java generation.

---

## Option B: Separate Bedrock library and wrapper

If the Bedrock implementation is a **separate** library with its own types (e.g. `GeneratorBedrock`, `setupGeneratorBedrock()`, `applySeedBedrock()`), then:

1. Add a small **wrapper** in the viewer (e.g. `src/cubiomes_wrapper.c` / `.h`) that:
   - Takes an “edition” (Java vs Bedrock) and the same logical parameters (seed, dimension, area, etc.).
   - For Java: calls the existing cubiomes API.
   - For Bedrock: calls the Bedrock library API and, if needed, maps its output (e.g. biome IDs) into what the viewer expects.

2. Change all viewer call sites that currently call cubiomes directly to go through this wrapper (e.g. `world.cpp`, `search.cpp`, `tabbiomes.cpp`, `tabstructures.cpp`, etc.).

3. Link the Bedrock library in the build (e.g. in `cubiomes-viewer.pro`) and use a define (e.g. `CUBIOMES_BEDROCK`) to compile the Bedrock branch of the wrapper.

Option B is more work but allows a completely different generator/biome/structure API behind one interface.

---

## Summary

- **Easiest path**: Use a cubiomes that defines **`MC_BEDROCK`** and implements **`setupGenerator`**, **`applySeed`**, **`genBiomes`**, and **`getBiomeAt`** for that version (Option A). Then replace the `cubiomes` subtree and rebuild; the viewer is already set up to pass `wi.mc = MC_BEDROCK` and `wi.edition = EDITION_BEDROCK` when the user selects Bedrock.
