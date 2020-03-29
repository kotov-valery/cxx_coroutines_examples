#include <experimental/coroutine>
#include <future>

#include <mutex>
#include <condition_variable>

#include <chrono>

#include <iostream>

using namespace std::experimental;

bool ready_ = false;
std::mutex mutex_ = {};
std::condition_variable condition_ = {};

class async_coroutine  {
public:
    struct promise_type;
    using coroutine_handle = std::experimental::coroutine_handle<promise_type>;

    async_coroutine(coroutine_handle handle): handle_(handle) { assert(handle_); }
    async_coroutine(async_coroutine&) = delete;
    async_coroutine(async_coroutine&&) = delete;

    ~async_coroutine() { handle_.destroy(); }

    int get_returned_value() const;

    void wait_for_async_op_on_main_thread() {
        std::unique_lock<std::mutex> lock(mutex_);
        condition_.wait(lock, [] { return ready_; });
    }

private:
    coroutine_handle handle_;
};

struct async_operation
{
    std::promise<int> promise_ = {};
    std::future<int> operation_ = {};
    std::experimental::coroutine_handle<async_coroutine::promise_type> handle_ = {};

    bool await_ready() { return false; }
    void await_suspend(std::experimental::coroutine_handle<async_coroutine::promise_type> handle) {
        std::cout << "\tSuspend the coroutine on thread " << std::this_thread::get_id() << "\n"
                  << "\tWich means:\n" <<
                        "\t *start an async operation on a different thread\n" <<
                        "\t *create another thread to wait for its' completeion\n";

        handle_ = handle;
        operation_ = promise_.get_future();

        // Start heavy async operation on a separate thread
        std::thread( [this] {
            std::cout << "\tStart work on a separate thread " <<
                std::this_thread::get_id() << "\n";
            std::this_thread::sleep_for(std::chrono::seconds(2));
            std::cout << "\tWork is done!\n";
            promise_.set_value_at_thread_exit(42);
        }).detach();

        // Create another thread to wait for a result of async operation
        std::thread( [this] {
            std::cout << "\tWait for async operation to complete on a separate thread "
                << std::this_thread::get_id() << "\n";
            operation_.wait();
            {
                std::lock_guard<std::mutex> lock(mutex_);
                ready_ = true;
            }
            std::cout << "\tOperation is complete, notify main thread\n";
            handle_.resume();
            condition_.notify_one();
        }).detach();
    }
    auto await_resume() {
        std::cout << "\tResume coroutine on thread " <<
            std::this_thread::get_id() << "\n";
        return operation_.get();
    }
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
    std::cout << "Invoke async operation on main thread " <<
        std::this_thread::get_id() << "\n";
    async_coroutine op = invoke_async_operation();

    for (int i=0; i<10; i++) {
        std::cout << "Main thread should not be blocked and can do something\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    std::cout << "Wait for the result on main thread\n" << std::endl;
    op.wait_for_async_op_on_main_thread();

    std::cout << "Get the result on main thread: " << op.get_returned_value() << "\n";
    return 0;
}
