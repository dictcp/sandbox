#ifndef __SKIP_LIST_H
#define __SKIP_LIST_H

#ifndef __SKIP_LIST_H_LEVEL
#define __SKIP_LIST_H_LEVEL 10
#endif

/* This file is from Linux Kernel (include/linux/list.h) 
 * and modified by simply removing hardware prefetching of list items. 
 * Here by copyright, credits attributed to wherever they belong.
 * Kulesh Shanmugasundaram (kulesh [squiggly] isis.poly.edu)
 */

/*
 * Simple doubly linked skiplist implementation.
 * by Dick Tang (tangcp _at_ cse.cuhk.edu.hk)
 * 
 * It is recommended to use skiplist function in order not 
 * to break the skiplist structure. The related functions
 * are marked with "(skiplist_fcn)".
 *
 * Some of the internal functions ("__xxx") are useful when
 * manipulating whole lists rather than single entries, as
 * sometimes we already know the next/prev entries and we can
 * generate better code by using them directly rather than
 * using the generic single-entry routines.
 */

struct list_head {
	struct list_head *prev;
	union { struct list_head *next; struct list_head *forward[__SKIP_LIST_H_LEVEL];};
	unsigned long long key;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
	struct list_head name = LIST_HEAD_INIT(name)

#define INIT_LIST_HEAD(ptr) do { \
	(ptr)->next = (ptr); (ptr)->prev = (ptr); \
} while (0)

#define INIT_SKIPLIST_HEAD(ptr) do { \
	(ptr)->next = (ptr); (ptr)->prev = (ptr); int i; for(i=0;i<__SKIP_LIST_H_LEVEL+1;i++) (ptr)->forward[i] = (ptr); (ptr)->key=0; \
} while (0)

/*
 * Insert a new entry between two known consecutive entries. 
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_add(struct list_head *new,
			      struct list_head *prev,
			      struct list_head *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

/**
 * list_add - add a new entry
 * @new: new entry to be added
 * @head: list head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
static inline void list_add(struct list_head *new, struct list_head *head)
{
	__list_add(new, head, head->next);
}

/**
 * list_add_tail - add a new entry
 * @new: new entry to be added
 * @head: list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static inline void list_add_tail(struct list_head *new, struct list_head *head)
{
	__list_add(new, head->prev, head);
}

/**
 * list_searchnear - search an near entry with specific key (skiplist_fcn)
 * @head: list head to query
 * @key: the query key
 *
 * Taking advantage of skiplist sturct, do a quick near-key-based search.
 * Return the largest-key entry with not larger than the target key.
 */
static inline struct list_head *list_searchnear(struct list_head *head, unsigned long long key)
{
	unsigned long long *level = &(head->key);
	struct list_head *header=head;
	int n = *level;
	do {
		while (head->forward[n]!=header&&head->forward[n]->key<key) {
			head=head->forward[n];
		}
		n--;
	} while (n>=0);
	if (head->next->key==key) head=head->next; //while??
	return head; 
}

/**
 * list_search - search an entry with specific key (skiplist_fcn)
 * @head: list head to query
 * @key: the query key
 *
 * Taking advantage of skiplist sturct, do a quick key-based search.
 * Return NULL if not found.
 */
static inline struct list_head *list_search(struct list_head *head, unsigned long long key)
{
	unsigned long long *level = &(head->key);
	struct list_head *header=head;
	int n = *level;
	do {
		while (head->forward[n]!=header&&head->forward[n]->key<key) {
			head=head->forward[n];
		}
		n--;
	} while (n>=0);
	head=head->next;
	if (head->key==key)
		return head; 
	else
		return NULL;
}

/*
 * list_sortingadd - add a new entry into skiplist (skiplist_fcn)
 * @new: new entry to be added
 * @head: list head to add it
 *
 * Insert a new entry into a specific skiplist head.
 * Note: the new entry will be inserted before the existing entry with the same key
 */
static inline void list_sortingadd(struct list_head *new, struct list_head *head)
{
	unsigned long long *level = &(head->key);
	struct list_head *header=head;
	unsigned long long key = new->key;

	struct list_head *update[__SKIP_LIST_H_LEVEL];
	int n = *level;
	do {
		while (head->forward[n]!=header&&head->forward[n]->key<key) {
			head=head->forward[n];
		}
		update[n]=head;
		n--;
	} while (n>=0);
	
	int i,v=0;
	while (((float)rand()/RAND_MAX) < 0.25 && v < __SKIP_LIST_H_LEVEL-1) v++;
	if (v > *level) {
		for (i=*level+1;i<=v;i++)
			update[i]=header;
		*level=v;
	}
	for (i=1;i<__SKIP_LIST_H_LEVEL;i++) new->forward[i]=header;
	__list_add(new, head, head->next);
	for (i=1;i<=v;i++) {
		new->forward[i] = update[i]->forward[i];
		update[i]->forward[i] = new;
	}
}

/**
 * list_sortedadd - add a new entry into skiplist (skiplist_fcn)
 * @new: new entry to be added
 * @head: list head to add it
 *
 * Insert a new entry into a specific skiplist head,
 * assuming the insert order is ascending.
 * Note: Currectly the implemenation requires a static variable,
 *       therefore the call cannot be reused elsewhere.
 */
