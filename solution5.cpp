#include <iostream>
#include <thread>
#include <mutex>
#include <semaphore.h>
#include <vector>

#define N 5
#define LEFT i
#define RIGHT (i + 1) % N

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define RESET "\033[0m"

class SimultaneousSemaphore {
private:
    std::vector<sem_t*> sems;
    std::mutex mtx;
    std::vector<bool> acquired;

public:
    SimultaneousSemaphore(int n) {
        sems.resize(n);
        acquired.resize(n, false);
        for(int i = 0; i < n; i++) {
            
            sems[i] = new sem_t;
            sem_init(sems[i], 0, 1);
        }
    }

    // Try to acquire all semaphores simultaneously
    bool wait_sim(const std::vector<int>& indices) {
        std::lock_guard<std::mutex> lock(mtx);
        
        // Try to acquire all without blocking
        for(int idx : indices) {
            if(sem_trywait(sems[idx]) != 0) {
                // Release any acquired semaphores
                for(int j = 0; j < idx; j++) {
                    if(acquired[indices[j]]) {
                        sem_post(sems[indices[j]]);
                        acquired[indices[j]] = false;
                    }
                }
                return false;
            }
            acquired[idx] = true;
        }
        return true;
    }

    // Release all semaphores
    void signal_sim(const std::vector<int>& indices) {
        std::lock_guard<std::mutex> lock(mtx);
        for(int idx : indices) {
            if(acquired[idx]) {
                sem_post(sems[idx]);
                acquired[idx] = false;
            }
        }
    }

    ~SimultaneousSemaphore() {
        for(auto sem : sems) {
            sem_destroy(sem);
            delete sem;
        }
    }
};

void philosopher(int i, SimultaneousSemaphore& simSem) {
    std::vector<int> my_forks = {LEFT, RIGHT};
    
    while(true) {
        // Think
        printf("%sPhilosopher %d thinking%s\n", GREEN, i, RESET);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        printf("%sPhilosopher %d hungry%s\n", YELLOW, i, RESET);
        
        // Try to get both forks
        while(!simSem.wait_sim(my_forks)) {
            printf("%sPhilosopher %d waiting for forks%s\n", RED, i, RESET);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // Eating
        printf("%sPhilosopher %d eating%s\n", GREEN, i, RESET);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // Release forks
        simSem.signal_sim(my_forks);
        printf("Philosopher %d finished eating\n", i);
    }
}

int main() {
    SimultaneousSemaphore simSem(N);
    std::thread philosophers[N];

    for(int i = 0; i < N; i++) {
        philosophers[i] = std::thread(philosopher, i, std::ref(simSem));
    }

    for(int i = 0; i < N; i++) {
        philosophers[i].join();
    }

    return 0;
}

/*
### Advantages:

1. **Deadlock Prevention**
- Atomic acquisition of both forks
- All-or-nothing approach prevents partial resource holding
- No circular wait condition possible

2. **Fairness**
```cpp
bool acquireAll(const std::vector<int>& indices) {
    // Either gets all resources or none
    // No philosopher can hold resources partially
```

3. **Performance**
- No central bottleneck (unlike mutex solution)
- Better concurrency than monitor solution
- Philosophers can eat simultaneously if they don't compete for same forks

4. **Resource Efficiency**
- No wasted holding of single fork
- Resources released immediately if full acquisition fails
```cpp
if(sem_trywait(sems[idx]) != 0) {
    // Release any acquired semaphores immediately
    for(int j = 0; j < idx; j++) {
        if(acquired[indices[j]]) {
            sem_post(sems[indices[j]]);
```

5. **Implementation Benefits**
- Simple state management
- Clear resource ownership
- Easy to extend for more resources
- No complex state tracking needed

### Disadvantages:
1. Potential for starvation
2. Busy waiting in while loop
3. Higher memory usage for tracking acquired state
4. May have higher contention under heavy load

*/