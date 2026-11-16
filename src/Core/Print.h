#pragma once

#include <atomic>
#include <functional>
#include <iostream>
#include <map>
#include <vector>

class Print {
  public:
    Print(std::atomic<bool>& stopSignal, const std::function<int()>& stackS,
          std::function<std::map<int, std::map<int, double>>()>& pr)
        : stop(stopSignal), StackSize(stackS), getProgress(pr){};
    void               printProgress();
    std::atomic<bool>& stop;

  private:
    std::function<int()>                                  StackSize;
    std::function<std::map<int, std::map<int, double>>()> getProgress;
    static inline void                                    clearScreen() {
        std::cout << "\033[H\033[J" << std::flush;
    }
    static inline void moveCursor(unsigned int col, unsigned int row) {
        std::cout << "\033[" << row << ";" << col << "H";
    }
};
