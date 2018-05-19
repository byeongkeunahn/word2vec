
#pragma once

typedef struct {
    int id;
    int num;
} heap_data;

class heap
{
public:
    heap(int nmax);
    ~heap();
    void add(const heap_data &item);
    heap_data extract_min();
    int count() const;
    int min_id() const;
    int min_num() const;

private:
    void bubble_up(int idx);
    void bubble_down(int idx);

private:
    heap_data *_heap;
    int _nmax;
    int _count;
};

