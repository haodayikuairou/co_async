#include <chrono>
#include <coroutine>
#include "debug.hpp"

struct PreviousAwaiter{
    std::coroutine_handle<> mPrevious;
        bool await_ready() const noexcept{return false;};                             // 是否立即就绪？true 表示不挂起
    std::coroutine_handle<> await_suspend(std::coroutine_handle<> coroutine) const noexcept{
        if(mPrevious) return mPrevious;
        else return std::noop_coroutine();
    } 
    void await_resume() const noexcept{};  
};
struct Promise
{
    auto initial_suspend(){
        return std::suspend_always();
    }
    auto final_suspend() noexcept{
        return PreviousAwaiter(mPrevious);
    }
    void unhandled_exception(){
        throw;
    }
    auto yield_value(int ret){
        mRetValue=ret;
        return std::suspend_always();
    }
    void return_void(){

    }
    std::coroutine_handle<Promise> get_return_object(){
        return std::coroutine_handle<Promise>::from_promise(*this);
    }
    int mRetValue;
    std::coroutine_handle<> mPrevious=nullptr;
};
struct Task{
    using promise_type=Promise;

    Task(std::coroutine_handle<promise_type> coroutine)
    :mCoroutine(coroutine){}

    std::coroutine_handle<promise_type> mCoroutine;
};

struct WorldTask{
    using promise_type=Promise;

    WorldTask(std::coroutine_handle<promise_type> coroutine)
    :mCoroutine(coroutine){}

    std::coroutine_handle<promise_type> mCoroutine;
    struct WorldAwaiter{
    bool await_ready() const noexcept{return false;};                             // 是否立即就绪？true 表示不挂起
    std::coroutine_handle<> await_suspend(std::coroutine_handle<> coroutine) const noexcept{
        mCoroutine.promise().mPrevious=coroutine;
        return  mCoroutine;
    } 
    void await_resume() const noexcept{};  
    std::coroutine_handle<promise_type> mCoroutine;
    };                           
    WorldTask(WorldTask &&)=delete;
    ~WorldTask(){
        mCoroutine.destroy();
    }
    auto operator co_await(){
        return WorldAwaiter(mCoroutine);
    }
};

WorldTask world(){
    debug(),"world";
    co_yield 442;
    co_yield 447; 
    co_return;
}

Task hello(){
    debug(),"hello doing worldTask";
    WorldTask worldTask=world();
    debug(),"hello done worldTask,wait world";
    co_await worldTask;
    debug(),"hello get world",worldTask.mCoroutine.promise().mRetValue; co_await worldTask;
    co_await worldTask;
    debug(),"hello get world",worldTask.mCoroutine.promise().mRetValue;
    debug(),"done world";
    debug(), "hello 42";
    co_yield 42;
    debug(), "hello 12";
    co_yield 12;
    debug(),"hello ending";
   
}

int main(){
 
    debug(),"main coming"; 
    Task t=hello();
    debug(),"main done";
    while(!t.mCoroutine.done()){
    t.mCoroutine.resume();
    debug(),"main get",t.mCoroutine.promise().mRetValue;
    }
    return 0;
}
