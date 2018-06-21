#include <stdio.h>

#include "dlist.h"


static dl_entry_t* new_entry(void* data);
static dl_entry_t* find_first_inst(dl_entry_t* head,void* data);

// Insert new entry
// Return malloced entry
dl_entry_t* insque(dl_entry_t* head, void* data) {
   dl_entry_t* cursor;

   if (head == NULL) {
      return new_entry(data);
   }

   cursor = head;
   while(cursor->next != NULL) {
       cursor = cursor->next;
   }
   cursor->next = new_entry(data);

   return cursor->next;
}

// remove entry
// return head
dl_entry_t* deque(dl_entry_t* head, dl_entry_t* entry) {
   dl_entry_t* cursor;

   if (head == NULL) {
      return NULL;
   }

   // check if the head changed
   if (head == entry) {
      cursor = head->next;
      free(head);
      return cursor;
   }

   cursor = head;
   while(cursor != NULL) {
      if (cursor->next == entry) {
         cursor->next = entry->next;
      }
      cursor = cursor->next;
   }

   free(entry);
   return head;
}

// Deque the first instance of the data (compaired as void *)
// Also remember you need to free or hold on to the value
dl_entry_t* deque_val(dl_entry_t* head, void* data) {
   dl_entry_t* found_entry = find_first_inst(head,data);
   if (found_entry == NULL) {
      return head;
   }
   return deque(head,found_entry);
}

// Assume all types are pointers?
dl_entry_t* find_first_inst(dl_entry_t* head,void* data){
   dl_entry_t* cursor;

   if (head == NULL) {
      return NULL;
   }

   cursor = head;

   if (cursor->data == data) {
      return cursor;
   }
   while (cursor->next != NULL) {
      if (cursor->next->data == data) {
         return cursor->next;
      }
      cursor = cursor->next;
   }

   return NULL;
}

dl_entry_t* new_entry(void* data) {
   dl_entry_t* tmp_entry;
   tmp_entry = malloc(sizeof(dl_entry_t));
   if (tmp_entry == NULL) {
       perror("Unable to malloc new linked list entry");
   }
   tmp_entry->next = NULL;
   tmp_entry->data = data;
   return tmp_entry;
}

void test_list() {
   dl_entry_t* head;
   dl_entry_t* tmp_entry;
   dl_entry_t* last_entry;
   void* wild = 99;

   head = insque(NULL,80);
   insque(head,81);
   tmp_entry = insque(head,82);
   insque(head,83);
   insque(head,84);
   insque(head,wild);
   last_entry = insque(head,85);

   printf("Inital list\n");
   print_list(head);

   head = deque(head,tmp_entry);
   printf("\nRemove 82\n");
   print_list(head);

   head = deque_val(head,wild);
   printf("\nRemove wild %p\n",wild);
   print_list(head);

   head = deque(head,head);
   printf("\nRemove head\n");
   print_list(head);

   head = deque(head,last_entry);
   printf("\nRemove last entry\n");
   print_list(head);

   while(head != NULL) {
      head = deque(head,head);
      printf("\nRemove head\n");
      print_list(head);
   }

   //
   printf("Add wild %p\n",wild);
   head=insque(head,wild);
   print_list(head);
   printf("Remove wild %p\n",wild);
   head = deque_val(head,wild);
   print_list(head);
}

void print_list(dl_entry_t* head) {
   dl_entry_t* tmp_entry = head;
   while(tmp_entry != NULL) {
      printf("entry:%p\n", (void *)tmp_entry->data);
      tmp_entry = tmp_entry->next;
   }
}