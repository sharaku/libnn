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

#ifndef _NN_H_
#define _NN_H_

#include <uuid/uuid.h>
#include <stdint.h>
#include <list.h>
#include <wq/wq.h>
#include <wq/wq-event.h>
#include <netinet/in.h>

// --------------------------------
// プロトコル

enum {
	NN_MSG_UPDATE,
};

enum {
	NN_OBJTYPE_RAW		= 0x0000,	// RAWデータ（制限なし）
	NN_OBJTYPE_TOUCH	= 0x0001,	// タッチセンサ情報
	NN_OBJTYPE_GYRO,			// ジャイロセンサ
	NN_OBJTYPE_COLOR,			// カラーセンサ
	NN_OBJTYPE_LIGHT,			// 光センサ
	NN_OBJTYPE_USONIC,			// 超音波センサ
	NN_OBJTYPE_TACHO_MOTOR	= 0x0400,	// タコモータ
	NN_OBJTYPE_COLOR_LIGHT,			// カラー＆光センサ(動的切替)
	NN_OBJTYPE_USER		= 0x8000,
};

typedef struct nn_msg_upd_header
{
	uuid_t		uuid;		// 0x00: ノードのUUID
	uint8_t		objects;	// 0x10: 登録されているオブジェクト数
	uint8_t		rsv[15];	// 0x11: 予約
} nn_msg_upd_header_t;

typedef struct nn_msg_updobj_header
{
	uint16_t	idx;		// 0x00: ノード内のindex
	uint16_t	type;		// 0x02: オブジェクトタイプ
	uint16_t	offset;		// 0x04: データオフセット
	uint16_t	size;		// 0x06: データサイズ
} nn_msg_updobj_header_t;

#define NN_DATAGRAM_PACKETMAXSZ (1500)

// --------------------------------

struct nn_context_node {
	uuid_t			uuid;		// ノードのUUID
};

typedef struct nn_context_object {
	uint8_t			idx;		// 0x00: ノード内のindex
	uint8_t			rsv;		// 0x01: 予約
	uint16_t		type;		// 0x02: オブジェクトタイプ
	uint32_t		sz;		// 0x04: サイズ
	char			addr[0];	// 0x08: オブジェクトデータ
} nn_context_object_t;

static inline void
nn_context_object_init(nn_context_object_t *obj, uint16_t idx, uint16_t type, uint32_t sz) {
	obj->idx = idx;
	obj->type = type;
	obj->sz = sz;
}

struct nn_send_buf
{
	struct list_head	list;
	uint32_t		sz;
	char			buf[0];
};

#define NN_CTX_OBJECTS	(32)
struct nn_context_objects {
	// ノード内の登録情報。
	// 登録順番はプログラムで固定することで、UUIDとindexで
	// 同一データにアクセス可能
	wq_item_t			async_send;
	wq_item_t			*async_item;
	uint64_t			used_bmp;
	struct nn_context_object	*object[NN_CTX_OBJECTS];
};

typedef struct nn_update_sendbuf {
	nn_msg_upd_header_t	header;
	char			buf[NN_DATAGRAM_PACKETMAXSZ - sizeof(nn_msg_upd_header_t)];
} nn_update_sendbuf_t;

typedef struct nn_context {
	struct nn_context_node		node;
	struct nn_context_objects	objects;
	
	struct {
		int			sock;
		wq_ev_item_t		ev_item;
		struct sockaddr_in	addr;
		list_head_t		send_list;
		uint32_t		send_cnt;
	} datagram;

	struct {
		uint32_t		usedsz;
		nn_update_sendbuf_t	buffer;
	} send;
} nn_context_t;

extern void nn_initialize(nn_context_t *ctx, uuid_t *uuid, int port);
extern void nn_start(nn_context_t *ctx);
extern int nn_add_object(nn_context_t *ctx, struct nn_context_object *addr);
extern int nn_update_object(nn_context_t *ctx, struct nn_context_object *obj,
			    uint32_t offset, uint32_t size);


// --------------------------------


/*
node networkの構成例

+ nodes
	+ UUID
		+ sensorA
			+ type (gyro)
			+ param ...
		+ sensorB
			+ type (usonic)
			+ param ...
		+ sensorC
			+ type (touch)
			+ param ...
		+ sensorD
			+ type (color)
			+ param ...
		+ event
		+ fileA
	+ UUID
		+ sensorA
			+ type (gyro)
			+ param ...
		+ sensorB
			+ type (usonic)
			+ param ...
		+ event
		+ fileA
	+ UUID
		+ fileA
		+ fileB

最大値: 
	ノード数		= 4G ノード
	ノード内のセンサ数	= 65535 センサ
	センサ内のデータ数	= 256 データ

	※ ノードはNICのMACアドレスで判断する。
*/

#endif /* _NN_H_ */

