#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>

#include <deque>
#include <vector>

#include <functional>
#include <mutex>
#include <thread>

#include <TFile.h>
#include <TRandom.h>
#include <TTree.h>

// to compile: g++ -Wall -pthread `root-config --cflags` -o MergeSortCaen
// MergeSortCaen.cxx `root-config --libs`

const int         MAX_NUMBER_OF_THREADS = 4;
const std::string PATH                  = "Resource";
const int         VERBOSE_LEVEL         = 1; // 0 is quiet

struct DataIn {
    UShort_t  Channel   = 0;
    ULong64_t Timestamp = 0;
    UShort_t  Board     = 0;
    UShort_t  Energy    = 0;
    UInt_t    Flags     = 0;
};

std::mutex mtx;

void       ProcessBlock(std::vector<std::vector<DataIn>*>* subBlockList,
                        const std::string&                 blockFileName);
void       SortSubBlock(std::deque<std::vector<DataIn>*>* subBlockDeque);
void       SortSubBlockThread(std::vector<DataIn>* subBlock);

void       MergeSubBlock(std::deque<std::vector<DataIn>*>*  subBlockDequeEven,
                         std::deque<std::vector<DataIn>*>*  subBlockDequeOdd,
                         std::vector<std::vector<DataIn>*>* subBlockListProcessed);

void       MergeSubBlockThread(std::vector<DataIn>* subBlockLeft,
                               std::vector<DataIn>* subBlockRight,
                               std::vector<DataIn>* subBlockProcessed);

void       MergeBlock(std::deque<std::string>*  blockDequeEven,
                      std::deque<std::string>*  blockDequeOdd,
                      std::vector<std::string>* blockListProcessed,
                      int                       currentGeneration);

void       MergeBlockThread(std::string blockLeft, std::string blockRight,
                            std::string blockProcessed);

bool       ReadBlockEntry(DataIn& dataEntry, std::ifstream& fin);
void       WriteBlockEntry(DataIn& dataEntry, std::ofstream& fBlock);
void       PrintBlockEntry(DataIn& dataEntry);

//-------------------------------------------------------------------------------------

