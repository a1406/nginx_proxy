#include <stdint.h>

#ifndef __HASH_H
#define __HASH_H

#define DICT_OK 0
#define DICT_ERR 1

typedef union dictVal {
	void *val;
	uint64_t u64;
	int64_t s64;
	double d;
} dictVal;

typedef struct dictEntry {
    dictVal key;
	dictVal val;
    struct dictEntry *next;
} dictEntry;

typedef struct dictType {
    unsigned int (*hashFunction)(const dictVal key);
    dictVal (*keyDup)(void *privdata, const dictVal key);
    dictVal (*valDup)(void *privdata, const dictVal obj);
    int (*keyCompare)(void *privdata, const dictVal key1, const dictVal key2);
    void (*keyDestructor)(void *privdata, dictVal key);
    void (*valDestructor)(void *privdata, dictVal obj);
} dictType;

typedef struct dictht {
    dictEntry **table;
	dictType *type;
    unsigned long size;
    unsigned long sizemask;
    unsigned long used;
	void *privdata;
} dictht;

#define DICT_HT_INITIAL_SIZE     4

/* ------------------------------- Macros ------------------------------------*/
#define dictFreeEntryVal(ht, entry) \
    if ((ht)->type->valDestructor) \
        (ht)->type->valDestructor((ht)->privdata, (entry)->val)

#define dictSetHashVal(ht, entry, _val_) do { \
    if ((ht)->type->valDup) \
        entry->val = (ht)->type->valDup((ht)->privdata, _val_); \
    else \
        entry->val = (_val_); \
} while(0)

#define dictFreeEntryKey(ht, entry) \
    if ((ht)->type->keyDestructor) \
        (ht)->type->keyDestructor((ht)->privdata, (entry)->key)

#define dictSetHashKey(ht, entry, _key_) do { \
    if ((ht)->type->keyDup) \
        entry->key = (ht)->type->keyDup((ht)->privdata, _key_); \
    else \
        entry->key = (_key_); \
} while(0)

#define dictCompareKeys(ht, key1, key2) \
    (((ht)->type->keyCompare) ? \
        (ht)->type->keyCompare((ht)->privdata, key1, key2) : \
        (key1.u64) == (key2.u64))

#define dictHashKey(ht, key) (ht)->type->hashFunction(key)

#define dictGetEntryKey(he) ((he)->key)
#define dictGetEntryVal(he) ((he)->v.val)
#define dictSlots(ht) ((ht)->size)
#define dictSize(ht) ((ht)->used)

int dictClear(dictht *ht);
dictht *dictCreate(dictType *type);
int dictExpand(dictht *ht, unsigned long size);
int dictAdd(dictht *ht, dictVal key, dictVal val);
dictEntry *dictFind(dictht *ht, const dictVal key);
int dictReplace(dictht *ht, dictVal key, dictVal val);
int dictDelete(dictht *ht, const dictVal key);

unsigned int dictIntHashFunction(dictVal key);
unsigned int dictGenHashFunction(const void *key, int len);
unsigned int dictGenCaseHashFunction(const unsigned char *buf, int len);
#endif
