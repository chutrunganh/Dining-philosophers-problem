/*
Created by: anh.ct225564@sis.hust.edu.vn
*/
#include <iostream>
#include <thread>
#include <mutex>
#include <semaphore.h>
#include <chrono>
#include <random>
using namespace std;

#define N 5
#define LEFT i
#define RIGHT (i + 1) % N

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define RESET "\033[0m"

sem_t forks[N];

void randomDelay() {
    static random_device rd;
    static mt19937 gen(rd());
    static uniform_int_distribution<> dis(100, 500);
    this_thread::sleep_for(chrono::milliseconds(dis(gen)));
}

bool tryPickForks(int i) {
    // Try to pick left fork
    printf("%sPhilosopher %d trying left fork %d%s\n", YELLOW, i, LEFT, RESET);
    sem_wait(&forks[LEFT]);
    
    // Try to pick right fork
    printf("%sPhilosopher %d trying right fork %d%s\n", YELLOW, i, RIGHT, RESET);
    if (sem_trywait(&forks[RIGHT]) == 0) {
        // Success - got both forks
        printf("%sPhilosopher %d got both forks%s\n", GREEN, i, RESET);
        return true;
    }
    
    // Failed to get right fork - release left and try again later
    printf("%sPhilosopher %d couldn't get right fork, releasing left%s\n", RED, i, RESET);
    sem_post(&forks[LEFT]);
    return false;
}

void philosopher(int i) {
    while(true) {
        // Think
        printf("%sPhilosopher %d is thinking%s\n", BLUE, i, RESET);
        randomDelay();

        // Try to eat
        while (!tryPickForks(i)) {
            printf("%sPhilosopher %d backing off%s\n", RED, i, RESET);
            randomDelay(); // Random backoff delay
        }

        // Eating
        printf("%sPhilosopher %d is eating%s\n", GREEN, i, RESET);
        randomDelay();

        // Put down forks
        sem_post(&forks[LEFT]);
        sem_post(&forks[RIGHT]);
        printf("Philosopher %d finished eating\n", i);
    }
}

int main() {
    // Initialize semaphores
    for(int i = 0; i < N; i++) {
        sem_init(&forks[i], 0, 1);
    }

    // Create philosophers
    std::thread philosophers[N];
    for(int i = 0; i < N; i++) {
        philosophers[i] = std::thread(philosopher, i);
    }

    // Join threads
    for(int i = 0; i < N; i++) {
        philosophers[i].join();
    }

    return 0;
}

/*
This solution can lead to livelock if all philosophers try to pick up the left fork first and then they will all
release the left fork and then wait. In case they all wait and return to pick up the left fork again at the same
time, livelock will occur. 

- In deadlock, processes are completely stuck and unable to proceed
- In livelock, processes are actively trying to proceed but make no progress
 because they're stuck in a repeating pattern

Currently, we can not simulate the livelock scenario in this code yet due to the difficulty of ensure all five
philosophers to pick up the left fork, return it then wait and return at the same time.


*/