int        main(int argc, char* argv[]) {
    if(argc < 3) {
        std::cout << "\nERROR: not enough parameters!" << std::endl;
        std::cout << "MergeSortCaen inputFile outputFile [max_events]"
                  << std::endl;
        return -1;
    }

    const ULong64_t   maxSubBlockSize   = 20;
    const ULong64_t   maxSubBlockNumber = 512 * 8192;

    const std::string outputfile        = argv[2];
    const std::string inputfile         = argv[1];
    int               it_max            = -1;
    if(argc >= 4) it_max = atoi(argv[3]);

    DataIn dataIn;

    TFile* input = new TFile(inputfile.c_str(), "read");
    if(!input || input->IsZombie()) {
        std::cerr << "\nERROR: Unable to open ROOT file " << inputfile
                  << std::endl;
        return -1;
    }
    TTree*    inTree   = dynamic_cast<TTree*>(input->Get("Data_R"));

    ULong64_t nEntries = inTree->GetEntries();
    if(nEntries == 0) {
        std::cerr << "\nERROR: number of entries in " << inputfile
                  << " is zero" << std::endl;
        return -1;
    }
    if(it_max != -1 && nEntries > static_cast<ULong64_t>(it_max))
        nEntries = static_cast<ULong64_t>(it_max);

    inTree->SetBranchAddress("Channel", &dataIn.Channel);
    inTree->SetBranchAddress("Timestamp", &dataIn.Timestamp);
    inTree->SetBranchAddress("Board", &dataIn.Board);
    inTree->SetBranchAddress("Energy", &dataIn.Energy);
    inTree->SetBranchAddress("Flags", &dataIn.Flags);

    system(("mkdir " + PATH).c_str());

    int nBlocks;
    if(nEntries % (maxSubBlockSize * maxSubBlockNumber) == 0)
        nBlocks = nEntries / (maxSubBlockSize * maxSubBlockNumber);
    else
        nBlocks = nEntries / ((maxSubBlockSize * maxSubBlockNumber)) + 1;
    std::cout << "\nMergeSort algorithm is starting. Number of blocks is "
              << nBlocks << "\n"
              << std::endl;

    std::vector<std::vector<DataIn>*>* subBlockList
        = new std::vector<std::vector<DataIn>*>();
    std::vector<DataIn>*      tmp     = new std::vector<DataIn>();

    int                       blockNr = 1;
    std::vector<std::string>* blockList = new std::vector<std::string>();

    for(ULong64_t i = 0; i < nEntries; i++) {
        if(VERBOSE_LEVEL >= 8) std::cout << "reading entry " << i << std::endl;

        inTree->GetEntry(i);
        tmp->push_back(dataIn);

        if(tmp->size() == maxSubBlockSize) {
            subBlockList->push_back(tmp);
            tmp = new std::vector<DataIn>();
            if(subBlockList->size() == maxSubBlockNumber) {
                std::cout << "\nStage 0 - block\t" << blockNr << " out of "
                          << nBlocks << std::endl;
                blockList->push_back(PATH + "/block_0_"
                                            + std::to_string(blockNr));
                ProcessBlock(subBlockList, blockList->back());
                blockNr++;
            }
        }
    }
    input->Close();

    // Process final block
    if(tmp->size() != 0) subBlockList->push_back(tmp);
    if(subBlockList->size() != 0) {
        std::cout << "\nStage 0 - block\t" << blockNr << " out of " << nBlocks
                  << std::endl;
        blockList->push_back(PATH + "/block_0_" + std::to_string(blockNr));
        ProcessBlock(subBlockList, blockList->back());
    }

    std::vector<std::string>* blockListProcessed;
    int nGenerations      = static_cast<int>(log2(nBlocks - 1) + 1);
    int currentGeneration = 1;

    while(blockList->size() > 2) {
        std::cout << "\nStage 1: Generation " << currentGeneration
                  << " out of " << nGenerations << "\t (" << blockList->size()
                  << " blocks)" << std::endl;

        blockListProcessed = new std::vector<std::string>();
        std::deque<std::string>* blockDequeEven
            = new std::deque<std::string>();
        std::deque<std::string>* blockDequeOdd = new std::deque<std::string>();
        for(unsigned int i = 0; i < blockList->size(); i++) {
            if(i % 2 == 0)
                blockDequeEven->push_back(blockList->at(i));
            else
                blockDequeOdd->push_back(blockList->at(i));
        }

        // start threads
        {
            std::vector<std::thread> threads;
            for(int i = 0; i < MAX_NUMBER_OF_THREADS; i++)
                threads.emplace_back(MergeBlock, blockDequeEven, blockDequeOdd,
                                            blockListProcessed, currentGeneration);
            for(auto& th : threads) th.join();
        }

        // clear blockList
        // for(unsigned int i=0; i<blockList->size(); i++) system(("rm
        // "+blockList->at(i)).c_str());
        blockList->clear();

        blockList = blockListProcessed;
        currentGeneration++;
    }

    std::cout << "\nStage 1: Generation " << currentGeneration << " out of "
              << nGenerations << "\t (" << blockList->size() << " blocks)"
              << std::endl;

    TFile* outFile = new TFile(outputfile.c_str(), "recreate");
    TTree* outTree = new TTree("Data_R", "Data_R");

    DataIn dataOut;

    outTree->Branch("Channel", &dataOut.Channel, "Channel/s");
    outTree->Branch("Timestamp", &dataOut.Timestamp, "Timestamp/l");
    outTree->Branch("Board", &dataOut.Board, "Board/s");
    outTree->Branch("Energy", &dataOut.Energy, "Energy/s");
    outTree->Branch("Flags", &dataOut.Flags, "Flags/i");

    if(blockList->size() == 1) {
        std::ifstream fin(blockList->front(), std::ios::in);
        while(ReadBlockEntry(dataOut, fin)) outTree->Fill();
    } else if(blockList->size() == 2) {
        std::ifstream fLeft(blockList->at(0), std::ios::in);
        std::ifstream fRight(blockList->at(1), std::ios::in);

        DataIn        dataLeft;
        DataIn        dataRight;

        // read first
        ReadBlockEntry(dataLeft, fLeft);
        ReadBlockEntry(dataRight, fRight);

        while(true) {
            if(dataLeft.Timestamp <= dataRight.Timestamp) {
                dataOut = dataLeft;
                outTree->Fill();
                if(VERBOSE_LEVEL >= 10) {
                    std::cout << "L  ";
                    PrintBlockEntry(dataLeft);
                }
                if(!ReadBlockEntry(dataLeft, fLeft)) {
                    dataOut = dataRight;
                    outTree->Fill();
                    if(VERBOSE_LEVEL >= 10) {
                        std::cout << "LR1 ";
                        PrintBlockEntry(dataRight);
                    }
                    while(ReadBlockEntry(dataRight, fRight)) {
                        dataOut = dataRight;
                        outTree->Fill();
                        if(VERBOSE_LEVEL >= 10) {
                            std::cout << "LR2 ";
                            PrintBlockEntry(dataRight);
                        }
                    }
                    break;
                }
            } else {
                dataOut = dataRight;
                outTree->Fill();
                if(VERBOSE_LEVEL >= 10) {
                    std::cout << "R  ";
                    PrintBlockEntry(dataRight);
                }
                if(!ReadBlockEntry(dataRight, fRight)) {
                    dataOut = dataLeft;
                    outTree->Fill();
                    if(VERBOSE_LEVEL >= 10) {
                        std::cout << "RL1 ";
                        PrintBlockEntry(dataLeft);
                    }
                    while(ReadBlockEntry(dataLeft, fLeft)) {
                        dataOut = dataLeft;
                        outTree->Fill();
                        if(VERBOSE_LEVEL >= 10) {
                            std::cout << "RL2 ";
                            PrintBlockEntry(dataLeft);
                        }
                    }
                    break;
                }
            }
        }
        fLeft.close();
        fRight.close();
    } else
        std::cout << "ERROR: blockList size is " << blockList->size()
                  << std::endl;

    outTree->Write();
    outFile->Close();

    system(("rm -rv " + PATH).c_str());

    std::cout << "\nEND" << std::endl;

    return 0;
}

