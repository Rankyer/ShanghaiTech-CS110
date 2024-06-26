#include "cache.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

void L2_store(struct cache *cache, uint8_t *data, uint32_t addr);
void L2_load(struct cache *cache, uint8_t *data, uint32_t addr);

struct cache_line *find_victim(struct cache *cache, uint32_t index)
{
    struct cache_line *victim = NULL;
    uint64_t oldest = UINT64_MAX;
    uint32_t num_of_sets = cache->config.lines / cache->config.ways;
    for (uint32_t i = 0; i < cache->config.ways; i++)
    {
        struct cache_line *line = &cache->lines[index + i * num_of_sets];
        if (!line->valid) // empty
        {
            return line;
        }
        else if (line->last_access < oldest)
        {
            victim = line;
            oldest = line->last_access;
        }
    }
    return victim;
}

static uint32_t calculate_mask(uint32_t bits)
{
    return (1 << bits) - 1;
}

static uint32_t calculate_bits(uint32_t value)
{
    uint32_t bits = 0;
    while (value >>= 1)
        ++bits;
    return bits;
}

/* Create a cache simulator according to the config */
struct cache *cache_create(struct cache_config config, struct cache *lower_level)
{
    /*YOUR CODE HERE*/
    struct cache *cache = malloc(sizeof(struct cache));
    if (!cache)
        return NULL;
    cache->config = config;
    uint32_t sets = config.lines / config.ways;
    cache->lines = malloc(sizeof(struct cache_line) * config.lines);
    if (!cache->lines)
    {
        free(cache);
        return NULL;
    }
    cache->offset_bits = calculate_bits(config.line_size);
    cache->index_bits = calculate_bits(sets);
    cache->tag_bits = config.address_bits - cache->index_bits - cache->offset_bits;
    cache->offset_mask = calculate_mask(cache->offset_bits);
    cache->index_mask = calculate_mask(cache->index_bits) << cache->offset_bits;
    cache->tag_mask = calculate_mask(cache->tag_bits) << (cache->offset_bits + cache->index_bits);
    for (uint32_t i = 0; i < config.lines; i++)
    {
        cache->lines[i].valid = false;
        cache->lines[i].dirty = false;
        cache->lines[i].tag = 0;
        cache->lines[i].last_access = 0;
        cache->lines[i].data = malloc(config.line_size);
        memset(cache->lines[i].data, 0, config.line_size);
    }
    if (lower_level != NULL)
    {
        cache->lower_cache = lower_level;
    }
    else
    {
        cache->lower_cache = NULL;
    }
    return cache;
}

/*
Release the resources allocated for the cache simulator.
Also writeback dirty lines

The order in which lines are evicted is:
set0-slot0, set1-slot0, set2-slot0, (the 0th way)
set0-slot1, set1-slot1, set2-slot1, (the 1st way)
set0-slot2, set1-slot2, set2-slot2, (the 2nd way)
and so on.
*/
void cache_destroy(struct cache *cache)
{
    /*YOUR CODE HERE*/

    for (uint32_t i = 0; i < cache->config.lines; i++)
    {
        if (cache->config.write_back && cache->lines[i].dirty)
        {
            uint32_t addr = (cache->lines[i].tag << (cache->index_bits + cache->offset_bits)) |
                            ((i << cache->offset_bits) & ((1 << (cache->index_bits + cache->offset_bits)) - 1));
            if(cache->lower_cache)
            {
                L2_store(cache->lower_cache, cache->lines[i].data, addr);
            }
            else
            {
                mem_store(cache->lines[i].data, addr, cache->config.line_size);
            }
        }
        free(cache->lines[i].data);
    }
    free(cache->lines);
    free(cache);
}

