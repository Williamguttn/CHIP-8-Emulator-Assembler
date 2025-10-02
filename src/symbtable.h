/* ================= symbtable.h ================= */


#ifndef SYMBTABLE_H
#define SYMBTABLE_H


#include <stdint.h>
#include <stddef.h>


#ifdef __cplusplus
extern "C" {
#endif


typedef struct symtab symtab_t;


/** Create a new symbol table. Pass 0 to use a reasonable default. */
symtab_t *symtab_create(size_t initial_capacity);


/** Destroy a table and free all memory. */
void symtab_destroy(symtab_t *tab);


/**
* Insert or update a mapping from key -> value.
* Returns: 1 if a new key was inserted, 0 if an existing key was updated,
* -1 on allocation error.
*/
int symtab_put(symtab_t *tab, const char *key, uint16_t value);


/**
* Lookup a key. Returns 1 and writes value to *out if found, 0 if not found.
*/
int symtab_get(const symtab_t *tab, const char *key, uint16_t *out);


/** Remove a key. Returns 1 if removed, 0 if key not found. */
int symtab_remove(symtab_t *tab, const char *key);


/** Number of entries currently stored. */
size_t symtab_count(const symtab_t *tab);


/** Current bucket capacity (for debugging/tuning). */
size_t symtab_capacity(const symtab_t *tab);


#ifdef __cplusplus
}
#endif


#endif /* SYMBTABLE_H */