void ProcessBlock(std::vector<std::vector<DataIn>*>* subBlockList,
                  const std::string&                 blockFileName) {
    int nGenerations = static_cast<int>(log2(subBlockList->size() - 1) + 1);
    int currentGen   = 0;
    std::cout << "Generations in block: " << nGenerations << "\t("
              << subBlockList->size() << " subBlocks)" << std::endl;

    {
        if(VERBOSE_LEVEL >= 1)
            std::cout << "Generation 0 - time sorting of each subBlock"
                      << std::endl;
        // create deque for easy handling of multiple threads
        std::deque<std::vector<DataIn>*>* subBlockDeque
            = new std::deque<std::vector<DataIn>*>();
        for(auto subBlock : (*subBlockList))
            subBlockDeque->push_back(subBlock);

        // start threads
        std::vector<std::thread> threads;
        for(int i = 0; i < MAX_NUMBER_OF_THREADS; i++)
            threads.emplace_back(SortSubBlock, subBlockDeque);
        for(auto& th : threads) th.join();
    }

    std::vector<std::vector<DataIn>*>* subBlockListProcessed;
    while(subBlockList->size() > 1) {
        if(VERBOSE_LEVEL >= 1)
            std::cout << "Generation " << currentGen + 1 << " out of "
                      << nGenerations << "\t (" << subBlockList->size()
                      << " subBlocks)" << std::endl;

        subBlockListProcessed = new std::vector<std::vector<DataIn>*>();
        std::deque<std::vector<DataIn>*>* subBlockDequeEven
            = new std::deque<std::vector<DataIn>*>();
        std::deque<std::vector<DataIn>*>* subBlockDequeOdd
            = new std::deque<std::vector<DataIn>*>();
        for(unsigned int i = 0; i < subBlockList->size(); i++) {
            if(i % 2 == 0)
                subBlockDequeEven->push_back(subBlockList->at(i));
            else
                subBlockDequeOdd->push_back(subBlockList->at(i));
        }

        if(VERBOSE_LEVEL >= 2) std::cout << "Starting threads" << std::endl;

        // start threads
        std::vector<std::thread> threads;
        for(int i = 0; i < MAX_NUMBER_OF_THREADS; i++)
            threads.emplace_back(MergeSubBlock, subBlockDequeEven,
                                 subBlockDequeOdd, subBlockListProcessed);
        for(auto& th : threads) th.join();

        if(VERBOSE_LEVEL >= 5)
            std::cout << "Threads have finished" << std::endl;
        if(VERBOSE_LEVEL >= 5)
            std::cout << "subBlockList->size() " << subBlockList->size()
                      << std::endl;
        if(VERBOSE_LEVEL >= 5)
            std::cout << "subBlockListProcessed->size() "
                      << subBlockListProcessed->size() << std::endl;

        // clear subBlockList
        for(unsigned int i = 0; i < subBlockList->size(); i++)
            delete subBlockList->at(i);
        subBlockList->clear();

        if(VERBOSE_LEVEL >= 5)
            std::cout << "subBlockList was cleared" << std::endl;

        subBlockList = subBlockListProcessed;
        currentGen++;

        if(VERBOSE_LEVEL >= 5)
            std::cout << "subBlockList was updated" << std::endl;
        if(VERBOSE_LEVEL >= 5)
            std::cout << "subBlockList->size() " << subBlockList->size()
                      << std::endl;
    }

    if(VERBOSE_LEVEL >= 2) std::cout << "Writing block on file" << std::endl;

    std::ofstream fBlock(blockFileName.c_str(), std::ios::out);
    for(unsigned int i = 0; i < subBlockList->at(0)->size(); i++)
        WriteBlockEntry(subBlockList->at(0)->at(i), fBlock);

    fBlock << std::flush;
    fBlock.close();

    // clear subBlockList
    for(unsigned int i = 0; i < subBlockList->size(); i++)
        delete subBlockList->at(i);
    delete subBlockList;

    if(VERBOSE_LEVEL >= 2) std::cout << "Block was processed" << std::endl;
}

