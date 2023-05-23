#include <atomic>
#include <chrono>
#include <cmath>
#include <errno.h>
#include <iostream>
#include <list>
#include <mutex>
#include <queue>
#include <string.h>
#include <thread>
#include <vector>

// all threads have yoursel queue

// Two queue: first - generated event, second go to handler threads.
void modifyTrivialVersion(const size_t &n, const size_t &numberEvent, const size_t &numberProcess)
{
  size_t globalCountEvent = n * numberEvent;
  std::mutex mtxEven;
  std::mutex mtxProcess;
  std::atomic<size_t> evenEnd{0};
  std::atomic<size_t> processEnd{0};
  std::atomic<size_t> countEven{0};
  std::list<std::pair<time_t, int>> quEvent, quProcess;
  std::vector<std::thread> vecEvent{}, vecProcess{};

  auto evenLambda = [&]()
  {
    for (size_t i = 0; i < n; ++i)
    {
      std::pair<time_t, int> tmp;

      {
        std::lock_guard lg(mtxEven);
        quEvent.push_back(std::pair<time_t, int>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()), rand() % 1000001 + 1));
        tmp = quEvent.back();
      }

      evenEnd.fetch_add(1);
      countEven.fetch_add(1);
      // std::cout << "Created event"
      //           << ":: event " << ctime(&tmp.first) << " number: " << tmp.second << std::endl
      //           << std::endl;
    }
  };

  auto processLambda = [&]()
  {
    std::pair<time_t, size_t> tmp;
    while (true)
    {
      // bool isPrime = true;
      {
        std::lock_guard lg(mtxProcess);
        if (quProcess.empty() && processEnd == globalCountEvent)
          break;
        else if (quProcess.empty())
          continue;
        tmp = quProcess.front();
        quProcess.pop_front();
      }

      for (size_t i = 2; i * i < tmp.second + 1; ++i)
      {
        if (!(tmp.second % i))
        {
          // isPrime = false;
          break;
        }
      }
      processEnd.fetch_add(1);

      // std::cout << "Process completed"
      //           << ":: event " << ctime(&tmp.first) << " number: " << tmp.second << " is prime: " << isPrime << std::endl;
    }
  };

  auto managerLambda = [&]()
  {
    double percent = globalCountEvent * 0.2 / numberProcess;
    std::list<std::pair<time_t, int>> tmp;
    while (true)
    {
      {
        std::lock_guard lg(mtxEven);
        if (evenEnd == globalCountEvent && quEvent.empty())
          break;
        else if (countEven < percent && evenEnd != globalCountEvent)
          continue;

        tmp = std::move(quEvent);
        countEven = 0;
      }
      {
        std::lock_guard lg(mtxProcess);
        std::lock_guard lg1(mtxEven);
        // std::cout << globalCountEvent - evenEnd << " ::::"
        //           << "Size quProcess: " << quProcess.size() << " Size move: " << tmp.size() << std::endl;
        quProcess.merge(std::move(tmp));

        // perror("Error: ");
      }
      // std::cout << "manager ready" << std::endl;
    }
  };

  std::thread managerThread(managerLambda);

  for (size_t i = 0; i < numberEvent; ++i)
    vecEvent.push_back(std::thread{evenLambda});

  for (size_t i = 0; i < numberProcess; ++i)
    vecProcess.push_back(std::thread{processLambda});

  for (size_t i = 0; i < numberEvent; ++i)
    vecEvent[i].join();

  for (size_t i = 0; i < numberProcess; ++i)
    vecProcess[i].join();

  managerThread.join();
  return;
}

// trivial version: consumer -- producer. One queue, n threads generate event, m handlers compute prime number
void trivialVersion(const size_t &n, const size_t &numberEvent, const size_t &numberProcess)
{
  size_t globalCountEvent = n * numberEvent;
  std::mutex mtx;
  std::atomic<size_t> evenEnd{0};
  std::queue<std::pair<time_t, int>> quEvent;
  std::vector<std::thread> vecEvent{}, vecProcess{};

  auto evenLambda = [&]()
  {
    for (size_t i = 0; i < n; ++i)
    {
      std::pair<time_t, int> tmp;

      {
        std::lock_guard lg(mtx);
        quEvent.push(std::pair<time_t, int>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()), rand() % 1000001 + 1));
        // tmp = quEvent.back();
      }

      evenEnd.fetch_add(1);
      // std::cout << "Created event" << ":: event " << ctime(&tmp.first) << " number: " << tmp.second << std::endl << std::endl;
    }
  };

  auto processLambda = [&]()
  {
    std::pair<time_t, size_t> tmp;
    while (true)
    {
      // bool isPrime = true;
      {
        std::lock_guard lg(mtx);

        if (quEvent.empty() && evenEnd.load() == globalCountEvent)
        {
          break;
        }
        else if (quEvent.empty())
          continue;

        tmp = quEvent.front();
        quEvent.pop();
      }

      for (size_t i = 2; i * i < tmp.second; ++i)
      {
        if (!(tmp.second % i))
        {
          // SisPrime = false;
          break;
        }
      }
      // std::cout << "Process completed" << ":: event " << ctime(&tmp.first) << " number: " << tmp.second << " is prime: " << isPrime << std::endl;
    }
  };

  for (size_t i = 0; i < numberEvent; ++i)
    vecEvent.push_back(std::thread{evenLambda});

  for (size_t i = 0; i < numberProcess; ++i)
    vecProcess.push_back(std::thread{processLambda});

  for (size_t i = 0; i < numberEvent; ++i)
    vecEvent[i].join();

  for (size_t i = 0; i < numberProcess; ++i)
    vecProcess[i].join();

  return;
}

int main(int argc, char *argv[])
{
  srand(time(NULL));

  size_t numberEvent = 2, numberProcess = 5;
  size_t n = 16e7;

  numberEvent = std::strtoull(argv[1]);
  n = std::strtoull(argv[2]);
  numberProcess = std::strtoull(argv[3]);

  if (argc == 3)
  {
    std::cout << "Default: trivial version." << std::endl;
    trivialVersion(n, numberEvent, numberProcess);
  }
  else if (strcmp(argv[4], "modify") == 0)
  {
    std::cout << "Runned modify version." << std::endl;
    modifyTrivialVersion(n, numberEvent, numberProcess);
  }
  else if (strcmp(argv[4], "trivial") == 0)
  {
    std::cout << "Runned trivial version." << std::endl;
    trivialVersion(n, numberEvent, numberProcess);
  }
  std::cout << "Programm done" << std::endl;
  return 0;
}