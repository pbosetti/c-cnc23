//   _     _       _            _   _ _     _
//  | |   (_)_ __ | | _____  __| | | (_)___| |_
//  | |   | | '_ \| |/ / _ \/ _` | | | / __| __|
//  | |___| | | | |   <  __/ (_| | | | \__ \ |_
//  |_____|_|_| |_|_|\_\___|\__,_| |_|_|___/\__|
// Lesson of March 6th, 2022
// Compile with clang src/linkedlist.c -o linkedlist
// Also see https://en.wikipedia.org/wiki/Linked_list
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// The list is a chain of structures linked with twin pointers:
//         ┌───┬──────┐     ┌───┬──────┐     ┌───┬──────┐
// NULL ◄──┤PRV│      │◄────┤ P │      │◄────┤ P │      │
//         ├───┘  ┌───┤     ├───┘  ┌───┤     ├───┘  ┌───┤
//         │      │NXT├────►│      │ N ├────►│      │ N ├──► NULL
//         └──────┴───┘     └──────┴───┘     └──────┴───┘
// So, strictly speaking we are implemting a doubly linked list
// A singly linked list would be simpler, but it can only be travelled
// forward.


//   _____ _                           _   
//  | ____| | ___ _ __ ___   ___ _ __ | |_ 
//  |  _| | |/ _ \ '_ ` _ \ / _ \ '_ \| __|
//  | |___| |  __/ | | | | |  __/ | | | |_ 
//  |_____|_|\___|_| |_| |_|\___|_| |_|\__|

// structure representing an object in the list
typedef struct element {
  char *id;
  // possibly more fields in here!
  struct element *next;
  struct element *prev;
} element_t;

// Create a new element
element_t *element_new(char *id) {
  element_t *e = malloc(sizeof(element_t));
  memset(e, 0, sizeof(element_t));
  e->id = malloc(strlen(id) + 1);
  strncpy(e->id, id, strlen(id));
  return e;
}

// Free memory allocated for an element
void element_free(element_t *e) {
  free(e->id);
  free(e);
}


//   _     _     _   
//  | |   (_)___| |_ 
//  | |   | / __| __|
//  | |___| \__ \ |_ 
//  |_____|_|___/\__|
                  
// structure representing the list itself
typedef struct {
  element_t *first, *last;
  size_t length;
} list_t;

// Create a new list with one element in it
list_t *list_new(char *id) {
  // allocate memory for the list
  list_t *l = malloc(sizeof(list_t));
  // create a new element
  element_t *e = element_new(id);
  // initialize list fields
  l->first = e;
  l->last = e;
  l->length = 1;
  return l;
}

// apending an existing element to the list
void list_append_element(list_t *list, element_t *e) {
  list->last->next = e; // the next element of current last becomes e
  e->prev = list->last; // the previus element of e becomes the previous last
  list->last = e;       // the new last element is e
  e->next = NULL;       // there is nothing after e
  list->length++;       // increase by 1 the list length
}

// creating and appending a new element
element_t *list_append(list_t *list, char *id) {
  element_t *e = element_new(id);
  list_append_element(list, e);
  return e;
}

// inserting an existing element after a given ID
//
//                             "after"
//         ┌───┬──────┐     ┌───┬──────┐       ┌───┬──────┐
// NULL ◄──┤PRV│      │◄────┤ P │      │     ┌─┤ P │      │
//         ├───┘  ┌───┤     ├───┘  ┌───┤     │ ├───┘  ┌───┤
//         │      │NXT├────►│      │ N ├──┐  │ │      │ N ├──► NULL
//         └──────┴───┘     └────▲─┴───┘  │  │ └───▲──┴───┘
//                               │   ┌───┐▼──▼──┐  │
//                               └───┤ P │      │  │
//                       new element ├───┘  ┌───┤  │
//                    to be inserted │      │ N ├──┘
//                                   └──────┴───┘
//
void list_insert_element(list_t *list, element_t *new, char *after) {
  element_t *e;
  e = list->first;
  do {
    if (strcmp(e->id, after) == 0) {
      new->prev = e;
      new->next = e->next;
      e->next->prev = new;
      e->next = new;
      list->length++;
      break;
    }
  } while ((e = e->next));
}

