//
// Created by jiam on 5/10/2018.
//
#include "ThreadPool1.h"
#include <random>
#include <algorithm>
#include <zconf.h>

#define K 1024
#define M 1024 * 1024
#define SIZE 100 * K //size of each part

//std::mutex mu;

void initializeV(std::vector<int>* vector, int start);

void sortV(std::vector<int>& vector, int start);

void tpQuickSort(std::vector<int>* vector, int start, int end, ThreadPool1* tp);

int main() {
    ThreadPool1 tp1;

    std::cout << "Initializing Vector......" << std::endl;
    std::vector<int> tobe_sorted(K * SIZE);

    // Initialization of the vector
    for(int i = 0; i < K * SIZE; i = i + SIZE) {
        tp1.submit(initializeV, &tobe_sorted, i);
    }
    tp1.waitAll();
    std::cout << "Initialization finish!" << std::endl;

    // Going to sort it;
    std::cout << "Sorting Vector......" << std::endl;
    auto start = std::chrono::system_clock::now();
    /*
    for(int i = 0; i < K * SIZE; i = i + SIZE) {
        // Can not pass a & into a thread;
        tp1.submit(sortV, std::ref(tobe_sorted), i);
    }
    */
    tp1.submit(tpQuickSort, &tobe_sorted, 0, tobe_sorted.size() - 1, &tp1);
    tp1.waitAll();
    auto end = std::chrono::system_clock::now();
    auto cost = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Sort time: " << cost.count() << std::endl;

    tp1.shutDown();
    std::cout << "Sort finish!" << std::endl;

    std::cout << "Checking answer......" << std::endl;
    for(unsigned int i = 0; i < K * SIZE; i++) {
        //if(i % SIZE == 0) continue;
        if(tobe_sorted[i] < tobe_sorted[i - 1]) {
            std::cout << "Sort failed at:" << i << std::endl;
            return 0;
        }
    }
    std::cout << "Sort successfully" << std::endl;
    //for(int i : tobe_sorted) std::cout<< i << std::endl;
    return 0;
}

void initializeV(std::vector<int>* vector, int start) {
    // Random engine: to generate the random number.
    std::default_random_engine e;

    // Distribute a set of random numbers uniformly.
    std::uniform_int_distribution<int> u;

    // Time duration between now and 1970.1.1
    auto curTime = std::chrono::system_clock::now().time_since_epoch();

    // Count this time duration by the given ratio (here is milliseconds)
    auto millisecondsCurTime = std::chrono::duration_cast<std::chrono::microseconds>(curTime).count();

    // Feed the engine with the seed based on the current milliseconds time.
    e.seed(millisecondsCurTime);

    // Generate the true random number.
    int end = start + SIZE;
    for (int i = start; i < end; i++) {
        (*vector)[i] = u(e);
    }
}

void sortV(std::vector<int>& vector, int start) {
    int end = start + SIZE;
    std::sort(vector.begin() + start, vector.begin() + end);
}

void tpQuickSort(std::vector<int>* vector, int start, int end, ThreadPool1* tp) {
    if(end - start <= 100) {
        std::sort(vector->begin() + start, vector->begin() + end + 1);
        return;
    }

    int pivot = (*vector)[start];
    int greater = end;
    int lessOrEquals = start + 1;
    while(greater >= lessOrEquals) {
        int val = (*vector)[lessOrEquals];
        if(val <= pivot) {
            lessOrEquals++;
        } else {
            int temp = (*vector)[greater];
            (*vector)[greater] = (*vector)[lessOrEquals];
            (*vector)[lessOrEquals] = temp;
            greater--;
        }
    }

    int temp = (*vector)[start];
    (*vector)[start] = (*vector)[greater];
    (*vector)[greater] = temp;

    tp->submit(tpQuickSort, vector, start, greater - 1, tp);
    tp->submit(tpQuickSort, vector, lessOrEquals, end, tp);
}

