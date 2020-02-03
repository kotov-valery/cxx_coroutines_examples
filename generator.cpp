#include <experimental/coroutine>
#include <iostream>

class resumable {
public:
    struct promise_type;
    using coroutine_handle = std::experimental::coroutine_handle<promise_type>;

    resumable(coroutine_handle handle) : handle_(handle) { assert(handle_); }
    resumable(resumable&) = delete;
    resumable(resumable&&) = delete;

    ~resumable() { handle_.destroy(); }

    bool resume() {
        if (not handle_.done())
            handle_.resume();
        return not handle_.done();
    }

    const char* recent_value() const;
private:
    coroutine_handle handle_;
};

struct resumable::promise_type {
    using coroutine_handle = std::experimental::coroutine_handle<promise_type>;

    const char* string_ = nullptr;

    auto get_return_object() {
        return coroutine_handle::from_promise(*this);
    }
    auto initial_suspend() { return std::experimental::suspend_always(); }
    auto final_suspend() { return std::experimental::suspend_always(); }
    void return_void() {}
    auto yield_value(const char* string) {
        string_ = string;
        return std::experimental::suspend_always();
    }
    void unhandled_exception() {
        std::terminate();
    }
};


const char* resumable::recent_value() const {
    return handle_.promise().string_;
}

resumable foo() {
    while (true) {
        co_yield "Hello";
        co_yield "Coroutine";
    }
}

int main() {
    resumable res = foo();
    for (int i=0; i<10; ++i) {
        res.resume();
        std::cout << res.recent_value() << std::endl;
    }
    return 0;
}
