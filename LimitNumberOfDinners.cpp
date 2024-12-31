#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <atomic>
#include <iomanip>
#include <random>

constexpr int NUM_PHILOSOPHERS = 5;

#define COLOR_RESET "\033[0m"
#define COLOR_THINKING "\033[36m"
#define COLOR_WAITING "\033[33m"
#define COLOR_EATING "\033[32m"

class CountingSemaphore {
private:
    std::mutex mtx;
    std::condition_variable cv;
    int count;

public:
    explicit CountingSemaphore(int initial_count) : count(initial_count) {}

    void acquire() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return count > 0; });
        --count;
    }

    void release() {
        std::lock_guard<std::mutex> lock(mtx);
        ++count;
        cv.notify_one();
    }
};

std::mutex forks[NUM_PHILOSOPHERS];
CountingSemaphore wait_to_sit(NUM_PHILOSOPHERS - 1);
std::mutex cout_mutex;
std::atomic<int> philosopher_states[NUM_PHILOSOPHERS];

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> dis(1000, 5000);

void print_table() {
    std::lock_guard<std::mutex> lock(cout_mutex);
    std::cout << "\033[H\033[2J";
    std::cout << std::left << std::setw(15) << "Eating" << std::setw(15) << "Thinking" << std::setw(20) << "Wait to Sit" << std::setw(20) << "Wait for Fork" << std::endl;
    for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        std::string eating = (philosopher_states[i] == 3) ? (COLOR_EATING + std::to_string(i) + COLOR_RESET) : "";
        std::string thinking = (philosopher_states[i] == 0) ? (COLOR_THINKING + std::to_string(i) + COLOR_RESET) : "";
        std::string wait_to_sit = (philosopher_states[i] == 1) ? (COLOR_WAITING + std::to_string(i) + COLOR_RESET) : "";
        std::string wait_for_fork = (philosopher_states[i] == 2) ? (COLOR_WAITING + std::to_string(i) + COLOR_RESET) : "";
        std::cout << std::setw(15) << eating << std::setw(15) << thinking << std::setw(20) << wait_to_sit << std::setw(20) << wait_for_fork << std::endl;
    }
}

void philosopher(int id) {
    while (true) {
        philosopher_states[id] = 0;
        print_table();
        std::this_thread::sleep_for(std::chrono::milliseconds(dis(gen)));

        philosopher_states[id] = 1;
        print_table();
        wait_to_sit.acquire();

        philosopher_states[id] = 2;
        print_table();
        std::lock_guard<std::mutex> left_fork(forks[id]);
        std::lock_guard<std::mutex> right_fork(forks[(id + 1) % NUM_PHILOSOPHERS]);

        philosopher_states[id] = 3;
        print_table();
        std::this_thread::sleep_for(std::chrono::milliseconds(dis(gen)));

        wait_to_sit.release();
        philosopher_states[id] = 0;
        print_table();
    }
}

int main() {
    std::fill(std::begin(philosopher_states), std::end(philosopher_states), 0);
    print_table();

    std::vector<std::thread> philosophers;

    for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        philosophers.emplace_back(philosopher, i);
    }

    for (auto& t : philosophers) {
        t.join();
    }

    return 0;
}
