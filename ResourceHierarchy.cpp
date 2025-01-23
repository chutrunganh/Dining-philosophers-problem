/*
Created by: anh.ct225564@sis.hust.edu.vn
*/
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <chrono>
#include <memory>

// ANSI color codes
#define COLOR_RESET   "\033[0m"
#define COLOR_THINKING "\033[36m"  // Cyan
#define COLOR_EATING  "\033[32m"   // Green
#define COLOR_WAITING "\033[33m"   // Yellow

class Chopstick {
public:
    int id;
    std::mutex mutex;
    Chopstick(int id) : id(id) {}
};

class Philosopher {
private:
    int id;
    std::shared_ptr<Chopstick> first;
    std::shared_ptr<Chopstick> second;
    static std::mutex cout_mutex;

    void think() {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << COLOR_THINKING "Philosopher " << id << " is thinking" COLOR_RESET << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    void eat() {
        {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << COLOR_WAITING "Philosopher " << id << " is waiting for forks" COLOR_RESET << std::endl;
        }

        std::lock_guard<std::mutex> lock1(first->mutex);
        std::lock_guard<std::mutex> lock2(second->mutex);

        {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << COLOR_EATING "Philosopher " << id << " is eating" COLOR_RESET << std::endl;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

public:
    Philosopher(int id, std::shared_ptr<Chopstick> left, std::shared_ptr<Chopstick> right)
        : id(id), 
          first(left->id < right->id ? left : right),
          second(left->id < right->id ? right : left) {}

    void dine() {
        while (true) {
            think();
            eat();
        }
    }
};

std::mutex Philosopher::cout_mutex;

// Main function remains the same

int main() {
    const int NUM_PHILOSOPHERS = 5;
    std::vector<std::shared_ptr<Chopstick>> chopsticks;
    std::vector<std::unique_ptr<Philosopher>> philosophers;
    std::vector<std::thread> threads;

    // Create chopsticks
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        chopsticks.push_back(std::make_shared<Chopstick>(i));
    }

    // Create philosophers
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        philosophers.push_back(
            std::make_unique<Philosopher>(
                i,
                chopsticks[i],
                chopsticks[(i + 1) % NUM_PHILOSOPHERS]
            )
        );
    }

    // Start dining
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        threads.emplace_back(&Philosopher::dine, philosophers[i].get());
    }

    // Join threads
    for (auto& thread : threads) {
        thread.join();
    }

    return 0;
}