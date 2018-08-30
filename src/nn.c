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

// uuid-dev

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/eventfd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <nn.h>
#include <wq/wq.h>
#include <wq/wq-event.h>
#include <log/log.h>
#include <bitops.h>
#include <nn_inode.h>
#include <slab.h>


static void __nn_notify_update(struct nn_context *ctx, char *buf, uint32_t sz);
static void __nn_init_buffer(nn_context_t *ctx);
static void nn_datagram_event(wq_item_t *item, wq_arg_t arg);

static void
nn_datagram_initialize(struct nn_context *ctx, int port)
{
	struct sockaddr_in udp_addr;
	struct ip_mreq mreq;
	int rc;

	ctx->datagram.sock = socket(AF_INET, SOCK_DGRAM, 0);
	udp_addr.sin_family = AF_INET;
	udp_addr.sin_port = htons(port);
	udp_addr.sin_addr.s_addr = INADDR_ANY;
	rc = bind(ctx->datagram.sock, (struct sockaddr *)&udp_addr, sizeof(udp_addr));
	if (rc) {
		wq_infolog64("bind() error. rc=%d errno=%d", rc, errno);
		return;
	}

	/* setsockoptは、bind以降で行う必要あり */
	memset(&mreq, 0, sizeof(mreq));
	mreq.imr_interface.s_addr = INADDR_ANY;
	mreq.imr_multiaddr.s_addr = inet_addr("239.192.1.2");
	rc = setsockopt(ctx->datagram.sock,
			IPPROTO_IP,
			IP_ADD_MEMBERSHIP,
			(char *)&mreq, sizeof(mreq));
	if (rc) {
		wq_infolog64("setsockopt() error. rc=%d errno=%d", rc, errno);
		return;
	}

	in_addr_t ipaddr = INADDR_ANY;
	rc = setsockopt(ctx->datagram.sock,
			IPPROTO_IP,
			IP_MULTICAST_IF,
			(char *)&ipaddr, sizeof(ipaddr));
	if (rc) {
		wq_infolog64("setsockopt() error. rc=%d errno=%d", rc, errno);
		return;
	}

	ctx->datagram.addr.sin_family = AF_INET;
	ctx->datagram.addr.sin_port = htons(port);
	ctx->datagram.addr.sin_addr.s_addr = inet_addr("239.192.1.2");

	return;
}
static void
nn_datagram_send(struct nn_context *ctx, void *b, int sz)
{
	struct nn_send_buf *buf = malloc(512);
	memset(buf, 0, 512);
	init_list_head(&buf->list);
	memcpy(buf->buf, b, sz);
	buf->sz = sz;

	list_add_tail(&buf->list, &ctx->datagram.send_list);
	if (!ctx->datagram.send_cnt) {
		wq_ev_sched(&ctx->datagram.ev_item, WQ_EVFL_FDIN|WQ_EVFL_FDOUT, nn_datagram_event);
	}
	ctx->datagram.send_cnt++;
}
static uint32_t
nn_do_send(struct nn_context *ctx)
{
	struct nn_send_buf	*buf;
	int rc;

	if (list_empty(&ctx->datagram.send_list)) {
		return 0;
	}

	// 送信リストの先頭を抜いて送信する。
	buf = (struct nn_send_buf*)list_first_entry(&ctx->datagram.send_list,
						    struct nn_send_buf, list);
	list_del_init(&buf->list);
	rc = sendto(ctx->datagram.sock, buf->buf, buf->sz,
		    0, (struct sockaddr *)&ctx->datagram.addr,
		    sizeof(ctx->datagram.addr));
	if (rc < 0) {
		wq_infolog64("sendto() error. rc=%d errno=%d", rc, errno);
	}

	ctx->datagram.send_cnt--;
	return ctx->datagram.send_cnt;
}
static void
nn_do_recv(struct nn_context *ctx)
{
	// 1pageのバッファを用意する。
	char buf[4096] = {0};
	ssize_t ret;
	ret = recv(ctx->datagram.sock, buf, sizeof(buf), 0);
	if (ret < 0) {
		wq_infolog64("recv() error. ret=%d errno=%d", ret, errno);
	} else if (ret == 0) {
	} else {
		__nn_notify_update(ctx, buf, ret);
	}

}
static void
nn_datagram_event(wq_item_t *item, wq_arg_t arg)
{
	wq_ev_item_t	*item_ev = (wq_ev_item_t*)arg;
	nn_context_t	*ctx = (nn_context_t*)list_entry(arg, nn_context_t, datagram.ev_item);
	uint32_t	events = WQ_EVFL_FDIN;

	if (item_ev->events & WQ_EVFL_FDOUT) {
		nn_do_send(ctx);
	}
	if (item_ev->events & WQ_EVFL_FDIN) {
		nn_do_recv(ctx);
	}

	if(ctx->datagram.send_cnt) {
		events |= WQ_EVFL_FDOUT;
	}
	wq_ev_sched(&ctx->datagram.ev_item, events, nn_datagram_event);
}

