#include "../../dataStructure/hashtable/hashtable.h"
#include "../../include/libCacheSim/evictionAlgo.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    cache_t *first;
    cache_t *second;
    bool enable_hit_promotion; // New flag: true if only hit objects are promoted
} MEFLICS_2Q_FF_params_t;

// Default setup
static const char *DEFAULT_CACHE_PARAMS = "first-ratio=0.5,hit-promotion=false";

// Function Declarations
cache_t *MEFLICS_2Q_FF_init(const common_cache_params_t ccache_params, const char *cache_specific_params);
static void MEFLICS_2Q_FF_free(cache_t *cache);
static bool MEFLICS_2Q_FF_get(cache_t *cache, const request_t *req);
static cache_obj_t *MEFLICS_2Q_FF_find(cache_t *cache, const request_t *req, const bool update_cache);
static cache_obj_t *MEFLICS_2Q_FF_insert(cache_t *cache, const request_t *req);
static cache_obj_t *MEFLICS_2Q_FF_to_evict(cache_t *cache, const request_t *req);
static void MEFLICS_2Q_FF_evict(cache_t *cache, const request_t *req);
static bool MEFLICS_2Q_FF_remove(cache_t *cache, const obj_id_t obj_id);
static inline int64_t MEFLICS_2Q_FF_get_occupied_byte(const cache_t *cache);
static inline int64_t MEFLICS_2Q_FF_get_n_obj(const cache_t *cache);
static inline bool MEFLICS_2Q_FF_can_insert(const cache_t *cache, const request_t *req);
static void MEFLICS_2Q_FF_parse_params(cache_t *cache, const char *cache_specific_params);

static void MEFLICS_2Q_FF_print_cache_state(const cache_t *cache);
static double parse_first_ratio(const char *params);

// Implementation

cache_t *MEFLICS_2Q_FF_init(const common_cache_params_t ccache_params, const char *cache_specific_params) {
    cache_t *cache = cache_struct_init("MEFLICS_2Q_FF", ccache_params, cache_specific_params);
    cache->cache_init = MEFLICS_2Q_FF_init;
    cache->cache_free = MEFLICS_2Q_FF_free;
    cache->get = MEFLICS_2Q_FF_get;
    cache->find = MEFLICS_2Q_FF_find;
    cache->insert = MEFLICS_2Q_FF_insert;
    cache->evict = MEFLICS_2Q_FF_evict;
    cache->remove = MEFLICS_2Q_FF_remove;
    cache->to_evict = MEFLICS_2Q_FF_to_evict;
    cache->get_occupied_byte = MEFLICS_2Q_FF_get_occupied_byte;
    cache->get_n_obj = MEFLICS_2Q_FF_get_n_obj;
    cache->can_insert = MEFLICS_2Q_FF_can_insert;

    cache->eviction_params = malloc(sizeof(MEFLICS_2Q_FF_params_t));
    memset(cache->eviction_params, 0, sizeof(MEFLICS_2Q_FF_params_t));
    MEFLICS_2Q_FF_params_t *params = (MEFLICS_2Q_FF_params_t *)cache->eviction_params;

    // 💥 먼저 기본 first, second 생성
    double first_ratio = 0.5;
    // 2. 사용자가 넘긴 값 반영
    if (cache_specific_params) {
        first_ratio = parse_first_ratio(cache_specific_params);
    }
    // 3. 비율에 맞게 First, Second 캐시 생성
    common_cache_params_t local_params = ccache_params;
    int64_t first_size = (int64_t)(ccache_params.cache_size * first_ratio);
    int64_t second_size = ccache_params.cache_size - first_size;

    local_params.cache_size = first_size;
    params->first = FIFO_init(local_params, NULL);

    local_params.cache_size = second_size;
    params->second = FIFO_init(local_params, NULL);

    // 4. 나머지 세부 옵션 파싱
    MEFLICS_2Q_FF_parse_params(cache, DEFAULT_CACHE_PARAMS);
    if (cache_specific_params) {
        MEFLICS_2Q_FF_parse_params(cache, cache_specific_params);
    }

    return cache;
}


static void MEFLICS_2Q_FF_free(cache_t *cache) {
    MEFLICS_2Q_FF_params_t *params = (MEFLICS_2Q_FF_params_t *)cache->eviction_params;
    params->first->cache_free(params->first);
    params->second->cache_free(params->second);
    free(cache->eviction_params);
    cache_struct_free(cache);
}

static bool MEFLICS_2Q_FF_get(cache_t *cache, const request_t *req) {
    MEFLICS_2Q_FF_print_cache_state(cache);
    return cache_get_base(cache, req);
}

static cache_obj_t *MEFLICS_2Q_FF_find(cache_t *cache, const request_t *req, const bool update_cache) {
    MEFLICS_2Q_FF_params_t *params = (MEFLICS_2Q_FF_params_t *)cache->eviction_params;

    cache_obj_t *obj = params->first->find(params->first, req, update_cache);
    if (obj) {
        // 👇 hit 카운트 증가
        obj->hit_count++;

        return obj;
    }

    obj = params->second->find(params->second, req, update_cache);
    if (obj) {
        // 👇 hit 카운트 증가
        obj->hit_count++;
    }

    return obj;
}


