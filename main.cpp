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

bool isPrime(size_t n)
{
  if (n == 2 || n == 3)
    return true;
  if (n % 2 == 0 || n < 2)
    return false;
  for (size_t i = 3; i * i <= n; i += 2)
    if (n % i == 0)
      return false;
  return true;
}

void modifyTrivialVersion(const size_t &n, const size_t &numberEvent, const size_t &numberProcess)
{
  size_t globalCountEvent = n * numberEvent;
  std::mutex mtxEven;
  std::mutex mtxProcess;
  std::atomic<size_t> evenEnd{0};
  std::atomic<size_t> processEnd{0};
  std::atomic<size_t> countEven{0};
  std::list<std::pair<time_t, int>> quEvent{};
  std::list<std::pair<time_t, int>> quProcess{};
  std::vector<std::thread> vecEvent{};
  std::vector<std::thread> vecProcess{};

  auto evenLambda = [&]()
  {
    for (size_t i = 0; i < n; ++i)
    {
      {
        std::lock_guard lg(mtxEven);
        quEvent.push_back(std::pair<time_t, int>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()), rand() % 1000001 + 1));
      }

      evenEnd.fetch_add(1);
      countEven.fetch_add(1);
    }
  };

  auto processLambda = [&]()
  {
    std::pair<time_t, size_t> tmp;
    while (true)
    {
      {
        std::lock_guard lg(mtxProcess);
        if (quProcess.empty() && processEnd == globalCountEvent)
          break;
        else if (quProcess.empty())
          continue;
        tmp = quProcess.front();
        quProcess.pop_front();
      }

      isPrime(tmp.second);
      processEnd.fetch_add(1);
    }
  };

  auto managerLambda = [&]()
  {
    size_t percent = globalCountEvent * 0.2 / numberProcess;
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
        std::lock_guard lgProcess(mtxProcess);
        std::lock_guard lgEven(mtxEven);

        quProcess.merge(std::move(tmp));
      }
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
      {
        std::lock_guard lg(mtx);
        quEvent.push(std::pair<time_t, int>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()),
                                            rand() % 1000001 + 1));
      }
      evenEnd.fetch_add(1);
    }
  };

  auto processLambda = [&]()
  {
    std::pair<time_t, size_t> tmp;
    while (true)
    {
      {
        std::lock_guard lg(mtx);

        if (quEvent.empty() && evenEnd.load() == globalCountEvent)
          break;
        else if (quEvent.empty())
          continue;

        tmp = quEvent.front();
        quEvent.pop();
      }

      isPrime(tmp.second);
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

  size_t numberProducers = 0;
  size_t numberConsumers = 0;
  size_t n = 0;
  char *end = nullptr;

  numberProducers = std::strtoull(argv[1], &end, 10);

  n = std::strtoull(argv[2], &end, 10);
  numberConsumers = std::strtoull(argv[3], &end, 10);

  std::cout << numberProducers << " " << n << " " << numberConsumers << std::endl;

  if (argc == 3)
  {
    std::cout << "Default: trivial version." << std::endl;
    trivialVersion(n, numberProducers, numberConsumers);
  }
  else if (strcmp(argv[4], "modify") == 0)
  {
    std::cout << "Runned modify version." << std::endl;
    modifyTrivialVersion(n, numberProducers, numberConsumers);
  }
  else if (strcmp(argv[4], "trivial") == 0)
  {
    std::cout << "Runned trivial version." << std::endl;
    trivialVersion(n, numberProducers, numberConsumers);
  }
  std::cout << "Programm done" << std::endl;
  return 0;
}