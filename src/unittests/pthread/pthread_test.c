/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 amigazen project
 * 
 * Version:  1.1
 * Created:  03/09/2025
 * Compiler: SAS/C (Amiga)
 */

#include <stdio.h>
#include <stdlib.h>
#include "//include/pthread.h"
#include "//include/semaphore.h"
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <errno.h>
#include <proto/dos.h>

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~ UTILITIES ~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* Test case counters */
static int tests_run = 0;
static int tests_failed = 0;

/* Function prototypes */
void amiga_sleep_ms(long milliseconds);
void test_thread_creation_and_join(void);
void test_mutexes(void);
void test_condition_variables(void);
void test_thread_attributes(void);
void test_mutex_trylock(void);
void test_condvar_stress(void);
void test_posix_semaphores(void);
void test_cancellation_stubs(void);

/* External function prototypes */
extern void chkabort(void);


/* Simple assertion macro */
#define ASSERT(condition) \
    do { \
        tests_run++; \
        if (!(condition)) { \
            fprintf(stderr, "[FAIL] %s:%d: Assertion '%s' failed.\n", __FILE__, __LINE__, #condition); \
            tests_failed++; \
        } else { \
            printf("[PASS] %s:%d: Assertion '%s' passed.\n", __FILE__, __LINE__, #condition); \
        } \
    } while (0)

/* Amiga sleep function using Delay() - much more efficient than busy-wait */
void amiga_sleep_ms(long milliseconds) {
    if (milliseconds > 0) {
        /* Delay() takes ticks, where 50 ticks = 1 second on PAL, 60 on NTSC */
        /* We'll use 50 ticks per second for PAL timing */
        long ticks = (milliseconds * 50) / 1000;
        if (ticks == 0) {
            ticks = 1; /* Wait at least one tick */
        }
        
        /* Check for user abort before and after delay */
        chkabort();
        Delay(ticks);
        chkabort();
    }
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~ TEST 1: THREAD CREATION AND JOINING ~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#define NUM_THREADS_CREATE 5  /* Reduced for Amiga testing */

void* simple_worker(void* arg) {
    long thread_num = (long)arg;
    /* Simulate some work using Amiga Delay */
    amiga_sleep_ms(50 + (thread_num % 3) * 10);
    printf("Thread %ld: completed work\n", thread_num);
    return (void*)(thread_num * 2);
}

void test_thread_creation_and_join() {
    pthread_t threads[NUM_THREADS_CREATE];
    int i;
    int ret;

    printf("\n--- Running Test 1: Thread Creation and Joining ---\n");

    for (i = 0; i < NUM_THREADS_CREATE; i++) {
        ret = pthread_create(&threads[i], NULL, simple_worker, (void*)(long)i);
        ASSERT(ret == 0);
        if (ret != 0) {
            fprintf(stderr, "Failed to create thread %d\n", i);
        }
    }

    for (i = 0; i < NUM_THREADS_CREATE; i++) {
        void* retval;
        ret = pthread_join(threads[i], &retval);
        ASSERT(ret == 0);
        ASSERT((long)retval == (long)(i * 2));
    }
    printf("--- Test 1 Finished ---\n");
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~ TEST 2: MUTEX FOR RACE CONDITION ~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#define NUM_THREADS_MUTEX 10  /* Reduced for Amiga testing */
#define ITERATIONS_PER_THREAD 100  /* Reduced for faster testing */

pthread_mutex_t counter_mutex;
volatile int shared_counter = 0;

void* mutex_worker(void* arg) {
    int i;
    for (i = 0; i < ITERATIONS_PER_THREAD; i++) {
        pthread_mutex_lock(&counter_mutex);
        shared_counter++;
        pthread_mutex_unlock(&counter_mutex);
        /* Small delay to allow other threads to run */
        amiga_sleep_ms(1);
    }
    return NULL;
}

void test_mutexes() {
    pthread_t threads[NUM_THREADS_MUTEX];
    int i;
    int expected_value;
    int ret;

    printf("\n--- Running Test 2: Mutex for Preventing Race Conditions ---\n");

    /* Initialize mutex explicitly */
    ret = pthread_mutex_init(&counter_mutex, NULL);
    ASSERT(ret == 0);

    shared_counter = 0; /* Reset counter */

    for (i = 0; i < NUM_THREADS_MUTEX; i++) {
        ret = pthread_create(&threads[i], NULL, mutex_worker, NULL);
        ASSERT(ret == 0);
    }

    for (i = 0; i < NUM_THREADS_MUTEX; i++) {
        ret = pthread_join(threads[i], NULL);
        ASSERT(ret == 0);
    }

    expected_value = NUM_THREADS_MUTEX * ITERATIONS_PER_THREAD;
    printf("Final counter value: %d, Expected: %d\n", shared_counter, expected_value);
    ASSERT(shared_counter == expected_value);
    
    /* Clean up mutex */
    ret = pthread_mutex_destroy(&counter_mutex);
    ASSERT(ret == 0);
    
    printf("--- Test 2 Finished ---\n");
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~ TEST 3: CONDITION VARIABLES ~~~~~~~~~~~~~~~~~~~~~~~~~~ */

pthread_mutex_t cond_mutex;
pthread_cond_t condition_var;
int work_ready = 0;

void* consumer_worker(void* arg) {
    int ret;
    ret = pthread_mutex_lock(&cond_mutex);
    ASSERT(ret == 0);
    
    while (work_ready == 0) {
        /* Wait for the producer to signal */
        ret = pthread_cond_wait(&condition_var, &cond_mutex);
        ASSERT(ret == 0);
    }
    /* Now work_ready is 1 */
    printf("Consumer: Woke up and consumed data.\n");
    work_ready = 2; /* Signal back to producer that we are done */
    
    ret = pthread_mutex_unlock(&cond_mutex);
    ASSERT(ret == 0);
    return NULL;
}

void* producer_worker(void* arg) {
    int ret;
    amiga_sleep_ms(500); /* Give consumer time to wait */
    
    ret = pthread_mutex_lock(&cond_mutex);
    ASSERT(ret == 0);
    
    work_ready = 1;
    printf("Producer: Produced data and signaling consumer.\n");
    
    ret = pthread_cond_signal(&condition_var);
    ASSERT(ret == 0);
    
    ret = pthread_mutex_unlock(&cond_mutex);
    ASSERT(ret == 0);
    return NULL;
}

void test_condition_variables() {
    pthread_t producer, consumer;
    int ret;
    
    printf("\n--- Running Test 3: Condition Variables for Synchronization ---\n");
    
    /* Initialize synchronization objects */
    ret = pthread_mutex_init(&cond_mutex, NULL);
    ASSERT(ret == 0);
    
    ret = pthread_cond_init(&condition_var, NULL);
    ASSERT(ret == 0);
    
    work_ready = 0; /* Reset state */

    ret = pthread_create(&consumer, NULL, consumer_worker, NULL);
    ASSERT(ret == 0);
    
    ret = pthread_create(&producer, NULL, producer_worker, NULL);
    ASSERT(ret == 0);

    ret = pthread_join(producer, NULL);
    ASSERT(ret == 0);
    
    ret = pthread_join(consumer, NULL);
    ASSERT(ret == 0);

    ASSERT(work_ready == 2);
    
    /* Clean up synchronization objects */
    ret = pthread_cond_destroy(&condition_var);
    ASSERT(ret == 0);
    
    ret = pthread_mutex_destroy(&cond_mutex);
    ASSERT(ret == 0);
    
    printf("--- Test 3 Finished ---\n");
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~ TEST 4: THREAD ATTRIBUTES ~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void* attr_worker(void* arg) {
    long thread_num = (long)arg;
    printf("Thread %ld: running with custom attributes\n", thread_num);
    amiga_sleep_ms(100);
    return (void*)(thread_num * 10);
}

void test_thread_attributes() {
    pthread_t thread;
    pthread_attr_t attr;
    int ret;
    void* retval;

    printf("\n--- Running Test 4: Thread Attributes ---\n");

    /* Initialize attributes */
    ret = pthread_attr_init(&attr);
    ASSERT(ret == 0);

    /* Set some attributes */
    ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    ASSERT(ret == 0);

    ret = pthread_attr_setstacksize(&attr, 8192); /* 8KB stack */
    ASSERT(ret == 0);

    /* Create thread with attributes */
    ret = pthread_create(&thread, &attr, attr_worker, (void*)42);
    ASSERT(ret == 0);

    /* Join thread */
    ret = pthread_join(thread, &retval);
    ASSERT(ret == 0);
    ASSERT((long)retval == 420); /* 42 * 10 */

    /* Clean up attributes */
    ret = pthread_attr_destroy(&attr);
    ASSERT(ret == 0);

    printf("--- Test 4 Finished ---\n");
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~ TEST 5: MUTEX TRYLOCK ~~~~~~~~~~~~~~~~~~~~~~~~~~ */

pthread_mutex_t trylock_mutex;

void* trylock_worker(void* arg) {
    int ret;
    long thread_num = (long)arg;
    
    printf("Thread %ld: attempting to lock mutex\n", thread_num);
    
    ret = pthread_mutex_lock(&trylock_mutex);
    ASSERT(ret == 0);
    
    printf("Thread %ld: acquired mutex, holding for 200ms\n", thread_num);
    amiga_sleep_ms(200);
    
    ret = pthread_mutex_unlock(&trylock_mutex);
    ASSERT(ret == 0);
    
    printf("Thread %ld: released mutex\n", thread_num);
    return NULL;
}

void test_mutex_trylock() {
    pthread_t thread1, thread2;
    int ret;
    int trylock_result;

    printf("\n--- Running Test 5: Mutex TryLock ---\n");

    /* Initialize mutex */
    ret = pthread_mutex_init(&trylock_mutex, NULL);
    ASSERT(ret == 0);

    /* Create first thread */
    ret = pthread_create(&thread1, NULL, trylock_worker, (void*)1);
    ASSERT(ret == 0);

    /* Small delay to let thread1 acquire the mutex */
    amiga_sleep_ms(50);

    /* Try to lock the mutex from main thread (should fail) */
    trylock_result = pthread_mutex_trylock(&trylock_mutex);
    ASSERT(trylock_result == EBUSY); /* Should be busy */

    /* Create second thread */
    ret = pthread_create(&thread2, NULL, trylock_worker, (void*)2);
    ASSERT(ret == 0);

    /* Wait for both threads to complete */
    ret = pthread_join(thread1, NULL);
    ASSERT(ret == 0);
    
    ret = pthread_join(thread2, NULL);
    ASSERT(ret == 0);

    /* Now trylock should succeed */
    trylock_result = pthread_mutex_trylock(&trylock_mutex);
    ASSERT(trylock_result == 0);
    
    ret = pthread_mutex_unlock(&trylock_mutex);
    ASSERT(ret == 0);

    /* Clean up mutex */
    ret = pthread_mutex_destroy(&trylock_mutex);
    ASSERT(ret == 0);

    printf("--- Test 5 Finished ---\n");
}

/* ~~~~~~~~~~~~~~~~~~ TEST 6: CONDITION VARIABLE STRESS TEST (RACE CONDITION) ~~~~~~~~~~~~~~~~~~ */
#define STRESS_ITERATIONS 50  /* Reduced for faster testing */
pthread_mutex_t stress_mutex;
pthread_cond_t stress_cond;
volatile int stress_turn = 0; /* 0 for pinger, 1 for ponger */
volatile int stress_counter = 0;

void* pinger_worker(void* arg) {
    int i;
    for (i = 0; i < STRESS_ITERATIONS; ++i) {
        chkabort();  /* Check for user interrupt */
        pthread_mutex_lock(&stress_mutex);
        while (stress_turn != 0) {
            pthread_cond_wait(&stress_cond, &stress_mutex);
        }
        stress_counter++;
        stress_turn = 1;
        pthread_cond_signal(&stress_cond);
        pthread_mutex_unlock(&stress_mutex);
    }
    return NULL;
}

void* ponger_worker(void* arg) {
    int i;
    for (i = 0; i < STRESS_ITERATIONS; ++i) {
        chkabort();  /* Check for user interrupt */
        pthread_mutex_lock(&stress_mutex);
        while (stress_turn != 1) {
            pthread_cond_wait(&stress_cond, &stress_mutex);
        }
        stress_counter++;
        stress_turn = 0;
        pthread_cond_signal(&stress_cond);
        pthread_mutex_unlock(&stress_mutex);
    }
    return NULL;
}

void test_condvar_stress() {
    pthread_t pinger, ponger;
    int ret;

    printf("\n--- Running Test 6: Condition Variable Stress Test ---\n");
    printf("This test will hang if a 'lost wakeup' race condition occurs.\n");

    ret = pthread_mutex_init(&stress_mutex, NULL);
    ASSERT(ret == 0);
    ret = pthread_cond_init(&stress_cond, NULL);
    ASSERT(ret == 0);

    stress_turn = 0;
    stress_counter = 0;

    ret = pthread_create(&pinger, NULL, pinger_worker, NULL);
    ASSERT(ret == 0);
    ret = pthread_create(&ponger, NULL, ponger_worker, NULL);
    ASSERT(ret == 0);

    ret = pthread_join(pinger, NULL);
    ASSERT(ret == 0);
    ret = pthread_join(ponger, NULL);
    ASSERT(ret == 0);
    
    printf("Final stress counter: %d, Expected: %d\n", stress_counter, STRESS_ITERATIONS * 2);
    ASSERT(stress_counter == STRESS_ITERATIONS * 2);

    ret = pthread_cond_destroy(&stress_cond);
    ASSERT(ret == 0);
    ret = pthread_mutex_destroy(&stress_mutex);
    ASSERT(ret == 0);

    printf("--- Test 6 Finished ---\n");
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~ TEST 7: POSIX SEMAPHORES ~~~~~~~~~~~~~~~~~~~~~~~~~~ */
volatile int sem_worker_unblocked = 0;
sem_t test_sem;

void* sem_worker(void* arg) {
    int ret;
    printf("Semaphore worker: starting, about to call sem_wait()\n");
    ret = sem_wait(&test_sem);
    printf("Semaphore worker: sem_wait() returned %d\n", ret);
    sem_worker_unblocked = 1;
    printf("Semaphore worker: set unblocked flag, exiting\n");
    return NULL;
}

void test_posix_semaphores() {
    pthread_t worker;
    int ret;
    int sval;

    printf("\n--- Running Test 7: POSIX Semaphores ---\n");

    /* Test basic wait/post */
    ret = sem_init(&test_sem, 0, 0);
    ASSERT(ret == 0);

    sem_worker_unblocked = 0;
    ret = pthread_create(&worker, NULL, sem_worker, NULL);
    ASSERT(ret == 0);

    printf("Main thread sleeping to let worker block...\n");
    amiga_sleep_ms(200);
    printf("Main thread: checking if worker is blocked, unblocked=%d\n", sem_worker_unblocked);
    ASSERT(sem_worker_unblocked == 0); /* Worker should be blocked */

    printf("Main thread posting to semaphore...\n");
    ret = sem_post(&test_sem);
    ASSERT(ret == 0);
    printf("Main thread: sem_post() returned %d\n", ret);

    ret = pthread_join(worker, NULL);
    ASSERT(ret == 0);
    ASSERT(sem_worker_unblocked == 1); /* Worker should now be unblocked */

    ret = sem_destroy(&test_sem);
    ASSERT(ret == 0);
    
    /* Test sem_getvalue - our implementation returns -1 to indicate unsupported */
    printf("Testing sem_getvalue...\n");
    ret = sem_init(&test_sem, 0, 5);
    ASSERT(ret == 0);
    
    ret = sem_getvalue(&test_sem, &sval);
    ASSERT(ret == 0);
    printf("Sem value: %d (expected -1 for unsupported)\n", sval);
    ASSERT(sval == -1);  /* Our implementation returns -1 to indicate unsupported */
    
    sem_wait(&test_sem);
    ret = sem_getvalue(&test_sem, &sval);
    ASSERT(ret == 0);
    printf("Sem value after wait: %d (expected -1 for unsupported)\n", sval);
    ASSERT(sval == -1);  /* Still returns -1 since it's not supported */

    ret = sem_destroy(&test_sem);
    ASSERT(ret == 0);

    printf("--- Test 7 Finished ---\n");
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~ TEST 8: CANCELLATION STUBS (ENOSYS) ~~~~~~~~~~~~~~~~~~~~~~~ */
void* dummy_worker(void* arg) {
    amiga_sleep_ms(200);
    return NULL;
}

void test_cancellation_stubs() {
    pthread_t thread;
    int ret;
    int oldstate;

    printf("\n--- Running Test 8: Cancellation Stubs (ENOSYS) ---\n");

    ret = pthread_create(&thread, NULL, dummy_worker, NULL);
    ASSERT(ret == 0);
    amiga_sleep_ms(50);

    /* Test that pthread_cancel returns 'Function not implemented' */
    ret = pthread_cancel(thread);
    printf("pthread_cancel returned %d (expected ENOSYS=%d)\n", ret, ENOSYS);
    ASSERT(ret == ENOSYS);
    
    /* Test other cancellation functions */
    ret = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldstate);
    printf("pthread_setcancelstate returned %d (expected ENOSYS=%d)\n", ret, ENOSYS);
    ASSERT(ret == ENOSYS);
    
    ret = pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &oldstate);
    printf("pthread_setcanceltype returned %d (expected ENOSYS=%d)\n", ret, ENOSYS);
    ASSERT(ret == ENOSYS);

    /* Clean up the thread */
    ret = pthread_join(thread, NULL);
    ASSERT(ret == 0);

    printf("--- Test 8 Finished ---\n");
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~ MAIN DRIVER ~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int main() {
    printf("======== PTHREAD IMPLEMENTATION TEST SUITE =========\n");
    printf("Testing Amiga native pthread implementation\n");
    printf("Using Delay() for efficient timing\n");

    test_thread_creation_and_join();
    test_mutexes();
    test_condition_variables();
    test_thread_attributes();
    test_mutex_trylock();
    test_condvar_stress();
    test_posix_semaphores();
    test_cancellation_stubs();

    printf("\n==================== SUMMARY =====================\n");
    printf("Tests Run:    %d\n", tests_run);
    if (tests_failed == 0) {
        printf("Tests Passed: %d\n", tests_run);
        printf("Result: ALL TESTS PASSED\n");
        return EXIT_SUCCESS;
    } else {
        printf("Tests Failed: %d\n", tests_failed);
        printf("Result: SOME TESTS FAILED\n");
        return EXIT_FAILURE;
    }
}

