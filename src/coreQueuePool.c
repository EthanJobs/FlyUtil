#include <coreQueuePool.h>
#include <stdlib.h>

QueuePool *QueuePool_init(void (*fun_free)(void *)) {
    QueuePool *qp = (QueuePool *)malloc(sizeof(QueuePool));
    qp->n_queue = Queue_init(fun_free);
    pthread_mutex_init(&qp->n_lock, NULL);
    pthread_cond_init(&qp->n_haveValue, NULL);

    return qp;
}

int QueuePool_push(QueuePool *qp, void *qpv) {
    pthread_mutex_lock(&qp->n_lock);

    int ret = Queue_push(qp->n_queue, (void *)qpv);

    pthread_cond_broadcast(&qp->n_haveValue);
    pthread_mutex_unlock(&qp->n_lock);
    return ret;
}

void *QueuePool_pop(QueuePool *qp) {
    pthread_mutex_lock(&qp->n_lock);

    while (Queue_isEmpty(qp->n_queue))
        pthread_cond_wait(&qp->n_haveValue, &qp->n_lock);

    void *ret = Queue_pop(qp->n_queue);

    pthread_mutex_unlock(&qp->n_lock);
    return ret;
}

void QueuePool_display(QueuePool *qp, void (*fun_print)(void *)) {
    pthread_mutex_lock(&qp->n_lock);

    Iterator *i = Queue_getIterator(qp->n_queue);

    while (i->fun_hasNext(i))
        fun_print(i->fun_next(i));

    pthread_mutex_unlock(&qp->n_lock);

    return;
}

void QueuePool_free(QueuePool *qp) {
    pthread_mutex_lock(&qp->n_lock);

    Queue_free(qp->n_queue);

    pthread_mutex_unlock(&qp->n_lock);
    pthread_mutex_destroy(&qp->n_lock);
    pthread_cond_destroy(&qp->n_haveValue);

    return;
}