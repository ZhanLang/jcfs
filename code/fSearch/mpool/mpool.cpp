#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "mpool.h"

/* must be power of 2 and greater than sizeof(void*) */
#define MPOOL_MIN_CELL 8

#define MPOOL_MIN_CHUNK 256

struct mpool_cell
{
    struct mpool_cell* next;
};

struct mpool_chunk
{
    struct mpool_cell* cell; // cell array
    size_t size; // size in bytes;
};

/**********************************************************************************/
/**
 * sallocator use malloc/free
 */
static void *malloc_salloc(struct sallocator* sa, size_t size)
{
    return malloc(size);
}

static void malloc_sfree(struct sallocator* sa, void* block, size_t size)
{
    free(block);
}

static void *malloc_srealloc(struct sallocator* sa, void* block, size_t old_size, size_t new_size)
{
    return realloc(block, new_size);
}
/**********************************************************************************/


void *default_srealloc(struct sallocator* sa, void* ptr, size_t old_size, size_t new_size)
{
    void* q = sa->salloc(sa, new_size);
    assert(old_size > 0);
    assert(new_size > 0);
    if (NULL == q) return NULL;
    memcpy(q, ptr, old_size < new_size ? old_size : new_size);
    sa->sfree(sa, ptr, old_size);
    return q;
}


static void chunk_init(struct mpool_cell* cell, size_t cell_count, size_t cell_size)
{
    size_t i;
    struct mpool_cell* p = cell;
    assert(cell_size % MPOOL_MIN_CELL == 0);
    for (i = 0; i < cell_count-1; ++i)
        p = p->next = (struct mpool_cell*)((char*)p + cell_size);
    p->next = NULL;
}

/**********************************************************************************/
static struct sallocator fal = {
    &malloc_salloc,
    &malloc_srealloc,
    &malloc_sfree
};

/**********************************************************************************/
void* sfixed_mpool_salloc(struct sfixed_mpool* fmp, size_t size);
void* sfixed_mpool_srealloc(struct sfixed_mpool* fmp, size_t old_size, size_t new_size);
void  sfixed_mpool_sfree(struct sfixed_mpool* fmp, void* ptr, size_t size);

/**
 * require initialized fields:
 *     cell_size
 *     size
 *     sa [0 OR initialized]
 */
void fixed_mpool_init(struct fixed_mpool* fmp)
{
	
    struct sallocator* al;
    if (NULL == fmp->sa)
        al = fmp->sa = &fal;
    else {
        al = fmp->sa;
        assert(NULL != al->salloc);
        assert(NULL != al->sfree);
        if (NULL == al->srealloc)
            al->srealloc = &default_srealloc;
    }
    assert(fmp->chunk_size > 0);
    assert(fmp->cell_size > 0);
    assert(fmp->cell_size < fmp->chunk_size);

    fmp->cell_size = (fmp->cell_size + MPOOL_MIN_CELL - 1) / MPOOL_MIN_CELL * MPOOL_MIN_CELL;
    fmp->chunk_size = (fmp->chunk_size + MPOOL_MIN_CHUNK - 1) / MPOOL_MIN_CHUNK * MPOOL_MIN_CHUNK;

    if (fmp->nChunks < MPOOL_MIN_CHUNK/sizeof(struct mpool_chunk))
        fmp->nChunks = MPOOL_MIN_CHUNK/sizeof(struct mpool_chunk);

    fmp->iNextChunk = 0;
    fmp->pChunks = (struct mpool_chunk*)
        al->salloc(al, sizeof(struct mpool_chunk) * fmp->nChunks);
    if (NULL == fmp->pChunks) {
        fprintf(stderr
            , "fatal: febird.fixed_mpool_init failed, chunk[size=%zd, count=%zd]\n"
            , fmp->chunk_size, fmp->nChunks);
        abort();
    }
    fmp->head = NULL;
    fmp->used_cells = 0;
}

void sfixed_mpool_init(struct sfixed_mpool* sfmp)
{
    fixed_mpool_init(&sfmp->fmp);
    sfmp->salloc   = (salloc_ft  )&sfixed_mpool_salloc;
    sfmp->sfree    = (sfree_ft   )&sfixed_mpool_sfree;
    sfmp->srealloc = (srealloc_ft)&sfixed_mpool_srealloc;
}

void fixed_mpool_destroy(struct fixed_mpool* fmp)
{
    struct sallocator* sa = fmp->sa;
    long i;
    for (i = fmp->iNextChunk - 1; i >= 0; --i)
        sa->sfree(sa, fmp->pChunks[i].cell, fmp->pChunks[i].size);

    sa->sfree(sa, fmp->pChunks, sizeof(struct mpool_chunk) * fmp->nChunks);
}

void sfixed_mpool_destroy(struct sfixed_mpool* sfmp)
{
    fixed_mpool_destroy(&sfmp->fmp);
}

struct mpool_cell* fixed_mpool_add_chunk(struct fixed_mpool* fmp)
{
    struct mpool_cell* cell;
    
    assert(fmp->pChunks != NULL);

