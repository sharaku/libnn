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
#include <stdio.h>
#include <wq/wq.h>
#include <nn.h>
#include <nn_inode.h>
#include <nn_sensor_data.h>
#include <linux/uuid.h>
#include <radix-tree.h>

static nn_context_t			__nn_ctx;
static nn_upd_sensor_usonic_t		__usonic;
static nn_upd_sensor_gyro_t		__gyro;
static nn_upd_sensor_light_color_t	__color;
static nn_upd_sensor_touch_t		__touch;
//static nn_sensor_tacho_motor_t	__tacho_motor[4];

static wq_item_t	__timer;
static int		__update_idx = 0;


// 定期的にノードの情報を更新する。
static void
timer_sched_cb(wq_item_t *item, wq_arg_t arg)
{
	switch (__update_idx) {
	case 0:
		__usonic.usonic.value++;
		nn_update_object(&__nn_ctx, (struct nn_context_object*)&__usonic, 0, sizeof __usonic.usonic);
		break;
	case 1:
		__gyro.gyro.angle++;
		nn_update_object(&__nn_ctx, (struct nn_context_object*)&__gyro, 0, sizeof __gyro.gyro);
		break;
	case 2:
		__color.color.light.value++;
		nn_update_object(&__nn_ctx, (struct nn_context_object*)&__color, 0, sizeof __color.color);
		break;
	case 3:
		__touch.touch.value++;
		nn_update_object(&__nn_ctx, (struct nn_context_object*)&__touch, 0, sizeof __touch.touch);
//		break;
//	case 4:
//		break;
//	case 5:
//		break;
//	case 6:
//		break;
//	case 7:
		{
			printf("timer_sched_cb()\n");
			uuid_t	uuid = {0};
			char	uuin_str[128] = {0};
			int ret = 0;
			for (;ret == 0;) {
				ret = nn_read_uuids(uuid);
				if (!ret) {
					int ret2 = 0;
					nn_d_object_t *obj = NULL;

					uuid_unparse(uuid, uuin_str);
					printf("  uuid=%s\n", uuin_str);
					for (;ret2 == 0;) {
						obj = nn_read_objects(uuid, obj);
						if (obj) {
							printf("  [%d] type=%d size=%u\n", obj->idx, obj->objtype, obj->size);
						} else {
							ret2 = -1;
						}
					}
				}
			}
		}
		break;
	}
	__update_idx++;
	__update_idx &= 0x03;

	// 100us周期で更新
	wq_timer_sched(item, WQ_TIME_US(100000), timer_sched_cb, NULL);
}

int
main(void)
{
	uuid_t node_uuid = {0};
	uuid_generate(node_uuid);

	char	uuin_str[128] = {0};
	uuid_unparse(node_uuid, uuin_str);
	printf("uuid=%s\n", uuin_str);

	// nnの初期化
	// ノードのUUIDは本来どこかに保存しておく。
	// 新規の場合はUUID生成すればいい。
	nn_initialize(&__nn_ctx, &node_uuid, 12345);
	nn_start(&__nn_ctx);
	
	nn_updsensor_usonic_init(&__usonic);
	nn_updsensor_gyro_init(&__gyro);
	nn_updsensor_light_color_init(&__color);
	nn_upd_sensor_touch_init(&__touch);
	nn_add_object(&__nn_ctx, (struct nn_context_object*)&__usonic);
	nn_add_object(&__nn_ctx, (struct nn_context_object*)&__gyro);
	nn_add_object(&__nn_ctx, (struct nn_context_object*)&__color);
	nn_add_object(&__nn_ctx, (struct nn_context_object*)&__touch);
//	nn_add_object(&__nn_ctx, &__tacho_motor[0], sizeof __tacho_motor[0]);
//	nn_add_object(&__nn_ctx, &__tacho_motor[1], sizeof __tacho_motor[1]);
//	nn_add_object(&__nn_ctx, &__tacho_motor[2], sizeof __tacho_motor[2]);
//	nn_add_object(&__nn_ctx, &__tacho_motor[3], sizeof __tacho_motor[3]);

	wq_init_item_prio(&__timer, 0);
	wq_sched(&__timer, timer_sched_cb, NULL);

	// 自信をworkerスレッドとする。
	printf("wq_run\n");
	wq_run();
	return 0;
}

