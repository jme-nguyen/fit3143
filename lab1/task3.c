/*
 * FIT3143 - Lab Task 3: Parallel Prime Number Search using OpenMP
 *
 * Searches for all prime numbers strictly less than an integer n supplied by
 * the user, and prints the sorted (ascending) list to:
 *   a) standard output, when n < 100
 *   b) a text file ("primes_parallel.txt"), when n >= 100
 *
 * Partitioning scheme:
 *   Testing one candidate k for primality is completely independent of
 *   testing any other candidate, so this is an embarrassingly parallel
 *   problem. Each thread writes its result (1 = prime, 0 = not prime) into
 *   its own index of a shared flag array, is_prime_flag[k]. Because every
 *   thread writes to a distinct array index, no locks, atomics, or critical
 *   sections are needed inside the parallel loop.
 *
 *   The cost of testing a single k is NOT uniform: a composite number with
 *   a small factor (e.g. any even number) is rejected almost immediately,
 *   while a prime (or a composite whose smallest factor is close to sqrt(k))
 *   needs every odd divisor up to sqrt(k) tested. Because primes become
 *   sparser but individually more expensive as k grows, a naive static
 *   block partition (splitting [2, n) into P equal contiguous chunks, one
 *   per thread) tends to be unbalanced: it's possible for one thread's
 *   block to contain a denser run of expensive primes than another's, so
 *   some threads finish and sit idle while others are still working.
 *
 *   To balance the load instead, the loop uses OpenMP's dynamic scheduling
 *   with a moderate chunk size (CHUNK_SIZE): each thread is handed a small
 *   chunk of consecutive k values, and as soon as it finishes, it requests
 *   the next unclaimed chunk, until the whole range is exhausted. This
 *   keeps every thread busy right up to the end, at the cost of a small
 *   amount of scheduling overhead per chunk (which is why the chunk size
 *   is a tunable trade-off rather than 1).
 *
 * Compile: gcc -o task3 task3.c -fopenmp -lm
 * Run:     ./task3 [num_threads]
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

#define OUTPUT_FILE     "primes_parallel.txt"
#define STDOUT_LIMIT    100     /* n below this is printed to the terminal */
#define CHUNK_SIZE      1000    /* tunable: see report discussion on scheduling */
#define MAX_THREADS     64      /* bounds the per-thread instrumentation arrays */

