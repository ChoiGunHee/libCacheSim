//
// Created by Juncheng Yang on 11/15/19.
//

#include "libCacheSim.h"

int main(int argc, char* argv[]) {
  if (argc != 4) {
    fprintf(stderr, "Usage: %s <workload_file> <cache_size_multiple> <eviction_policy>\n", argv[0]);
    return 1;
  }
  const char* workload_file = argv[1];

  // 캐시 사이즈 정수로 파싱
  uint64_t cache_size_multiple = atoi(argv[2]);
  if (cache_size_multiple <= 0) {
    fprintf(stderr, "Invalid cache size: %s\n", argv[2]);
    return 1;
  }

  // eviction 정책
  int eviction = atoi(argv[3]);
  //print info
  printf("workload : %s\n", workload_file);
  printf("cache size : 1G*%d\n", cache_size_multiple);
  switch (eviction) {
    case 0:
      printf("Eviction policy : FIFO\n");
      break;
    case 1:
      printf("Eviction policy : LRU\n");
      break;
    case 2:
      printf("Eviction policy : LFU\n");
      break;
    case 3:
      printf("Eviction policy : ARC\n");
      break;
    case 4:
      printf("Eviction policy : Two-Q\n");
      break;
    case 5:
      printf("Eviction policy : S3-FIFO\n");
      break;
    case 6:
      printf("Eviction policy : Mycache\n");
    case 7:
      printf("Eviction policy : MEFLICS-FIFO\n");
      break;
    case 8:
      printf("Eviction policy : MEFLICS-FSC\n");
      break;
    default:
      break;
  }

  reader_init_param_t init_params = default_reader_init_params();
  init_params.obj_id_field = 2;
  init_params.obj_size_field = 3;
  init_params.has_header_set = true;
  init_params.has_header = true;
  init_params.delimiter = ',';

  /* open trace, see quickstart_lib.md for opening csv and binary trace */
  reader_t* reader = open_trace(workload_file, CSV_TRACE, &init_params);

  /* create a container for reading from trace */
  request_t* req = new_request();

  /* create a cache */
  // common_cache_params_t cc_params = {.cache_size = cache_size_multiple * 1024 * 1024U};
  common_cache_params_t cc_params = {.cache_size = 500};
  
  cache_t* cache;
  switch (eviction) {
    case 0: //FIFO
      cache = FIFO_init(cc_params, NULL);
      break;
    case 1: //LRU
      cache = LRU_init(cc_params, NULL);
      break;
    case 2: //LFU
      cache = LFU_init(cc_params, NULL);
      break;
    case 3: //ARC
      cache = ARC_init(cc_params, NULL);
      break;
    case 4: //Two-Q
      cache = TwoQ_init(cc_params, NULL);
      break;
    case 5: //S3-FIFO
      cache = S3FIFO_init(cc_params, NULL);
      break;
    case 6:
      cache = MyCache_init(cc_params, NULL);
    case 7 :
      cache = MEFLIC_FIFO_init(cc_params, NULL);
      break;
    case 8 :
      cache = MEFLIC_FSC_init(cc_params, NULL);
      break;
    default:
      break;
  }

  /* counters */
  uint64_t n_req = 0, n_hit = 0;

  /* loop through the trace */
  while (read_one_req(reader, req) == 0) {
    //    printf("obj id : %d, hv : %d\n", req->obj_id, req->obj_size);
    if (cache->get(cache, req)) {
      n_hit++;
    }
    n_req++;
  }

  printf("hit ratio: %.4lf\n", (double)n_hit / n_req);
  printf("hit : %d, req : %d\n", n_hit, n_req);

  printf("Cache Size : %d\n", cache->cache_size);
  printf("Data Size : %d\n", cache->get_occupied_byte(cache));
  printf("Number of objects : %d\n", cache->get_n_obj(cache));

  printf("--------------------------------\n\n");
  /* cleaning */
  close_trace(reader);
  free_request(req);
  cache->cache_free(cache);
}