#include <experimental/coroutine>
#include <iostream>
#include <cassert>

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
private:
    coroutine_handle handle_;
};

struct resumable::promise_type {
    using coroutine_handle = std::experimental::coroutine_handle<promise_type>;
    auto get_return_object() {
        return coroutine_handle::from_promise(*this);
    }
    auto initial_suspend() { return std::experimental::suspend_always(); }
    auto final_suspend() { return std::experimental::suspend_always(); }
    void return_void() {}
    void unhandled_exception() {
        std::terminate();
    }
};

resumable foo(){
    std::cout << "Hello" << std::endl;
    co_await std::experimental::suspend_always();
    std::cout << "World" << std::endl;
}

int main() {
    resumable res = foo();
    while (res.resume());
    return 0;
}
