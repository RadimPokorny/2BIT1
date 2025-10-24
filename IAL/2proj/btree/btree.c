#include "btree.h"
#include <stdio.h>
#include <stdlib.h>

/*
 * Pomocná funkce pro výpis uzlu stromu.
 */
void bst_print_node(bst_node_t *node)
{
  printf("[%c,", node->key);
  bst_print_node_content(&node->content);
  printf("]");
}

/*
 * Pomocná funkce pro výpis obsahu uzlu.
 */
void bst_print_node_content(bst_node_content_t *content)
{
  if (content == NULL)
  {
    printf("NULL");
    return;
  }
  switch (content->type)
  {
  case INTEGER:
    printf("%d",*(int *)content->value);
    break;
    
  default:
    printf("Unknown");
    break;
  }
}


/*
 * Pomocná funkce pro uložení uzlu stromu do pomocné stuktury.
 */
void bst_add_node_to_items(bst_node_t *node, bst_items_t *items)
{
  if (items->capacity < items->size + 1)
  {
    items->capacity = items->capacity * 2 + 8;
    items->nodes = realloc(items->nodes, items->capacity * (sizeof(bst_node_t *)));
  }
  items->nodes[items->size] = node;
  items->size++;
}