#include <iostream>
#include <thread>
#include <semaphore.h>
#include <unistd.h>
#include <random>
using namespace std;

#define N 5
#define LEFT i                 // Left fork has same number as philosopher
#define RIGHT (i + 1) % N     // Right fork is next number (wrapping around)

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define RESET "\033[0m"

sem_t forks[N];

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
        sleep(1);

        printf("%sPhilosopher %d is hungry%s\n", YELLOW, i, RESET);

        if (i % 2 == 0) {  // Even philosophers
            printf("%sPhilosopher %d (even) tries left fork first%s\n", BLUE, i, RESET);
            sem_wait(&forks[LEFT]);
            printf("Philosopher %d picked up left fork %d\n", i, LEFT);
            
            sem_wait(&forks[RIGHT]);
            printf("Philosopher %d picked up right fork %d\n", i, RIGHT);
        } else {  // Odd philosophers
            printf("%sPhilosopher %d (odd) tries right fork first%s\n", BLUE, i, RESET);
            sem_wait(&forks[RIGHT]);
            printf("Philosopher %d picked up right fork %d\n", i, RIGHT);
            
            sem_wait(&forks[LEFT]);
            printf("Philosopher %d picked up left fork %d\n", i, LEFT);
        }

        // Eating
        printf("%sPhilosopher %d is eating%s\n", GREEN, i, RESET);
        sleep(2);

        // Release forks in reverse order of acquisition
        if (i % 2 == 0) {
            sem_post(&forks[RIGHT]);
            sem_post(&forks[LEFT]);
        } else {
            sem_post(&forks[LEFT]);
            sem_post(&forks[RIGHT]);
        }
        
        printf("Philosopher %d finished eating\n", i);
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

/*

Advantages:

Prevents deadlock without extra synchronization primitives
Better performance than mutex solution
Simple to implement
No additional memory overhead
Disadvantages:

Still possible to have starvation
Philosophers have different waiting times based on their position
Not as fair as other solutions
Resource utilization might be uneven
Harder to scale if number of philosophers changes
*/