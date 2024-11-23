#include <iostream>
#include <list>
#include <functional>
#include <pthread.h>
#include <cstring>
#include <cstdlib>

using namespace std;

struct oneD_thread_args {
  int low;
  int high;
  std::function<void(int)> lambda;
};

void* oneD_thread_function(void* ptr) {
  oneD_thread_args* p = (oneD_thread_args*) ptr;
  for (int i = p->low; i < p->high; i++) {
    p->lambda(i);
  }
  return nullptr;
}

struct twoD_thread_args {
  int low1;
  int high1;
  int low2;
  int high2;
  std::function<void(int, int)> lambda;
};

void* twoD_thread_function(void* ptr) {
  twoD_thread_args* p = (twoD_thread_args*) ptr;
  for (int i = p->low1; i < p->high1; i++) {
    for (int j = p->low2; j < p->high2; j++) {
      p->lambda(i, j);
    }
  }
  return nullptr;
}

void parallel_for(int low, int high, std::function<void(int)> &&lambda, int numThreads) {
  pthread_t tid[numThreads];
  oneD_thread_args args[numThreads];
  int chunk = (high - low) / numThreads;
  for (int i = 0; i < numThreads; i++) {
    args[i].low = low + chunk * i;
    if(i==numThreads-1){args[i].high=high;}
    else{args[i].high = min(low + chunk * (i + 1), high);}
    args[i].lambda = lambda;
    pthread_create(&tid[i], nullptr, oneD_thread_function, &args[i]);
  }
  for (int i = 0; i < numThreads; i++) {
    pthread_join(tid[i], nullptr);
  }
}

void parallel_for(int low1, int high1, int low2, int high2, std::function<void(int, int)> &&lambda, int numThreads) {
  pthread_t tid[numThreads];
  twoD_thread_args args[numThreads];
  int chunk1 = (high1 - low1) / numThreads;
  int chunk2 = (high2 - low2) / numThreads;
  for (int i = 0; i < numThreads; i++) {
    args[i].low1 = low1 + chunk1 * i;
    if(i==numThreads-1){
      args[i].high1=high1;
      args[i].high2=high2;
    }
    else{
      args[i].high1 = min(low1 + chunk1 * (i + 1), high1);
      args[i].high2 = high2;
    }
    args[i].low2 = low2;
    args[i].lambda = lambda;
    pthread_create(&tid[i], nullptr, twoD_thread_function, &args[i]);
  }
  for (int i = 0; i < numThreads; i++) {
    pthread_join(tid[i], nullptr);
  }
}
