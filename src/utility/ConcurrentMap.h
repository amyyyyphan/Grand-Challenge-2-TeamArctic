#ifndef CMAP_H
#define CMAP_H


#include <mutex>
#include <unordered_map>

#pragma once

template <class K, class V>
class ConcurrentMap {
    private:
        std::mutex mLock;
        std::unordered_map<K,V> map;

    public:
        V get(K const& k);

        template<class newVal>
        void set(K const& k, newVal&& v);
};

template <class K, class  V>
template<class newVal>
void ConcurrentMap<K,V>::set(K const& k, newVal&& v) {
    std::unique_lock<decltype(mLock)> lock(mLock);
    map[k] = std::forward<newVal>(v);
};

template <class K, class  V>
V ConcurrentMap<K,V>::get(K const& k) {
    std::unique_lock<decltype(mLock)> lock(mLock);
    return map[k];
}

#endif