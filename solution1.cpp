/*
Created by: anh.ct225564@sis.hust.edu.vn

g++ -pthread solution1.cpp -o solution1

# Run in normal mode:
./solution1

# Run in deadlock simulation mode:
./solution1 deadlock
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


// Utility function for random delays, this will help simulate the eating/ thinking time of each philosopher
static void randomDelay() {
    static random_device rd;
    static mt19937 gen(rd());
    static uniform_int_distribution<> dis(100, 1000);
    this_thread::sleep_for(chrono::milliseconds(dis(gen)));
}


// ANSI color codes ffor visualizing the output
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define RESET "\033[0m"

sem_t forks[N]; // Array of semaphores for 5 forks
bool deadlock_mode = false;

void philosopher(int i) {
    while(true) {
        // Think
        printf("%sPhilosopher %d is thinking%s\n", GREEN, i, RESET);
        randomDelay();
        
        // Get hungry
        printf("%sPhilosopher %d is hungry%s\n", YELLOW, i, RESET);
        
        // Pick up left fork
        sem_wait(&forks[LEFT]);
        printf("Philosopher %d picked up left fork %d\n", i, LEFT);

        if (deadlock_mode) {
            // In deadlock mode, wait to ensure all philosophers pick up left fork
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }

        // Try to pick up right fork
        printf("%sPhilosopher %d trying to pick up right fork %d%s\n", RED, i, RIGHT, RESET);
        sem_wait(&forks[RIGHT]);
        printf("Philosopher %d picked up right fork %d\n", i, RIGHT);

        // Eat
        printf("%sPhilosopher %d is eating%s\n", GREEN, i, RESET);
        randomDelay();

        // Put down forks
        sem_post(&forks[LEFT]);
        printf("Philosopher %d put down left fork %d\n", i, LEFT);
        sem_post(&forks[RIGHT]);
        printf("Philosopher %d put down right fork %d\n", i, RIGHT);
    }
}

int main(int argc, char* argv[]) {
    std::thread philosophers[N];

    // Check command line arguments for mode
    if (argc > 1 && std::string(argv[1]) == "deadlock") {
        deadlock_mode = true;
        printf("%sRunning in DEADLOCK simulation mode%s\n", RED, RESET);
    } else {
        printf("%sRunning in NORMAL mode%s\n", GREEN, RESET);
    }

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

    // Destroy semaphores
    for(int i = 0; i < N; i++) {
        sem_destroy(&forks[i]);
    }

    return 0;
}

