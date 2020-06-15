#include <str_queue.h>

#include <memory.h>
#include <stdlib.h>

// Note this implementation assumes static lifetime of strings inserted to queue

static list *new_list_element(const char *str) {
    list *elem = (list *)malloc(sizeof(list));
    if (elem == NULL) {
        return NULL;
    }
    elem->data = str;
    elem->next = NULL;
    return elem;
}

bool queue_put(str_queue *q, const char *str) {
    list *elem = new_list_element(str);
    if (elem == NULL) {
        return false;
    }

    if (q->head == NULL) {
        // head and tail are NULL
        elem->next = elem;
        q->head = elem;
        q->tail = elem;
    } else {
        q->head->next = elem;
        q->head = elem;
    }

    return true;
}

const char *queue_get(str_queue *q) {
    if (queue_is_empty(q)) {
        return NULL;
    }

    const char *ret = q->tail->data;
    list *tail = q->tail;

    if (tail->next == tail) {
        // There is only one item in queue
        q->head = NULL;
        q->tail = NULL;
    } else {
        q->tail = tail->next;
    }
    free(tail);

    return ret;
}

bool queue_is_empty(const str_queue *q) {
    return q->tail == NULL;
}