/*
 * Returns 1 if k is prime, 0 otherwise. Uses only local variables, so it is
 * safe to call from multiple threads concurrently with no synchronisation.
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

    limit = (long)sqrt((double)k);

    for (divisor = 3; divisor <= limit; divisor += 2) {
        if (k % divisor == 0) {
            return 0;
        }
    }

    return 1;
}

int main(int argc, char *argv[])
{
    long n;
    long k;
    long count = 0;
    long *primes = NULL;
    char *is_prime_flag = NULL;
    int num_threads;
    double start, end, elapsed;
    /* Per-thread work counters, for the load-balance discussion (mirrors
     * Task 2's "primes found per thread" diagnostic). Each cell is only
     * ever written by the thread it belongs to, so no synchronisation is
     * needed even though the arrays themselves are shared. */
    long thread_candidates_tested[MAX_THREADS] = {0};
    long thread_primes_found[MAX_THREADS] = {0};

    /* Number of threads: optional command-line argument (./task3 8), so it
     * can be varied across runs without recompiling for the speed-up
     * comparison. Falls back to the OpenMP default (usually the number of
     * available cores) if not supplied. */
    if (argc == 2) {
        num_threads = atoi(argv[1]);
        if (num_threads < 1 || num_threads > MAX_THREADS) {
            fprintf(stderr, "Error: number of threads must be between 1 and %d.\n", MAX_THREADS);
            return EXIT_FAILURE;
        }
        omp_set_num_threads(num_threads);
    } else {
        num_threads = omp_get_max_threads();
        if (num_threads > MAX_THREADS) {
            num_threads = MAX_THREADS;
            omp_set_num_threads(num_threads);
        }
    }

    printf("Enter n (search for primes strictly less than n): ");
    if (scanf("%ld", &n) != 1) {
        fprintf(stderr, "Error: invalid input.\n");
        return EXIT_FAILURE;
    }

    if (n < 2) {
        printf("There are no prime numbers less than %ld.\n", n);
        return EXIT_SUCCESS;
    }

    primes = (long *)malloc((size_t)n * sizeof(long));
    is_prime_flag = (char *)calloc((size_t)n, sizeof(char));
    if (primes == NULL || is_prime_flag == NULL) {
        fprintf(stderr, "Error: memory allocation failed.\n");
        free(primes);
        free(is_prime_flag);
        return EXIT_FAILURE;
    }

    /* ---------------- Timed section: the prime search ---------------- */
    start = omp_get_wtime();

    /* Parallel phase: each thread tests a distinct subset of k values and
     * writes to its own index of is_prime_flag, so the loop body needs no
     * synchronisation. schedule(dynamic, CHUNK_SIZE) hands out work chunk
     * by chunk on demand, which balances the uneven per-k testing cost
     * described above. */
    #pragma omp parallel for schedule(dynamic, CHUNK_SIZE) default(none) \
        shared(n, is_prime_flag, thread_candidates_tested, thread_primes_found) private(k)
    for (k = 2; k < n; k++) {
        int tid = omp_get_thread_num();
        int prime = is_prime(k);

        is_prime_flag[k] = (char)prime;
        thread_candidates_tested[tid]++;
        if (prime) {
            thread_primes_found[tid]++;
        }
    }

    /* Compaction phase: a single sequential pass turns the flag array into
     * a compact, sorted (ascending) list, since k is visited in increasing
     * order. This step is deliberately kept serial: parallelising it would
     * need either a critical section around count++ (which would serialise
     * the threads anyway and add lock overhead) or a parallel prefix-sum,
     * whereas this O(n) pass is already far cheaper than the O(n * sqrt(n))
     * primality testing above, so it isn't worth parallelising. */
    for (k = 2; k < n; k++) {
        if (is_prime_flag[k]) {
            primes[count++] = k;
        }
    }

    end = omp_get_wtime();
    elapsed = end - start;
    /* ----------------------------------------------------------------- */

    if (n < STDOUT_LIMIT) {
        printf("Prime numbers less than %ld:\n", n);
        for (k = 0; k < count; k++) {
            printf("%ld%s", primes[k], (k == count - 1) ? "\n" : ", ");
        }
    } else {
        FILE *fp = fopen(OUTPUT_FILE, "w");

        if (fp == NULL) {
            fprintf(stderr, "Error: could not open %s for writing.\n", OUTPUT_FILE);
            free(primes);
            free(is_prime_flag);
            return EXIT_FAILURE;
        }

        fprintf(fp, "Prime numbers less than %ld:\n", n);
        for (k = 0; k < count; k++) {
            fprintf(fp, "%ld\n", primes[k]);
        }
        fclose(fp);

        printf("Prime numbers written to %s\n", OUTPUT_FILE);
    }

    printf("Threads used: %d\n", num_threads);
    printf("Total primes found: %ld\n", count);
    printf("Elapsed time: %.6f seconds\n", elapsed);

    /* Per-thread breakdown shows how evenly the dynamic schedule shared out
       the work - useful evidence when discussing load balance. */
    printf("Candidates tested per thread:");
    for (k = 0; k < num_threads; k++) {
        printf(" [T%ld]=%ld", k, thread_candidates_tested[k]);
    }
    printf("\n");
    printf("Primes found per thread:");
    for (k = 0; k < num_threads; k++) {
        printf(" [T%ld]=%ld", k, thread_primes_found[k]);
    }
    printf("\n");

    free(primes);
    free(is_prime_flag);
    return EXIT_SUCCESS;
}