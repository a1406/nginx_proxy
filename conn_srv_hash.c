#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <ctype.h>
#include "conn_srv_hash.h"

static void _dictReset(dictht *ht, dictType *type)
{
    ht->table = NULL;
    ht->size = 0;
    ht->sizemask = 0;
    ht->used = 0;
	ht->type = type;
}

/* Create a new hash table */
dictht *dictCreate(dictType *type)
{
    dictht *ht = malloc(sizeof(*ht));
//    _dictInit(ht,type,privDataPtr);
	_dictReset(ht, type);
    return ht;
}

static unsigned long _dictNextPower(unsigned long size)
{
    unsigned long i = DICT_HT_INITIAL_SIZE;

    if (size >= LONG_MAX) return LONG_MAX;
    while(1) {
        if (i >= size)
            return i;
        i *= 2;
    }
}

/* Expand or create the hashtable */
int dictExpand(dictht *ht, unsigned long size)
{
    dictht n; /* the new hashtable */
    unsigned long realsize = _dictNextPower(size), i;

    /* the size is invalid if it is smaller than the number of
     * elements already inside the hashtable */
    if (ht->used > size)
        return DICT_ERR;

    _dictReset(&n, ht->type);
    n.size = realsize;
    n.sizemask = realsize-1;
    n.table = calloc(realsize,sizeof(dictEntry*));

    /* Copy all the elements from the old to the new table:
     * note that if the old hash table is empty ht->size is zero,
     * so dictExpand just creates an hash table. */
    n.used = ht->used;
    for (i = 0; i < ht->size && ht->used > 0; i++) {
        dictEntry *he, *nextHe;

        if (ht->table[i] == NULL) continue;

        /* For each hash entry on this slot... */
        he = ht->table[i];
        while(he) {
            unsigned int h;

            nextHe = he->next;
            /* Get the new element index */
            h = dictHashKey(ht, he->key) & n.sizemask;
            he->next = n.table[h];
            n.table[h] = he;
            ht->used--;
            /* Pass to the next element */
            he = nextHe;
        }
    }
    assert(ht->used == 0);
    free(ht->table);

    /* Remap the new hashtable in the old */
    *ht = n;
    return DICT_OK;
}

static unsigned int dict_force_resize_ratio = 5;
static int _dictExpandIfNeeded(dictht *ht)
{

    /* If the hash table is empty expand it to the initial size. */
    if (ht->size == 0) return dictExpand(ht, DICT_HT_INITIAL_SIZE);

    /* If we reached the 1:1 ratio, and we are allowed to resize the hash
     * table (global setting) or we should avoid it but the ratio between
     * elements/buckets is over the "safe" threshold, we resize doubling
     * the number of buckets. */
    if (ht->used >= ht->size &&
         ht->used/ht->size > dict_force_resize_ratio)
    {
        return dictExpand(ht, ht->used * 2);
    }
    return DICT_OK;
}

static int _dictKeyIndex(dictht *ht, const dictVal key)
{
    unsigned int h, idx;
    dictEntry *he;

    /* Expand the hash table if needed */
    if (_dictExpandIfNeeded(ht) == DICT_ERR)
        return -1;
    /* Compute the key hash value */
    h = dictHashKey(ht, key);
    idx = h & ht->sizemask;
        /* Search if this slot does not already contain the given key */
    he = ht->table[idx];
    while(he) {
        if (dictCompareKeys(ht, key, he->key))
            return -1;
        he = he->next;
    }
    return idx;
}


/* Add an element to the target hash table */
int dictAdd(dictht *ht, dictVal key, dictVal val)
{
    int index;
    dictEntry *entry;

    /* Get the index of the new element, or -1 if
     * the element already exists. */
    if ((index = _dictKeyIndex(ht, key)) == -1)
        return DICT_ERR;

    /* Allocates the memory and stores key */
    entry = malloc(sizeof(*entry));
    entry->next = ht->table[index];
    ht->table[index] = entry;

    /* Set the hash entry fields. */
    dictSetHashKey(ht, entry, key);
    dictSetHashVal(ht, entry, val);
    ht->used++;
    return DICT_OK;
}

/* Add an element, discarding the old if the key already exists.
 * Return 1 if the key was added from scratch, 0 if there was already an
 * element with such key and dictReplace() just performed a value update
 * operation. */
