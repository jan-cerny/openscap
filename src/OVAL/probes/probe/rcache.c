#include <stddef.h>
#include <sexp.h>

#include "common/public/alloc.h"
#include "common/assume.h"
#include "../SEAP/generic/rbt/rbt.h"

#include "rcache.h"

probe_rcache_t *probe_rcache_new(void)
{
	probe_rcache_t *cache;

	cache = oscap_talloc(probe_rcache_t);
	cache->tree = rbt_str_new();

	return (cache);
}

static void probe_rcache_free_node(struct rbt_str_node *n)
{
        oscap_free(n->key);
        SEXP_free(n->data);
}

void probe_rcache_free(probe_rcache_t *cache)
{
        rbt_str_free_cb(cache->tree, &probe_rcache_free_node);
	oscap_free(cache);
	return;
}

int probe_rcache_sexp_add(probe_rcache_t *cache, const SEXP_t *id, SEXP_t *item)
{
        SEXP_t *r;
        char   *k;

	assume_d(cache != NULL, -1);
	assume_d(id    != NULL, -1);
	assume_d(item  != NULL, -1);

        k = SEXP_string_cstr(id);
        r = SEXP_ref(item);

        if (rbt_str_add(cache->tree, k, (void *)r) != 0) {
                SEXP_free(r);
                oscap_free(k);
                return (-1);
        }

	return (0);
}

int probe_rcache_cstr_add(probe_rcache_t *cache, const char *id, SEXP_t * item)
{
	return (-1);
}

int probe_rcache_sexp_del(probe_rcache_t *cache, const SEXP_t * id)
{
	return (-1);
}

int probe_rcache_cstr_del(probe_rcache_t *cache, const char *id)
{
	return (-1);
}

SEXP_t *probe_rcache_sexp_get(probe_rcache_t *cache, const SEXP_t * id)
{
        char    b[128], *k = b;
        SEXP_t *r = NULL;

        if (SEXP_string_cstr_r(id, k, sizeof b) == ((size_t)-1))
                k = SEXP_string_cstr(id);

        if (k == NULL)
                return(NULL);

        rbt_str_get(cache->tree, k, (void *)&r);

        if (k != b)
                oscap_free(k);

        return (r != NULL ? SEXP_ref(r) : NULL);
}

SEXP_t *probe_rcache_cstr_get(probe_rcache_t *cache, const char *k)
{
        SEXP_t *r = NULL;

        rbt_str_get(cache->tree, k, (void *)&r);

        return (r != NULL ? SEXP_ref(r) : NULL);
}
