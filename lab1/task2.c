/*
 * FIT3143 - Lab Task 2: Parallel Prime Number Search using POSIX Threads
 *
 * Parallel version of the serial program in Task 1. Searches for all prime
 * numbers strictly less than an integer n and prints the sorted (ascending)
 * list to:
 *   a) standard output, when n < 100
 *   b) a text file ("primes_pthread.txt"), when n >= 100
 *
 * Primality test (unchanged from Task 1, following the hint):
 *   If k is composite then k = m * n for some integers m, n > 1. Both factors
 *   cannot be greater than sqrt(k), so at least one factor is <= sqrt(k) and
 *   testing divisors from 2 up to sqrt(k) is sufficient.
 *
 * Parallel partitioning scheme: CYCLIC (interleaved) DECOMPOSITION
 *   Thread t tests the candidates  k = 2+t, 2+t+T, 2+t+2T, ...  where T is the
 *   number of threads.
 *
 *   Why not a contiguous block decomposition? The cost of is_prime(k) grows
 *   with sqrt(k), so a thread given the top block [n/2, n) does far more work
 *   than the thread given [2, n/2). Interleaving hands every thread a mixture
 *   of small and large candidates, so the workloads differ by roughly one
 *   candidate rather than by a factor of several. It also needs no runtime
 *   scheduling or locking - each thread computes its own index range from its
 *   id alone.
 *
 * Synchronisation: none is required during the search. Every thread appends to
 * its own private buffer, so there are no shared writes and therefore no race
 * conditions and no mutex overhead. The per-thread lists are combined by a
 * k-way merge after pthread_join(), which restores the global ascending order.
 *
 * Compile: gcc -o lab4task2 lab4task2.c -lpthread -lm
 * Run:     ./lab4task2
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <pthread.h>

#define OUTPUT_FILE     "primes_pthread.txt"
#define STDOUT_LIMIT    100     /* n below this is printed to the terminal */
#define MAX_THREADS     64

/* Work descriptor handed to each thread. */
typedef struct {
    int   thread_id;        /* 0 .. num_threads-1, selects this thread's slice */
    int   num_threads;      /* stride of the cyclic decomposition              */
    long  n;                /* search primes strictly less than n              */
    long *primes;           /* private output buffer (no sharing between threads) */
    long  count;            /* number of primes this thread found              */
} ThreadData;

/*
 * Returns 1 if k is prime, 0 otherwise.
 *
 * Only divisors up to sqrt(k) are tested (see the reasoning in the header).
 * Even numbers are rejected up front so the loop can step by 2 and skip all
 * even divisors, halving the number of divisions performed.
 */
int is_prime(long k)
{
    long divisor;
    long limit;

    if (k < 2) {
        return 0;
    }
    if (k == 2) {
        return 1;
    }
    if (k % 2 == 0) {
        return 0;
    }

    /* Largest divisor that needs to be tested. */
    limit = (long)sqrt((double)k);

    for (divisor = 3; divisor <= limit; divisor += 2) {
        if (k % divisor == 0) {
            return 0;
        }
    }

    return 1;
}

/*
 * Thread entry point. Tests only the candidates belonging to this thread's
 * slice of the cyclic decomposition and stores the primes it finds, in
 * ascending order, in its own private buffer.
 */
void *find_primes(void *arg)
{
    ThreadData *data = (ThreadData *)arg;
    long k;

    data->count = 0;

    for (k = 2 + data->thread_id; k < data->n; k += data->num_threads) {
        if (is_prime(k)) {
            data->primes[data->count++] = k;
        }
    }

    return NULL;
}

/*
 * k-way merge of the num_threads private buffers into a single ascending list.
 *
 * Each buffer is already sorted, so repeatedly taking the smallest unconsumed
 * head value produces the globally sorted list. With a small number of threads
 * a linear scan over the heads is the simplest and fastest choice.
 * Returns the total number of primes written to result.
 */
long merge_results(ThreadData *data, int num_threads, long *result)
{
    long heads[MAX_THREADS];    /* next unconsumed index in each buffer */
    long total = 0;
    long written;
    int  t;

    for (t = 0; t < num_threads; t++) {
        heads[t] = 0;
        total += data[t].count;
    }

    for (written = 0; written < total; written++) {
        int  best = -1;
        long best_value = 0;

        for (t = 0; t < num_threads; t++) {
            if (heads[t] < data[t].count) {
                long value = data[t].primes[heads[t]];

                if (best == -1 || value < best_value) {
                    best = t;
                    best_value = value;
                }
            }
        }

        result[written] = best_value;
        heads[best]++;
    }

    return total;
}

