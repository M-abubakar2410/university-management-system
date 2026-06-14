#pragma once
#include <cstring>
#include <cstdlib>

// ─────────────────────────────────────────────────────────────────────────────
//  MyArray<T>  —  heap-managed dynamic array, no STL
// ─────────────────────────────────────────────────────────────────────────────
template<typename T>
class MyArray {
    T*  data;
    int sz;
    int cap;

    void grow() {
        int newCap = (cap==0) ? 4 : cap*2;
        T* newData = new T[newCap];
        for(int i=0;i<sz;i++) newData[i] = data[i];
        delete[] data;
        data = newData;
        cap  = newCap;
    }
public:
    MyArray()  : data(nullptr), sz(0), cap(0) {}
    ~MyArray() { delete[] data; }

    // copy
    MyArray(const MyArray& o) : data(nullptr), sz(0), cap(0) {
        for(int i=0;i<o.sz;i++) push(o.data[i]);
    }
    MyArray& operator=(const MyArray& o) {
        if(this!=&o){ clear(); for(int i=0;i<o.sz;i++) push(o.data[i]); }
        return *this;
    }

    void push(const T& v) {
        if(sz==cap) grow();
        data[sz++] = v;
    }
    void pop() { if(sz>0) sz--; }

    // remove by index
    void removeAt(int idx) {
        if(idx<0||idx>=sz) return;
        for(int i=idx;i<sz-1;i++) data[i]=data[i+1];
        sz--;
    }

    T& operator[](int i)       { return data[i]; }
    const T& operator[](int i) const { return data[i]; }

    int size()  const { return sz; }
    bool empty() const { return sz==0; }

    void clear() { sz=0; }

    // find index where predicate is true, else -1
    template<typename Pred>
    int findIf(Pred p) const {
        for(int i=0;i<sz;i++) if(p(data[i])) return i;
        return -1;
    }
};
