/*

To run:
g++ -pthread solution2.cpp -o solution2
./solution2

This solution prevents deadlock but trades it for reduced performance and concurrency. The mutex 
creates a bottleneck since philosophers must wait their turn to even attempt picking up forks.

*/

#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <random>
#include <condition_variable>
#include <semaphore.h>
#include <time.h>
using namespace std;

#define N 5
#define LEFT i                 // Left fork has same number as philosopher
#define RIGHT (i + 1) % N     // Right fork is next number (wrapping around)

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define RESET "\033[0m"

sem_t forks[N];
std::mutex pickupMutex;  // Mutex for atomic fork pickup


// Utility function for random delays, this will help simulate the eating/ thinking time of each philosopher
static void randomDelay() {
    static random_device rd;
    static mt19937 gen(rd());
    static uniform_int_distribution<> dis(100, 1000);
    this_thread::sleep_for(chrono::milliseconds(dis(gen)));
}

void philosopher(int i) {
    while(true) {
        // Think
        printf("%sPhilosopher %d is thinking%s\n", GREEN, i, RESET);
        randomDelay();

        printf("%sPhilosopher %d is hungry%s\n", YELLOW, i, RESET);

        // Critical section for picking up forks
        pickupMutex.lock(); //Mutex lock: only allow one philosopher to pick up forks at a time
        {
            // Try to get both forks atomically
            sem_wait(&forks[LEFT]);
            sem_wait(&forks[RIGHT]);
        }
        pickupMutex.unlock();

        // Eating
        printf("%sPhilosopher %d is eating with forks %d and %d%s\n", 
               GREEN, i, LEFT, RIGHT, RESET);
        randomDelay();

        // Put down forks
        sem_post(&forks[LEFT]);
        sem_post(&forks[RIGHT]);
    }
}

int main() {
    std::thread philosophers[N];

    // Initialize semaphores
    for(int i = 0; i < N; i++) {
        sem_init(&forks[i], 0, 1);
    }

    // Create philosopher threads
    for(int i = 0; i < N; i++) {
        philosophers[i] = std::thread(philosopher, i);
    }

    // Join threads
    for(int i = 0; i < N; i++) {
        philosophers[i].join();
    }

    // Cleanup
    for(int i = 0; i < N; i++) {
        sem_destroy(&forks[i]);
    }

    return 0;
}