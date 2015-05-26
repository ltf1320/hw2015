#ifndef THREADSAFEQUEUE_H
#define THREADSAFEQUEUE_H
#include <queue>
#include <mutex>
template<typename T>
class ThreadSafeQueue
{
    public:
        ThreadSafeQueue();
        virtual ~ThreadSafeQueue();
        void push(T t);
        void pop();
        std::pair<bool,T> popFront();
        T front();
        int size();
    protected:
    std::queue<T> que;
    std::mutex mtx;
    private:
};

template<typename T>
void ThreadSafeQueue<T>::push(T t)
{
    std::lock_guard<std::mutex> lck(mtx);
    que.push(t);
}
template<typename T>
void ThreadSafeQueue<T>::pop()
{
    std::lock_guard<std::mutex> lck(mtx);
    que.pop();
}
template<typename T>
T ThreadSafeQueue<T>::front()
{
    std::lock_guard<std::mutex> lck(mtx);
    return que.front();
}
template<typename T>
int ThreadSafeQueue<T>::size()
{
        std::lock_guard<std::mutex> lck(mtx);
        return que.size();
}
template<typename T>
ThreadSafeQueue<T>::ThreadSafeQueue()
{
    //ctor
}

template<typename T>
ThreadSafeQueue<T>::~ThreadSafeQueue()
{
    //dtor
}
template<typename T>
std::pair<bool,T > ThreadSafeQueue<T>::popFront()
{
    std::lock_guard<std::mutex> lck(mtx);
    if(que.size()==0)
    {
        return std::make_pair(false,(T)NULL);
    }
    auto p=std::make_pair(true,que.front());
    que.pop();
    return p;
}

#endif // THREADSAFEQUEUE_H