    if (fmp->iNextChunk == fmp->nChunks) {
        size_t old_size = sizeof(struct mpool_chunk) * fmp->nChunks;
        size_t new_size = 2 * old_size;

        // allocate new chunk array
        struct mpool_chunk* c = (struct mpool_chunk*)
            fmp->sa->srealloc(fmp->sa, fmp->pChunks, old_size, new_size);

        if (NULL == c) return NULL;
        fmp->pChunks = c;
        fmp->nChunks *= 2;     // chunk array expanded 2
        fmp->chunk_size *= 2;  // chunk_size  expanded 2 also
    }

    // allocate a new cell array
    cell = (struct mpool_cell*)fmp->sa->salloc(fmp->sa, fmp->chunk_size);
    if (NULL == cell) return NULL;
    fmp->pChunks[fmp->iNextChunk].cell = cell;
    fmp->pChunks[fmp->iNextChunk].size = fmp->chunk_size;
    fmp->iNextChunk++;
    chunk_init(cell, fmp->chunk_size / fmp->cell_size, fmp->cell_size);

    /* alloc cell */
    fmp->used_cells++;
    fmp->head = cell->next;

    return cell;
}

#define FIXED_MPOOL_IMPL_ALLOC(fmp)     \
{                                        \
    struct mpool_cell* cell = fmp->head;\
    if (cell) {                            \
        fmp->used_cells++;                \
        fmp->head = cell->next;            \
        return cell;                    \
    }                                    \
    return fixed_mpool_add_chunk(fmp);    \
}
/***************************************************************/

#define FIXED_MPOOL_IMPL_FREE(fmp, ptr)    \
{                                        \
    struct mpool_cell* cell = (struct mpool_cell*)ptr; \
    cell->next = fmp->head;    \
    fmp->used_cells--;        \
    fmp->head = cell;        \
}
/***************************************************************/

void* fixed_mpool_alloc(struct fixed_mpool* fmp)
FIXED_MPOOL_IMPL_ALLOC(fmp)

void* sfixed_mpool_salloc(struct sfixed_mpool* sfmp, size_t size)
{
    if (size != sfmp->fmp.cell_size) {
        fprintf(stderr, "fatal: sfixed_mpool_salloc:[cell_size=%zd, request_size=%zd]\n", sfmp->fmp.cell_size, size);
        abort();
    }
    FIXED_MPOOL_IMPL_ALLOC((&sfmp->fmp))
}

void fixed_mpool_free(struct fixed_mpool* fmp, void* ptr)
FIXED_MPOOL_IMPL_FREE(fmp, ptr)

void sfixed_mpool_sfree(struct sfixed_mpool* sfmp, void* ptr, size_t size)
{
    if (size != sfmp->fmp.cell_size) {
        fprintf(stderr, "fatal: sfixed_mpool_salloc:[cell_size=%zd, request_size=%zd]\n", sfmp->fmp.cell_size, size);
        abort();
    }
    FIXED_MPOOL_IMPL_FREE((&sfmp->fmp), ptr)
}

static void *sfixed_mpool_srealloc(struct sfixed_mpool* fmp, size_t old_size, size_t new_size)
{
    fprintf(stderr, "fatal: sfixed_mpool_srealloc: this function shuold not be called\n");
    abort();
    return NULL; // avoid warning
}

/**********************************************************************************/
/**
 * require initialized fields:
 *   max_cell_size
 *     size
 *     sa [0 OR initialized]
 */
void mpool_init(struct mpool* mp)
{

    int i, nFixed;
    struct sallocator* al;
    
//	mp->m_cs = new msdk::CAutoCriticalSection();
    assert(mp->max_cell_size < mp->chunk_size);

    if (NULL == mp->salloc)
        al = mp->sa = &fal;
    else {
        al = mp->sa;
        assert(NULL != al->salloc);
        assert(NULL != al->sfree);
        if (NULL == al->srealloc)
            al->srealloc = &default_srealloc;
    }
    mp->salloc = (salloc_ft)&mpool_salloc;
    mp->sfree = (sfree_ft)&mpool_sfree;
    mp->srealloc = (srealloc_ft)&default_srealloc;

    mp->max_cell_size = (mp->max_cell_size + MPOOL_MIN_CELL - 1) / MPOOL_MIN_CELL * MPOOL_MIN_CELL;
    mp->chunk_size = (mp->chunk_size + MPOOL_MIN_CHUNK - 1) / MPOOL_MIN_CHUNK * MPOOL_MIN_CHUNK;
    nFixed = mp->max_cell_size / MPOOL_MIN_CELL;

    mp->fixed = (struct fixed_mpool*)al->salloc(al, sizeof(struct fixed_mpool) * nFixed);
    if (NULL == mp->fixed) {
        fprintf(stderr, "fatal: febird.mpool_init[max_cell_size=%zd, size=%zd]\n"
                , mp->max_cell_size, mp->chunk_size);
        abort();
    }

    for (i = 0; i < nFixed; ++i)
    {
        mp->fixed[i].cell_size = (i + 1) * MPOOL_MIN_CELL;
        mp->fixed[i].chunk_size = mp->chunk_size;
        mp->fixed[i].nChunks = 16;
        mp->fixed[i].sa = mp->sa;
        fixed_mpool_init(&mp->fixed[i]);
    }
#ifdef FEBIRD_MPOOL_ALLOW_BIG_BLOCK
    mp->big_blocks = 0;
    mp->big_list.prev = mp->big_list.next = &mp->big_list;
#endif
}

