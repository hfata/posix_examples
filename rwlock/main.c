//
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <unistd.h>
#include <mqueue.h>
#include <time.h>

// Number of concurrent reader and writer threads for the synchronization demo
#define READER_THREAD_NUM   9
#define WRITER_THREAD_NUM   1
#define TOTAL_THREAD_NUM    (READER_THREAD_NUM + WRITER_THREAD_NUM)

pthread_t thread_id[TOTAL_THREAD_NUM];

/**
 * @brief Structure for writer thread structures
 */
typedef struct {
    void *(*writer_func)(void *);     /**< Function pointer for writer thread function */
    pthread_t writer_thread_id[WRITER_THREAD_NUM];  /**< Array of writer thread IDs */
} ts_writer;

/**
 * @brief Structure for reader thread management
 */
typedef struct {
    void *(*reader_func)(void *);     /**< Function pointer for reader thread function */
    pthread_t reader_thread_id[READER_THREAD_NUM];  /**< Array of reader thread IDs */
} ts_reader;

/**
 * @brief Enumeration for lock types
 * @details This enum helps identify lock types (reader or writer) at runtime 
 * if user wants to track lock ownership manually. 
 */
typedef enum te_lock {
    E_WRITER_LOCK = 0x1,  /**< Writer lock flag */
    E_READER_LOCK = 0X2   /**< Reader lock flag */
} te_lock;

/**
 * @brief Main thread structure containing shared resources and synchronization primitives
 */
typedef struct thread_struct {
    pthread_mutex_t mutex;        /**< Mutex for protecting shared data */
    pthread_rwlock_t rw_lock;     /**< Read-write lock for reader-writer synchronization */
    int data;                    /**< Shared data value */
} ts_thread_struct;

/**
 * @brief Reader thread function that reads shared data
 * @param arg Pointer to thread structure containing shared data and synchronization primitives
 * @return void pointer (NULL)
 * 
 * This function implements a reader thread that:
 * - Acquires a read lock using pthread_rwlock_rdlock
 * - Reads and displays the current value of shared data
 * - Releases the read lock
 * - Sleeps for 1 second to allow other threads to run
 */
void* reader_thread_func(void* arg) {
    ts_thread_struct *s_reader_thread = (ts_thread_struct *)arg;
    while(1) {
        pthread_rwlock_rdlock(&s_reader_thread->rw_lock);
        printf("\n[READER] [Thread-%lu] acquired read lock\n", pthread_self());
        
        struct timespec local_time;
        clock_gettime(CLOCK_REALTIME, &local_time);
        printf("[READER] Time - Seconds: %ld, Nanoseconds: %ld\n", 
               local_time.tv_sec, local_time.tv_nsec);
        
        printf("[READER] Current shared data value: %d\n", s_reader_thread->data);
        fflush(stdout);
        
        pthread_rwlock_unlock(&s_reader_thread->rw_lock);
        printf("[READER] [Thread-%lu] released read lock\n", pthread_self());
        sleep(1);
    }
    return NULL;
}

/**
 * @brief Writer thread function that modifies shared data
 * @param arg Pointer to thread structure containing shared data and synchronization primitives
 * @return void pointer (NULL)
 * 
 * This function implements a writer thread that:
 * - Acquires a write lock using pthread_rwlock_wrlock
 * - Increments the shared data value
 * - Releases the write lock
 * - Sleeps for 2 seconds to allow other threads to run
 */
void* writer_thread_func(void* arg) {
    ts_thread_struct *s_writer_thread = (ts_thread_struct *)arg;
    while(1) {
        pthread_rwlock_wrlock(&s_writer_thread->rw_lock);
        printf("\n[WRITER] [Thread-%lu] acquired write lock\n", pthread_self());
        
        pthread_mutex_lock(&s_writer_thread->mutex);
        s_writer_thread->data++;
        printf("[WRITER] Updated shared data to: %d\n", s_writer_thread->data);
        pthread_mutex_unlock(&s_writer_thread->mutex);
        
        pthread_rwlock_unlock(&s_writer_thread->rw_lock);
        printf("[WRITER] [Thread-%lu] released write lock\n\n", pthread_self());
        fflush(stdout);
        sleep(3);
    }
    return NULL;
}

/**
 * @brief Main function that initializes and manages reader-writer threads
 * @return 0 on successful execution, 1 on error
 * 
 * This function:
 * - Initializes mutex and rwlock synchronization primitives
 * - Creates reader and writer threads
 * - Waits for all threads to complete
 * - Cleans up resources
 */
int main(void) {
    // Initialize writer and reader structures
    ts_writer s_writer = {
        .writer_func = &writer_thread_func
    };

    ts_reader s_reader = {
        .reader_func = &reader_thread_func
    };

    ts_thread_struct s_thread_struct = {
        .data = 0
    };

    // Initialize synchronization primitives
    if (pthread_mutex_init(&s_thread_struct.mutex, NULL) != 0) {
        perror("pthread_mutex_init");
        exit(1);
    }

    if (pthread_rwlock_init(&s_thread_struct.rw_lock, NULL) != 0) {
        perror("pthread_rwlock_init");
        pthread_mutex_destroy(&s_thread_struct.mutex);
        exit(1);
    }
    
    // Create writer threads
    for (int i = 0; i < WRITER_THREAD_NUM; i++) {
        pthread_create(&s_writer.writer_thread_id[i], NULL, writer_thread_func, &s_thread_struct);
    }
        
    // Create reader threads
    for (int i = 0; i < READER_THREAD_NUM; i++) {
        pthread_create(&s_reader.reader_thread_id[i], NULL, reader_thread_func, &s_thread_struct);
    }

    // Wait for writer threads to complete
    for (int i = 0; i < WRITER_THREAD_NUM; i++) {
        pthread_join(s_writer.writer_thread_id[i], NULL);
    }

    // Wait for reader threads to complete
    for (int i = 0; i < READER_THREAD_NUM; i++) {
        pthread_join(s_reader.reader_thread_id[i], NULL);
    }

    // Cleanup: destroy synchronization primitives
    pthread_mutex_destroy(&s_thread_struct.mutex);
    pthread_rwlock_destroy(&s_thread_struct.rw_lock);

    printf("[MAIN] All threads completed. Program finished.\n");
    return 0;
}