void SortSubBlock(std::deque<std::vector<DataIn>*>* subBlockDeque) {
    while(true) {
        std::vector<DataIn>* subBlock;
        {
            std::lock_guard<std::mutex> guard(mtx);
            if(subBlockDeque->empty()) return;

            subBlock = subBlockDeque->front();
            subBlockDeque->pop_front();
        }
        SortSubBlockThread(subBlock);
    }
}

void SortSubBlockThread(std::vector<DataIn>* subBlock) {
    int          indexMin;
    unsigned int subBlockSize = subBlock->size();
    DataIn       tmp;
    for(unsigned int i = 0; i < subBlockSize; i++) {
        // find minimum
        indexMin = i;
        for(unsigned int j = i; j < subBlockSize; j++) {
            if(subBlock->at(j).Timestamp < subBlock->at(indexMin).Timestamp)
                indexMin = j;
        }

        // put minimum at start
        tmp                    = subBlock->at(i);
        subBlock->at(i)        = subBlock->at(indexMin);
        subBlock->at(indexMin) = tmp;
    }

    if(VERBOSE_LEVEL >= 5) std::cout << "Sub Block was sorted" << std::endl;
}

void MergeSubBlock(std::deque<std::vector<DataIn>*>*  subBlockDequeEven,
                   std::deque<std::vector<DataIn>*>*  subBlockDequeOdd,
                   std::vector<std::vector<DataIn>*>* subBlockListProcessed) {
    std::vector<DataIn>* subBlockLeft      = nullptr;
    std::vector<DataIn>* subBlockRight     = nullptr;
    std::vector<DataIn>* subBlockProcessed = nullptr;
    bool                 dequeEvenIsEmpty  = false;
    bool                 dequeOddIsEmpty   = false;

    while(true) {
        {
            std::lock_guard<std::mutex> guard(mtx);
            if(subBlockDequeEven->empty() && subBlockDequeOdd->empty()) return;

            if(subBlockDequeEven->empty())
                dequeEvenIsEmpty = true;
            else {
                subBlockLeft = subBlockDequeEven->front();
                subBlockDequeEven->pop_front();
            }

            if(subBlockDequeOdd->empty())
                dequeOddIsEmpty = true;
            else {
                subBlockRight = subBlockDequeOdd->front();
                subBlockDequeOdd->pop_front();
            }
        }
        subBlockProcessed = new std::vector<DataIn>();
        if(dequeEvenIsEmpty)
            for(auto it : (*subBlockRight)) subBlockProcessed->push_back(it);
        if(dequeOddIsEmpty)
            for(auto it : (*subBlockLeft)) subBlockProcessed->push_back(it);
        if(!dequeEvenIsEmpty && !dequeOddIsEmpty)
            MergeSubBlockThread(subBlockLeft, subBlockRight,
                                subBlockProcessed);
        {
            std::lock_guard<std::mutex> guard(mtx);
            subBlockListProcessed->push_back(subBlockProcessed);
        }
    }
}

