#include "../inc/hash_chaining.h"
#include "../inc/hash_func.h"
#include <stdlib.h>

hash_chaining *hash_chaining_init(uint32_t size)
{
  hash_chaining *table = malloc(sizeof(hash_chaining));
  if (!table)
  {
    return NULL;
  }
  table->size = size;
  table->slots = malloc(sizeof(list_node *) * size);
  if (!table->slots)
  {
    free(table);
    return NULL;
  }
  for (uint32_t i = 0; i < size; ++i)
  {
    table->slots[i] = NULL;
  }
  table->parameters = generate_hash_parameters();
  return table;
}

void hash_chaining_insert(hash_chaining *table, uint32_t key)
{
  if (table)
  {
    uint32_t index = hash_func(key, table->parameters, table->size);
    list_node *new_node = malloc(sizeof(list_node));
    if (new_node)
    {
      new_node->key = key;
      new_node->next = table->slots[index];
      table->slots[index] = new_node;
    }
  }
}

bool hash_chaining_search(hash_chaining *table, uint32_t key)
{
  if (table)
  {
    uint32_t index = hash_func(key, table->parameters, table->size);
    for (list_node *current = table->slots[index]; current; current = current->next)
    {
      if (current->key == key)
      {
        return true;
      }
    }
  }
  return false;
}

void hash_chaining_destroy(hash_chaining *table)
{
  if (!table)
    return;

  for (uint32_t i = 0; i < table->size; ++i)
  {
    list_node *current = table->slots[i];
    while (current)
    {
      list_node *tmp = current;
      current = current->next;
      free(tmp);
    }
  }
  free(table->slots);
  free(table);
}