/* Read one byte at a specific address. return hit=true/miss=false */
bool cache_read_byte(struct cache *cache, uint32_t addr, uint8_t *byte)
{
    /*YOUR CODE HERE*/

    uint32_t offset = addr & (cache->config.line_size - 1);
    uint32_t index = (addr >> cache->offset_bits) & ((1 << cache->index_bits) - 1);
    uint32_t tag = addr >> (cache->index_bits + cache->offset_bits);
    uint32_t num_of_sets = cache->config.lines / cache->config.ways;
    for (uint32_t i = 0; i < cache->config.ways; i++)
    {
        struct cache_line *line = &cache->lines[index + i * num_of_sets];
        if (line->valid && line->tag == tag)
        {
            *byte = line->data[offset];
            line->last_access = get_timestamp();
            return true;
        }
    }

    // Miss
    struct cache_line *victim = find_victim(cache, index);
    if (cache->config.write_back == true) // write back
    {
        if (victim->dirty)
        {
            uint32_t victim_addr = (victim->tag << (cache->index_bits + cache->offset_bits)) |
                                   (index << cache->offset_bits);

            if (cache->lower_cache)
            {
                L2_store(cache->lower_cache, victim->data, victim_addr);
            }
            else
            {
                mem_store(victim->data, victim_addr, cache->config.line_size);
            }
        }
    }
    victim->tag = tag;
    victim->valid = true;
    victim->dirty = false;
    uint32_t block_start = addr & ~(cache->config.line_size - 1);
    if (cache->lower_cache)
    {
        L2_load(cache->lower_cache, victim->data, block_start);
    }
    else
    {
        mem_load(victim->data, block_start, cache->config.line_size);
    }
    *byte = victim->data[offset];
    victim->last_access = get_timestamp();
    return false;
}

/* Write one byte into a specific address. return hit=true/miss=false*/
bool cache_write_byte(struct cache *cache, uint32_t addr, uint8_t byte)
{
    /*YOUR CODE HERE*/

    uint32_t offset = addr & (cache->config.line_size - 1);
    uint32_t index = (addr >> cache->offset_bits) & ((1 << cache->index_bits) - 1);
    uint32_t tag = addr >> (cache->index_bits + cache->offset_bits);
    uint32_t num_of_sets = cache->config.lines / cache->config.ways;
    for (uint32_t i = 0; i < cache->config.ways; i++)
    {
        struct cache_line *line = &cache->lines[index + i * num_of_sets];
        if (line->valid && line->tag == tag)
        {
            line->data[offset] = byte;
            line->dirty = true;
            line->last_access = get_timestamp();
            if (!cache->config.write_back) // Write-through
            {
                if (cache->lower_cache)
                {
                    L2_store(cache->lower_cache, line->data, addr & ~(cache->config.line_size - 1));
                }
                else
                {
                    mem_store(line->data, addr & ~(cache->config.line_size - 1), cache->config.line_size);
                }

                line->dirty = false;
            }
            return true;
        }
    }

    // Miss
    struct cache_line *victim = find_victim(cache, index);
    if (cache->config.write_back == true) // write back
    {
        if (victim->dirty)
        {
            uint32_t victim_addr = (victim->tag << (cache->index_bits + cache->offset_bits)) |
                                   (index << cache->offset_bits);
            if (cache->lower_cache)
            {
                L2_store(cache->lower_cache, victim->data, victim_addr);
            }
            else
            {
                mem_store(victim->data, victim_addr, cache->config.line_size);
            }
        }
        victim->tag = tag;
        victim->valid = true;
        victim->dirty = true;
        uint32_t block_start = addr & ~(cache->config.line_size - 1);

        if (cache->lower_cache)
        {
            L2_load(cache->lower_cache, victim->data, block_start);
        }
        else
        {
            mem_load(victim->data, block_start, cache->config.line_size);
        }
        victim->data[offset] = byte;
        victim->last_access = get_timestamp();
    }
    else // write through
    {
        victim->tag = tag;
        victim->valid = true;
        victim->dirty = false;
        victim->last_access = get_timestamp();
        uint32_t block_start = addr & ~(cache->config.line_size - 1);
        if (cache->lower_cache)
        {
            L2_load(cache->lower_cache, victim->data, block_start);
            victim->data[offset] = byte;
            L2_store(cache->lower_cache, victim->data, block_start);
        }
        else
        {
            mem_load(victim->data, block_start, cache->config.line_size);
            victim->data[offset] = byte;
            mem_store(victim->data, block_start, cache->config.line_size);
        }
    }
    // uint32_t block_start = addr & ~(cache->config.line_size - 1);
    // if (!cache->config.write_back)
    // {
    //     mem_store(victim->data, block_start, cache->config.line_size);
    // }
    return false; // Miss
}




