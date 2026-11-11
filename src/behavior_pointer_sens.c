/*
 * SPDX-License-Identifier: MIT
 *
 * ポインタ感度(パーセント)をキーマップ上に保持するためのビヘイビア。
 * 押しても何も起きない。値の範囲をメタデータとして公開することで、
 * ZMK Studio の通信(set_layer_binding)から書き換えられるようにしている。
 */

#define DT_DRV_COMPAT sosho_behavior_pointer_sens

#include <zephyr/device.h>

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) &&                                                    \
    (!IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL))

#include <drivers/behavior.h>
#include <zmk/behavior.h>

static int pointer_sens_pressed(struct zmk_behavior_binding *binding,
                                struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

static int pointer_sens_released(struct zmk_behavior_binding *binding,
                                 struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)

static const struct behavior_parameter_value_metadata param_values[] = {
    {
        .display_name = "Percent",
        .type = BEHAVIOR_PARAMETER_VALUE_TYPE_RANGE,
        .range = {.min = 10, .max = 400},
    },
};

static const struct behavior_parameter_metadata_set param_metadata_set[] = {{
    .param1_values = param_values,
    .param1_values_len = ARRAY_SIZE(param_values),
}};

static const struct behavior_parameter_metadata metadata = {
    .sets_len = ARRAY_SIZE(param_metadata_set),
    .sets = param_metadata_set,
};

#endif // IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)

static const struct behavior_driver_api pointer_sens_driver_api = {
    .binding_pressed = pointer_sens_pressed,
    .binding_released = pointer_sens_released,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .parameter_metadata = &metadata,
#endif // IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
};

#define POINTER_SENS_INST(n)                                                                       \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL, POST_KERNEL,                                \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &pointer_sens_driver_api);

DT_INST_FOREACH_STATUS_OKAY(POINTER_SENS_INST)

#endif
