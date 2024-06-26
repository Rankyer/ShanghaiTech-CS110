#include "../inc/simulator.h"
#include "../inc/TLB.h"
#include <assert.h>
#include <stdlib.h>

#include <stdbool.h>
#include <stdio.h>

void context_switch(Process *current_process) {
  if (global_tlb->pid != current_process->pid) {
    flush_TLB();
    global_tlb->pid = current_process->pid;
  }
}

unsigned page_walk(Process *proc, unsigned virtual_page_number) {
  unsigned idx_l1 = virtual_page_number >> L2_BITS;
  unsigned idx_l2 = virtual_page_number & ((1 << L2_BITS) - 1);

  L2_PAGE_TABLE *table_l2 = &proc->page_table.entries[idx_l1];
  if (!table_l2->entries || !table_l2->entries[idx_l2].valid) {
    return ERROR;
  }
  return table_l2->entries[idx_l2].frame;
}

void prepare_page_table_entry(Process *proc, addr_t addr, unsigned *idx_l1,
                              unsigned *idx_l2) {
  (void)proc;
  *idx_l1 = (addr >> (L2_BITS + OFFSET_BITS)) & ((1 << L1_BITS) - 1);
  *idx_l2 = (addr >> OFFSET_BITS) & ((1 << L2_BITS) - 1);
}

status_t initialize_l2_table(Process *proc, unsigned idx_l1) {
  if (!proc->page_table.entries[idx_l1].entries) {
    proc->page_table.entries[idx_l1].entries =
        calloc(L2_PAGE_TABLE_SIZE, sizeof(PTE));
    if (!proc->page_table.entries[idx_l1].entries) {
      return ERROR;
    }
  }
  return SUCCESS;
}

void set_page_table_entry(L2_PAGE_TABLE *l2_table, unsigned idx_l2,
                          unsigned frame, bool valid) {
  l2_table->entries[idx_l2].frame = frame;
  l2_table->entries[idx_l2].valid = valid ? 1 : 0;
  if (valid) {
    l2_table->valid_count++;
  } else {
    l2_table->valid_count--;
  }
}

status_t allocate_page(Process *proc, addr_t addr, addr_t phys_addr) {
  // This is guaranteed by the simulator
  assert(addr >> OFFSET_BITS << OFFSET_BITS == addr);
  assert(phys_addr >> OFFSET_BITS << OFFSET_BITS == phys_addr);
  // 1. Check if the process is valid
  if (proc == NULL) {
    return ERROR;
  }
  // TODO: Implement me!
  unsigned idx_l1, idx_l2;
  prepare_page_table_entry(proc, addr, &idx_l1, &idx_l2);

  status_t status = initialize_l2_table(proc, idx_l1);
  if (status != SUCCESS)
    return ERROR;

  L2_PAGE_TABLE *l2_table = &proc->page_table.entries[idx_l1];
  if (l2_table->entries[idx_l2].valid)
    return ERROR;
  set_page_table_entry(l2_table, idx_l2, phys_addr >> OFFSET_BITS, true);
  return SUCCESS;
}

status_t deallocate_page(Process *proc, addr_t addr) {
  // This is guaranteed by the simulator
  assert(addr >> OFFSET_BITS << OFFSET_BITS == addr);
  // 1. Check if the process is valid
  if (proc == NULL) {
    return ERROR;
  }
  // TODO: Implement me!
  unsigned idx_l1, idx_l2;
  prepare_page_table_entry(proc, addr, &idx_l1, &idx_l2);

  L2_PAGE_TABLE *l2_table = &proc->page_table.entries[idx_l1];
  if (!l2_table->entries || !l2_table->entries[idx_l2].valid) {
    return ERROR;
  }

  set_page_table_entry(l2_table, idx_l2, 0, false);

  if (l2_table->valid_count == 0) {
    free(l2_table->entries);
    l2_table->entries = NULL;
  }
  return SUCCESS;
}

status_t access(Process *proc, addr_t addr, byte_t *data, bool write) {
  if (!proc || !data)
    return ERROR;

  context_switch(proc);

  unsigned vpn = addr >> OFFSET_BITS, offset = addr & (PAGE_SIZE - 1);
  unsigned ppn = read_TLB(proc->pid, vpn);

  if (ppn != ERROR) {
    if (write)
      main_memory->pages[ppn]->data[offset] = *data;
    else
      *data = main_memory->pages[ppn]->data[offset];
    return TLB_HIT;
  }

  ppn = page_walk(proc, vpn);
  if (ppn == ERROR)
    return ERROR;

  write_TLB(proc->pid, vpn, ppn);
  if (write)
    main_memory->pages[ppn]->data[offset] = *data;
  else
    *data = main_memory->pages[ppn]->data[offset];
  return SUCCESS;
}

status_t write_byte(Process *proc, addr_t addr, const byte_t *byte) {
  return access(proc, addr, (byte_t *)byte, true);
}

status_t read_byte(Process *proc, addr_t addr, byte_t *byte) {
  return access(proc, addr, byte, false);
}