#ifndef CONFIGQUEUE_H
#define CONFIGQUEUE_H

#include <mutex>
#include <queue>
#include <memory>
#include <thread>
#include <chrono>

template<typename T>
class SafeQueue
{
public:

    SafeQueue(unsigned size) : size_(size) {}
    SafeQueue() : size_(1000){}

    void setSize(unsigned size) {size_ = size;}

    std::shared_ptr<T> get()
    {
        proctor_.lock();

        std::shared_ptr<T> t = nullptr;
        while (dataQueue_.size() > 1)
        {
            dataQueue_.pop();
        }

        if (dataQueue_.size() > 0)
        {
            t = dataQueue_.front();
            dataQueue_.pop();
        }

        proctor_.unlock();

        return t;
    }

    void clear()
    {
        proctor_.lock();
        while(!dataQueue_.empty()) dataQueue_.pop();
        proctor_.unlock();
    }

    std::shared_ptr<T> getLastWhenPossible(std::atomic<bool> *fnishFlag)
    {
        std::shared_ptr<T> t = nullptr;
        bool done = false;
        while (!done && !fnishFlag->load())
        {
            proctor_.lock();
            while (dataQueue_.size() > 1)
            {
                dataQueue_.pop();
            }

            if (dataQueue_.size() > 0)
            {
                t = dataQueue_.front();
                dataQueue_.pop();
                done = true;
            }
            proctor_.unlock();
            if(!done)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }
        }
        return t;
    }

    std::shared_ptr<T> getWhenPossible(std::atomic<bool> *fnishFlag)
    {
        std::shared_ptr<T> t = nullptr;
        bool done = false;
        while (!done && !fnishFlag->load())
        {
            proctor_.lock();
            if (dataQueue_.size() > 0)
            {
                t = dataQueue_.front();
                dataQueue_.pop();
                done = true;
            }
            proctor_.unlock();
            if(!done)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }
        }
        return t;
    }

    std::shared_ptr<std::vector<std::shared_ptr<T>>> getAllWhenPossible(std::atomic<bool> *fnishFlag)
    {
        std::shared_ptr<std::vector<std::shared_ptr<T>>> t = nullptr;
        bool done = false;
        while (!done && !fnishFlag->load())
        {
            proctor_.lock();
            if (dataQueue_.size() > 0)
            {
                t = std::make_shared<std::vector<std::shared_ptr<T>>>();
                while(dataQueue_.size() > 0)
                {
                    t->push_back(dataQueue_.front());
                    dataQueue_.pop();
                }
                done = true;
            }
            proctor_.unlock();
            if(!done)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }
        }
        return t;
    }

    int push_back(const std::shared_ptr<T> &data)
    {
        proctor_.lock();
        if (dataQueue_.size() < size_)
        {
           dataQueue_.push(data);
        }
        int size = dataQueue_.size();
        proctor_.unlock();
        return size;
    }

    size_t push_backWhenPossible(const std::shared_ptr<T> &data, std::atomic<bool> *fnishFlag)
    {
        bool done = false;
        size_t size;
        while (!done  && !fnishFlag->load())
        {
            proctor_.lock();
            if (dataQueue_.size() < size_)
            {
               dataQueue_.push(data);
               done = true;
               size = dataQueue_.size();
            }
            proctor_.unlock();
            if(!done)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }
        }
        return size;
    }

    unsigned size()
    {
        unsigned out = 0;
        proctor_.lock();
        out = dataQueue_.size();
        proctor_.unlock();
        return out;
    }
private:
    unsigned size_;
    std::mutex proctor_;
    std::queue<std::shared_ptr<T>> dataQueue_;

};

template<typename T>
bool compareUnderPointers(const std::shared_ptr<T> &a, const std::shared_ptr<T> &b)
{
    return *a > *b;
}

template<typename T>
class SortingLifoQueue
{
public:

    using ErrorHandler = std::function <void(const std::string&)>;

    SortingLifoQueue(unsigned size, ErrorHandler errorHandler) : size_(size), errorHandler_(errorHandler) {}

    std::shared_ptr<T> get()
    {
        proctor_.lock();

        std::shared_ptr<T> t = nullptr;
        if (dataQueue_.size() > 0)
        {
            t = dataQueue_.back();
            dataQueue_.pop_back();
        }

        proctor_.unlock();

        return t;
    }

    std::shared_ptr<T> getWhenPossible(std::atomic<bool> *fnishFlag)
    {
        std::shared_ptr<T> t = nullptr;
        bool done = false;
        while (!done && !fnishFlag->load())
        {
            proctor_.lock();
            if (dataQueue_.size() > size_ / 2)
            {
                t = dataQueue_.back();
                dataQueue_.pop_back();
                done = true;
            }
            proctor_.unlock();

            if(!done)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }
        }
        return t;
    }

    std::shared_ptr<std::vector<std::shared_ptr<T>>> getAllWhenPossible(std::atomic<bool> *fnishFlag)
    {
        std::shared_ptr<std::vector<std::shared_ptr<T>>> t = nullptr;
        bool done = false;
        while (!done && !fnishFlag->load())
        {
            proctor_.lock();
            if (dataQueue_.size() > size_ / 2)
            {
                t = std::make_shared<std::vector<std::shared_ptr<T>>>(dataQueue_);
                dataQueue_.clear();
                done = true;
            }
            proctor_.unlock();

            if(!done)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }
        }
        return t;
    }

    void push_back(const std::shared_ptr<T> &data)
    {
        proctor_.lock();

        dataQueue_.push_back(data);
        std::sort(dataQueue_.begin(), dataQueue_.end(), compareUnderPointers<T>);

        if (dataQueue_.size() > size_)
        {
            dataQueue_.resize(size_);
            errorHandler_("Sorting Lifo Queue overload!");
        }

        proctor_.unlock();
    }

    unsigned size() {return dataQueue_.size();}

private:
    unsigned size_;
    std::mutex proctor_;
    std::vector<std::shared_ptr<T>> dataQueue_;
    ErrorHandler errorHandler_;
};



#endif // CONFIGQUEUE_H