void MergeSubBlockThread(std::vector<DataIn>* subBlockLeft,
                         std::vector<DataIn>* subBlockRight,
                         std::vector<DataIn>* subBlockProcessed) {
    ULong64_t indexLeft  = 0;
    ULong64_t indexRight = 0;
    ULong64_t leftSize   = subBlockLeft->size();
    ULong64_t rightSize  = subBlockRight->size();

    while(indexLeft != leftSize || indexRight != rightSize) {
        if(indexRight == rightSize) {
            subBlockProcessed->push_back(subBlockLeft->at(indexLeft));
            indexLeft++;
            continue;
        }

        if(indexLeft == leftSize) {
            subBlockProcessed->push_back(subBlockRight->at(indexRight));
            indexRight++;
            continue;
        }

        if(subBlockLeft->at(indexLeft).Timestamp
           <= subBlockRight->at(indexRight).Timestamp) {
            subBlockProcessed->push_back(subBlockLeft->at(indexLeft));
            indexLeft++;
            continue;
        }

        if(subBlockLeft->at(indexLeft).Timestamp
           > subBlockRight->at(indexRight).Timestamp) {
            subBlockProcessed->push_back(subBlockRight->at(indexRight));
            indexRight++;
            continue;
        }
    }

    if(VERBOSE_LEVEL >= 3) std::cout << "Sub Blocks were merged" << std::endl;
}

void MergeBlock(std::deque<std::string>*  blockDequeEven,
                std::deque<std::string>*  blockDequeOdd,
                std::vector<std::string>* blockListProcessed,
                int                       currentGeneration) {
    std::string blockLeft;
    std::string blockRight;
    std::string blockProcessed;
    bool        dequeEvenIsEmpty = false;
    bool        dequeOddIsEmpty  = false;

    while(true) {
        {
            std::lock_guard<std::mutex> guard(mtx);
            if(blockDequeEven->empty() && blockDequeOdd->empty()) return;

            blockProcessed = PATH + "/block_"
                           + std::to_string(currentGeneration) + "_"
                           + std::to_string(blockListProcessed->size());
            blockListProcessed->push_back(blockProcessed);

            if(blockDequeEven->empty())
                dequeEvenIsEmpty = true;
            else {
                blockLeft = blockDequeEven->front();
                blockDequeEven->pop_front();
            }

            if(blockDequeOdd->empty())
                dequeOddIsEmpty = true;
            else {
                blockRight = blockDequeOdd->front();
                blockDequeOdd->pop_front();
            }
        }
        if(dequeEvenIsEmpty) {
            system(("cp " + blockRight + " " + blockProcessed).c_str());
            system(("rm " + blockRight).c_str());
            continue;
        }
        if(dequeOddIsEmpty) {
            system(("cp " + blockLeft + " " + blockProcessed).c_str());
            system(("rm " + blockLeft).c_str());
            continue;
        }
        MergeBlockThread(blockLeft, blockRight, blockProcessed);
        system(("rm " + blockRight).c_str());
        system(("rm " + blockLeft).c_str());
    }
}

