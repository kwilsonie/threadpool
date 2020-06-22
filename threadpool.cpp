#include <condition_variable>
#include <functional>
#include <vector>
#include <thread>
#include <queue>
#include <iostream>

using namespace std;


void DrawTask()
{
   int number = (rand() % 10) + 1;
   for (int i = 0; i < 10; i++) { 
       cout << number << " Draw Thread: " << i <<"\n";
   }
}

void DataTask()
{
   int number = (rand() % 10) + 1;
   for (int i = 0; i < 10; i++) { 
       cout << number << " Data Thread: " << i <<"\n";
   }
}

void MemTask()
{
   int number = (rand() % 10) + 1;
   for (int i = 0; i < 10; i++) { 
       cout << number << " Mem Thread: " << i <<"\n";
   }
}

class ThreadPool
{
  public:
  ThreadPool()
  {
    start(thread::hardware_concurrency() - 1);
  }

  ~ThreadPool()
  {
    stop();
  }

  void queue(function<void()> f_task)
  {
    {
      unique_lock<mutex> lock{mEventMutex};
      mTasks.emplace(move(f_task)); 
    }

    mEventVar.notify_one(); 
  }
 
  private:
  vector<thread> mThreads;
  condition_variable mEventVar;
  mutex mEventMutex;
  bool mStopping = false;
  std::queue<function<void()>> mTasks; 
  
  void start(size_t numThreads)
  { 
    for (int x =0; x < numThreads; ++x)
    {
          mThreads.emplace_back([=] {
	  while (true)
	  {
	    function<void()> f_task;
	    {
	      unique_lock<mutex> lock{mEventMutex};
	      mEventVar.wait(lock, [=] { return (!mTasks.empty() || mStopping); });
		     
	      if (mStopping && mTasks.empty())
		break;

	      f_task = move(mTasks.front());
	      mTasks.pop();	 
	    }
  	    f_task();
	  }
	});
    }
  }
  
  void stop() noexcept
  {
      {
         unique_lock<std::mutex> lock{mEventMutex};
         mStopping = true;
      }
      mEventVar.notify_all();
 
      for (auto &thread : mThreads)
           thread.join();
  }
};
  
  


int main()
{
  ThreadPool pool;

  function<void()> f_task1  = DrawTask;
  function<void()> f_task2  = MemTask;
  function<void()> f_task3  = DataTask;

  pool.queue(f_task1);
  pool.queue(f_task2);
  pool.queue(f_task3);

  return 0;
}


