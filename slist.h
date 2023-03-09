#ifndef _SLIST_H
#define _SLIST_H

typedef struct slist_s {
    void *data;
    struct slist_s *next;
} slist_t;

slist_t *slist_new(void);
slist_t *slist_append(slist_t *head, slist_t *add);
slist_t *slist_add_start(slist_t *head, slist_t *item);
slist_t *slist_insert_before(slist_t *head, slist_t *add, slist_t *before);
slist_t *slist_remove(slist_t *head, slist_t *del);
slist_t *slist_sort(slist_t *head, int (*cmp)(slist_t *,slist_t *,void *), void *data);
slist_t *slist_index(slist_t *head, unsigned int idx);
void slist_destroy(slist_t *head);
// only defined as char * to get around void * type limitations
// it is a contiguous array of pointers to the data pointers stored in the list

void **slist_array(slist_t *head);
#define slist_add_end slist_append


#endif
