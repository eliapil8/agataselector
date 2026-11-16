#include <cstdlib>
#include <deque>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include <TFile.h>
#include <TRandom.h>
#include <TTree.h>

// to compile: g++ -Wall `root-config --cflags` -o SimToTreeMaster *.cxx
// `root-config --libs`

struct DataOut {
    Int_t     nbSAURON = 0;
    ULong64_t TSSAURON = 0;
    ULong64_t TSSAURON_Hit[100];
    UShort_t  SAURONChannel[100];
    UShort_t  SAURONBoard[100];
    Float_t   SAURONEnergy[100];
    Float_t   SAURONTime[100];
};

struct DataIn {
    UShort_t  Channel   = 0;
    ULong64_t Timestamp = 0;
    UShort_t  Board     = 0;
    UShort_t  Energy    = 0;
    UInt_t    Flags     = 0;
};

int main(int argc, char* argv[]) {
    std::map<int, std::map<int, Long64_t>> TimeOffset;
    std::map<int, Long64_t>                BoardTimeOffset;
    BoardTimeOffset[0] = 0;
    BoardTimeOffset[1] = -6000;

    // Board 0
    TimeOffset[0][0]   = 0;
    TimeOffset[0][16]  = -7000;
    TimeOffset[0][32]  = -7000;
    TimeOffset[0][48]  = -7000;
    TimeOffset[0][1]   = 0;
    TimeOffset[0][17]  = -11000;
    TimeOffset[0][33]  = -7000;
    TimeOffset[0][49]  = -7000;
    TimeOffset[0][2]   = 0;
    TimeOffset[0][18]  = 0;
    TimeOffset[0][34]  = -7000;
    TimeOffset[0][50]  = -7000;
    TimeOffset[0][3]   = 0;
    TimeOffset[0][19]  = -11000;
    TimeOffset[0][35]  = -7000;
    TimeOffset[0][51]  = -7000;
    TimeOffset[0][4]   = 0;
    TimeOffset[0][20]  = 0;
    TimeOffset[0][36]  = -7000;
    TimeOffset[0][52]  = -7000;
    TimeOffset[0][5]   = 0;
    TimeOffset[0][21]  = 0;
    TimeOffset[0][37]  = -7000;
    TimeOffset[0][53]  = -7000;
    TimeOffset[0][6]   = 0;
    TimeOffset[0][22]  = -11000;
    TimeOffset[0][38]  = -7000;
    TimeOffset[0][54]  = -7000;
    TimeOffset[0][7]   = 0;
    TimeOffset[0][23]  = 0;
    TimeOffset[0][39]  = -7000;
    TimeOffset[0][55]  = -7000;
    TimeOffset[0][8]   = 0;
    TimeOffset[0][24]  = -11000;
    TimeOffset[0][40]  = -7000;
    TimeOffset[0][56]  = -7000;
    TimeOffset[0][9]   = 0;
    TimeOffset[0][25]  = -11000;
    TimeOffset[0][41]  = -7000;
    TimeOffset[0][57]  = -7000;
    TimeOffset[0][10]  = 0;
    TimeOffset[0][26]  = 0;
    TimeOffset[0][42]  = -7000;
    TimeOffset[0][58]  = -7000;
    TimeOffset[0][11]  = 0;
    TimeOffset[0][27]  = 0;
    TimeOffset[0][43]  = -7000;
    TimeOffset[0][59]  = -7000;
    TimeOffset[0][12]  = 0;
    TimeOffset[0][28]  = 0;
    TimeOffset[0][44]  = -7000;
    TimeOffset[0][60]  = -7000;
    TimeOffset[0][13]  = 0;
    TimeOffset[0][29]  = 0;
    TimeOffset[0][45]  = -7000;
    TimeOffset[0][61]  = -7000;
    TimeOffset[0][14]  = 0;
    TimeOffset[0][30]  = 0;
    TimeOffset[0][46]  = -7000;
    TimeOffset[0][62]  = -7000;
    TimeOffset[0][15]  = 0;
    TimeOffset[0][31]  = 0;
    TimeOffset[0][47]  = -7000;
    TimeOffset[0][63]  = -7000;

    // Board 1
    TimeOffset[1][0]   = 8700;
    TimeOffset[1][1]   = 8000;
    TimeOffset[1][2]   = 7500;
    TimeOffset[1][3]   = 8200;
    TimeOffset[1][4]   = 6600;
    TimeOffset[1][5]   = 5800;
    TimeOffset[1][6]   = 5700;
    TimeOffset[1][7]   = 7300;
    TimeOffset[1][8]   = 3400;
    TimeOffset[1][9]   = 2200;
    TimeOffset[1][10]  = 1500;
    TimeOffset[1][11]  = 2500;
    TimeOffset[1][12]  = 1500;
    TimeOffset[1][13]  = 1600;
    TimeOffset[1][14]  = 1300;
    TimeOffset[1][15]  = 2000;

    if(argc < 3) {
        std::cout << "\nERROR: not enough parameters!" << std::endl;
        std::cout << "CaenToTreeMaster inputFile outputFile [max_events]"
                  << std::endl;
        return -1;
    }

    std::deque<DataIn> timeSortBuff;
    DataIn             dataIn;
    DataOut            dataOut;

    const int          SortedBuffMaxSize = 1000000;

    // Time coincidence window for building of events, in TS units ( 500TS =
    // 1ns )
    const int          TimeCoincWindow   = 1000 * 500;

    const std::string  outputfile        = argv[2];
    const std::string  inputfile         = argv[1];
    int                it_max            = -1;
    if(argc >= 4) it_max = atoi(argv[3]);

    TFile* input = new TFile(inputfile.c_str(), "read");
    if(!input || input->IsZombie()) {
        std::cerr << "\nERROR: Unable to open ROOT file " << inputfile
                  << std::endl;
        return -1;
    }
    TTree* inTree  = dynamic_cast<TTree*>(input->Get("Data_R"));

    TFile* output  = new TFile(outputfile.c_str(), "recreate");
    TTree* outTree = new TTree("TreeMaster", "TreeMaster");

    inTree->SetBranchAddress("Channel", &dataIn.Channel);
    inTree->SetBranchAddress("Timestamp", &dataIn.Timestamp);
    inTree->SetBranchAddress("Board", &dataIn.Board);
    inTree->SetBranchAddress("Energy", &dataIn.Energy);
    inTree->SetBranchAddress("Flags", &dataIn.Flags);

    outTree->Branch("nbSAURON", &dataOut.nbSAURON, "nbSAURON/I");
    outTree->Branch("TSSAURON", &dataOut.TSSAURON, "TSSAURON/l");
    outTree->Branch("TSSAURON_Hit", &dataOut.TSSAURON_Hit,
                    "TSSAURON_Hit[nbSAURON]/l");
    outTree->Branch("SAURONChannel", &dataOut.SAURONChannel,
                    "SAURONChannel[nbSAURON]/s");
    outTree->Branch("SAURONBoard", &dataOut.SAURONBoard,
                    "SAURONBoard[nbSAURON]/s");
    outTree->Branch("SAURONEnergy", &dataOut.SAURONEnergy,
                    "SAURONEnergy[nbSAURON]/F");
    outTree->Branch("SAURONTime", &dataOut.SAURONTime,
                    "SAURONTime[nbSAURON]/F");

    ULong64_t nEntries = inTree->GetEntries();
    if(nEntries == 0) {
        std::cerr << "\nERROR: number of entries in " << inputfile
                  << " is zero" << std::endl;
        return -1;
    }
    if(it_max != -1 && nEntries > static_cast<ULong64_t>(it_max))
        nEntries = static_cast<ULong64_t>(it_max);

    ULong64_t nrEvts    = 0;
    ULong64_t iteration = 0;

    // read input data
    while(iteration < nEntries) {
        if(iteration % 100000 == 0)
            std::cout << "\r                                                  "
                         "          \r"
                      << iteration << "\t" << nrEvts << "\tREADING DATA"
                      << std::flush;
        // std::cout << iteration << std::endl;

        inTree->GetEntry(iteration);
        // dataIn.Timestamp += TimeOffset[dataIn.Board][dataIn.Channel] +
        // BoardTimeOffset[dataIn.Board];
        timeSortBuff.push_back(dataIn);

        if(timeSortBuff.size() > SortedBuffMaxSize) {
            dataOut.nbSAURON = 0;
            dataOut.TSSAURON = timeSortBuff[0].Timestamp;

            while(timeSortBuff[0].Timestamp
                  < dataOut.TSSAURON + TimeCoincWindow) {
                if(timeSortBuff[0].Timestamp < dataOut.TSSAURON) {
                    std::cerr << "\nERROR: input data is not time sorted"
                              << std::endl;
                    return -1;
                }

                dataOut.nbSAURON++;
                dataOut.TSSAURON_Hit[dataOut.nbSAURON - 1]
                    = timeSortBuff[0].Timestamp;
                dataOut.SAURONChannel[dataOut.nbSAURON - 1]
                    = timeSortBuff[0].Channel;
                dataOut.SAURONBoard[dataOut.nbSAURON - 1]
                    = timeSortBuff[0].Board;
                dataOut.SAURONEnergy[dataOut.nbSAURON - 1]
                    = timeSortBuff[0].Energy;

                timeSortBuff.pop_front();
                if(timeSortBuff.size() == 0) {
                    std::cout << "\nWARNING: time sorted buffer was emptied "
                                 "in the reading phase, time sorted buffer "
                                 "size may be too small"
                              << std::endl;
                }
            }
            outTree->Fill();
            nrEvts++;
        }

        iteration++;
    }

    // empty time sorted buffer
    while(timeSortBuff.size() > 0) {
        if(nrEvts % 10000 == 0)
            std::cout << "\r                                                  "
                         "          \r"
                      << iteration << "\t" << nrEvts
                      << "\tEMPTYING TIME SORTED BUFFER" << std::flush;
        // std::cout << iteration << std::endl;

        dataOut.nbSAURON = 0;
        dataOut.TSSAURON = timeSortBuff[0].Timestamp;

        while(timeSortBuff[0].Timestamp < dataOut.TSSAURON + TimeCoincWindow) {
            if(timeSortBuff[0].Timestamp < dataOut.TSSAURON) {
                std::cerr << "\nERROR: time sorting was not successfull, "
                             "please increase buffer size"
                          << std::endl;
                return -1;
            }

            dataOut.nbSAURON++;
            dataOut.TSSAURON_Hit[dataOut.nbSAURON - 1]
                = timeSortBuff[0].Timestamp;
            dataOut.SAURONChannel[dataOut.nbSAURON - 1]
                = timeSortBuff[0].Channel;
            dataOut.SAURONBoard[dataOut.nbSAURON - 1] = timeSortBuff[0].Board;
            dataOut.SAURONEnergy[dataOut.nbSAURON - 1]
                = timeSortBuff[0].Energy;

            timeSortBuff.pop_front();
            if(timeSortBuff.size() == 0) break;
        }
        outTree->Fill();
        nrEvts++;
    }
    std::cout << std::endl;

    std::cout << "Read input events:\t" << iteration
              << "\t\tProduced output events:\t" << nrEvts << std::endl;

    output->cd();
    outTree->Write();
    output->Write();
    output->Close();
}
