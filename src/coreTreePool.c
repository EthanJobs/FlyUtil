#include <coreTreePool.h>
#include <stdlib.h>

TreePool *TreePool_init(int (*fun_compare)(void *, void *), void (*fun_free)(void *)) {
    TreePool *tp = (TreePool *)malloc(sizeof(TreePool));
    tp->n_tree = Tree_init(fun_compare, fun_free);
    pthread_mutex_init(&tp->n_lock, NULL);

    return tp;
}

int TreePool_insert(TreePool *tp, void *tpv) {
    pthread_mutex_lock(&tp->n_lock);

    int ret = Tree_insertValue(tp->n_tree, tpv);

    pthread_mutex_unlock(&tp->n_lock);

    return ret;
}

void *TreePool_get(TreePool *tp, int clientFd, int (*fun_compare)(void *, void *)) {
    pthread_mutex_lock(&tp->n_lock);

    void *ret = Tree_getValue(tp->n_tree, (void *)&clientFd, fun_compare);

    pthread_mutex_unlock(&tp->n_lock);

    return ret;
}

void TreePool_display(TreePool *tp, int tmp, void (*fun_print)(void *)) {
    pthread_mutex_lock(&tp->n_lock);

    Iterator *i;

    if (tmp = 0)
        i = Tree_getPreorderIterator(tp->n_tree);
    else if (tmp == 1)
        i = Tree_getInorderIterator(tp->n_tree);
    else if (tmp == 2)
        i = Tree_getPostorderIterator(tp->n_tree);
    else
        i = Tree_getLevelorderIterator(tp->n_tree);

    while (i->fun_hasNext(i))
        fun_print(i->fun_next(i));

    pthread_mutex_unlock(&tp->n_lock);

    return;
}

void TreePool_free(TreePool *tp) {
    pthread_mutex_lock(&tp->n_lock);

    Tree_free(tp->n_tree);

    pthread_mutex_unlock(&tp->n_lock);
    pthread_mutex_destroy(&tp->n_lock);

    return;
}