void MergeBlockThread(std::string blockLeft, std::string blockRight,
                      std::string blockProcessed) {
    std::ifstream fLeft(blockLeft.c_str(), std::ios::in);
    std::ifstream fRight(blockRight.c_str(), std::ios::in);
    std::ofstream fProcessed(blockProcessed.c_str(), std::ios::out);

    if(!fLeft.is_open())
        throw std::runtime_error("Cannot open input file " + blockLeft);
    if(!fRight.is_open())
        throw std::runtime_error("Cannot open input file " + blockRight);
    if(!fProcessed.is_open())
        throw std::runtime_error("Cannot open output file " + blockProcessed);

    if(VERBOSE_LEVEL >= 3) {
        std::cout << "Opened input file " << blockLeft << "\n";
        std::cout << "Opened input file " << blockRight << "\n";
        std::cout << "Opened output file " << blockProcessed << std::endl;
    }

    DataIn dataLeft;
    DataIn dataRight;

    // read first
    ReadBlockEntry(dataLeft, fLeft);
    ReadBlockEntry(dataRight, fRight);

    while(true) {
        if(dataLeft.Timestamp <= dataRight.Timestamp) {
            WriteBlockEntry(dataLeft, fProcessed);
            if(VERBOSE_LEVEL >= 10) {
                std::cout << "L  ";
                PrintBlockEntry(dataLeft);
            }
            if(!ReadBlockEntry(dataLeft, fLeft)) {
                WriteBlockEntry(dataRight, fProcessed);
                if(VERBOSE_LEVEL >= 10) {
                    std::cout << "LR1 ";
                    PrintBlockEntry(dataRight);
                }
                while(ReadBlockEntry(dataRight, fRight)) {
                    WriteBlockEntry(dataRight, fProcessed);
                    if(VERBOSE_LEVEL >= 10) {
                        std::cout << "LR2 ";
                        PrintBlockEntry(dataRight);
                    }
                }
                break;
            }
        } else {
            WriteBlockEntry(dataRight, fProcessed);
            if(VERBOSE_LEVEL >= 10) {
                std::cout << "R  ";
                PrintBlockEntry(dataRight);
            }
            if(!ReadBlockEntry(dataRight, fRight)) {
                WriteBlockEntry(dataLeft, fProcessed);
                if(VERBOSE_LEVEL >= 10) {
                    std::cout << "RL1 ";
                    PrintBlockEntry(dataLeft);
                }
                while(ReadBlockEntry(dataLeft, fLeft)) {
                    WriteBlockEntry(dataLeft, fProcessed);
                    if(VERBOSE_LEVEL >= 10) {
                        std::cout << "RL2 ";
                        PrintBlockEntry(dataLeft);
                    }
                }
                break;
            }
        }
    }
    fLeft.close();
    fRight.close();
    fProcessed.close();

    if(VERBOSE_LEVEL >= 3) std::cout << "Two Blocks were merged" << std::endl;
}

bool ReadBlockEntry(DataIn& dataEntry, std::ifstream& fin) {
    if(fin >> dataEntry.Timestamp >> dataEntry.Board >> dataEntry.Channel
       >> dataEntry.Energy >> dataEntry.Flags)
        return true;
    return false;
}

void WriteBlockEntry(DataIn& dataEntry, std::ofstream& fBlock) {
    fBlock << dataEntry.Timestamp << "\t";
    fBlock << dataEntry.Board << "\t";
    fBlock << dataEntry.Channel << "\t";
    fBlock << dataEntry.Energy << "\t";
    fBlock << dataEntry.Flags << "\n";
}

void PrintBlockEntry(DataIn& dataEntry) {
    std::cout << dataEntry.Timestamp << "\t" << dataEntry.Board << "\t"
              << dataEntry.Channel << "\t" << dataEntry.Energy << "\t"
              << dataEntry.Flags << std::endl;
}
