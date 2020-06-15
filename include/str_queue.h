#include <stdbool.h>

typedef struct list {
    const char *data;
    struct list *next;
} list;

typedef struct {
    list *tail;
    list *head;
} str_queue;

bool queue_put(str_queue *, const char *);

const char *queue_get(str_queue *);

bool queue_is_empty(const str_queue *);
