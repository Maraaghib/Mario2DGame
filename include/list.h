
#ifndef LIST_IS_DEF
#define LIST_IS_DEF

typedef struct list_cell {
  void *value;
  struct list_cell *prev, *next;
} *list_t;

void list_init (list_t *l);
void list_add (list_t *l, void *value);
void list_remove (list_t *l, list_t cell);

#define list_for_each (l, p) \
  for(p = (l)->next; p != (l); p = p->next)

#endif
