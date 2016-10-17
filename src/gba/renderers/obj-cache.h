/* Copyright (c) 2013-2016 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef GBA_OBJ_CACHE_H
#define GBA_OBJ_CACHE_H

#include "util/common.h"

struct GBAObj;
struct GBAVideo;
struct GBAVideoTileCache;

DECL_BITFIELD(GBACachedObjAttributes, uint32_t);
DECL_BIT(GBACachedObjAttributes, 256Color, 0);
DECL_BITS(GBACachedObjAttributes, Shape, 1, 2);
DECL_BITS(GBACachedObjAttributes, Size, 3, 2);
DECL_BITS(GBACachedObjAttributes, Tile, 5, 10);
DECL_BITS(GBACachedObjAttributes, Priority, 15, 2);
DECL_BITS(GBACachedObjAttributes, Palette, 17, 4);
DECL_BIT(GBACachedObjAttributes, ObjCharacterMapping, 21);

typedef GBACachedObjAttributes GBAVideoObjCacheStatus[128];

struct GBAVideoObjCache {
	uint16_t* cache;
	GBAVideoObjCacheStatus status;
	uint32_t tileOwnership[1024][4];
	bool objCharacterMapping;

	struct GBAVideoTileCache* tileCache;
	struct GBAObj* oam;
};

void GBAVideoObjCacheInit(struct GBAVideoObjCache* cache);
void GBAVideoObjCacheDeinit(struct GBAVideoObjCache* cache);
void GBAVideoObjCacheAssociate(struct GBAVideoObjCache* cache, struct GBAVideo* video);
void GBAVideoObjCacheWriteOAM(struct GBAVideoObjCache* cache, uint32_t address);
void GBAVideoObjCacheWriteDispcnt(struct GBAVideoObjCache* cache, uint16_t value);
const uint16_t* GBAVideoObjCacheGetObj(struct GBAVideoObjCache* cache, unsigned objId, unsigned* width, unsigned* height);
const uint16_t* GBAVideoObjCacheGetObjIfDirty(struct GBAVideoObjCache* cache, GBAVideoObjCacheStatus handle, unsigned objId, unsigned* width, unsigned* height);

#endif