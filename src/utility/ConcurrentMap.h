#ifndef CMAP_H
#define CMAP_H


#include <mutex>
#include <unordered_map>
#include <vector>
#include <limits.h>
#include <utility>

#pragma once

template <class K, class V>
class ConcurrentMap {
    private:
        std::mutex mLock;
        std::unordered_map<K,V> map;
        int maxKey; //highest integer key
    public:
        V get(K const& k);
        void erase(K const& k);
        int size();
        int getMaxKey();
        std::pair<K,V> getLowestValKeyPair();
        std::vector<V> getValues();
        

        template<class newVal>
        void set(K const& k, newVal&& v);
};

template <class K, class  V>
template<class newVal>
void ConcurrentMap<K,V>::set(K const& k, newVal&& v) {
    std::unique_lock<decltype(mLock)> lock(mLock);
    map[k] = std::forward<newVal>(v);
};

template <>
template <>
void ConcurrentMap<int,int>::set(int const& k, int&& v) {
    std::unique_lock<decltype(mLock)> lock(mLock);
    map[k] = std::forward<int>(v);

    maxKey = k; // should maintain key with highest integer as long as always insert (maxKey + 1) as key
}

template <class K, class  V>
V ConcurrentMap<K,V>::get(K const& k) {
    std::unique_lock<decltype(mLock)> lock(mLock);
    return map[k];
}

template <class K, class V>
void ConcurrentMap<K,V>::erase(K const& k) {
    std::unique_lock<decltype(mLock)> lock(mLock);
    map.erase(k);
}

template<class K, class V>
int ConcurrentMap<K,V>::size() {
    std::unique_lock<decltype(mLock)> lock(mLock);
    return map.size();
}

template<class K, class V> 
int ConcurrentMap<K,V>::getMaxKey() {
    std::unique_lock<decltype(mLock)> lock(mLock);
    if (map.size() != 0)
        return maxKey;
    else 
        return -1;
}

template<, > 
std::pair<int,int> ConcurrentMap<int,int>::getLowestValKeyPair() {
    std::unique_lock<decltype(mLock)> lock(mLock);
    int valLowest = INT_MAX;
    int keyLowest = -1;

    for (auto& it : map) {
        if (it.second < valLowest) {
            keyLowest = it.first;
            valLowest = it.second;
        }
    }

    return std::make_pair(keyLowest,valLowest); // return -1 key if map empty
}

template <class K, class V>
std::vector<V> ConcurrentMap<K,V>::getValues() {
    std::unique_lock<decltype(mLock)> lock(mLock);
    std::vector<V> temp;
   
    for (auto& it: map) {
        temp.push_back(it.second);
    }
    
    return temp;
}

#endif