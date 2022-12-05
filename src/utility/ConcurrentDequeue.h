#ifndef CQUE_H
#define CQUE_H


#include <mutex>
#include <deque>
#include <limits.h>
#include <utility>

#pragma once

//std::deque<std::pair<MPI_Status, int>> requests;
/*
    ref: https://stackoverflow.com/questions/19143228/creating-a-standard-map-that-is-thread-safe
*/
template<class T>
class ConcurrentQueue {
private:
    std::mutex mut;
    std::deque<T> data;
    std::condition_variable condition;

public:

    void Push(T node){
        std::lock_guard<std::mutex> lk(mut);
        data.push_back(node);
        condition.notify_one(); // wakes up one thread if many trying to use data
    }

    void Get(T& node){
        std::unique_lock<std::mutex> lk(mut);
        condition.wait(lk, [this]{ return !data.empty(); }); // in this case it waits if queue is empty, if not needed  you can remove this line
        node = data.front();
        data.pop_front();
        lk.unlock();
    }

    int size() {
        std::lock_guard<std::mutex> lk(mut);
        return data.size();
    }
};


#endif