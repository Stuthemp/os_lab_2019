#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <getopt.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>


#include "pthread.h"

struct FactorialArgs {
  uint64_t begin;
  uint64_t end;
  uint64_t mod;
};

uint64_t MultModulo(uint64_t a, uint64_t b, uint64_t mod) {
  uint64_t result = 0;
  a = a % mod;
  while (b > 0) {
    if (b % 2 == 1)
      result = (result + a) % mod;
    a = (a * 2) % mod;
    b /= 2;
  }

  return result % mod;
}

uint64_t Factorial(const struct FactorialArgs *args) {
  uint64_t ans = 1;
  for (int i = args->begin; i < args->end; i++)
      ans = MultModulo(ans, i, args->mod);

  return ans;
}

void *ThreadFactorial(void *args) {
  struct FactorialArgs *fargs = (struct FactorialArgs *)args;
  return (void *)(uint64_t *)Factorial(fargs);
}

int main(int argc, char **argv) {
  int tnum = -1;
  int port = -1;

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"port", required_argument, 0, 0},
                                      {"tnum", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);

    if (c == -1)
      break;

    switch (c) {
    case 0: {
      switch (option_index) {
      case 0:
        port = atoi(optarg);
        if(port <= 0 || port > 65535)
        {
            printf("port is positive number less then 65536\n");
            return 1;
        }
        break;
      case 1:
        tnum = atoi(optarg);
        if(tnum <= 0)
        {
            printf("tnum is positive numder\n");
            return 1;
        }
        break;
      default:
        printf("Index %d is out of options\n", option_index);
      }
    } break;

    case '?':
      printf("Unknown argument\n");
      break;
    default:
      fprintf(stderr, "getopt returned character code 0%o?\n", c);
    }
  }

  if (port == -1 || tnum == -1) {
    fprintf(stderr, "Using: %s --port 20001 --tnum 4\n", argv[0]);
    return 1;
  }

  int server_fd = socket(AF_INET6,  SOCK_DGRAM, 0);
  if (server_fd < 0) {
    fprintf(stderr, "Can not create server socket!");
    return 1;
  }

  struct sockaddr_in6 server;
  server.sin6_family = AF_INET6;
  server.sin6_port = htons((uint16_t)port);
  server.sin6_addr = in6addr_any;

  int opt_val = 1;
  //setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));

  int err = bind(server_fd, (struct sockaddr *)&server, sizeof(server));
  if (err < 0) {
    fprintf(stderr, "Can not bind to socket!\n");
    return 1;
  }

 printf("SERVER starts at %d\n",port);

    while (true) {
        struct sockaddr_in6 client;
        socklen_t client_len = sizeof(client);
      unsigned int buffer_size = sizeof(uint64_t) * 3;
      char from_client[buffer_size];
      unsigned int len = sizeof(server);
      int read = recvfrom(server_fd, from_client, buffer_size, 0, (struct sockaddr *)&client, &len);

      if (!read)
        break;
      if (read < 0) {
        fprintf(stderr, "Client read failed\n");
        break;
      }
      if (read < buffer_size) {
        fprintf(stderr, "Client send wrong data format\n");
        break;
      }

      pthread_t threads[tnum];

      uint64_t begin = 0;
      uint64_t end = 0;
      uint64_t mod = 0;
      memcpy(&begin, from_client, sizeof(uint64_t));
      memcpy(&end, from_client + sizeof(uint64_t), sizeof(uint64_t));
      memcpy(&mod, from_client + 2 * sizeof(uint64_t), sizeof(uint64_t));

      fprintf(stdout, "Receive: %llu %llu %llu\n", begin, end, mod);
      if (tnum > end - begin) tnum = end - begin;

      struct FactorialArgs args[tnum];
      uint64_t ars = (end - begin)/tnum;
      uint64_t left = (end - begin)%tnum;
      for (uint64_t i = 0; i < tnum; i++) 
      {
        args[i].begin = begin + ars*i + (left < i ? left : i);
        args[i].end = begin + ars*(i+1) + (left < i+1 ? left : i+1);
        args[i].mod = mod;

        if (pthread_create(&threads[i], NULL, ThreadFactorial,
                           (void *)&args[i])) {
          printf("Error: pthread_create failed!\n");
          return 1;
        }
      }

      uint64_t total = 1;
      for (uint32_t i = 0; i < tnum; i++) {
        uint64_t result = 0;
        pthread_join(threads[i], (void **)&result);
        total = MultModulo(total, result, mod);
      }

      printf("Total: %lu\n", total);

      char buffer[sizeof(total)];
      memcpy(buffer, &total, sizeof(total));
      err = sendto(server_fd, buffer, buffer_size, 0, (struct sockaddr *)&client, len);
      if (err < 0) {
        fprintf(stderr, "Can't send data to client\n");
        break;
      }
    

    //shutdown(client_fd, SHUT_RDWR);
    //close(client_fd);
  }

  return 0;
}