#ifndef DLIST_H
#define DLIST_H


typedef struct dl_entry {
	struct dl_entry* next;
	void* data;
} dl_entry_t;

dl_entry_t* insque(dl_entry_t* head, void* data);
dl_entry_t* deque(dl_entry_t* head, dl_entry_t* entry);
void test_list();

#endif /*DLIST_H*/