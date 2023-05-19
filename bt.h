#include "pico/util/queue.h"

void bt_init(queue_t *write_queue, queue_t *read_queue, uint8_t battery_level);
void bt_set_battery_level(uint8_t level);
void bt_deinit();
void bt_process_queue();