/* --
 *
 * MIT License
 * 
 * Copyright (c) 2018 Abe Takafumi
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. *
 *
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <uuid/uuid.h>
#include <list.h>
#include <slab.h>
#include <nn_inode.h>
#include <log/log.h>


static void __nn_duuid_constructor(void *buf, size_t sz);
static void __nn_duuid_destructor(void *buf, size_t sz);
static void __nn_dobject_constructor(void *buf, size_t sz);
static void __nn_dobject_destructor(void *buf, size_t sz);
static uint32_t __nn_uuid2hashkey(uuid_t uuid);
static int __nn_lookup_uuid(nn_d_uuidctx_t *ctx, uuid_t uuid, nn_d_uuid_t **dent_uuid);
static int __nn_add_uuid(nn_d_uuidctx_t *ctx, uuid_t uuid, nn_d_uuid_t *dent_uuid);
static int __nn_del_uuid(nn_d_uuid_t *dent_uuid);
static int __nn_lookup_object(nn_d_uuid_t *dent_uuid, uint32_t idx, nn_d_object_t **dent_object);
static int __nn_add_object(nn_d_uuid_t *dent_uuid, uint32_t idx, nn_d_object_t *dent_object);
static int __nn_del_object(nn_d_uuid_t *dent_uuid, uint32_t idx);


static nn_d_uuidctx_t __uuid_ctx;

// 1つ4MBのバッファを使う
struct slab_cache __duuid_slab;
struct slab_cache __dobject_slab;

static void
__nn_duuid_constructor(void *buf, size_t sz)
{
	nn_d_uuid_t	*d_uuid = (nn_d_uuid_t *)buf;

	memset(buf, 0, sz);

	wq_infolog64("__nn_duuid_constructor");
	init_list_head(&d_uuid->list);
	init_list_head(&d_uuid->list_entries);
	d_uuid->ino = 0;
}

static void
__nn_duuid_destructor(void *buf, size_t sz)
{
	wq_infolog64("__nn_duuid_destructor");
	nn_d_uuid_t *dent_uuid = (nn_d_uuid_t *)buf;
	__nn_del_uuid(dent_uuid);
}

static void
__nn_dobject_constructor(void *buf, size_t sz)
{
	memset(buf, 0, sz);
}

static void
__nn_dobject_destructor(void *buf, size_t sz)
{
	nn_d_object_t *dent_object = (nn_d_object_t *)buf;
	__nn_del_object(dent_object->d_uuid, dent_object->idx);
	if (dent_object->d_uuid) {
		slab_put(dent_object->d_uuid);
	}
}

void
nn_init(void)
{
	nn_d_uuidctx_t *ctx = &__uuid_ctx;

	INIT_SLAB_SZ(&__duuid_slab, sizeof(nn_d_uuid_t), 4194304);
	INIT_SLAB_SZ(&__dobject_slab, 512, 4194304);

	ctx->ino = 0;
	int i;
	for (i = 0; i < 256; i++) {
		init_list_head(&ctx->uuid_hash[i]);
	}
	init_list_head(&ctx->list_entries);

	slab_set_constructor(&__duuid_slab, __nn_duuid_constructor);
	slab_set_destructor(&__duuid_slab, __nn_duuid_destructor);
	slab_set_constructor(&__dobject_slab, __nn_dobject_constructor);
	slab_set_destructor(&__dobject_slab, __nn_dobject_destructor);
}

static uint32_t
__nn_uuid2hashkey(uuid_t uuid)
{
	uint8_t		*addr = uuid;
	uint32_t	key = 0;

	// ひとまず8byteごとにxorをとる。
	key = key ^ addr[0x0];
	key = key ^ addr[0x1];
	key = key ^ addr[0x2];
	key = key ^ addr[0x3];
	key = key ^ addr[0x4];
	key = key ^ addr[0x5];
	key = key ^ addr[0x6];
	key = key ^ addr[0x7];
	key = key ^ addr[0x8];
	key = key ^ addr[0x9];
	key = key ^ addr[0xa];
	key = key ^ addr[0xb];
	key = key ^ addr[0xc];
	key = key ^ addr[0xd];
	key = key ^ addr[0xe];
	key = key ^ addr[0xf];
	return key;
}



static int
__nn_lookup_uuid(nn_d_uuidctx_t *ctx, uuid_t uuid, nn_d_uuid_t **dent_uuid)
{
	int ret = -ENOENT;

	uint32_t key = __nn_uuid2hashkey(uuid) & 0xff;
	// UUIDのハッシュから指定された文字列のuuidを検索する。
	// UUIDは16byteの値である。これを1byteのハッシュにする。
	if (list_empty(&ctx->uuid_hash[key])) {
		// ハッシュが空ということは探しているエントリは存在しない。
		*dent_uuid = NULL;
		wq_infolog64("hash list is empty. key=%u", key);
		goto out;
	}

	nn_d_uuid_t *d_uuid;
	list_head_t *pos = NULL;
	list_for_each(pos, &ctx->uuid_hash[key]) {
		d_uuid = list_entry(pos, nn_d_uuid_t, list);
		if (uuid_compare(d_uuid->uuid, uuid) == 0) {
			// 見つかった。
			slab_get(d_uuid);
			*dent_uuid = d_uuid;
			ret = 0;
			break;
		}
	}

	if (ret) {
		wq_infolog64("not found.");
	}
out:
	return ret;
}

static int
__nn_add_uuid(nn_d_uuidctx_t *ctx, uuid_t uuid, nn_d_uuid_t *dent_uuid)
{
	int ret = 0;

	uint32_t key = __nn_uuid2hashkey(uuid) & 0xff;
	wq_infolog64("uuid=%016lx-%016lx key=%u", *((uint64_t*)&uuid[0]), *((uint64_t*)&uuid[8]), key);
	list_add_tail(&dent_uuid->list, &ctx->uuid_hash[key]);
	list_add_tail(&dent_uuid->list_entries, &ctx->list_entries);
	return ret;
}

static int
__nn_del_uuid(nn_d_uuid_t *dent_uuid)
{
	int ret = 0;

	list_del_init(&dent_uuid->list);
	list_del_init(&dent_uuid->list_entries);
	return ret;
}

nn_d_uuid_t *
nn_get_duuid(uuid_t uuid)
{
	nn_d_uuidctx_t *ctx = &__uuid_ctx;
	nn_d_uuid_t *dent_uuid;
	int ret;

	ret = __nn_lookup_uuid(ctx, uuid, &dent_uuid);
	if (!ret) {
		// 取得できた。
		goto fined;
	}

	dent_uuid = (nn_d_uuid_t *)slab_alloc(&__duuid_slab);
	__nn_add_uuid(ctx, uuid, dent_uuid);

fined:
	// 参照を獲得して返す
	slab_get(dent_uuid);
	return dent_uuid;
}

// inode開放
void
nn_put_duuid(nn_d_uuid_t *dent_uuid)
{
	slab_put(dent_uuid);
}

static int
__nn_lookup_object(nn_d_uuid_t *dent_uuid, uint32_t idx, nn_d_object_t **dent_object)
{
	int ret = 0;

	if (idx >= 32) {
		return -1;
	}
	*dent_object = dent_uuid->objects[idx];
	return ret;
}

static int
__nn_add_object(nn_d_uuid_t *dent_uuid, uint32_t idx, nn_d_object_t *dent_object)
{
	int ret = 0;

	if (idx >= 32) {
		return -1;
	}
	slab_get(dent_uuid);
	dent_uuid->objects[idx] = dent_object;
	dent_object->d_uuid = dent_uuid;
	wq_infolog64("uuid=%p objects[%d]=%p", dent_uuid, idx, dent_uuid->objects[idx]);
	return ret;
}

static int
__nn_del_object(nn_d_uuid_t *dent_uuid, uint32_t idx)
{
	int ret = 0;

	if (idx >= 32) {
		return -1;
	}
	dent_uuid->objects[idx] = NULL;
	return ret;
}

nn_d_object_t *
nn_get_dobject(nn_d_uuid_t *dent_uuid, uint32_t idx)
{
	nn_d_object_t *dent_object;
	int ret;

	ret = __nn_lookup_object(dent_uuid, idx, &dent_object);
	if (!ret && dent_object != NULL) {
		// 取得できた。
		goto fined;
	}

	dent_object = (nn_d_object_t *)slab_alloc(&__dobject_slab);
	__nn_add_object(dent_uuid, idx, dent_object);

fined:
	// 参照を獲得して返す
	slab_get(dent_object);
	return dent_object;
}

// inode開放
void
nn_put_dobject(nn_d_object_t *dent_object)
{
	slab_put(dent_object);
}

// inode開放
int
nn_read_uuids(uuid_t uuid)
{
	nn_d_uuidctx_t *ctx = &__uuid_ctx;
	nn_d_uuid_t *dent_uuid;
	int ret;

	ret = __nn_lookup_uuid(ctx, uuid, &dent_uuid);
	if (ret) {
		// 前回の指定がなければ先頭を返す。
		dent_uuid = list_first_entry_or_null(&(ctx->list_entries), nn_d_uuid_t, list_entries);
	} else {
		// 今のエントリの次を取り出す
		dent_uuid = list_next_entry_or_null(&(dent_uuid->list_entries), &(ctx->list_entries), nn_d_uuid_t, list_entries);
	}

	if (!dent_uuid) {
		// 次の登録がなければNULL応答
		return -ENOENT;
	}
	memcpy(uuid, &(dent_uuid->uuid), sizeof(uuid_t));
	return 0;
}

// inode開放
nn_d_object_t *
nn_read_objects(uuid_t uuid, nn_d_object_t *object)
{
	nn_d_uuidctx_t *ctx = &__uuid_ctx;
	nn_d_uuid_t *dent_uuid;
	int ret;

	ret = __nn_lookup_uuid(ctx, uuid, &dent_uuid);
	if (ret) {
		// エントリがない。
		wq_infolog64("ENOENT");
		return NULL;
	}

	if (object == NULL) {
		return dent_uuid->objects[0];
	} else {
		if (object->d_uuid != dent_uuid) {
			wq_infolog64("error. unmatch.");
			// 第一引数と第二引数が矛盾している。
			return NULL;
		}
		return object->d_uuid->objects[object->idx + 1];
	}
}
