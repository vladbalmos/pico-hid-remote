#include "pico/util/queue.h"

void bt_init(queue_t *write_queue, queue_t *read_queue);
void bt_deinit();
void bt_process_queue();