static inline void list_sortedadd(struct list_head *new, struct list_head *head)
{
	unsigned long long *level = &(head->key);
	struct list_head *header=head;

	static struct list_head *update[__SKIP_LIST_H_LEVEL];
	static int v=0;
	
	if (v==0) head=head->prev; 
	else {
		int n = v;
		head = update[v];
		if (head==0x0) head=header;
		do {
			while (head->forward[n]!=header) {
				head=head->forward[n];
			}
			update[n]=head;
			n--;
		} while (n>=0);
	}
	int i;
	v=0;
	while (((float)rand()/RAND_MAX) < 0.25 && v < __SKIP_LIST_H_LEVEL-1) v++;
	if (v > *level) {
		for (i=*level+1;i<=v;i++)
			update[i]=header;
		*level=v;
	}
	for (i=1;i<__SKIP_LIST_H_LEVEL;i++) new->forward[i]=header;
	__list_add(new, head, head->next);
	for (i=1;i<=v;i++) {
		new->forward[i] = update[i]->forward[i];
		update[i]->forward[i] = new;
	}
}

/**
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_del(struct list_head *prev, struct list_head *next)
{
	next->prev = prev;
	prev->next = next;
}

/**
 * list_del - deletes entry from list.
 * @entry: the element to delete from the list.
 * Note: list_empty on entry does not return true after this, the entry is in an undefined state.
 */
static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	entry->next = (void *) 0;
	entry->prev = (void *) 0;
}

/**
 * list_safedel_by_head - deletes entry from list, with handling the skiplist metadata (skiplist_fcn)
 * @head: the list.
 * @node: the element to delete from the list.
 */
static inline void list_safedel_by_head(struct list_head *head, struct list_head *node)
{
	unsigned long long *level = &(head->key);
	struct list_head *header=head;
	unsigned long long key=node->key;

	struct list_head *update[__SKIP_LIST_H_LEVEL];
	int n = *level;
	do {
		while (head->forward[n]!=header&&head->forward[n]->key<key) {
			head=head->forward[n];
		}
		update[n]=head;
		n--;
	} while (n>=0);

	while (node!=head) {
		for (int i=1;i<*level;i++) if (head->forward[i]==head->next) update[i]=head;
		head=head->next;
	}

	if (head->key==key) {
		int i;
		for(i=1;i<=*level;i++) {
			if (update[i]->forward[i] != head) break;
			update[i]->forward[i]=head->forward[i];
		}
		list_del(head);
		while (*level>0&&(header->forward[*level]==header)) --*level;
	}

}

/**
 * list_del_init - deletes entry from list and reinitialize it.
 * @entry: the element to delete from the list.
 */
static inline void list_del_init(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	INIT_LIST_HEAD(entry); 
}

/**
 * list_move - delete from one list and add as another's head
 * @list: the entry to move
 * @head: the head that will precede our entry
 */
static inline void list_move(struct list_head *list, struct list_head *head)
{
        __list_del(list->prev, list->next);
        list_add(list, head);
}

/**
 * list_move_tail - delete from one list and add as another's tail
 * @list: the entry to move
 * @head: the head that will follow our entry
 */
static inline void list_move_tail(struct list_head *list,
				  struct list_head *head)
{
        __list_del(list->prev, list->next);
        list_add_tail(list, head);
}

/**
 * list_empty - tests whether a list is empty
 * @head: the list to test.
 */
static inline int list_empty(struct list_head *head)
{
	return head->next == head;
}

static inline void __list_splice(struct list_head *list,
				 struct list_head *head)
{
	struct list_head *first = list->next;
	struct list_head *last = list->prev;
	struct list_head *at = head->next;

	first->prev = head;
	head->next = first;

	last->next = at;
	at->prev = last;
}

/**
 * list_splice - join two lists
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 */
static inline void list_splice(struct list_head *list, struct list_head *head)
{
	if (!list_empty(list))
		__list_splice(list, head);
}

/**
 * list_splice_init - join two lists and reinitialise the emptied list.
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 *
 * The list at @list is reinitialised
 */
static inline void list_splice_init(struct list_head *list,
				    struct list_head *head)
{
	if (!list_empty(list)) {
		__list_splice(list, head);
		INIT_LIST_HEAD(list);
	}
}

/**
 * list_entry - get the struct for this entry
 * @ptr:	the &struct list_head pointer.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 */
#define list_entry(ptr, type, member) \
	((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

/**
 * list_for_each	-	iterate over a list
 * @pos:	the &struct list_head to use as a loop counter.
 * @head:	the head for your list.
 */
#define list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); \
        	pos = pos->next)
/**
 * list_for_each_prev	-	iterate over a list backwards
 * @pos:	the &struct list_head to use as a loop counter.
 * @head:	the head for your list.
 */
#define list_for_each_prev(pos, head) \
	for (pos = (head)->prev; pos != (head); \
        	pos = pos->prev)
        	
/**
 * list_for_each_safe	-	iterate over a list safe against removal of list entry
 * @pos:	the &struct list_head to use as a loop counter.
 * @n:		another &struct list_head to use as temporary storage
 * @head:	the head for your list.
 */
#define list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); \
		pos = n, n = pos->next)

/**
 * list_for_each_entry	-	iterate over list of given type
 * @pos:	the type * to use as a loop counter.
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 */
#define list_for_each_entry(pos, head, member)				\
	for (pos = list_entry((head)->next, typeof(*pos), member);	\
	     &pos->member != (head); 					\
	     pos = list_entry(pos->member.next, typeof(*pos), member))

/**
 * list_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 * @pos:	the type * to use as a loop counter.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 */
#define list_for_each_entry_safe(pos, n, head, member)			\
	for (pos = list_entry((head)->next, typeof(*pos), member),	\
		n = list_entry(pos->member.next, typeof(*pos), member);	\
	     &pos->member != (head); 					\
	     pos = n, n = list_entry(n->member.next, typeof(*n), member))

#endif
