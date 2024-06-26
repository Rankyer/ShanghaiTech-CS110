#include "../inc/fks_level1.h"
#include "../inc/hash_func.h"
#include <stdio.h>

static int get_list_len(list_node *node)
{
  int len = 0;
  while (node)
  {
    len++;
    node = node->next;
  }
  return len;
}

fks_level1 *fks_level1_build(hash_chaining *hash_chaining_table)
{
  if (!hash_chaining_table)
    return NULL;

  fks_level1 *level1_table = malloc(sizeof(fks_level1));
  if (!level1_table)
    return NULL;

  level1_table->size = hash_chaining_table->size;
  level1_table->parameters = hash_chaining_table->parameters;
  level1_table->level2_tables = malloc(sizeof(fks_level2 *) * (level1_table->size));

  if (!level1_table->level2_tables)
  {
    free(level1_table);
    return NULL;
  }

  for (uint32_t i = 0; i < level1_table->size; i++)
  {
    list_node *head = hash_chaining_table->slots[i];
    int list_len = get_list_len(head);

    if (list_len > 0)
    {
      int temp_len = list_len * list_len;
      fks_level2 *level2_table = fks_level2_build(head, temp_len, generate_hash_parameters());
      level1_table->level2_tables[i] = level2_table;
    }
    else
    {
      level1_table->level2_tables[i] = NULL;
    }
  }

  return level1_table;
}

bool fks_level1_search(fks_level1 *table, uint32_t key)
{
  if (!table)
    return false;
  uint32_t index = hash_func(key, table->parameters, table->size);
  return table->level2_tables[index] && fks_level2_search(table->level2_tables[index], key);
}

void fks_level1_destroy(fks_level1 *table)
{
  if (!table)
    return;

  for (uint32_t i = 0; i < table->size; i++)
  {
    if (table->level2_tables[i])
    {
      fks_level2_destroy(table->level2_tables[i]);
    }
  }

  free(table->level2_tables);
  free(table);
}