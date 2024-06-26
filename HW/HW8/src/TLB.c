#include "../inc/TLB.h"

#include <stdlib.h>

unsigned global_TLB_init(void) {
  global_tlb = calloc(1, sizeof(TLB));
  return 0;
}

void global_TLB_free(void) {
  if (global_tlb == NULL) {
    return;
  }
  free(global_tlb);
  global_tlb = NULL;
}

int find_lru_index() {
  int index_least_recently_used = 0;
  uint32_t least_recently_used_time = global_tlb->entries[0].lut;

  for (int i = 1; i < TLB_SIZE; i++) {
    if (!global_tlb->entries[i].valid ||
        global_tlb->entries[i].lut < least_recently_used_time) {
      index_least_recently_used = i;
      least_recently_used_time = global_tlb->entries[i].lut;
    }
  }
  return index_least_recently_used;
}

void update_tlb_entry(int index, unsigned vpn, unsigned ppn) {
  global_tlb->entries[index].vpn = vpn;
  global_tlb->entries[index].ppn = ppn;
  global_tlb->entries[index].valid = 1;
  global_tlb->entries[index].lut = ++global_tlb->clock;
}

unsigned read_TLB(proc_id_t pid, unsigned vpn) {
  (void)pid;
  for (int i = 0; i < TLB_SIZE; i++) {
    if (global_tlb->entries[i].valid && global_tlb->entries[i].vpn == vpn) {
      global_tlb->entries[i].lut = ++global_tlb->clock;
      return global_tlb->entries[i].ppn;
    }
  }
  return ERROR;
}

void write_TLB(proc_id_t pid, unsigned vpn, unsigned ppn) {
  (void)pid;
  int idx_to_update = find_lru_index();
  update_tlb_entry(idx_to_update, vpn, ppn);
}

void flush_TLB() {
  for (int i = 0; i < TLB_SIZE; i++) {
    global_tlb->entries[i].valid = 0;
  }
}