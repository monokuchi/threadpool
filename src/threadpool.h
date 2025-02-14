
#pragma once

#include <thread>
#include <mutex>
#include <vector>
#include <queue>
#include <future>
#include <functional>
#include <condition_variable>



template<typename R, typename... ArgsTypes>
class ThreadPool
{
   public:
       ThreadPool(int32_t num_workers) : m_stop(false)
       {
            // Pre-allocate vector
            m_workers.reserve(std::thread::hardware_concurrency());
            
            // Spawn threads
            add_workers(num_workers);
       }

       ~ThreadPool()
       {
            // Set "m_stop" to true which will indicate to the running threads to stop their execution
            m_stop = true;
            // Notify all threads that "m_stop" has changed
            m_cond_var.notify_all();
            // Join all the threads
            for (auto& thread : m_workers)
            {
                if (thread.joinable())
                {
                    thread.join();
                }
            }
       }

       /*
       Spawn "num_workers" amount of workers in the thread pool
       */
       void add_workers(int32_t num_workers)
       {
            // No workers to add, return
            if (num_workers == 0) { return; }
            // Catch negative inputs
            if (num_workers < 0) { throw std::invalid_argument("Negative num_workers not supported!"); }
            const size_t u_num_workers = static_cast<size_t>(num_workers);

            const size_t total_threads_requested = m_workers.size() + u_num_workers;
            const size_t total_threads_supported = std::thread::hardware_concurrency();

            if (total_threads_requested <= total_threads_supported)
            {
                // Add threads
                for (size_t i=0; i<u_num_workers; ++i)
                {
                    auto thread_function = [this]()
                    {
                        while (true)
                        {
                            // Generate lock for m_mutex
                            std::unique_lock<std::mutex> lock(m_mutex);
                            // Conditionally blocks the thread until we received a "m_stop" or a task
                            m_cond_var.wait(lock, [this]() { return m_stop || !m_tasks.empty(); });
                            // Terminate the thread of execution if "m_stop" was set and if we ran out of tasks to do in the queue
                            if (m_stop && m_tasks.empty()) { return; }
                            // Retrieve and pop off the task from the queue
                            auto task = std::move(m_tasks.front());
                            m_tasks.pop();
                            // Done with the queue now can free it for other threads to access
                            lock.unlock();
                            // Have the thread run the task
                            task();
                        }
                    };

                    // Construct a thread at the back of the vector that runs thread_function()
                    m_workers.emplace_back(thread_function);
                }
            }
            else
            {
                // Requested more threads then hardware can handle
                const std::string error_msg("Requested a total of " + std::to_string(total_threads_requested)
                + " threads while current hardware can only handle up to " + std::to_string(total_threads_supported) + " threads!");
                throw std::invalid_argument(error_msg);
            }
       }

       /*
       Adds a task to the task queue
       */
       template<typename F>
       std::future<R> add_task(F&& func, ArgsTypes&&... args)
       {
            // Wrap the function with it's arguments into a lambda
            auto func_with_args = [&func, &args...]() { return func(args...); };
            // Put the lambda into a std::packaged_task
            std::packaged_task<R()> task(func_with_args);
            // Get the future associated with the packaged_task we just retrieved
            std::future<R> result = task.get_future();

            // Generate lock for m_mutex
            std::unique_lock<std::mutex> lock(m_mutex);
            // Put the task/args pair into the tasks queue
            m_tasks.push(std::move(task));
            // Done with the queue now can free it for other threads to access
            lock.unlock();
            // Notify one of the threads that a task is in the queue and ready to be worked on
            m_cond_var.notify_one();

            return result;
       }

       /*
       Returns the number of workers currently in the thread pool
       */
       size_t workers_size() { return m_workers.size(); }

       /*
       Returns the number of tasks currently in the queue
       */
       size_t tasks_size() { return m_tasks.size(); }


    private:
       std::vector<std::thread> m_workers;
       std::queue<std::packaged_task<R()>> m_tasks;
       std::mutex m_mutex;
       std::condition_variable m_cond_var;
       std::atomic<bool> m_stop;
};

