/*
 * SPDX-License-Identifier: MIT
 *
 * キーマップ上の sosho,behavior-pointer-sens の値(パーセント)で X/Y の移動量を拡縮する。
 *
 * 設定用レイヤーの「スロット番号」番目のキーに &ptr_sens <パーセント> を置いておき、
 * このプロセッサがイベントごとにその値を読む。キーマップは ZMK Studio の通信で
 * 書き換え・保存できるので、専用の通信路なしに実行時の感度変更と永続化ができる。
 */

#define DT_DRV_COMPAT sosho_input_processor_pointer_sens

#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/input/input.h>

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) &&                                                    \
    DT_HAS_COMPAT_STATUS_OKAY(sosho_behavior_pointer_sens) &&                                      \
    (!IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL))

#include <drivers/input_processor.h>
#include <zmk/behavior.h>
#include <zmk/keymap.h>

#define SENS_BEHAVIOR_NAME DEVICE_DT_NAME(DT_COMPAT_GET_ANY_STATUS_OKAY(sosho_behavior_pointer_sens))

#define SENS_DEFAULT_PERCENT 100
#define SENS_MIN_PERCENT 10
#define SENS_MAX_PERCENT 400

/* 最後に設定用レイヤーが見つかった ID。毎回の全レイヤー走査を避けるために覚えておく */
static zmk_keymap_layer_id_t cached_layer = ZMK_KEYMAP_LAYER_ID_INVAL;

static bool read_percent(zmk_keymap_layer_id_t layer, uint8_t slot, uint32_t *percent) {
    const struct zmk_behavior_binding *binding = zmk_keymap_get_layer_binding_at_idx(layer, slot);

    if (!binding || !binding->behavior_dev ||
        strcmp(binding->behavior_dev, SENS_BEHAVIOR_NAME) != 0) {
        return false;
    }

    *percent = binding->param1;
    return true;
}

static uint32_t lookup_percent(uint8_t slot) {
    uint32_t percent = SENS_DEFAULT_PERCENT;

    if (cached_layer == ZMK_KEYMAP_LAYER_ID_INVAL || !read_percent(cached_layer, slot, &percent)) {
        cached_layer = ZMK_KEYMAP_LAYER_ID_INVAL;
        for (zmk_keymap_layer_id_t layer = 0; layer < ZMK_KEYMAP_LAYERS_LEN; layer++) {
            if (read_percent(layer, slot, &percent)) {
                cached_layer = layer;
                break;
            }
        }
    }

    return CLAMP(percent, SENS_MIN_PERCENT, SENS_MAX_PERCENT);
}

static int pointer_sens_handle_event(const struct device *dev, struct input_event *event,
                                     uint32_t param1, uint32_t param2,
                                     struct zmk_input_processor_state *state) {
    if (event->type != INPUT_EV_REL ||
        (event->code != INPUT_REL_X && event->code != INPUT_REL_Y)) {
        return ZMK_INPUT_PROC_CONTINUE;
    }

    int32_t percent = (int32_t)lookup_percent((uint8_t)param1);
    int32_t value = event->value * percent;

    if (state && state->remainder) {
        value += *state->remainder;
    }

    int32_t scaled = value / 100;

    if (state && state->remainder) {
        *state->remainder = (int16_t)(value - scaled * 100);
    }

    event->value = scaled;

    return ZMK_INPUT_PROC_CONTINUE;
}

static struct zmk_input_processor_driver_api pointer_sens_driver_api = {
    .handle_event = pointer_sens_handle_event,
};

#define POINTER_SENS_IP_INST(n)                                                                    \
    DEVICE_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL, POST_KERNEL,                                  \
                          CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &pointer_sens_driver_api);

DT_INST_FOREACH_STATUS_OKAY(POINTER_SENS_IP_INST)

#endif