static cache_obj_t *MEFLICS_2Q_FF_insert(cache_t *cache, const request_t *req) {
    MEFLICS_2Q_FF_params_t *params = (MEFLICS_2Q_FF_params_t *)cache->eviction_params;

    // First가 꽉 찼으면 evict + (조건부) 승격
    while (params->first->get_occupied_byte(params->first) + req->obj_size > params->first->cache_size) {
        cache_obj_t *obj = params->first->to_evict(params->first, req);
        assert(obj);

        // 👇 여기가 핵심: 임시로 작은 request_t를 하나 만든다
        request_t tmp_req;
        tmp_req.obj_id = obj->obj_id;
        tmp_req.obj_size = obj->obj_size;

        // 승격 조건 체크 후 Second에 삽입
        if (!params->enable_hit_promotion || (params->enable_hit_promotion && obj->hit_count > 0)) {
            params->second->insert(params->second, &tmp_req);
        }

        // First에서 실제로 Evict
        params->first->evict(params->first, req);
    }

    // 새로운 객체 삽입
    return params->first->insert(params->first, req);
}

static cache_obj_t *MEFLICS_2Q_FF_to_evict(cache_t *cache, const request_t *req) {
    assert(false);
    return NULL;
}

static void MEFLICS_2Q_FF_evict(cache_t *cache, const request_t *req) {
    MEFLICS_2Q_FF_params_t *params = (MEFLICS_2Q_FF_params_t *)cache->eviction_params;

    if (params->first->get_occupied_byte(params->first) > params->first->cache_size) {
        cache_obj_t *obj = params->first->to_evict(params->first, req);
        assert(obj);
        copy_cache_obj_to_request(req, obj);

        if (!params->enable_hit_promotion || (params->enable_hit_promotion && obj->hit_count > 0)) {
            params->second->insert(params->second, req);
        }

        params->first->evict(params->first, req);
    } else {
        params->second->evict(params->second, req);
    }
}

static bool MEFLICS_2Q_FF_remove(cache_t *cache, const obj_id_t obj_id) {
    MEFLICS_2Q_FF_params_t *params = (MEFLICS_2Q_FF_params_t *)cache->eviction_params;

    bool removed = false;
    removed |= params->first->remove(params->first, obj_id);
    removed |= params->second->remove(params->second, obj_id);

    return removed;
}

static inline int64_t MEFLICS_2Q_FF_get_occupied_byte(const cache_t *cache) {
    MEFLICS_2Q_FF_params_t *params = (MEFLICS_2Q_FF_params_t *)cache->eviction_params;
    return params->first->get_occupied_byte(params->first) + params->second->get_occupied_byte(params->second);
}

static inline int64_t MEFLICS_2Q_FF_get_n_obj(const cache_t *cache) {
    MEFLICS_2Q_FF_params_t *params = (MEFLICS_2Q_FF_params_t *)cache->eviction_params;
    return params->first->get_n_obj(params->first) + params->second->get_n_obj(params->second);
}

static inline bool MEFLICS_2Q_FF_can_insert(const cache_t *cache, const request_t *req) {
    MEFLICS_2Q_FF_params_t *params = (MEFLICS_2Q_FF_params_t *)cache->eviction_params;
    return req->obj_size <= params->first->cache_size && cache_can_insert_default(cache, req);
    //return cache_can_insert_default(cache, req);
}

static void MEFLICS_2Q_FF_parse_params(cache_t *cache, const char *cache_specific_params) {
    MEFLICS_2Q_FF_params_t *params = (MEFLICS_2Q_FF_params_t *)(cache->eviction_params);
    char *params_str = strdup(cache_specific_params);
    char *old_params_str = params_str;
    char *saveptr;

    char *token = strtok_r(params_str, ",", &saveptr);
    while (token) {
        char *equal = strchr(token, '=');
        if (equal) {
            *equal = '\0';
            const char *key = token;
            const char *value = equal + 1;
            if (strcasecmp(key, "hit-promotion") == 0) {
                params->enable_hit_promotion = (strcasecmp(value, "true") == 0);
            }
        }
        token = strtok_r(NULL, ",", &saveptr);
    }

    free(old_params_str);
}

static void MEFLICS_2Q_FF_print_cache_state(const cache_t *cache) {
    MEFLICS_2Q_FF_params_t *params = (MEFLICS_2Q_FF_params_t *)cache->eviction_params;

    FIFO_params_t *first_params = (FIFO_params_t *)params->first->eviction_params;
    FIFO_params_t *second_params = (FIFO_params_t *)params->second->eviction_params;

    printf("=== MEFLICS_2Q_FF Cache State ===\n");

    printf("First Queue:\n");
    cache_obj_t *curr = first_params->q_head;
    while (curr) {
        printf("(ObjID: %ld, HitCount: %ld) ", curr->obj_id, curr->hit_count);
        curr = curr->queue.next;
    }
    printf("\n");

    printf("Second Queue:\n");
    curr = second_params->q_head;
    while (curr) {
        printf("(ObjID: %ld, HitCount: %ld) ", curr->obj_id, curr->hit_count);
        curr = curr->queue.next;
    }
    printf("\n");

    printf("Total Cache Size Used: %ld / %ld bytes\n", cache->get_occupied_byte(cache), cache->cache_size);
    printf("Total Objects: %ld\n", cache->get_n_obj(cache));
    printf("=================================\n\n");
}

// first-ratio만 먼저 추출
static double parse_first_ratio(const char *params) {
    char *params_copy = strdup(params);
    char *saveptr;
    char *token = strtok_r(params_copy, ",", &saveptr);
    double ratio = 0.5;  // default

    while (token) {
        char *equal = strchr(token, '=');
        if (equal) {
            *equal = '\0';
            const char *key = token;
            const char *value = equal + 1;
            if (strcasecmp(key, "first-ratio") == 0) {
                ratio = atof(value);
                break;
            }
        }
        token = strtok_r(NULL, ",", &saveptr);
    }

    free(params_copy);
    return ratio;
}


#ifdef __cplusplus
}
#endif
