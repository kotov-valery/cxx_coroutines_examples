#include <experimental/coroutine>
#include <future>

#include <iostream>

using namespace std::experimental;

class async_coroutine  {
public:
    struct promise_type;
    using coroutine_handle = std::experimental::coroutine_handle<promise_type>;

    async_coroutine(coroutine_handle handle): handle_(handle) { assert(handle_); }
    async_coroutine(async_coroutine&) = delete;
    async_coroutine(async_coroutine&&) = delete;

    ~async_coroutine() { handle_.destroy(); }

    int get_returned_value() const;

private:
    coroutine_handle handle_;
};

struct async_operation
{
    int result = {};

    bool await_ready() { return false; }
    void await_suspend(std::experimental::coroutine_handle<async_coroutine::promise_type> handle) {
        auto op = std::async(std::launch::async, 
                []{ 
                    std::cout << "Start async operation on a different thread\n";
                    return 42;
                });
        std::cout << "Wait for the operation to complete\n";
        op.wait(); // <-- here we block main thread...
        std::cout << "Async operation is done\n";
        result = op.get();
        handle.resume();
    }
    auto await_resume() { return result; }
};

struct async_coroutine::promise_type {
    using coroutine_handle = std::experimental::coroutine_handle<promise_type>;

    int value_;

    auto get_return_object() {
        return coroutine_handle::from_promise(*this);
    }
    auto initial_suspend() { return std::experimental::suspend_never(); }
    auto final_suspend() { return std::experimental::suspend_always(); }
    void return_value(int value) { value_ = value; }
    void unhandled_exception() { std::terminate(); }
};

int async_coroutine::get_returned_value() const {
    return handle_.promise().value_;
}

async_coroutine invoke_async_operation() {
    std::cout << "Start async operation\n";
    const auto result = co_await async_operation();
    std::cout << "Async operation completed. " <<
        "Got result: " << result << '\n';
    co_return result;
}

int main() {
    std::cout << "Invoke async operation on main thread\n";
    async_coroutine op = invoke_async_operation();
    std::cout << "Get the result on main thread: " << op.get_returned_value() << "\n";
    return 0;
}