#include <timeofday.h>

void
nn_initialize(nn_context_t *ctx, uuid_t *uuid, int port)
{
	wq_infolog64("nn init. port=%d", port);

	nn_init();

	init_list_head(&ctx->datagram.send_list);
	ctx->datagram.send_cnt = 0;
	ctx->datagram.sock = -1;
	memcpy(ctx->node.uuid, uuid, sizeof ctx->node.uuid);
	nn_datagram_initialize(ctx, port);
	wq_ev_init(&ctx->datagram.ev_item, ctx->datagram.sock);

	wq_init_item(&ctx->objects.async_send);
	ctx->objects.async_item = &ctx->objects.async_send;
	ctx->objects.used_bmp = 0;
	memset(ctx->objects.object, 0, sizeof ctx->objects.object);
	__nn_init_buffer(ctx);
}

void
nn_start(nn_context_t *ctx)
{
	wq_infolog64("nn start.");
	wq_ev_sched(&ctx->datagram.ev_item, WQ_EVFL_FDIN|WQ_EVFL_FDOUT, nn_datagram_event);
}

void
nn_stop(nn_context_t *ctx)
{
	wq_infolog64("nn stop.");
}

int
nn_add_object(nn_context_t *ctx, struct nn_context_object *addr)
{
	int bit;
	for_each_clear_bit64(bit, &ctx->objects.used_bmp) {
		ctx->objects.object[bit] = addr;
		addr->idx = bit;
		ctx->objects.used_bmp |= (1 << bit);
		return 0;
	}
	return -1;
}

static void
__nn_init_buffer(nn_context_t *ctx)
{
	memset(&ctx->send, 0, sizeof(nn_update_sendbuf_t));

	// ヘッダは初期で消費している。
	memcpy(ctx->send.buffer.header.uuid,
	       ctx->node.uuid, sizeof ctx->node.uuid);
}

static void
__nn_update_send(wq_item_t *item, wq_arg_t arg)
{
	// updateプロトコル構築
	nn_context_t *ctx = (nn_context_t *)arg;

	nn_datagram_send(ctx, &ctx->send.buffer, ctx->send.usedsz + sizeof(nn_msg_upd_header_t));
	__nn_init_buffer(ctx);
	ctx->objects.async_item = item;
}

static int
__nn_add_buffer(nn_context_t *ctx, struct nn_context_object *obj, uint32_t offset, uint32_t size)
{
	nn_msg_updobj_header_t *objh = (nn_msg_updobj_header_t *)&(ctx->send.buffer.buf[ctx->send.usedsz]);
	char *addr = &(ctx->send.buffer.buf[offset + sizeof(nn_msg_updobj_header_t)]);

	if ((NN_DATAGRAM_PACKETMAXSZ - ctx->send.usedsz) < (sizeof(nn_msg_updobj_header_t) + size)) {
		// 入らなければエラーする
		return -1;
	}

	objh->idx	= obj->idx;
	objh->type	= obj->type;
	objh->offset	= offset;
	objh->size	= size;
	memcpy(addr, obj->addr + offset, size);
	ctx->send.usedsz += sizeof(nn_msg_updobj_header_t) + size;
	ctx->send.buffer.header.objects++;
	return 0;
}

