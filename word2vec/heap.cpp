
#include "stdafx.h"
#include "heap.h"


heap::heap(int nmax) {
    int num = 1;
    while (num < nmax) num <<= 1;
    _heap = new heap_data[num + 1];
    _nmax = num;
    _count = 0;
}
heap::~heap() {
    delete[] _heap;
}

void heap::add(const heap_data &item) {
    if (_count == _nmax)
        throw std::runtime_error(u8"heap: add: cannot exceed maximum number limit");
    int idx_new = ++_count;
    _heap[idx_new] = item;
    this->bubble_up(idx_new);
}
heap_data heap::extract_min() {
    if (!_count) return { -1, 0 };
    heap_data top = _heap[1];
    if (_count > 1) {
        _heap[1] = _heap[_count--];
        this->bubble_down(1);
    }
    else {
        _count = 0;
    }
    return top;
}

int heap::count() const {
    return _count;
}

int heap::min_id() const {
    if (!_count) return -1;
    return _heap[1].id;
}
int heap::min_num() const {
    return _heap[1].num;
}

void heap::bubble_up(int idx) {
    while (idx > 1) {
        int idx_par = idx >> 1;
        if (_heap[idx_par].num <= _heap[idx].num) break; // the heap is correct now
        heap_data tmp = _heap[idx];
        _heap[idx] = _heap[idx_par];
        _heap[idx_par] = tmp;
        idx = idx_par;
    }
}
void heap::bubble_down(int idx) {
    while (2 * idx <= _count) {
        int idx_min = idx;
        int idx_left = 2 * idx;
        int idx_right = idx_left + 1;
        if (_heap[idx_left].num < _heap[idx_min].num) idx_min = idx_left;
        if (idx_right <= _count && _heap[idx_right].num < _heap[idx_min].num) idx_min = idx_right;
        if (idx_min == idx) break; // the heap is correct now
        heap_data tmp = _heap[idx];
        _heap[idx] = _heap[idx_min];
        _heap[idx_min] = tmp;
        idx = idx_min;
    }
}
