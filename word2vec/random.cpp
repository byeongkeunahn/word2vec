
#include "stdafx.h"
#include "random.h"

void rand_gen(int range_from, int range_to, int count) {
    std::random_device                  rand_dev;
    std::mt19937                        generator(rand_dev());
    std::uniform_int_distribution<int>  distr(range_from, range_to);
}