int
nn_update_object(nn_context_t *ctx, struct nn_context_object *obj,
		 uint32_t offset, uint32_t size)
{
	int ret;

	// バッファへ追加する。
	ret = __nn_add_buffer(ctx, obj, offset, size);
	if (ret != 0) {
		wq_infolog64("buffer full. ret=%d", ret);
		// もし、バッファがいっぱいであれば先に送信する。
		// 送信は、バッファ内容がコピーされるのでクリアしてから
		// 再利用する。
//		wq_cancel(ctx->objects.async_item);
		nn_datagram_send(ctx, &ctx->send.buffer, ctx->send.usedsz + sizeof(nn_msg_upd_header_t));
		__nn_init_buffer(ctx);
		ret = __nn_add_buffer(ctx, obj, offset, size);
		if (ret != 0) {
			return -1;
		}
	}

	if (ctx->objects.async_item) {
		// 送信がスケジュールされていないならスケジュールする。
		// この送信が行われる前にnn_update_object()が再度呼ばれたら、
		// その更新はまとめて送信される。
		wq_item_t *item = ctx->objects.async_item;

		ctx->objects.async_item = NULL;
		wq_sched(item, __nn_update_send, (void*)ctx);
	}
	return 0;
}

static void
__nn_notify_update(struct nn_context *ctx, char *buf, uint32_t sz)
{
	// 通知された情報をバラシて指定ノード情報へ登録する。
	// 登録されているuuid一覧をハッシュから取得する。
	// もし存在しなければ新規登録する。
	// ノード数は数十万にも及ぶので、ハッシュを使わないと
	// 検索コストが高くなる。
	nn_msg_upd_header_t	*hd = (nn_msg_upd_header_t *)buf;
	nn_msg_updobj_header_t	*objh;
	uint32_t cnt;
	uint32_t offset;
	char *addr;
	nn_d_uuid_t *d_uuid;
	nn_d_object_t *d_object;

	// uuidの構造体を取得
	d_uuid = nn_get_duuid(hd->uuid);
	memcpy(d_uuid->uuid, hd->uuid, sizeof hd->uuid);

	wq_infolog64("notify. uuid=%016lx-%016lx objects=%d buf=%p sz=%lu",
		     *((uint64_t*)&hd->uuid[0]),
		     *((uint64_t*)&hd->uuid[8]),
		     hd->objects,
		     buf, sz);

	for (cnt = 0, offset = sizeof(nn_msg_upd_header_t); cnt < hd->objects;
	     cnt++, offset += sizeof(nn_msg_updobj_header_t) + objh->size) {
		objh = (nn_msg_updobj_header_t *)&buf[offset];
		addr = &buf[offset + sizeof(nn_msg_updobj_header_t)];
		d_object = nn_get_dobject(d_uuid, objh->idx);
		d_object->objtype	= objh->type;
		d_object->idx		= objh->idx;

		if (d_object->size < objh->offset + objh->size) {
			d_object->size		= objh->offset + objh->size;
		}
		memcpy(&d_object->addr[objh->offset], addr, objh->size);

		wq_infolog64("index[%u] type=%u offset=%u size=%u",
			     objh->idx, objh->type, objh->offset, objh->size);

#if 0
printf("index[%u] type=%u offset=%u size=%u\n", objh->idx, objh->type, objh->offset, objh->size);
int i;
for (i = 0; i < objh->size; i++) {
	printf("%02x", d_object->addr[i]);
	if ((i % 4) == 3) {
		printf(" ");
	}
	if ((i % 16) == 15) {
		printf("\n");
	}
}
printf("\n");
#endif
		nn_put_dobject(d_object);

	}

	nn_put_duuid(d_uuid);
}

