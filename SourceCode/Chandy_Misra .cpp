/*
Created by: vuong.tnt225540@sis.hust.edu.vn
*/
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <iomanip>

#define NUM_PHILOSOPHERS 5

using namespace std;
using namespace std::chrono;

// ANSI color codes
const string RED = "\033[31m";
const string GREEN = "\033[32m";
const string YELLOW = "\033[33m";
const string RESET = "\033[0m";

// Struct representing a fork
struct Fork {
    atomic<int> owner;  // ID of the philosopher who owns the fork
    atomic<bool> dirty; // True if the fork is dirty
    mutex lock;         // Mutex for thread safety
};

// Struct representing a philosopher
struct Philosopher {
    int id;                         // Philosopher ID
    Fork* left_fork;                // Pointer to the left fork
    Fork* right_fork;               // Pointer to the right fork
    atomic<bool> deferred[NUM_PHILOSOPHERS]; // Deferred requests for forks
    atomic<int> state;              // Current state: 0=thinking, 1=hungry, 2=eating

    Philosopher() {
        state = 0; // Initially thinking
        for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
            deferred[i] = false;
        }
    }
};

// Global arrays for philosophers and forks
vector<Philosopher> philosophers(NUM_PHILOSOPHERS);
vector<Fork> forks(NUM_PHILOSOPHERS);
mutex cout_mutex;

void print_table() {
    lock_guard<mutex> lock(cout_mutex);
    cout << left
         << GREEN << setw(15) << "Eating" << RESET
         << YELLOW << setw(15) << "Thinking" << RESET
         << RED << setw(20) << "Wait for Fork" << RESET << endl;

    for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        string eating = (philosophers[i].state == 2) ? (to_string(i)) : "";
        string thinking = (philosophers[i].state == 0) ? (to_string(i)) : "";
        string waiting = (philosophers[i].state == 1) ? (to_string(i)) : "";

        cout << GREEN << setw(15) << eating << RESET
             << YELLOW << setw(15) << thinking << RESET
             << RED << setw(20) << waiting << RESET << endl;
    }
}


void think(int id) {
    philosophers[id].state = 0; // Thinking
    print_table();
    this_thread::sleep_for(milliseconds(rand() % 1000 + 1000)); // Random sleep between 1 and 2 seconds
}

void eat(int id) {
    philosophers[id].state = 2; // Eating
    print_table();
    this_thread::sleep_for(milliseconds(rand() % 1000 + 1000)); // Random sleep between 1 and 2 seconds
}

void request_fork(Philosopher& philosopher, Fork* fork, int neighbor_id) {
    lock_guard<mutex> lock(fork->lock);

    // Check the neighbor's state
    int neighbor_state = philosophers[neighbor_id].state;

    if (neighbor_state == 0) { // Neighbor is thinking
        fork->dirty = false;
        fork->owner = philosopher.id;
    }

    if (neighbor_state == 1 && fork->dirty) { // Neighbor is hungry, but fork is clean
        fork->dirty = false;
        fork->owner = philosopher.id;
    }

    philosopher.deferred[neighbor_id] = true; // Request deferred
    

}

void handle_deferred_requests(Philosopher& philosopher) {
    for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        if (philosopher.deferred[i]) {
            Fork* fork = (i == (philosopher.id - 1 + NUM_PHILOSOPHERS) % NUM_PHILOSOPHERS) 
                            ? philosopher.left_fork 
                            : philosopher.right_fork;
            lock_guard<mutex> lock(fork->lock);
            fork->dirty = false;
            fork->owner = i;
            philosopher.deferred[i] = false;
        }
    }
}

void philosopher_routine(Philosopher& philosopher) {
    while (true) {
        think(philosopher.id);

        philosophers[philosopher.id].state = 1; // Hungry
        print_table();

        // Keep trying until both forks are acquired
        while (!(philosopher.left_fork->owner == philosopher.id && philosopher.right_fork->owner == philosopher.id)) { 
            request_fork(philosopher, philosopher.left_fork, (philosopher.id - 1 + NUM_PHILOSOPHERS) % NUM_PHILOSOPHERS);
            request_fork(philosopher, philosopher.right_fork, (philosopher.id + 1) % NUM_PHILOSOPHERS);
            this_thread::sleep_for(milliseconds(100)); // Small delay to prevent busy-waiting
        }

        eat(philosopher.id);

        philosopher.left_fork->dirty = true;
        philosopher.right_fork->dirty = true;

        handle_deferred_requests(philosopher);
    }
}

int main() {
    srand(time(nullptr)); // Seed random number generator

    // Initialize forks
    for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        forks[i].owner = i;
        forks[i].dirty = true;
    }

    // Initialize philosophers
    for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        philosophers[i].id = i;
        philosophers[i].left_fork = &forks[i];
        philosophers[i].right_fork = &forks[(i + 1) % NUM_PHILOSOPHERS];
    }

    // Create philosopher threads
    vector<thread> threads;
    for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        threads.emplace_back(philosopher_routine, ref(philosophers[i]));
    }

    // Allow the simulation to run for a fixed time
    this_thread::sleep_for(seconds(60));

    // Detach threads to finish on their own
    for (auto& thread : threads) {
        thread.detach();
    }

    cout << "Simulation ended.";
    return 0;
}
