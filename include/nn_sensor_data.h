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

#ifndef _NN_SENSOR_DATA_H_
#define _NN_SENSOR_DATA_H_

#include <stdint.h>
#include <nn.h>

typedef struct nn_sensor_gyro {
	int32_t		angle;		// 角度
	int32_t		rate;		// 加速度
} nn_sensor_gyro_t;


typedef struct nn_sensor_light {
	int32_t		value;		// 光度
} nn_sensor_light_t;

typedef struct nn_sensor_color {
	int32_t		red;
	int32_t		green;
	int32_t		blue;
} nn_sensor_color_t;

enum nn_color_mode {
	NN_COLOR_MODE_UNKNOWN = 0,	// 不明
	NN_COLOR_MODE_REFLECTED,	// 反射光モード
	NN_COLOR_MODE_AMBIENT,		// 環境光モード
	NN_COLOR_MODE_CORRECTION,	// 反射光 - 環境光モード
	NN_COLOR_MODE_FULLCOLOR		// 簡易カラーモード
};

typedef struct nn_sensor_light_color {
	int32_t		mode;
	nn_sensor_color_t	color;
	nn_sensor_light_t	light;
} nn_sensor_light_color_t;

typedef struct nn_sensor_usonic {
	int32_t		value;
} nn_sensor_usonic_t;

typedef struct nn_sensor_touch {
	int32_t		value;
} nn_sensor_touch_t;


// ---------------------------------------------

typedef struct nn_upd_sensor_gyro {
	nn_context_object_t	header;	// ヘッダ
	nn_sensor_gyro_t	gyro;
} nn_upd_sensor_gyro_t;
static inline void
nn_updsensor_gyro_init(nn_upd_sensor_gyro_t *obj) {
	memset(obj, 0, sizeof *obj);
	nn_context_object_init(&obj->header, 0, NN_OBJTYPE_GYRO, sizeof(nn_sensor_gyro_t));
}

typedef struct nn_upd_sensor_light {
	nn_context_object_t	header;	// ヘッダ
	nn_sensor_light_t	light;
} nn_upd_sensor_light_t;
static inline void
nn_updsensor_light_init(nn_upd_sensor_light_t *obj) {
	memset(obj, 0, sizeof *obj);
	nn_context_object_init(&obj->header, 0, NN_OBJTYPE_LIGHT, sizeof(nn_sensor_light_t));
}

typedef struct nn_upd_sensor_color {
	nn_context_object_t	header;	// ヘッダ
	nn_sensor_color_t	color;
} nn_upd_sensor_color_t;
static inline void
nn_updsensor_color_init(nn_upd_sensor_color_t *obj) {
	memset(obj, 0, sizeof *obj);
	nn_context_object_init(&obj->header, 0, NN_OBJTYPE_COLOR, sizeof(nn_sensor_color_t));
}

typedef struct nn_upd_sensor_light_color {
	nn_context_object_t	header;	// ヘッダ
	nn_sensor_light_color_t	color;
} nn_upd_sensor_light_color_t;
static inline void
nn_updsensor_light_color_init(nn_upd_sensor_light_color_t *obj) {
	memset(obj, 0, sizeof *obj);
	nn_context_object_init(&obj->header, 0, NN_OBJTYPE_COLOR_LIGHT, sizeof(nn_sensor_light_color_t));
	obj->color.mode = NN_COLOR_MODE_REFLECTED;
}

typedef struct nn_upd_sensor_usonic {
	nn_context_object_t	header;	// ヘッダ
	nn_sensor_usonic_t	usonic;
} nn_upd_sensor_usonic_t;
static inline void
nn_updsensor_usonic_init(nn_upd_sensor_usonic_t *obj) {
	memset(obj, 0, sizeof *obj);
	nn_context_object_init(&obj->header, 0, NN_OBJTYPE_USONIC, sizeof(nn_sensor_usonic_t));
}

typedef struct nn_upd_sensor_touch {
	nn_context_object_t	header;	// ヘッダ
	nn_sensor_touch_t	touch;
} nn_upd_sensor_touch_t;
static inline void
nn_upd_sensor_touch_init(nn_upd_sensor_touch_t *obj) {
	memset(obj, 0, sizeof *obj);
	nn_context_object_init(&obj->header, 0, NN_OBJTYPE_TOUCH, sizeof(nn_sensor_touch_t));
}





#endif /* _NN_SENSOR_DATA_H_ */
