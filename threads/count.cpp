/// Assignment 3: Jonathan Harris 30062368
/// counts number of primes from standard input
///
/// compile with:
///   $ gcc countPrimes.c -O2 -o count -lm
///
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <iostream>
#include <vector>
#include <pthread.h>
#include <atomic>
using namespace std;

pthread_mutex_t flag_mutex;
atomic<bool> flag(true);

int nThreads;
struct tParts
{
  int64_t start, end, n;
};

#define THRESH 100 * 100
/// primality test, if n is prime, return 1, else return 0

void *threadWork(void *);

int isPrime(int64_t n)
{

  if (n <= 1)
    return 0; // small numbers are not primes
  if (n <= 3)
    return 1; // 2 and 3 are prime
  if (n % 2 == 0 || n % 3 == 0)
    return 0; // multiples of 2 and 3 are not primes
  int64_t i = 5;
  int64_t max = sqrt(n);

  while (i <= max)
  {
    if (n % i == 0 || n % (i + 2) == 0)
      return 0;
    i += 6;
  }
  return 1;
}

bool threadPrime(int64_t n)
{
  pthread_t threads[nThreads];
  tParts data[nThreads];
  int64_t start, end;
  int64_t max = sqrt(n);

  //create the divisions that are divisible by 6
  int64_t divisions = max / nThreads;

  start = 5;
  end = 5 + divisions;
  for (int i = 0; i < nThreads; i++)
  {

    data[i].start = start;
    data[i].end = end;
    data[i].n = n;

    if (nThreads - 1 == i)
    {
      data[i].end = data[i].end + (max - data[i].end);
    }

    //cout << i << ' ' << data[i].start << ' ' << data[i].end << endl;
    //create the threads
    int t = pthread_create(&threads[i], nullptr, &threadWork, &data[i]);
    if (t != 0)
    {
      cout << "Thread failed" << endl;
      continue;
    }

    start = (data[i].end / 6) * 6 + 5;
    end = start + divisions;
  }

  //join the threads
  for (int i = 0; i < nThreads; i++)
  {
    pthread_join(threads[i], NULL);
  }

  //at this point we've determined if the number was prime...
  return flag;
  //return count;
}

//Function to do work
void *threadWork(void *data)
{
  tParts *p = (tParts *)data;

  if (p->n % 2 == 0 || p->n % 3 == 0)
  {

    pthread_mutex_lock(&flag_mutex);
    flag = false;
    pthread_mutex_unlock(&flag_mutex);
    pthread_exit(0);
  }
  int64_t i = p->start;
  //     int64_t max = sqrt(p->n);

  //loop until the flag, or has checked all the numbers.
  while (i <= p->end && flag)
  {
    if (p->n % i == 0 || p->n % (i + 2) == 0)
    {
      pthread_mutex_lock(&flag_mutex);
      flag = false;
      pthread_mutex_unlock(&flag_mutex);
      pthread_exit(0);
    }
    i += 6;
  }

  //if we are here, we did not find a factor but it does not mean that the number is prime...
  pthread_exit(0);
}

int main(int argc, char **argv)
{

  pthread_mutex_init(&flag_mutex, NULL);
  /// parse command line arguments
  nThreads = 1;

  if (argc != 1 && argc != 2)
  {
    printf("Uasge: countPrimes [nThreads]\n");
    exit(-1);
  }
  if (argc == 2)
    nThreads = atoi(argv[1]);

  /// handle invalid arguments
  if (nThreads < 1 || nThreads > 256)
  {
    printf("Bad arguments. 1 <= nThreads <= 256!\n");
  }

  /// count the primes
  printf("Counting primes using %d thread%s.\n",
         nThreads, nThreads == 1 ? "s" : "");

  int64_t count = 0;
  while (1)
  {
    int64_t num;
    if (1 != scanf("%ld", &num))
      break;

    //check to see if the thread is active
    if (num < THRESH)
    {
      //solve this just with one thread
      count += isPrime(num);
    }
    else
    {
      //call big primes handler
      if (threadPrime(num))
      {
        count += 1;
      }
    }
    //reset the flag to be true
    flag = true;
  }

  /// report results
  printf("Found %ld primes.\n", count);

  return 0;
}
