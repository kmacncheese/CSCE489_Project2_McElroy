#include <pthread.h>
#include <stdexcept>
#include "Semaphore.h"

/*************************************************************************************
 * Semaphore (constructor) - this should take count and place it into a local variable.
 *						Here you can do any other initialization you may need.
 *
 *    Params:  count - initialization count for the semaphore
 *
 *************************************************************************************/

Semaphore::Semaphore(int count) {
    S = count;
    // initialize both the mutex for int S and the condition variable.
    if (pthread_mutex_init(&s_mutex, nullptr) != 0){
        throw std::runtime_error("Mutex init failed.");

    }
    if (pthread_cond_init(&cond, nullptr) != 0) {
        pthread_mutex_destroy(&s_mutex);
        throw std::runtime_error("Condition Variable init failed.");
    }
}


/*************************************************************************************
 * ~Semaphore (destructor) - called when the class is destroyed. Clean up any dynamic
 *						memory.
 *
 *************************************************************************************/

Semaphore::~Semaphore() {
    //smite them
    pthread_mutex_destroy(&s_mutex);
    pthread_cond_destroy(&cond);
}


/*************************************************************************************
 * wait - implement a standard wait Semaphore method here
 *
 *************************************************************************************/

void Semaphore::wait() {
    //lock S so we can decrement it later.
    if (pthread_mutex_lock(&s_mutex) != 0){
        perror("pthread_mutex_lock failed.");
        exit(1);
    }
    
    //continue to wait so long as there is no resources available.
    while (S<=0){
        if (pthread_cond_wait(&cond, &s_mutex) != 0) {
            perror("pthread_cond_wait failed.");
            exit(1);
        }
    }
    
    // change the critical value
    S--;

    // unlock S
    if (pthread_mutex_unlock(&s_mutex) != 0){
        perror("pthread_mutex_unlock failed.");
        exit(1);
    }
}


/*************************************************************************************
 * signal - implement a standard signal Semaphore method here
 *
 *************************************************************************************/

void Semaphore::signal() {
    //lock s so we can increment it
    if (pthread_mutex_lock(&s_mutex) != 0 ){
        perror("pthread_mutex_lock failed.");
        exit(1);
    }

    // increment S to "release" the resource
    S++;

    //signal its release
    if (pthread_cond_signal(&cond) != 0){
        perror("pthread_cond_signal failed.");
        exit(1);
    }

    //unlock S since we are done.
    if (pthread_mutex_unlock(&s_mutex) != 0){
        perror("pthread_mutex_unlock failed.");
        exit(1);
    }
}


