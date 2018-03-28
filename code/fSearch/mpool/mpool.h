#ifndef __febird_c_mpool_h__
#define __febird_c_mpool_h__

//#include "config.h"

#include <syncobject/criticalsection.h>

#ifdef __cplusplus
extern "C" {
#endif

//#define FEBIRD_DLL_EXPORT    
//------------------------------------------------------------------------------------------
struct sallocator
{
	void* (*salloc)(struct sallocator* self, size_t size);
	void* (*srealloc)(struct sallocator* self, void* block, size_t old_size, size_t new_size);
	void  (*sfree)(struct sallocator* self, void* block, size_t size);
};
typedef void* (*salloc_ft)(struct sallocator* self, size_t size);
typedef void* (*srealloc_ft)(struct sallocator* self, void* block, size_t old_size, size_t new_size);
typedef void  (*sfree_ft)(struct sallocator* self, void* block, size_t size);

struct fixed_mpool
{
	struct sallocator* sa;
	struct mpool_chunk* pChunks;
	struct mpool_cell*  head;
	size_t iNextChunk;
	size_t nChunks;
	size_t cell_size;
	size_t chunk_size;
	size_t used_cells;
};

struct sfixed_mpool
{
//------------------------------------------------------------------------------------------
/// export a sallocator interface
	void* (*salloc)(struct sallocator* self, size_t size);
	void* (*srealloc)(struct sallocator* self, void* block, size_t old_size, size_t new_size);
	void  (*sfree)(struct sallocator* self, void* block, size_t size);
//------------------------------------------------------------------------------------------
	struct fixed_mpool fmp;
};

// 是否允许 mpool 分配超出 max_cell_size 的内存块
//#define FEBIRD_MPOOL_ALLOW_BIG_BLOCK
#ifdef FEBIRD_MPOOL_ALLOW_BIG_BLOCK
struct big_block_header
{
	struct big_block_header *next, *prev;
};
#endif

struct mpool
{
//------------------------------------------------------------------------------------------
/// export a sallocator interface
	void* (*salloc)(struct sallocator* self, size_t size);
	void* (*srealloc)(struct sallocator* self, void* block, size_t old_size, size_t new_size);
	void  (*sfree)(struct sallocator* self, void* block, size_t size);
//------------------------------------------------------------------------------------------

	/// sallocator for this mpool self
	struct sallocator* sa;

	struct fixed_mpool* fixed;
	size_t max_cell_size;
	size_t chunk_size;
#ifdef FEBIRD_MPOOL_ALLOW_BIG_BLOCK
	size_t big_blocks; // size > max_cell_size, use malloc, this is rare case
	struct big_block_header big_list;
#endif

	//msdk::CAutoCriticalSection* m_cs;
};

/***********************************************************************/
//FEBIRD_DLL_EXPORT 
void fixed_mpool_init(struct fixed_mpool* mpf);
//FEBIRD_DLL_EXPORT 
void fixed_mpool_destroy(struct fixed_mpool* mpf);

//FEBIRD_DLL_EXPORT 
void* fixed_mpool_alloc(struct fixed_mpool* mpf);
//FEBIRD_DLL_EXPORT 
void  fixed_mpool_free(struct fixed_mpool* mpf, void* ptr);
/***********************************************************************/


/***********************************************************************/
/**
 * sfixed_mpool_{salloc|sfree} should only called by sallocator interface
 * sfixed_mpool_srealloc is an assert only hook.
 */
//FEBIRD_DLL_EXPORT 
void sfixed_mpool_init(struct sfixed_mpool* mp);
//FEBIRD_DLL_EXPORT 
void sfixed_mpool_destroy(struct sfixed_mpool* mp);
/***********************************************************************/


/***********************************************************************/
/**
 * mpool_{salloc|sfree} may called by function name, or by interface
 */
//FEBIRD_DLL_EXPORT 
void mpool_init(struct mpool* mp);
//	chunk_size = 4096;max_cell_size = 256
void mpool_init_default(struct mpool* mp, size_t chunk_size, size_t max_cell_size);

//FEBIRD_DLL_EXPORT 
void mpool_destroy(struct mpool* mp);

//FEBIRD_DLL_EXPORT 
void* mpool_salloc(struct mpool* mp, size_t size);
//FEBIRD_DLL_EXPORT 
void  mpool_sfree(struct mpool* mp, void* ptr, size_t size);

//FEBIRD_DLL_EXPORT 
size_t mpool_used_cells(const struct mpool* mp);
//FEBIRD_DLL_EXPORT 
size_t mpool_used_bytes(const struct mpool* mp);
/***********************************************************************/


/***********************************************************************/
//FEBIRD_DLL_EXPORT 
struct mpool* mpool_get_global(void);

//FEBIRD_DLL_EXPORT 
void* gsalloc(size_t size);
//FEBIRD_DLL_EXPORT 
void  gsfree(void* ptr, size_t size);
void  mpool_destroy_global(void);
/***********************************************************************/

#ifdef __cplusplus
}
#endif

#endif // __febird_c_mpool_h__