// creating and inserting a new element after a given ID
element_t *list_insert(list_t *list, char *id, char *after) {
  element_t *e = element_new(id);
  list_insert_element(list, e, after);
  return e;
}

// delete an element with a given ID
void list_delete(list_t *list, char *id) {
  element_t *e;
  e = list->first;
  do {
    if (strcmp(e->id, id) == 0) { // id is found
      e->prev->next = e->next;
      e->next->prev = e->prev;
      element_free(e);
      list->length--;
      break;
    }
  } while ((e = e->next));
}

// Free all memory
void list_free(list_t *list) {
  element_t *e, *next;
  e = list->first;
  do {
    // before freeing element e, take note of its next element
    next = e->next;
    element_free(e);
  } while ((e = next));
  free(list);
}

// possible directions of traveling along the list
typedef enum {
  ASC, // from first to last, forward
  DESC // from last to first, backward
} loop_order_t;

// prototype of a function to be iterated over all elements in the list
// userdata is a pointer to anything: a value, an aray or a struct; this
// enables the user to implement the function with maximum flexibility
typedef void (*loop_fun_t)(element_t *e, loop_order_t o, void *userdata);
//     |-a-|   |---b----|  |--------------------c---------------------|
// (a) is the return function
// (b) is the name of the type
// (c) is the function prototype, i.e. the list of arguments
// any function passed as 2nd argument of list_loop() must adhere this
// signature, i.e. return void and take arguments as in (a)

// loop over all elements in the list in a given order, calling fun over
// each element in turn
void list_loop(list_t *list, loop_fun_t fun, loop_order_t order,
               void *userdata) {
  element_t *e;
  // start from beginning or end, according to order
  if (order == ASC)
    e = list->first;
  else
    e = list->last;

  // loop over all elements
  do {
    // and call fun on each of them
    fun(e, order, userdata);
    // determine the next element according to order
    if (order == ASC)
      e = e->next;
    else
      e = e->prev;
  } while (e);
}


//    ____      _ _ _                _        
//   / ___|__ _| | | |__   __ _  ___| | _____ 
//  | |   / _` | | | '_ \ / _` |/ __| |/ / __|
//  | |__| (_| | | | |_) | (_| | (__|   <\__ \
//   \____\__,_|_|_|_.__/ \__,_|\___|_|\_\___/
// 
// Loop callback 1: print an element and its pointers
void print_element(element_t *e, loop_order_t o, void *userdata) {
  printf("%10s: %15p -> %15p -> %15p\n", e->id, e->prev, e, e->next);
}

// Loop callback 2: Print and element and its order
void print_element_with_index(element_t *e, loop_order_t o, void *userdata) {
  size_t *i = (size_t *)userdata;
  printf("list[%lu]: %s\n", *i, e->id);
  if (o == ASC) {
    (*i)++;
  }
  else {
    (*i)--;
  }
}


//   __  __       _       
//  |  \/  | __ _(_)_ __  
//  | |\/| |/ _` | | '_ \
//  | |  | | (_| | | | | |
//  |_|  |_|\__,_|_|_| |_|
// 
int main() {
  char id[4][10] = {"one", "two", "three", "four"};
  element_t *e;
  size_t i;

  // create a list
  list_t *list = list_new("zero");

  // populate the list with the other elements
  for (i = 0; i < 4; i++) {
    // create an element and append to the list
    e = element_new(id[i]);
    list_append_element(list, e);
    // or, alternatively:
    // list_append(list, id[i]);
  }

  // loop forward
  list_loop(list, print_element, ASC, NULL);

  // insert a new element
  list_insert(list, "two.five", "two");

  // loop backward
  list_loop(list, print_element, DESC, NULL);

  // print with index
  i = 0;
  list_loop(list, print_element_with_index, ASC, &i);

  i = list->length - 1;
  list_loop(list, print_element_with_index, DESC, &i);

  // free memory
  list_free(list);

  return 0;
}
