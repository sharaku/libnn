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

#ifndef _NN_INODE_H_
#define _NN_INODE_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <uuid/uuid.h>
#include <list.h>


// libnnのUUID, オブジェクトはinodeにて管理する。
// inodeはradix-treeにて管理する。
// inode番号は64bitであり、上位 xxbitがUUID, 下位xxbitがオブジェクトindexとなる。
// inode番号からオブジェクトindexとUUIDの関連付けがわかる。

enum {
	NN_DIRENTRY_TYPE_STRING,
	NN_DIRENTRY_TYPE_UUID,
};
struct nn_direntry {
	uint8_t		name_len;
	uint8_t		type;
	uint8_t		rsv[6];
	uint64_t	ino;
};

struct nn_object;
struct nn_d_uuid;
struct nn_d_uuidctx;

// ファイル名がUUID Onlyの場合の構造体。
typedef struct nn_object {
	uint64_t		ino;		// inode番号 (下位6bitは データ管理用)
	uint16_t		objtype;	// オブジェクトのタイプ
	uint16_t		idx;
	uint32_t		size;		// inodeで管理しているオブジェクトのサイズ
	struct nn_d_uuid	*d_uuid;
	char			addr[0];	// 実データ。
} nn_d_object_t;

typedef struct nn_d_uuid {
	list_head_t		list;		// ハッシュにつながるリスト
	list_head_t		list_entries;	// 全ノードのつながるリスト
	uint64_t		ino;		// inode番号
	uuid_t			uuid;		// UUID
	struct nn_object	*objects[32];	// オブジェクトリスト
} nn_d_uuid_t;

typedef struct nn_d_uuidctx {
	uint64_t		ino;		// inode番号
	list_head_t		list_entries;	// 全ノードのつながるリスト
	list_head_t		uuid_hash[256];	// UUIDハッシュ
} nn_d_uuidctx_t;

extern void nn_init(void);
extern nn_d_uuid_t* nn_get_duuid(uuid_t uuid);
extern void nn_put_duuid(nn_d_uuid_t *dent_uuid);
extern nn_d_object_t* nn_get_dobject(nn_d_uuid_t *dent_uuid, uint32_t idx);
extern void nn_put_dobject(nn_d_object_t *dent_object);

// uuidリストを取得する。
extern int nn_read_uuids(uuid_t uuid);
extern nn_d_object_t * nn_read_objects(uuid_t uuid, nn_d_object_t *object);

#endif /* _NN_INODE_H_ */

