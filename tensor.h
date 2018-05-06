
#pragma once

template <typename T, long long N>
class tensor
{
public:
    template <typename... Args>
    tensor(Args... args) {
        static_assert(sizeof...(Args) == N, "Wrong number of arguments.");
        
    }
    ~tensor() {

    }

public:
    template <typename... Args>
        //typename = typename std::enable_if<are_all_convertible<T, Args...>::value>::type >> //typename std::enable_if<sizeof... (args) == N, int>::type == 0
    T get(Args... args) {
        static_assert(sizeof...(Args) == N, "Wrong number of arguments.");
        return T();
    }

private:

};
