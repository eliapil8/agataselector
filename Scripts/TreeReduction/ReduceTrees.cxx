#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>

#include <deque>
#include <iomanip>
#include <string>
#include <vector>

#include <functional>
#include <mutex>
#include <thread>

#include <TChain.h>
#include <TFile.h>
#include <TTree.h>

// to compile: g++ -Wall -pthread `root-config --cflags` -o ReduceTrees
// ReduceTrees.cxx `root-config --libs`

const std::string CONDITION = "nbLABR==0 && nbOSCAR>=2";

std::mutex        mtx;

std::deque<int>   runsToAnalyze;

std::string       nCharInt(int I, int n) {
    std::stringstream ID;
    ID << std::setfill('0') << std::setw(n) << I;
    return ID.str();
}

void threadFunction(int runNr);
void runThread();

using namespace std;

//------------------------------------------------------------------------------||

int main(int argc, char* argv[]) {
    if(argc < 2) throw std::runtime_error("Too few parameters provided\n");

    int MAX_NUMBER_OF_THREADS = 4;

    for(int i = 1; i < argc; i++) {
        string option = string(argv[i]);
        if(option == "-h" || option == "--help") {
            cout << "Usage:\n";
            cout << "./ReduceTrees [options, ...] [#run, ...]\n";
            cout << "\t--nrthr\t\t[#]\tSpecify maximum number of threads, "
                    "default is 4\n";
            cout << endl;
            return 0;
        }

        if(option == "--nrthr") {
            try {
                MAX_NUMBER_OF_THREADS = stoi(argv[++i]);
            } catch(...) {
                throw std::runtime_error("Error with provided nrthr\n");
            }
            continue;
        }

        try {
            runsToAnalyze.push_back(stoi(option));
        } catch(...) {
            throw std::runtime_error(
                string("Provided run number does not make sense : ") + option);
        }
    }

    vector<thread> threads;
    for(int i = 0; i < MAX_NUMBER_OF_THREADS; i++)
        threads.emplace_back(runThread);
    for(auto& th : threads) th.join();

    return 0;
}

//------------------------------------------------------------------------------||

void runThread() {
    while(true) {
        int run;
        {
            std::lock_guard<std::mutex> guard(mtx);
            if(runsToAnalyze.empty()) return;

            run = runsToAnalyze.front();
            runsToAnalyze.pop_front();
        }
        threadFunction(run);
    }
}

void threadFunction(int runNr) {
    std::cout << "Processing run " << runNr << std::endl;
    TChain* ch = new TChain("TreeMaster");
    ch->Add(("Data/run_" + nCharInt(runNr, 4) + "/Out/Analysis/Tree_*.root")
                .c_str());
    TFile file(
        ("Data/run_" + nCharInt(runNr, 4) + "/Out/Analysis/redTree_0000.root")
            .c_str(),
        "recreate");
    TTree* tree = ch->CopyTree(CONDITION.c_str());
    tree->Write();
    file.Close();
    std::cout << "End of run " << runNr << std::endl;
}