int dictReplace(dictht *ht, dictVal key, dictVal val)
{
    dictEntry *entry, auxentry;

    /* Try to add the element. If the key
     * does not exists dictAdd will suceed. */
    if (dictAdd(ht, key, val) == DICT_OK)
        return 1;
    /* It already exists, get the entry */
    entry = dictFind(ht, key);
    /* Free the old value and set the new one */
    /* Set the new value and free the old one. Note that it is important
     * to do that in this order, as the value may just be exactly the same
     * as the previous one. In this context, think to reference counting,
     * you want to increment (set), and then decrement (free), and not the
     * reverse. */
    auxentry = *entry;
    dictSetHashVal(ht, entry, val);
    dictFreeEntryVal(ht, &auxentry);
    return 0;
}

/* Search and remove an element */
int dictDelete(dictht *ht, const dictVal key)
{
    unsigned int h;
    dictEntry *de, *prevde;

    if (ht->size == 0)
        return DICT_ERR;
    h = dictHashKey(ht, key) & ht->sizemask;
    de = ht->table[h];

    prevde = NULL;
    while(de) {
        if (dictCompareKeys(ht,key,de->key)) {
            /* Unlink the element from the list */
            if (prevde)
                prevde->next = de->next;
            else
                ht->table[h] = de->next;

            dictFreeEntryKey(ht,de);
            dictFreeEntryVal(ht,de);
            free(de);
            ht->used--;
            return DICT_OK;
        }
        prevde = de;
        de = de->next;
    }
    return DICT_ERR; /* not found */
}

/* Destroy an entire hash table */
int dictClear(dictht *ht)
{
    unsigned long i;

    /* Free all the elements */
    for (i = 0; i < ht->size && ht->used > 0; i++) {
        dictEntry *he, *nextHe;

        if ((he = ht->table[i]) == NULL) continue;
        while(he) {
            nextHe = he->next;
            dictFreeEntryKey(ht, he);
            dictFreeEntryVal(ht, he);
            free(he);
            ht->used--;
            he = nextHe;
        }
    }
    /* Free the table and the allocated cache structure */
    free(ht->table);
    /* Re-initialize the table */
    _dictReset(ht, NULL);
    return DICT_OK; /* never fails */
}

dictEntry *dictFind(dictht *ht, const dictVal key)
{
    dictEntry *he;
    unsigned int h;

    if (ht->size == 0) return NULL;
    h = dictHashKey(ht, key) & ht->sizemask;
    he = ht->table[h];
    while(he) {
        if (dictCompareKeys(ht, key, he->key))
            return he;
        he = he->next;
    }
    return NULL;
}


unsigned int dictIntHashFunction(dictVal key)
{
    key.u64 += ~(key.u64 << 15);
    key.u64 ^=  (key.u64 >> 10);
    key.u64 +=  (key.u64 << 3);
    key.u64 ^=  (key.u64 >> 6);
    key.u64 += ~(key.u64 << 11);
    key.u64 ^=  (key.u64 >> 16);
    return key.u64;
}

static uint32_t dict_hash_function_seed = 5381;
unsigned int dictGenHashFunction(const void *key, int len)
{
    /* 'm' and 'r' are mixing constants generated offline.
     They're not really 'magic', they just happen to work well.  */
    uint32_t seed = dict_hash_function_seed;
    const uint32_t m = 0x5bd1e995;
    const int r = 24;

    /* Initialize the hash to a 'random' value */
    uint32_t h = seed ^ len;

    /* Mix 4 bytes at a time into the hash */
    const unsigned char *data = (const unsigned char *)key;

    while(len >= 4) {
        uint32_t k = *(uint32_t*)data;

        k *= m;
        k ^= k >> r;
        k *= m;

        h *= m;
        h ^= k;

        data += 4;
        len -= 4;
    }

    /* Handle the last few bytes of the input array  */
    switch(len) {
    case 3: h ^= data[2] << 16;
    case 2: h ^= data[1] << 8;
    case 1: h ^= data[0]; h *= m;
    };

    /* Do a few final mixes of the hash to ensure the last few
     * bytes are well-incorporated. */
    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return (unsigned int)h;
}

unsigned int dictGenCaseHashFunction(const unsigned char *buf, int len)
{
    unsigned int hash = (unsigned int)dict_hash_function_seed;

    while (len--)
        hash = ((hash << 5) + hash) + (tolower(*buf++)); /* hash * 33 + c */
    return hash;
}
