
#include <iostream>
#include <chrono>

#include "threadpool.h"



float dummy_task(int x)
{
    std::cout << "Thread ID: " << std::this_thread::get_id() << " executing!" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));

    return x+200;
}




int main()
{
    ThreadPool<float, int> tp(2);
    std::cout << "Number of Workers: " << tp.workers_size() << std::endl;
    tp.add_workers(18);
    std::cout << "Number of Workers: " << tp.workers_size() << std::endl;



    constexpr size_t num_tasks = 100;
    std::vector<std::future<float>> future_results;
    future_results.reserve(num_tasks);

    std::cout << "Number of Tasks: " << tp.tasks_size() << std::endl;
    for (size_t i=0; i<num_tasks; ++i)
    {
        future_results.push_back(tp.add_task(dummy_task, 100));
    }
    std::cout << "Number of Tasks: " << tp.tasks_size() << std::endl;



    for (size_t i=0; i<num_tasks; ++i)
    {
        if (future_results[i].valid())
        {
            std::cout << "Result of task: " << future_results[i].get() << std::endl;
        }
        else
        {
            std::cout << "Future not valid!" << std::endl;
        }
    }

}

