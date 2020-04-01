#include <experimental/coroutine>

#include <iostream>
#include <sstream>

#include <future>

#include <mutex>
#include <condition_variable>

#include <chrono>
#include <optional>

//
// Parser is from boost/context/cc exmple on inverting
// the control flow: https://www.boost.org/doc/libs/1_72_0/libs/context/doc/html/context/cc.html
//

/*
 * Grammar:
 *   P ---> E '\0'
 *   E ---> T {('+'|'-') T}
 *   T ---> S {('*'|'/') S}
 *   S ---> digit | '(' E ')'
 */
class Parser {
    char next;
    std::istream& is;
    std::function<void(char)> cb;

    char pull() {
        return std::char_traits<char>::to_char_type(is.get());
    }

    void scan() {
        do { next = pull(); }
        while(isspace(next));
    }

public:
    Parser(std::istream& is_,std::function<void(char)> cb_) :
        next(), is(is_), cb(cb_)
    {}

    void run() {
        scan();
        E();
    }

private:
    void E(){
        T();
        while (next=='+'||next=='-') {
            cb(next);
            scan();
            T();
        }
    }

    void T() {
        S();
        while (next=='*'||next=='/') {
            cb(next);
            scan();
            S();
        }
    }

    void S() {
        if (isdigit(next)) {
            cb(next);
            scan();
        } else if(next=='(') {
            cb(next);
            scan();
            E();
            if (next==')') {
                cb(next);
                scan();
            } else {
                throw std::runtime_error("parsing failed");
            }
        } else {
            throw std::runtime_error("parsing failed");
        }
    }
};

class parser_coroutine {
public:
    struct promise_type;
    using coroutine_handle = std::experimental::coroutine_handle<promise_type>;

    parser_coroutine(coroutine_handle handle) : handle_(handle) { assert(handle_); }

    parser_coroutine(parser_coroutine&) = delete;
    parser_coroutine(parser_coroutine&&) = delete;

    ~parser_coroutine() { handle_.destroy(); }

    bool resume() {
        if (not handle_.done())
            handle_.resume();
        return not handle_.done();
    }

    char recent_value() const;
private:
    coroutine_handle handle_;
};

struct parser_coroutine::promise_type {
    using coroutine_handle = std::experimental::coroutine_handle<promise_type>;

    char character_;

    auto get_return_object() {
        return coroutine_handle::from_promise(*this);
    }

    auto initial_suspend() { return std::experimental::suspend_always(); }
    auto final_suspend() { return std::experimental::suspend_always(); }
    void return_void() {}

    auto yield_value(char ch) {
        character_ = ch;
        return std::experimental::suspend_always();
    }

    void unhandled_exception() {
        std::terminate();
    }
};

char parser_coroutine::recent_value() const {
    return handle_.promise().character_;
}

struct fetch_token {
    using coroutine_handle = parser_coroutine::coroutine_handle;

    struct state {
        std::mutex mutex;
        std::condition_variable condition;
        bool ready;
        std::optional<char> value;
    };

    state& state_;

    fetch_token(state& s) : state_(s) {}

    bool await_ready() {
        std::unique_lock<std::mutex> lock{state_.mutex};
        return state_.ready;
    }

    void await_suspend(coroutine_handle handle) {
        {
            std::unique_lock<std::mutex> lock{state_.mutex};
            state_.ready = false;
            state_.condition.wait(lock, [this] { return state_.ready; });
        }

        handle.resume();
    }

    auto await_resume() {
        std::unique_lock<std::mutex> lock{state_.mutex};
        state_.ready = false;
        return state_.value;
    }
};

parser_coroutine parse_string(const std::string& str) {
    std::cout << "[M] Invoke parse string coroutine with \"" << str << "\"\n";
    fetch_token::state fetch_state = {};

    struct parser_state {
        std::mutex mutex = {};
        std::condition_variable condition = {};
        bool ready = false;
    } parser_state = {};

    std::thread( [&str, &fetch_state, &parser_state]() {
        std::cout << "[P] Start async parser on a separate thread\n";
        std::istringstream is(str);
        Parser parser(is,
            [&fetch_state, &parser_state](char ch) -> void {
                std::cout << "[P] Parser parsed next token " << ch << ". Notify awaiting main thread.\n";
                {
                    std::unique_lock<std::mutex> lock(fetch_state.mutex);
                    fetch_state.ready = true;
                    fetch_state.value = ch;
                }
                fetch_state.condition.notify_one();

                std::cout << "[P] Block parser thread until token is consumed\n";
                std::unique_lock<std::mutex> lock{parser_state.mutex};
                parser_state.ready = false;
                parser_state.condition.wait(lock, [&parser_state] { return parser_state.ready; });
            }
        );
        parser.run();

        std::cout << "[P] Parsing is done, notify main thread with empty optional.\n";
        std::unique_lock<std::mutex> lock(fetch_state.mutex);
        fetch_state.ready = true;
        fetch_state.value.reset();
        fetch_state.condition.notify_one();
    }).detach();

    while (true) {
        std::cout << "[M] Co await on the next token. Block main thread while waiting for another token.\n";
        auto token = co_await fetch_token(fetch_state);
        if (!token.has_value())
            break;

        std::cout << "[M] Got next token [" << token.value() << "]. Yielding it back to main()\n";
        co_yield token.value();

        std::unique_lock<std::mutex> lock{parser_state.mutex};
        parser_state.ready = true;
        parser_state.condition.notify_one();
    }
}

int main() {
    parser_coroutine res = parse_string("1+1");
    while (res.resume()) {
        std::cerr << "[M] Parsed: " << res.recent_value() << '\n';
    }
    return 0;
}
