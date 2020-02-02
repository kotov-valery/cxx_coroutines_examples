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

    const char* get_returned_value() const;
private:
    coroutine_handle handle_;
};

struct resumable::promise_type {
    using coroutine_handle = std::experimental::coroutine_handle<promise_type>;

    const char* value_;

    auto get_return_object() {
        return coroutine_handle::from_promise(*this);
    }
    auto initial_suspend() { return std::experimental::suspend_always(); }
    auto final_suspend() { return std::experimental::suspend_always(); }
    void return_value(const char* value) { value_ = value; }
    void unhandled_exception() {
        std::terminate();
    }
};

const char* resumable::get_returned_value() const {
    return handle_.promise().value_;
}

resumable foo(){
    std::cout << "Hello" << std::endl;
    co_await std::experimental::suspend_always();
    co_return "Coroutine";
}

int main() {
    resumable res = foo();
    while (res.resume());
    std::cout << res.get_returned_value() << std::endl;
    return 0;
}
