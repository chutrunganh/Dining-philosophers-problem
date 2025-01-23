/*
Created by: anh.ct225564@sis.hust.edu.vn
*/
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <array>

#define N 5
#define LEFT (i + 4) % 5  // Left neighbor
#define RIGHT (i + 1) % 5 // Right neighbor

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define RESET "\033[0m"

class Monitor {
private:
    enum State { THINKING, HUNGRY, EATING };
    std::array<State, N> state;
    std::array<std::condition_variable, N> self;
    std::mutex mtx;

    void test(int i) {
        if (state[LEFT] != EATING && 
            state[i] == HUNGRY && 
            state[RIGHT] != EATING) {
            
            state[i] = EATING;
            printf("%sPhilosopher %d granted permission to eat%s\n", GREEN, i, RESET);
            self[i].notify_one();
        }
    }

public:
    Monitor() {
        state.fill(THINKING);
    }

    void pickup(int i) {
        std::unique_lock<std::mutex> lock(mtx);
        
        state[i] = HUNGRY;
        printf("%sPhilosopher %d is hungry%s\n", YELLOW, i, RESET);
        
        test(i);
        while (state[i] != EATING) {
            printf("%sPhilosopher %d waiting for forks%s\n", RED, i, RESET);
            self[i].wait(lock);
        }
    }

    void putdown(int i) {
        std::unique_lock<std::mutex> lock(mtx);
        
        state[i] = THINKING;
        printf("Philosopher %d finished eating\n", i);
        
        // Test left and right neighbors
        test(LEFT);
        test(RIGHT);
    }
};

void philosopher(int i, Monitor& monitor) {
    while(true) {
        // Think
        printf("%sPhilosopher %d thinking%s\n", BLUE, i, RESET);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        monitor.pickup(i);

        // Eat
        printf("%sPhilosopher %d eating%s\n", GREEN, i, RESET);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        monitor.putdown(i);
    }
}

int main() {
    Monitor monitor;
    std::thread philosophers[N];

    for(int i = 0; i < N; i++) {
        philosophers[i] = std::thread(philosopher, i, std::ref(monitor));
    }

    for(int i = 0; i < N; i++) {
        philosophers[i].join();
    }

    return 0;
}