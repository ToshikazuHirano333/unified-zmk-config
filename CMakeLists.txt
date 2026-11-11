zephyr_sources(src/boot_delay_hack.c)

# ポインタ感度をキーマップ上の値から読む仕組み(設定アプリから ZMK Studio の通信で書き換える)
target_sources(app PRIVATE
  src/behavior_pointer_sens.c
  src/input_processor_pointer_sens.c
)
