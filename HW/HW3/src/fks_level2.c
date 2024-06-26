#include "../inc/fks_level2.h"
#include "../inc/hash_func.h"
#include <string.h>

fks_level2 *fks_level2_init(uint32_t size, hash_parameters parameters)
{
  fks_level2 *table = malloc(sizeof(fks_level2));
  if (!table)
    return NULL;

  table->size = size;
  table->parameters = parameters;
  table->slots = malloc(size * sizeof(uint32_t));

  if (!table->slots)
  {
    free(table);
    return NULL;
  }

  for (uint32_t i = 0; i < size; i++)
  {
    table->slots[i] = FKS_LEVEL2_EMPTY;
  }

  return table;
}

fks_level2 *fks_level2_build(list_node *head, uint32_t size, hash_parameters parameters)
{
  fks_level2 *table = NULL;
  bool collision = false;

  while (1)
  {
    if (collision == true)
    {
      break;
    }
    table = fks_level2_init(size, parameters);
    if (!table)
    {
      return NULL;
    }
    list_node *current = head;
    while (current)
    {
      bool search;
      search = fks_level2_search(table, current->key);
      if (search != true)
      {
        collision = fks_level2_insert(table, current->key);
        if (!(collision))
        {
          fks_level2_destroy(table);
          break;
        }
        current = current->next;
      }
      else
      {
        current = current->next;
      }
    }
    parameters = generate_hash_parameters();
  }

  return table;
}

bool fks_level2_insert(fks_level2 *table, uint32_t key)
{
  if (!table)
    return false;

  uint32_t index = hash_func(key, table->parameters, table->size);
  if (table->slots[index] == FKS_LEVEL2_EMPTY)
  {
    table->slots[index] = key;
    return true;
  }
  return false;
}

bool fks_level2_search(fks_level2 *table, uint32_t key)
{
  if (!table)
    return false;
  uint32_t index = hash_func(key, table->parameters, table->size);
  bool flag = (table->slots[index] == key);
  return flag;
}

void fks_level2_destroy(fks_level2 *table)
{
  if (!table)
    return;
  free(table->slots);
  free(table);
}