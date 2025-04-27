#include "libCacheSim.h"

int main(int argc, char* argv[]) {
  if (argc != 4) {
    fprintf(stderr, "Usage: %s <workload_file> <cache_size_multiple> <eviction_policy>\n", argv[0]);
    return 1;
  }
  const char* workload_file = argv[1];

  uint64_t cache_size_multiple = atoi(argv[2]);
  if (cache_size_multiple <= 0) {
    fprintf(stderr, "Invalid cache size: %s\n", argv[2]);
    return 1;
  }

  int eviction = atoi(argv[3]);

  printf("workload : %s\n", workload_file);
  printf("cache size : 1G*%d\n", cache_size_multiple);
  switch (eviction) {
    case 0: printf("Eviction policy : FIFO\n"); break;
    case 1: printf("Eviction policy : LRU\n"); break;
    case 2: printf("Eviction policy : LFU\n"); break;
    case 3: printf("Eviction policy : ARC\n"); break;
    case 4: printf("Eviction policy : Two-Q\n"); break;
    case 5: printf("Eviction policy : S3-FIFO\n"); break;
    case 6: printf("Eviction policy : MyCache\n"); break;
    case 7: printf("Eviction policy : MEFLICS-FIFO\n"); break;
    case 8: printf("Eviction policy : MEFLICS-FSC\n"); break;
    case 9: printf("Eviction policy : MEFLICS-2Q-FF\n"); break;
    default: printf("Unknown eviction policy\n"); break;
  }

  reader_init_param_t init_params = default_reader_init_params();
  init_params.obj_id_field = 2;
  init_params.obj_size_field = 3;
  init_params.has_header_set = true;
  init_params.has_header = true;
  init_params.delimiter = ',';

  reader_t* reader = open_trace(workload_file, CSV_TRACE, &init_params);
  request_t* req = new_request();

  /* Create a cache */
  //common_cache_params_t cc_params = {.cache_size = cache_size_multiple * 1024 * 1024 * 1024ULL}; // 1GB * multiple
  common_cache_params_t cc_params = {.cache_size = cache_size_multiple * 100 * 10};
  
  cache_t* cache = NULL;
  switch (eviction) {
    case 0: cache = FIFO_init(cc_params, NULL); break;
    case 1: cache = LRU_init(cc_params, NULL); break;
    case 2: cache = LFU_init(cc_params, NULL); break;
    case 3: cache = ARC_init(cc_params, NULL); break;
    case 4: cache = TwoQ_init(cc_params, NULL); break;
    case 5: cache = S3FIFO_init(cc_params, NULL); break;
    case 6: cache = MyCache_init(cc_params, NULL); break;
    case 7: cache = MEFLIC_FIFO_init(cc_params, NULL); break;
    case 8: cache = MEFLIC_FSC_init(cc_params, NULL); break;
    //case 9: cache = MEFLICS_2Q_FF_init(cc_params, "first-ratio=0.5,hit-promotion=true"); break;
    case 9: cache = MEFLICS_2Q_FF_init(cc_params, "first-ratio=0.5,hit-promotion=false"); break;
    default: break;
  }

  if (cache == NULL) {
    fprintf(stderr, "Cache init failed.\n");
    return 1;
  }

  uint64_t n_req = 0, n_hit = 0;

  /* Main loop through workload trace */
  while (read_one_req(reader, req) == 0) {
    if (cache->get(cache, req)) {
      n_hit++;
    }
    n_req++;

    // ★ 중간 상태 출력: 1000 요청마다 캐시 상태 표시
    if (n_req % 1000 == 0) {
      printf("[Progress] req = %lu, hit = %lu (hit ratio = %.4lf)\n", n_req, n_hit, (double)n_hit / n_req);
      printf("Cache Used Bytes = %ld / %ld\n", cache->get_occupied_byte(cache), cache->cache_size);
      printf("Cache Object Count = %ld\n", cache->get_n_obj(cache));
      printf("------------------------------------------\n");
    }
  }

  printf("\n=== Final Result ===\n");
  printf("Total Requests: %lu\n", n_req);
  printf("Total Hits: %lu\n", n_hit);
  printf("Final Hit Ratio: %.4lf\n", (double)n_hit / n_req);
  printf("Final Cache Size: %ld / %ld bytes\n", cache->get_occupied_byte(cache), cache->cache_size);
  printf("Final Number of Objects: %ld\n", cache->get_n_obj(cache));
  printf("=====================\n");

  /* Clean up */
  close_trace(reader);
  free_request(req);
  cache->cache_free(cache);
}