void mpool_destroy(struct mpool* mp)
{
    size_t i, nFixed;
//	msdk::CStackAutoCSLock((*(mp->m_cs)));
#ifdef FEBIRD_MPOOL_ALLOW_BIG_BLOCK
    struct big_block_header *p;
    for (i = 0, p = mp->big_list.next; p != &mp->big_list; ++i)
    {
        struct big_block_header *q;
        if (i > mp->big_blocks) {
            fprintf(stderr, "fatal: febird.mpool_destroy\n");
            abort();
        }
        q = p->next;
        free(p);
        p = q;
    }
    assert(i == mp->big_blocks);
#endif
    nFixed = mp->max_cell_size / MPOOL_MIN_CELL;
    for (i = 0; i < nFixed; ++i)
    {
        fixed_mpool_destroy(&mp->fixed[i]);
    }
    mp->sa->sfree(mp->sa, mp->fixed, sizeof(struct fixed_mpool) * nFixed);

//	delete mp->m_cs;
}

void *mpool_salloc(struct mpool* mp, size_t size)
{

    size_t idx;
    struct fixed_mpool* fmp;
//	msdk::CStackAutoCSLock((*(mp->m_cs)));
#ifdef FEBIRD_MPOOL_ALLOW_BIG_BLOCK
    if (size > mp->max_cell_size) {
        // this is rare case
        struct big_block_header *p, *h;
        p = (struct big_block_header*)malloc(sizeof(struct big_block_header) + size);
        if (p) {
            h = &mp->big_list;
            p->prev = h;
            p->next = h->next;
            h->next->prev = p;
            h->next = p;
            mp->big_blocks++;
            return (p + 1);
        } else
            return NULL;
    }
#else
    if (size > mp->max_cell_size) {
        fprintf(stderr, "fatal: mpool_salloc: [size=%zd, max_cell_size=%zd]\n", size, mp->max_cell_size);
        abort();
    }
#endif
    assert(size > 0);

    idx = (size - 1) / MPOOL_MIN_CELL;
    fmp = &mp->fixed[idx];

    FIXED_MPOOL_IMPL_ALLOC(fmp)
}

void mpool_sfree(struct mpool* mp, void* ptr, size_t size)
{
    size_t idx;
    struct fixed_mpool* fmp;

//	msdk::CStackAutoCSLock((*(mp->m_cs)));
#ifdef FEBIRD_MPOOL_ALLOW_BIG_BLOCK
    if (size > mp->max_cell_size) {
        // this is rare case
        struct big_block_header* bbh = (struct big_block_header*)ptr - 1;
        bbh->prev->next = bbh->next;
        bbh->next->prev = bbh->prev;
        free(bbh);
        mp->big_blocks--;
        return;
    }
#else
    assert(size <= mp->max_cell_size);
#endif
    assert(size > 0);

    idx = (size - 1) / MPOOL_MIN_CELL;
    fmp = &mp->fixed[idx];

    FIXED_MPOOL_IMPL_FREE(fmp, ptr)
}

size_t mpool_used_cells(const struct mpool* mp)
{
    size_t i, n = mp->max_cell_size / MPOOL_MIN_CELL;
    size_t used = 0;
    for (i = 0; i < n; ++i)
        used += mp->fixed[i].used_cells;
    return used;
}

size_t mpool_used_bytes(const struct mpool* mp)
{
    size_t i, n = mp->max_cell_size / MPOOL_MIN_CELL;
    size_t used = 0;
    for (i = 0; i < n; ++i)
        used += mp->fixed[i].cell_size * mp->fixed[i].used_cells;
    return used;
}

static struct mpool global_mpool = {0};
struct mpool* mpool_get_global(void)
{
    if (NULL == global_mpool.sa) {
        global_mpool.chunk_size = 4096;
        global_mpool.max_cell_size = 256;
        global_mpool.sa = &fal;
        mpool_init(&global_mpool);
        //atexit(&destroy_global_mpool);
    }
    return &global_mpool;
}

/*
	chunk_size = 4096
	max_cell_size = 256
*/
void mpool_init_default(struct mpool* mp, size_t chunk_size, size_t max_cell_size)
{
	if (NULL == mp->sa) {
		mp->chunk_size = chunk_size;
		mp->max_cell_size = max_cell_size;
		mp->sa = &fal;
		mpool_init(mp);
	}
}

void  mpool_destroy_global(void)
{
	if (global_mpool.sa)
		mpool_destroy(&global_mpool);
}

void* gsalloc(size_t size)
{
    return mpool_salloc(&global_mpool, size);
}
void  gsfree(void* ptr, size_t size)
{
    mpool_sfree(&global_mpool, ptr, size);
}