void L2_store(struct cache *cache, uint8_t *data, uint32_t addr)
{
    
    //uint32_t offset = addr & (cache->config.line_size - 1);
    uint32_t index = (addr >> cache->offset_bits) & ((1 << cache->index_bits) - 1);
    uint32_t tag = addr >> (cache->index_bits + cache->offset_bits);
    uint32_t num_of_sets = cache->config.lines / cache->config.ways;
    for (uint32_t i = 0; i < cache->config.ways; i++)
    {
        struct cache_line *line = &cache->lines[index + i * num_of_sets];
        if (line->valid && line->tag == tag)
        {
            memcpy(line->data, data, cache->config.line_size);
            //line->data[offset] = byte;
            line->dirty = true;
            line->last_access = get_timestamp();
            if (!cache->config.write_back) // Write-through
            {
                mem_store(line->data, addr & ~(cache->config.line_size - 1), cache->config.line_size);
                line->dirty = false;
            }
            return;
        }
    }

    // Miss
    struct cache_line *victim = find_victim(cache, index);
    if (cache->config.write_back == true) // write back
    {
        if (victim->dirty)
        {
            uint32_t victim_addr = (victim->tag << (cache->index_bits + cache->offset_bits)) |
                                   (index << cache->offset_bits);
            mem_store(victim->data, victim_addr, cache->config.line_size);
        }
        victim->tag = tag;
        victim->valid = true;
        victim->dirty = true;
        uint32_t block_start = addr & ~(cache->config.line_size - 1);

        mem_load(victim->data, block_start, cache->config.line_size);
        memcpy(victim->data, data, cache->config.line_size);

        victim->last_access = get_timestamp();
    }
    else // write through
    {
        victim->tag = tag;
        victim->valid = true;
        victim->dirty = false;
        victim->last_access = get_timestamp();
        uint32_t block_start = addr & ~(cache->config.line_size - 1);

        mem_load(victim->data, block_start, cache->config.line_size);
        memcpy(victim->data, data, cache->config.line_size);
        mem_store(victim->data, block_start, cache->config.line_size);
    }
    // uint32_t block_start = addr & ~(cache->config.line_size - 1);
    // if (!cache->config.write_back)
    // {
    //     mem_store(victim->data, block_start, cache->config.line_size);
    // }
    return; // Miss

}

void L2_load(struct cache *cache, uint8_t *data, uint32_t addr)
{
    // uint32_t offset = addr & (cache->config.line_size - 1);
    uint32_t index = (addr >> cache->offset_bits) & ((1 << cache->index_bits) - 1);
    uint32_t tag = addr >> (cache->index_bits + cache->offset_bits);
    uint32_t num_of_sets = cache->config.lines / cache->config.ways;
    for (uint32_t i = 0; i < cache->config.ways; i++)
    {
        struct cache_line *line = &cache->lines[index + i * num_of_sets];
        if (line->valid && line->tag == tag)
        {
            memcpy(data, line->data, cache->config.line_size);
            line->last_access = get_timestamp();
            return;
        }
    }

    // Miss
    struct cache_line *victim = find_victim(cache, index);
    if (cache->config.write_back == true) // write back
    {
        if (victim->dirty)
        {
            uint32_t victim_addr = (victim->tag << (cache->index_bits + cache->offset_bits)) |
                                   (index << cache->offset_bits);
            mem_store(victim->data, victim_addr, cache->config.line_size);
        }
    }
    victim->tag = tag;
    victim->valid = true;
    victim->dirty = false;
    uint32_t block_start = addr & ~(cache->config.line_size - 1);
    mem_load(victim->data, block_start, cache->config.line_size);
    memcpy(data, victim->data, cache->config.line_size);
    victim->last_access = get_timestamp();
    return;

}