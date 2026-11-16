#include "Print.h"

#include <iomanip>
#include <sstream>
#include <sys/ioctl.h>
#include <unistd.h>

#include "Settings.h"

#include <TStopwatch.h>

void Print::printProgress() {
    clearScreen();

    TStopwatch timerSelector;
    timerSelector.Start();

    while(!stop.load()) {
        // while(1){
        sleep(1);
        struct winsize w {};
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        auto columns{w.ws_col};
        if(Settings::getInstance().window_size != -1)
            columns = Settings::getInstance().window_size;

        moveCursor(0, 0);
        std::cout << "\033[0;97;42m" << std::string(columns, ' ');

        moveCursor(2, 0);
        std::cout << "Active threads";

        moveCursor(0, 2);
        std::cout << "\033[0;97m" << std::string(columns, ' ');
        moveCursor(0, 2);
        std::cout << "Number of jobs in the stack : " << std::setfill(' ')
                  << std::setw(3) << StackSize(); // << std::flush;

        unsigned int rowCnt{3};
        auto         progress = getProgress();
        for(const auto& cp : progress) {
            for(const auto& wk : cp.second) {
                int  nTicks{columns - 39};
                auto prg = wk.second;
                moveCursor(0, rowCnt++);
                std::stringstream progressString;
                progressString << "\033[0;97mThread nr. " << cp.first << "."
                               << std::setfill('0') << std::setw(3) << wk.first
                               << " progress:  \033[0;31m|";
                for(int t{0}; t < nTicks; ++t) {
                    if(prg / 100. < static_cast<double>(t) / nTicks) {
                        progressString << "-";
                    } else {
                        progressString << "=";
                    }
                }
                progressString << "|\033[0;97m  "; // << std::flush;
                if(prg <= -2.) {
                    progressString << "opening ";
                } else if(prg < 0.) {
                    progressString << "alloc.   ";
                } else if(prg > 0. && prg < 100.) {
                    progressString << std::fixed << std::setprecision(1)
                                   << std::setfill(' ') << std::setw(5) << prg
                                   << " %  ";
                } else if(prg > 100. && prg < 200.) {
                    progressString << "writing ";
                } else if(prg >= 200.) {
                    progressString << "finished";
                } else {
                    progressString << "idle    ";
                }
                std::cout << progressString.str();
            }
        }
        moveCursor(0, rowCnt);
        std::cout << "\033[0;97;42m" << std::string(columns, ' ');
        moveCursor(2, rowCnt);
        Double_t realt = timerSelector.RealTime();
        std::cout << std::setprecision(0) << (Int_t)(realt / 60)
                  << "m:" << (Int_t)((Int_t)realt % 60) << "s";
        timerSelector.Continue();

        moveCursor(2, rowCnt + 1);
        std::cout << "\033[0;97m" << std::string(columns, ' ');
        moveCursor(2, rowCnt + 1);
        std::cout << "\nLog messages: " << std::endl;
    }
    timerSelector.Stop();
}
