#include "libCacheSim.h"
#include <time.h>

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
    case 5: printf("Eviction policy : LHD\n"); break;
    case 6: printf("Eviction policy : LeCaR\n"); break;
    default: printf("Unknown eviction policy\n"); break;
  }

  //meta kvcache 202206 prameter
  reader_init_param_t init_params = default_reader_init_params();
  init_params.obj_id_field = 1;
  init_params.obj_size_field = 3;
  init_params.has_header_set = true;
  init_params.has_header = true;
  init_params.op_field = 2;
  init_params.delimiter = ',';

  reader_t* reader = open_trace(workload_file, CSV_TRACE, &init_params);
  request_t* req = new_request();

  /* Create a cache */
  common_cache_params_t cc_params = {.cache_size = cache_size_multiple * 1024 * 1024 * 1024ULL}; // 1GB * multiple
  //common_cache_params_t cc_params = {.cache_size = cache_size_multiple * 100 * 10};
  
  cache_t* cache = NULL;
  switch (eviction) {
    case 0: cache = FIFO_init(cc_params, NULL); break;
    case 1: cache = LRU_init(cc_params, NULL); break;
    case 2: cache = LFU_init(cc_params, NULL); break;
    case 3: cache = ARC_init(cc_params, NULL); break;
    case 4: cache = TwoQ_init(cc_params, NULL); break;
    case 5: cache = LHD_init(cc_params, NULL); break;
    case 6: cache = LeCaR_init(cc_params, NULL); break;
    default: break;
  }

  if (cache == NULL) {
    fprintf(stderr, "Cache init failed.\n");
    return 1;
  }

  uint64_t n_req = 0, n_hit = 0;
  //time
  struct timespec start_time, end_time;
  clock_gettime(CLOCK_MONOTONIC, &start_time);

  /* Main loop through workload trace */
  while (read_one_req(reader, req) == 0) {
    if (cache->get(cache, req)) {
      n_hit++;
    }
    n_req++;

    // ★ 중간 상태 출력: 1000 요청마다 캐시 상태 표시
    // if (n_req % 1000000 == 0) {
    //   printf("[Progress] req = %lu, hit = %lu (hit ratio = %.4lf)\n", n_req, n_hit, (double)n_hit / n_req);
    //   printf("Cache Used Bytes = %ld / %ld\n", cache->get_occupied_byte(cache), cache->cache_size);
    //   printf("Cache Object Count = %ld\n", cache->get_n_obj(cache));
    //   printf("------------------------------------------\n");
    // }
  }
  // === 시간 측정 종료 ===
  clock_gettime(CLOCK_MONOTONIC, &end_time);
  double elapsed_sec = (end_time.tv_sec - start_time.tv_sec) +
                      (end_time.tv_nsec - start_time.tv_nsec) / 1e9;
  double throughput = n_req / elapsed_sec;
  // =====================

  printf("\n=== Final Result ===\n");
  printf("Total Requests: %lu\n", n_req);
  printf("Total Hits: %lu\n", n_hit);
  printf("Final Hit Ratio: %.4lf\n", (double)n_hit / n_req);
  printf("Final Cache Size: %ld / %ld bytes\n", cache->get_occupied_byte(cache), cache->cache_size);
  printf("Final Number of Objects: %ld\n", cache->get_n_obj(cache));
  printf("Total Time: %.3lf sec\n", elapsed_sec);
  printf("Throughput: %.2lf requests/sec\n", throughput);
  printf("=====================\n");

  /* Clean up */
  close_trace(reader);
  free_request(req);
  cache->cache_free(cache);

  return 0;
}