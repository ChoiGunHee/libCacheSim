//
//  first in first out with seconed chacne
////
//  --> MEFLICS_fsc.c
//  by Gunhee Choi, 2025.04.09
//

#include "../../dataStructure/hashtable/hashtable.h"
#include "../../include/libCacheSim/evictionAlgo.h"

#ifdef __cplusplus
extern "C" {
#endif

// ***********************************************************************
// ****                                                               ****
// ****                   function declarations                       ****
// ****                                                               ****
// ***********************************************************************

static void MEFLIC_FSC_parse_params(cache_t *cache,
                                     const char *cache_specific_params);
static void MEFLIC_FSC_free(cache_t *cache);
static bool MEFLIC_FSC_get(cache_t *cache, const request_t *req);
static cache_obj_t *MEFLIC_FSC_find(cache_t *cache, const request_t *req,
                                     const bool update_cache);
static cache_obj_t *MEFLIC_FSC_insert(cache_t *cache, const request_t *req);
static cache_obj_t *MEFLIC_FSC_to_evict(cache_t *cache, const request_t *req);
static void MEFLIC_FSC_evict(cache_t *cache, const request_t *req);
static bool MEFLIC_FSC_remove(cache_t *cache, const obj_id_t obj_id);

// ***********************************************************************
// ****                                                               ****
// ****                   end user facing functions                   ****
// ****                                                               ****
// ****                       init, free, get                         ****
// ***********************************************************************

bool cache_can_insert(cache_t *cache, uint64_t obj_size) {
    return (cache->get_occupied_byte(cache) + obj_size <= cache->cache_size);
}

/**
 * @brief initialize a ARC cache
 *
 * @param ccache_params some common cache parameters
 * @param cache_specific_params ARC specific parameters, should be NULL
 */
cache_t *MEFLIC_FSC_init(const common_cache_params_t ccache_params,
                          const char *cache_specific_params) {
  cache_t *cache = cache_struct_init("MEFLICS_FSC", ccache_params, cache_specific_params);
  cache->cache_init = MEFLIC_FSC_init;
  cache->cache_free = MEFLIC_FSC_free;
  cache->get = MEFLIC_FSC_get;
  cache->find = MEFLIC_FSC_find;
  cache->insert = MEFLIC_FSC_insert;
  cache->evict = MEFLIC_FSC_evict;
  cache->remove = MEFLIC_FSC_remove;
  cache->to_evict = MEFLIC_FSC_to_evict;
  cache->get_occupied_byte = cache_get_occupied_byte_default;
  cache->get_n_obj = cache_get_n_obj_default;
  cache->can_insert = cache_can_insert_default;
  cache->obj_md_size = 0;

  cache->eviction_params = malloc(sizeof(FIFO_params_t));
  FIFO_params_t *params = (FIFO_params_t *)cache->eviction_params;
  params->q_head = NULL;
  params->q_tail = NULL;

  //Test, Gunhee
  printf("Inside msg : MEFLICS FSC cache initialized\n");
  return cache;
}

/**
 * free resources used by this cache
 *
 * @param cache
 */
static void MEFLIC_FSC_free(cache_t *cache) {
  free(cache->eviction_params);
  cache_struct_free(cache);
}

/**
 * @brief this function is the user facing API
 * it performs the following logic
 *
 * ```
 * if obj in cache:
 *    update_metadata
 *    return true
 * else:
 *    if cache does not have enough space:
 *        evict until it has space to insert
 *    insert the object
 *    return false
 * ```
 *
 * @param cache
 * @param req
 * @return true if cache hit, false if cache miss
 */
static bool MEFLIC_FSC_get(cache_t *cache, const request_t *req) {
    cache_obj_t *obj = cache_find_base(cache, req, true);
    if (obj != NULL) {
        obj->second_chance = true; // hit된 객체에 두 번째 기회를 부여
        return true;
    }

    // 객체가 없으면 삽입
    MEFLIC_FSC_insert(cache, req);

    //Test, Gunhee
    printf("Inside msg : MEFLICS FSC cache get\n");
    return false;
}
  
// ***********************************************************************
// ****                                                               ****
// ****       developer facing APIs (used by cache developer)         ****
// ****                                                               ****
// ***********************************************************************

/**
 * @brief find an object in the cache
 *
 * @param cache
 * @param req
 * @param update_cache whether to update the cache,
 *  if true, the object is promoted
 *  and if the object is expired, it is removed from the cache
 * @return the object or NULL if not found
 */
static cache_obj_t *MEFLIC_FSC_find(cache_t *cache, const request_t *req,
                                     const bool update_cache) {
  //Test, Gunhee
  printf("Inside msg : MEFLICS FSC cache Find\n");
  
  return cache_find_base(cache, req, update_cache);
}

/**
 * @brief insert an object into the cache,
 * update the hash table and cache metadata
 * this function assumes the cache has enough space
 * and eviction is not part of this function
 *
 * @param cache
 * @param req
 * @return the inserted object
 */
static cache_obj_t *MEFLIC_FSC_insert(cache_t *cache, const request_t *req) {
    FIFO_params_t *params = (FIFO_params_t *)cache->eviction_params;
    // 캐시가 가득 찬 경우 eviction 수행
    while (!cache_can_insert(cache, req->obj_size)) {
        MEFLIC_FSC_evict(cache, req);
    }

    cache_obj_t *obj = cache_insert_base(cache, req);
    prepend_obj_to_head(&params->q_head, &params->q_tail, obj);

    obj->second_chance = false; // 새로 삽입된 객체는 두 번째 기회를 갖지 않음

    // Test, Gunhee
    printf("Object %d inserted into cache\n", req->obj_id);

    return obj;
}

/**
 * @brief find the object to be evicted
 * this function does not actually evict the object or update metadata
 * not all eviction algorithms support this function
 * because the eviction logic cannot be decoupled from finding eviction
 * candidate, so use assert(false) if you cannot support this function
 *
 * @param cache the cache
 * @return the object to be evicted
 */
static cache_obj_t *MEFLIC_FSC_to_evict(cache_t *cache, const request_t *req) {
  FIFO_params_t *params = (FIFO_params_t *)cache->eviction_params;

    //Test, Gunhee
    printf("Inside msg : MEFLICS FSC cache evict\n");
  return params->q_tail;
}

/**
 * @brief evict an object from the cache
 * it needs to call cache_evict_base before returning
 * which updates some metadata such as n_obj, occupied size, and hash table
 *
 * @param cache
 * @param req not used
 * @param evicted_obj if not NULL, return the evicted object to caller
 */
static void MEFLIC_FSC_evict(cache_t *cache, const request_t *req) {
    FIFO_params_t *params = (FIFO_params_t *)cache->eviction_params;
    cache_obj_t *obj_to_evict = params->q_tail;
    DEBUG_ASSERT(params->q_tail != NULL);

    // 순회하면서 두 번째 기회를 모두 소진한 객체를 찾음
    while (obj_to_evict->second_chance) {
        obj_to_evict->second_chance = false; // 두 번째 기회를 소진
        remove_obj_from_list(&params->q_head, &params->q_tail, obj_to_evict);
        prepend_obj_to_head(&params->q_head, &params->q_tail, obj_to_evict);

        // 다음 객체로 이동
        obj_to_evict = params->q_tail;
        DEBUG_ASSERT(obj_to_evict != NULL); // 캐시가 비어 있지 않아야 함
    }

    // 두 번째 기회가 없는 객체를 실제로 제거
    params->q_tail = params->q_tail->queue.prev;
    if (likely(params->q_tail != NULL)) {
        params->q_tail->queue.next = NULL;
    } else {
        params->q_head = NULL;
    }

    // 퇴출될 객체 정보 출력
    printf("Evicting object: ID=%d, Size=%lu\n", obj_to_evict->obj_id, obj_to_evict->obj_size);

    cache_evict_base(cache, obj_to_evict, true);
}

/**
 * @brief remove an object from the cache
 * this is different from cache_evict because it is used to for user trigger
 * remove, and eviction is used by the cache to make space for new objects
 *
 * it needs to call cache_remove_obj_base before returning
 * which updates some metadata such as n_obj, occupied size, and hash table
 *
 * @param cache
 * @param obj_id
 * @return true if the object is removed, false if the object is not in the
 * cache
 */
static bool MEFLIC_FSC_remove(cache_t *cache, const obj_id_t obj_id) {
  cache_obj_t *obj = hashtable_find_obj_id(cache->hashtable, obj_id);
  if (obj == NULL) {
    return false;
  }

  FIFO_params_t *params = (FIFO_params_t *)cache->eviction_params;

  remove_obj_from_list(&params->q_head, &params->q_tail, obj);
  cache_remove_obj_base(cache, obj, true);

  return true;
}

#ifdef __cplusplus
}
#endif
