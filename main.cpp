//
// Created by jiam on 5/10/2018.
//
#include "ThreadPool1.h"
#include <random>
#include <algorithm>

#define K 1024
#define M 1024 * 1024
#define SIZE 16 * K //size of each part

// Mutex for cout
std::mutex mu;
static std::atomic_uint gstart(0);

void initializeV(std::vector<int>& vector);

int main() {
    std::unique_ptr<ThreadPool1> uniquePtr(new ThreadPool1(4));
    /*
    for(int i = 0; i < 77; i++) {
        uniquePtr->submit([i] {
            std::lock_guard<std::mutex> lockGuard(mu);
            std::cout << "From "<< i <<"th thread " << std::this_thread::get_id() << " : hello Chenhang Jiao! " << std::endl;
        });
    }
     */
    std::vector<int> tobe_sorted(K * SIZE);
    // Initialization of the vector
    initializeV(tobe_sorted);

    // Going to sort it;
    for(int i = 0; i < K; i++) {
        uniquePtr->submit([&tobe_sorted] {
            int start = gstart.fetch_add(SIZE);
            int end = start + SIZE;
            std::sort(tobe_sorted.begin() + start, tobe_sorted.begin() + end);
        });
    }
    std::cout << "Sort finished" << std::endl;
    uniquePtr.reset(nullptr);

    std::cout << "check answer:" << std::endl;
    for(unsigned int i = 1; i < K * SIZE; i++) {
        //if(i % SIZE == 0) continue;
        if(tobe_sorted[i] < tobe_sorted[i - 1]) {
            std::cout << "sort failed at:" << i << std::endl;
            return 0;
        }
    }
    std::cout << "sort successfully" << std::endl;
    return 0;
}

void initializeV(std::vector<int>& vector) {
    std::default_random_engine e;
    std::uniform_int_distribution<int> u;
    for (int i = 0; i < vector.size(); i++) {
        vector[i] = u(e);
    }
    std::cout << "Initialization finished" << std::endl;
}

