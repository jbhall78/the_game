#include <stdio.h>
#include <stdlib.h>


#include <assert.h>

#include "slist.h"


//static slist_t *head;


slist_t *
slist_new(void)
{
    slist_t *item = malloc(sizeof(slist_t));
    assert(item != NULL);
    item->data = NULL;
    item->next = NULL;
    return item;
}


slist_t *
slist_add_start(slist_t *head, slist_t *item)
{
    item->next = head;
    head = item;

    return head;
}

slist_t *
slist_insert_before(slist_t *head, slist_t *add, slist_t *before)
{
    slist_t *item, *prev;

    for (item = head; item; item = item->next) {
	if (item == before) {
	    if (item == head) {
		add->next = head;
		head = add;
	    } else {
		prev->next = add;
		add->next = item;
	    }
	    break;
	}
	prev = item;
    }
    return head;
}

slist_t *
slist_append(slist_t *head, slist_t *add)
{
    slist_t *item;

    if (head == NULL) {
	return add;
    }

    for (item = head; item; item = item->next) {
	if (item->next == NULL) {
	    item->next = add;
	    break;
	}
    }

    return head;
}


slist_t *
slist_remove(slist_t *head, slist_t *del)
{
    slist_t *prev = NULL;
    slist_t *item;

    for (item = head; item; item = item->next) {
	// check to see if this is the item to delete
	if (item != del) {
	    prev = item;
	    continue;
	}

	// if we are mid-slist, update the next pointer on the previous item
	// thus removing the reference to the item to delete
	if (prev) {
	    prev->next = item->next;
	}

	// if the item to delete is at the head of the slist, update the head
	// item's next pointer to be its child item removing the item from the
	// slist
	// NOTE: this could be a "else if" instead of "if" as well for one less
	//       comparison in the loop
	if (item == head) {
	    head = head->next;
	}

	// removal completed, free memory associated with the item and exit loop
	break;
    }
    return head;
}

slist_t *
slist_index(slist_t *list, unsigned int idx)
{
    slist_t *item;
    unsigned int i;

    i = 0;
    for (item = list; item; item = item->next) {
	if (i == idx)
	    return item;
	i++;
    }
    return NULL;
}

void
slist_print(slist_t *head)
{
    slist_t *lp;
    int i;

    for (i = 0, lp = head; lp; lp = lp->next, i++) {
	printf("%d ", *((int *)lp->data));
    }
    printf("\n");
}


slist_t *
slist_sort(slist_t *head, int (*cmp)(slist_t *,slist_t *,void *), void *data)
{
    slist_t *A, *B;

start:
    for (A = head; A; A = A->next) {
	for (B = A->next; B; B = B->next) {
	    if (cmp(A, B, data) == -1) {
		head = slist_remove(head, B);
		head = slist_insert_before(head, /*item*/B, /*before*/A);
		goto start;
	    }
	}
    }

    return head;
}

unsigned int
slist_items(slist_t *head)
{
    slist_t *item;
    unsigned int i = 0;

    for (item = head; item; item = item->next)
	i++;

    return i;
}

void **
slist_array(slist_t *head)
{
    unsigned int n = slist_items(head);
    struct empty_s **array;
    unsigned int i;
    slist_t *item;

    array = malloc(n * sizeof(struct empty_s *) + 1);
    for (item = head, i = 0; item; item = item->next, i++)
	array[i] = (struct empty_s *)item->data;

    array[i] = NULL;

    return (void **)array;
}


void
slist_destroy(slist_t *head)
{
    slist_t *item, *next;

    for (item = head; item; item = next) {
	next = item->next;
	free(item);
    }
}

/*
 * returns: -1 for A greater
 *           0 for equal
 *           1 for B greater
 */
#if 0
int
cmp(slist_t *a, slist_t *b, void *data)
{
    int A = *((int *)a->data);
    int B = *((int *)b->data);

    if (A > B) {
	return -1;
    }
    if (B > A) {
	return 1;
    }
    return 0;
}
#endif

#if 0
int
main(int argc, char **argv)
{
    int i;
    slist_t *item;

    srand(1234);

    for (i = 0; i < 20; i++) {
	item = malloc(sizeof(slist_t));
	item->data = malloc(sizeof(int));
	*((int *)item->data) = rand() % 5 + 1;
	head = slist_add_start(head, item);
    }


    slist_print(head);
    head = slist_sort(head, cmp, NULL);
    slist_print(head);

    return 0;
}
#endif

/*
add to end of slist (NOTE: check for errors)
===========================================
new = malloc()
// fast forward to end of slist
for (item = head; item->next; item = item->next)
	;;
item->next = new


insert after (NOTE: check for errors)
=====================================
new = malloc()
after = item_to_insert_after

for (item = head; item; item = item->next) {
    if (item == after) {
	new->next = item->next
	item->next = new
	break
    }
}

insert before (NOTE: check for errors)
======================================
new = malloc()
before = item_to_insert_before

for (item = head; item; item = item->next) {
    if (item == before) {
        if (item == head) {
	    new->next = head
	    head = new
	} else {
	    prev->next = new
	    new->next = item
	}
	break;
    }
    prev = item;
}


loop through slist
=================
for (item = head; item; item = item->next) {
    .. do something ..
}

remove a particular slist item
=============================
prev = NULL
del = item_to_delete

for (item = head; item; item = item->next) {
    // check to see if this is the item to delete
    if (item != del) {
	prev = item;
	continue;
    }

    // if we are mid-slist, update the next pointer on the previous item
    // thus removing the reference to the item to delete
    if (prev) {
	prev->next = item->next;
    }

    // if the item to delete is at the head of the slist, update the head item's
    // next pointer to be its child item removing the item from the slist
    // NOTE: this could be a "else if" instead of "if" as well for one less
    //       comparison in the loop
    if (item == head) {
	head = head->next;
    }

    // removal completed, free memory associated with the item and exit loop
    free(item);
    break;
}


destroy entire slist
===================

for (item = head; item; item = next) {
    next = item->next;
    free(item);
}
*/
