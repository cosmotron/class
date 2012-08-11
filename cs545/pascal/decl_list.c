#include <stdio.h>
#include <stdlib.h>
#include "decl_list.h"

/*
 * Code here is functionally equivalent to arg_elem.c
 */
void decl_destroy(decl_elem *head) {
  if (head == NULL)
    return;

  decl_elem *curr, *tmp;

  curr = head->next;
  head->next = NULL;

  while (curr != NULL) {
    tmp = curr->next;
    free(curr);
    curr = tmp;
  }
}


decl_elem *decl_append(decl_elem *head, char *name) {
  decl_elem *tmp;

  if (head == NULL) {
    //fprintf(stderr, "head is null\n");
    if ((head = (decl_elem *) malloc(sizeof(decl_elem))) == NULL) {
      printf("could not malloc\n");
      exit(1);
    }

    head->name = name;
    head->next = head;
  }
  else {
    //fprintf(stderr, "entered else, name = %s\n", name);
    tmp = head;

    while (tmp->next != head)
      tmp = tmp->next;

    if ((tmp->next = (decl_elem *)malloc(sizeof(decl_elem))) == NULL) {
      printf("coul not malloc\n");
      exit(1);
    }

    tmp = tmp->next;
    tmp->name = name;
    tmp->next = head;
  }

  //fprintf(stderr, "head->name before return: %s\n", head->name);

  return head;
}
