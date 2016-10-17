/* Copyright (c) 2013-2016 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "obj-cache.h"

#include "gba/gba.h"
#include "gba/io.h"
#include "gba/video.h"
#include "gba/renderers/tile-cache.h"
#include "util/memory.h"

#define CACHE_SIZE (64 * 64 * 2 * 128)

void GBAVideoObjCacheInit(struct GBAVideoObjCache* cache) {
	// TODO: Reconfigurable cache for space savings
	cache->cache = anonymousMemoryMap(CACHE_SIZE);
	memset(cache->status, 0, sizeof(cache->status));
	memset(cache->tileOwnership, 0, sizeof(cache->tileOwnership));
	cache->objCharacterMapping = false;
}

void GBAVideoObjCacheDeinit(struct GBAVideoObjCache* cache) {
	mappedMemoryFree(cache->cache, CACHE_SIZE);
	cache->cache = NULL;
}

void GBAVideoObjCacheAssociate(struct GBAVideoObjCache* cache, struct GBAVideo* video) {
	cache->oam = video->oam.obj;
	cache->objCharacterMapping = GBARegisterDISPCNTIsObjCharacterMapping(video->p->memory.io[REG_DISPCNT >> 1]);
	video->renderer->objCache = cache;
}

void GBAVideoObjCacheWriteOAM(struct GBAVideoObjCache* cache, uint32_t address) {
	// TODO
}

void GBAVideoObjCacheWriteDispcnt(struct GBAVideoObjCache* cache, uint16_t value) {
	cache->objCharacterMapping = GBARegisterDISPCNTIsObjCharacterMapping(value);
}

static inline GBACachedObjAttributes _objToAttr(const struct GBAObj* obj) {
	GBACachedObjAttributes objAttr = 0;
	objAttr |= obj->a >> 13;
	objAttr |= (obj->b & 0xC000) >> 11;
	objAttr |= obj->c << 5;
	return objAttr;
}

static void _regenerateSprite(struct GBAVideoObjCache* cache, uint16_t* sprite, GBACachedObjAttributes attr) {
	unsigned stride = GBAVideoObjSizes[GBACachedObjAttributesGetShape(attr) * 4 + GBACachedObjAttributesGetSize(attr)][0];
	unsigned h = GBAVideoObjSizes[GBACachedObjAttributesGetShape(attr) * 4 + GBACachedObjAttributesGetSize(attr)][1] >> 3;
	unsigned w = stride >> 3;
	unsigned palette = GBACachedObjAttributesGetPalette(attr);
	const uint16_t* tile;
	unsigned tileId = GBACachedObjAttributesGetTile(attr) + 2048;
	unsigned tx, ty;
	for (ty = 0; ty < h; ++ty) {
		for (tx = 0; tx < w; ++tx) {
			// TODO: 256
			tile = GBAVideoTileCacheGetTile16(cache->tileCache, tileId, palette);
			memcpy(&sprite[0 * stride], &tile[0 * 8], 16);
			memcpy(&sprite[1 * stride], &tile[1 * 8], 16);
			memcpy(&sprite[2 * stride], &tile[2 * 8], 16);
			memcpy(&sprite[3 * stride], &tile[3 * 8], 16);
			memcpy(&sprite[4 * stride], &tile[4 * 8], 16);
			memcpy(&sprite[5 * stride], &tile[5 * 8], 16);
			memcpy(&sprite[6 * stride], &tile[6 * 8], 16);
			memcpy(&sprite[7 * stride], &tile[7 * 8], 16);
			++tileId;
		}
		if (!GBACachedObjAttributesIsObjCharacterMapping(attr)) {
			tileId += 0x10 - tx;
		}
	}
}

static bool _revalidateSprite(struct GBAVideoObjCache* cache, uint16_t* sprite, GBACachedObjAttributes attr) {
	// TODO
	_regenerateSprite(cache, sprite, attr);
	return false;
}

const uint16_t* GBAVideoObjCacheGetObj(struct GBAVideoObjCache* cache, unsigned objId, unsigned* width, unsigned* height) {
	GBACachedObjAttributes attr = cache->status[objId];
	uint16_t* sprite = &cache->cache[objId * 64 * 64];
	GBACachedObjAttributes objAttr = _objToAttr(&cache->oam[objId]);
	objAttr = GBACachedObjAttributesSetObjCharacterMapping(objAttr, cache->objCharacterMapping);
	if (objAttr != attr) {
		_regenerateSprite(cache, sprite, objAttr);
	} else {
		_revalidateSprite(cache, sprite, objAttr);	
	}
	cache->status[objId] = objAttr;
	*width = GBAVideoObjSizes[GBACachedObjAttributesGetShape(objAttr) * 4 + GBACachedObjAttributesGetSize(objAttr)][0];
	*height = GBAVideoObjSizes[GBACachedObjAttributesGetShape(objAttr) * 4 + GBACachedObjAttributesGetSize(objAttr)][1];
	return sprite;
}

const uint16_t* GBAVideoObjCacheGetObjIfDirty(struct GBAVideoObjCache* cache, GBAVideoObjCacheStatus handle, unsigned objId, unsigned* width, unsigned* height) {
	GBACachedObjAttributes attr = cache->status[objId];
	uint16_t* sprite = &cache->cache[objId * 64 * 64];
	GBACachedObjAttributes objAttr = _objToAttr(&cache->oam[objId]);
	objAttr = GBACachedObjAttributesSetObjCharacterMapping(objAttr, cache->objCharacterMapping);
	if (objAttr != attr) {
		_regenerateSprite(cache, sprite, objAttr);
		cache->status[objId] = objAttr;
	} else if (_revalidateSprite(cache, sprite, objAttr) && handle[objId] == objAttr) {
		return NULL;
	}
	handle[objId] = objAttr;
	*width = GBAVideoObjSizes[GBACachedObjAttributesGetShape(objAttr) * 4 + GBACachedObjAttributesGetSize(objAttr)][0];
	*height = GBAVideoObjSizes[GBACachedObjAttributesGetShape(objAttr) * 4 + GBACachedObjAttributesGetSize(objAttr)][1];
	return sprite;
}

