/*
 * FIT3143 - Lab Task 1: Serial Prime Number Search
 *
 * Searches for all prime numbers strictly less than an integer n supplied by
 * the user, and prints the sorted (ascending) list to:
 *   a) standard output, when n < 100
 *   b) a text file ("primes.txt"), when n >= 100
 *
 * Approach (following the hint):
 *   If k is composite then k = m * n for some integers m, n > 1. Both factors
 *   cannot be greater than sqrt(k), because their product would then exceed k.
 *   Hence at least one factor is <= sqrt(k), so testing divisors from 2 up to
 *   sqrt(k) is sufficient to decide primality. This reduces the work per number
 *   from O(k) to O(sqrt(k)), which matters greatly for n up to 10,000,000.
 *
 * Compile: gcc -o task1 task1.c -lm
 * Run:     ./task1
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define OUTPUT_FILE     "primes.txt"
#define STDOUT_LIMIT    100     /* n below this is printed to the terminal */

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

int main(void)
{
    long n;
    long k;
    long count = 0;
    long *primes = NULL;
    struct timespec start, end, startComp, endComp;
    double elapsed, elapsedComp;

    printf("Enter n (search for primes strictly less than n): ");
    if (scanf("%ld", &n) != 1) {
        fprintf(stderr, "Error: invalid input.\n");
        return EXIT_FAILURE;
    }

    if (n < 2) {
        printf("There are no prime numbers less than %ld.\n", n);
        return EXIT_SUCCESS;
    }

    /*
     * Upper bound on the number of primes below n; allocating once avoids any
     * reallocation cost being included in the timed section.
     */
    primes = (long *)malloc((size_t)n * sizeof(long));
    if (primes == NULL) {
        fprintf(stderr, "Error: memory allocation failed.\n");
        return EXIT_FAILURE;
    }

    /* ---- Timed section: the prime search plus writing the output ---- */
    clock_gettime(CLOCK_MONOTONIC, &start);

    /* ------------ Timed sub-section: the prime search only ----------- */
    clock_gettime(CLOCK_MONOTONIC, &startComp);

    /* 2 is the only even prime, so it is recorded directly and every other
     * candidate tested is odd. */
    if (n > 2) {
        primes[count++] = 2;
    }

    for (k = 3; k < n; k += 2) {
        if (is_prime(k)) {
            primes[count++] = k;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &endComp);
    /* ----------------------------------------------------------------- */

    /*
     * Numbers are tested in ascending order, so the list is already sorted.
     */
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
            return EXIT_FAILURE;
        }

        fprintf(fp, "Prime numbers less than %ld:\n", n);
        for (k = 0; k < count; k++) {
            fprintf(fp, "%ld\n", primes[k]);
        }
        fclose(fp);

        printf("Prime numbers written to %s\n", OUTPUT_FILE);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    /* ----------------------------------------------------------------- */

    elapsedComp = (double)(endComp.tv_sec - startComp.tv_sec) +
                  (double)(endComp.tv_nsec - startComp.tv_nsec) / 1.0e9;
    elapsed = (double)(end.tv_sec - start.tv_sec) +
              (double)(end.tv_nsec - start.tv_nsec) / 1.0e9;

    printf("Total primes found: %ld\n", count);
    printf("Computational time only (s): %.6f\n", elapsedComp);
    printf("Overall time, including output (s): %.6f\n", elapsed);

    free(primes);
    return EXIT_SUCCESS;
}