int main(void)
{
    pthread_t  threads[MAX_THREADS];
    ThreadData data[MAX_THREADS];
    long       n;
    int        num_threads;
    int        t;
    long       i;
    long       count;
    long      *primes = NULL;
    long       slice_size;
    struct timespec start, end, startComp, endComp;
    double     elapsed, elapsedComp;

    printf("Enter n (search for primes strictly less than n): ");
    if (scanf("%ld", &n) != 1) {
        fprintf(stderr, "Error: invalid input for n.\n");
        return EXIT_FAILURE;
    }

    printf("Enter number of threads (1 - %d): ", MAX_THREADS);
    if (scanf("%d", &num_threads) != 1 ||
        num_threads < 1 || num_threads > MAX_THREADS) {
        fprintf(stderr, "Error: invalid number of threads.\n");
        return EXIT_FAILURE;
    }

    if (n < 2) {
        printf("There are no prime numbers less than %ld.\n", n);
        return EXIT_SUCCESS;
    }

    /*
     * Upper bound on the candidates handed to any one thread by the cyclic
     * decomposition, and therefore on the primes it can find. Allocating up
     * front keeps allocation out of the timed section.
     */
    slice_size = (n / num_threads) + 1;

    for (t = 0; t < num_threads; t++) {
        data[t].thread_id   = t;
        data[t].num_threads = num_threads;
        data[t].n           = n;
        data[t].count       = 0;
        data[t].primes      = (long *)malloc((size_t)slice_size * sizeof(long));

        if (data[t].primes == NULL) {
            fprintf(stderr, "Error: memory allocation failed for thread %d.\n", t);
            while (--t >= 0) {
                free(data[t].primes);
            }
            return EXIT_FAILURE;
        }
    }

    primes = (long *)malloc((size_t)n * sizeof(long));
    if (primes == NULL) {
        fprintf(stderr, "Error: memory allocation failed for merged list.\n");
        for (t = 0; t < num_threads; t++) {
            free(data[t].primes);
        }
        return EXIT_FAILURE;
    }

    /* -- Timed section: thread creation, search, join, merge and output -- */
    clock_gettime(CLOCK_MONOTONIC, &start);

    /* --- Timed sub-section: thread creation, search, join and merge --- */
    clock_gettime(CLOCK_MONOTONIC, &startComp);

    for (t = 0; t < num_threads; t++) {
        if (pthread_create(&threads[t], NULL, find_primes, &data[t]) != 0) {
            fprintf(stderr, "Error: could not create thread %d.\n", t);
            return EXIT_FAILURE;
        }
    }

    for (t = 0; t < num_threads; t++) {
        pthread_join(threads[t], NULL);
    }

    count = merge_results(data, num_threads, primes);

    clock_gettime(CLOCK_MONOTONIC, &endComp);
    /* ---------------------------------------------------------------------- */

    /* The merge guarantees the list is in ascending order. */
    if (n < STDOUT_LIMIT) {
        printf("Prime numbers less than %ld:\n", n);
        for (i = 0; i < count; i++) {
            printf("%ld%s", primes[i], (i == count - 1) ? "\n" : ", ");
        }
    } else {
        FILE *fp = fopen(OUTPUT_FILE, "w");

        if (fp == NULL) {
            fprintf(stderr, "Error: could not open %s for writing.\n", OUTPUT_FILE);
            free(primes);
            for (t = 0; t < num_threads; t++) {
                free(data[t].primes);
            }
            return EXIT_FAILURE;
        }

        fprintf(fp, "Prime numbers less than %ld:\n", n);
        for (i = 0; i < count; i++) {
            fprintf(fp, "%ld\n", primes[i]);
        }
        fclose(fp);

        printf("Prime numbers written to %s\n", OUTPUT_FILE);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    /* ---------------------------------------------------------------------- */

    elapsedComp = (double)(endComp.tv_sec - startComp.tv_sec) +
                  (double)(endComp.tv_nsec - startComp.tv_nsec) / 1.0e9;
    elapsed = (double)(end.tv_sec - start.tv_sec) +
              (double)(end.tv_nsec - start.tv_nsec) / 1.0e9;

    printf("Threads used: %d\n", num_threads);
    printf("Total primes found: %ld\n", count);
    printf("Computational time only (s): %.6f\n", elapsedComp);
    printf("Overall time, including output (s): %.6f\n", elapsed);

    /* Per-thread counts show how evenly the cyclic decomposition shared out
       the work - useful evidence when discussing load balance. */
    printf("Primes found per thread:");
    for (t = 0; t < num_threads; t++) {
        printf(" [T%d]=%ld", t, data[t].count);
    }
    printf("\n");

    free(primes);
    for (t = 0; t < num_threads; t++) {
        free(data[t].primes);
    }

    return EXIT_SUCCESS;
}
