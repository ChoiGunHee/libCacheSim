#include "libCacheSim.h"
#include <time.h>

int main(int argc, char* argv[]) {
  if (argc != 5) {
    fprintf(stderr, "Usage: %s <workload_file> <cache_size_multiple> <first_policy> <second_policy>\n", argv[0]);
    return 1;
  }

  const char* workload_file = argv[1];
  uint64_t cache_size_multiple = atoi(argv[2]);
  const char* first_policy = argv[3];
  const char* second_policy = argv[4];

  if (cache_size_multiple <= 0) {
    fprintf(stderr, "Invalid cache size: %s\n", argv[2]);
    return 1;
  }

  printf("workload : %s\n", workload_file);
  printf("cache size : 1G*%d\n", cache_size_multiple);
  printf("First policy : %s\n", first_policy);
  printf("Second policy : %s\n", second_policy);

  reader_init_param_t init_params = default_reader_init_params();
  init_params.obj_id_field = 1;
  init_params.obj_size_field = 3;
  init_params.op_field = 2;
  init_params.has_header_set = true;
  init_params.has_header = true;
  init_params.delimiter = ',';

    // //Twitter cluster
    // reader_init_param_t init_params = default_reader_init_params();
    // init_params.obj_id_field = 2;
    // init_params.obj_size_field = 4;
    // init_params.has_header_set = true;
    // init_params.has_header = false;
    // init_params.op_field = 6;
    // init_params.delimiter = ',';


  reader_t* reader = open_trace(workload_file, CSV_TRACE, &init_params);
  request_t* req = new_request();

  common_cache_params_t cc_params = {.cache_size = cache_size_multiple * 1024 * 1024 * 1024ULL}; // 1GB * multiple
  //common_cache_params_t cc_params = {.cache_size = cache_size_multiple * 100 * 10};
  
  char param_buf[256];
  snprintf(param_buf, sizeof(param_buf), "first-ratio=0.2,first=%s,second=%s,hit-promotion=true", first_policy, second_policy);

  cache_t* cache = MEFLICS_2Q_FF_init(cc_params, param_buf);
  if (cache == NULL) {
    fprintf(stderr, "Cache init failed.\n");
    return 1;
  }

  uint64_t n_req = 0, n_hit = 0;


  //time
  struct timespec start_time, end_time;
  clock_gettime(CLOCK_MONOTONIC, &start_time);
  
  while (read_one_req(reader, req) == 0) {
    if (cache->get(cache, req)) {
      n_hit++;
    }
    n_req++;

    if (n_req % 10000000 == 0) {
      printf("[Progress] req = %lu, hit = %lu (hit ratio = %.4lf)\n", n_req, n_hit, (double)n_hit / n_req);
      printf("Cache Used Bytes = %ld / %ld\n", cache->get_occupied_byte(cache), cache->cache_size);
      printf("Cache Object Count = %ld\n", cache->get_n_obj(cache));
      printf("------------------------------------------\n");
    }
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
  MEFLICS_2Q_FF_params_t *params = (MEFLICS_2Q_FF_params_t *)cache->eviction_params;
  printf("First Queue Accesses: %d, Hits: %d, Hit Rate: %.4lf\n",
         params->first_access_count, params->first_hit_count,
         (double)params->first_hit_count / params->first_access_count);

  printf("Second Queue Accesses: %d, Hits: %d, Hit Rate: %.4lf\n",
         params->second_access_count, params->second_hit_count,
         (double)params->second_hit_count / params->second_access_count);
  printf("=====================\n");

  close_trace(reader);
  free_request(req);
  cache->cache_free(cache);
  return 0;
}
