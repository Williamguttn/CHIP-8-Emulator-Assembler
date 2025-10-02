#include "symbtable.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

/* configuration */
static const double ST_MAX_LOAD = 0.75;
static const double ST_MIN_LOAD = 0.20;
static const size_t ST_MIN_CAPACITY = 8;

/* Entry/node */
typedef struct st_entry {
    char *key; /* owned copy */
    uint16_t value;
    struct st_entry *next;
} st_entry_t;

struct symtab {
    st_entry_t **buckets;
    size_t capacity;
    size_t count;
};

/* FNV-1a 64-bit hash */
static uint64_t fnv1a_hash(const char *s) {
    const uint64_t FNV_OFFSET = 14695981039346656037ULL;
    const uint64_t FNV_PRIME  = 1099511628211ULL;
    uint64_t h = FNV_OFFSET;
    while (*s) {
        h ^= (unsigned char)*s++;
        h *= FNV_PRIME;
    }
    return h;
}

/* strdup fallback that's portable */
static char *st_strdup(const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s) + 1;
    char *p = (char *)malloc(n);
    if (!p) return NULL;
    memcpy(p, s, n);
    return p;
}

static st_entry_t *entry_create(const char *key, uint16_t value) {
    st_entry_t *e = (st_entry_t *)malloc(sizeof(st_entry_t));
    if (!e) return NULL;
    e->key = st_strdup(key);
    if (!e->key) { free(e); return NULL; }
    e->value = value;
    e->next = NULL;
    return e;
}

static void entry_free(st_entry_t *e) {
    if (!e) return;
    free(e->key);
    free(e);
}

/* rehash to new_capacity; returns 0 on success, -1 on OOM */
static int st_rehash(symtab_t *tab, size_t new_capacity) {
    if (new_capacity < ST_MIN_CAPACITY) new_capacity = ST_MIN_CAPACITY;
    st_entry_t **new_buckets = (st_entry_t **)calloc(new_capacity, sizeof(st_entry_t *));
    if (!new_buckets) return -1;

    for (size_t i = 0; i < tab->capacity; ++i) {
        st_entry_t *e = tab->buckets[i];
        while (e) {
            st_entry_t *next = e->next;
            uint64_t h = fnv1a_hash(e->key);
            size_t idx = (size_t)(h % new_capacity);
            e->next = new_buckets[idx];
            new_buckets[idx] = e;
            e = next;
        }
    }

    free(tab->buckets);
    tab->buckets = new_buckets;
    tab->capacity = new_capacity;
    return 0;
}

symtab_t *symtab_create(size_t initial_capacity) {
    if (initial_capacity < ST_MIN_CAPACITY) initial_capacity = ST_MIN_CAPACITY;
    symtab_t *tab = (symtab_t *)malloc(sizeof(symtab_t));
    if (!tab) return NULL;
    tab->buckets = (st_entry_t **)calloc(initial_capacity, sizeof(st_entry_t *));
    if (!tab->buckets) { free(tab); return NULL; }
    tab->capacity = initial_capacity;
    tab->count = 0;
    return tab;
}

void symtab_destroy(symtab_t *tab) {
    if (!tab) return;
    for (size_t i = 0; i < tab->capacity; ++i) {
        st_entry_t *e = tab->buckets[i];
        while (e) {
            st_entry_t *next = e->next;
            entry_free(e);
            e = next;
        }
    }
    free(tab->buckets);
    free(tab);
}

int symtab_put(symtab_t *tab, const char *key, uint16_t value) {
    if (!tab || !key) return -1;
    double load = (double)(tab->count + 1) / (double)tab->capacity;
    if (load > ST_MAX_LOAD) {
        if (st_rehash(tab, tab->capacity * 2) != 0) return -1;
    }

    uint64_t h = fnv1a_hash(key);
    size_t idx = (size_t)(h % tab->capacity);
    st_entry_t *e = tab->buckets[idx];
    while (e) {
        if (strcmp(e->key, key) == 0) {
            e->value = value;
            return 0; /* updated */
        }
        e = e->next;
    }

    st_entry_t *ne = entry_create(key, value);
    if (!ne) return -1;
    ne->next = tab->buckets[idx];
    tab->buckets[idx] = ne;
    tab->count += 1;
    return 1; /* inserted new */
}

int symtab_get(const symtab_t *tab, const char *key, uint16_t *out) {
    if (!tab || !key) return 0;
    uint64_t h = fnv1a_hash(key);
    size_t idx = (size_t)(h % tab->capacity);
    st_entry_t *e = tab->buckets[idx];
    while (e) {
        if (strcmp(e->key, key) == 0) {
            if (out) *out = e->value;
            return 1;
        }
        e = e->next;
    }
    return 0;
}

int symtab_remove(symtab_t *tab, const char *key) {
    if (!tab || !key) return 0;
    uint64_t h = fnv1a_hash(key);
    size_t idx = (size_t)(h % tab->capacity);
    st_entry_t *prev = NULL;
    st_entry_t *e = tab->buckets[idx];
    while (e) {
        if (strcmp(e->key, key) == 0) {
            if (prev) prev->next = e->next;
            else tab->buckets[idx] = e->next;
            entry_free(e);
            tab->count -= 1;
            double load = (double)tab->count / (double)tab->capacity;
            if (tab->capacity > ST_MIN_CAPACITY && load < ST_MIN_LOAD) {
                size_t newcap = tab->capacity / 2;
                if (newcap < ST_MIN_CAPACITY) newcap = ST_MIN_CAPACITY;
                (void)st_rehash(tab, newcap); /* ignore OOM on shrink */
            }
            return 1;
        }
        prev = e;
        e = e->next;
    }
    return 0;
}

size_t symtab_count(const symtab_t *tab) {
    return tab ? tab->count : 0;
}

size_t symtab_capacity(const symtab_t *tab) {
    return tab ? tab->capacity : 0;
}