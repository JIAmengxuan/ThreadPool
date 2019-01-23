//
// Created by jiam on 5/10/2018.
//
#include "ThreadPool1.h"
#include <random>
#include <algorithm>
#include <zconf.h>

#define K 1024
#define M 1024 * 1024
#define SIZE 16 * K //size of each part

// Mutex for cout
std::mutex mu;

void initializeV(std::vector<int>* vector, int start);

void sortV(std::vector<int>& vector, int start);

/*
int hello(int i) {
    std::lock_guard<std::mutex> lockGuard(mu);
    std::cout << "From " << i << "th thread " << std::this_thread::get_id() << " : hello ZION! " << std::endl;
    return i + 100;
}
*/

int main() {
    ThreadPool1 tp1(2);
    /*
    std::vector<std::future<int>> v;
    for(int i = 0; i < 77; i++) {
        std::future<int> myF = tp1.submit(hello, i);
        v.push_back(std::move(myF));
    }
    tp1.waitAll();
    for(auto& i : v) {
        std::cout<< i.get() << std::endl;
    }
    */

    std::cout << "Initializing Vector......" << std::endl;
    std::vector<int> tobe_sorted(K * SIZE);
    std::vector<int>* p = &tobe_sorted;
    //std::vector<int>& ref = tobe_sorted;
    // Initialization of the vector
    for(int i = 0; i < K; i = i + K) {
        tp1.submit(initializeV, p, i);
    }
    tp1.waitAll();
    std::cout << "Initialization finish!" << std::endl;

    // Going to sort it;
    std::cout << "Sorting Vector......" << std::endl;
    for(int i = 0; i < K; i = i + K) {
        // Can not pass a & into a thread;
        tp1.submit(sortV, std::ref(tobe_sorted), i);
    }
    tp1.waitAll();
    std::cout << "Sort finish!" << std::endl;

    std::cout << "Checking answer......" << std::endl;
    for(unsigned int i = 1; i < K * SIZE; i++) {
        if(i % SIZE == 0) continue;
        if(tobe_sorted[i] < tobe_sorted[i - 1]) {
            std::cout << "Sort failed at:" << i << std::endl;
            return 0;
        }
    }
    std::cout << "Sort successfully" << std::endl;
    return 0;
}

void initializeV(std::vector<int>* vector, int start) {
    std::default_random_engine e;
    std::uniform_int_distribution<int> u;
    int end = start + SIZE;
    for (int i = start; i < end; i++) {
        (*vector)[i] = u(e);
    }
}

void sortV(std::vector<int>& vector, int start) {
    int end = start + SIZE;
    std::sort(vector.begin() + start, vector.begin() + end);
}

