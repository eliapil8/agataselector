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
    // SAURON
    Int_t     nbSAURON = 0;
    ULong64_t TSSAURON = 0;
    ULong64_t TSSAURON_Hit[100];
    UShort_t  SAURONChannel[100];
    UShort_t  SAURONBoard[100];
    Float_t   SAURONEnergy[100];
    Float_t   SAURONTime[100];
    Float_t   SAURONCfd[100];
    Float_t   SAURONImax[100];
    Float_t   SAURONPid[100];

    // OSCAR
    Int_t     nbOSCAR = 0;
    ULong64_t TSOSCAR = 0;
    ULong64_t TSOSCAR_Hit[100];
    UShort_t  OSCARChannel[100];
    UShort_t  OSCARBoard[100];
    Float_t   OSCAREnergy[100];
    Float_t   OSCARTime[100];
    Float_t   OSCARCfd[100];
    Float_t   OSCARImax[100];
    Float_t   OSCARPid[100];

    // EXSSiDe
    Int_t       nbEXSSiDe=0;
    ULong64_t   TSEXSSiDe=0;
    ULong64_t   TSEXSSiDe_Hit[100];
    UShort_t    EXSSiDeChannel[100];
    UShort_t    EXSSiDeBoard[100];
    Float_t     EXSSiDeEnergy[100];
    Float_t     EXSSiDeTime[100];
    Float_t     EXSSiDeCfd[100];

    // EXOTIC MCP
    Int_t       nbMCP=0;
    ULong64_t   TSMCP=0;
    ULong64_t   TSMCP_Hit[100];
    UShort_t    MCPChannel[100];
    UShort_t    MCPBoard[100];
    Float_t     MCPEnergy[100];
    Float_t     MCPTime[100];    
        
    
};

struct DataIn {
    UShort_t  Channel   = 0;
    ULong64_t Timestamp = 0;
    UShort_t  Board     = 0;
    UShort_t  Energy    = 0;
    UInt_t    Flags     = 0;
};

const bool SauronOn = false;
const bool OscarOn  = false;
const bool ExssideOn = true;
const bool McpOn  = true;

int        main(int argc, char* argv[])
{
    bool                                   doTimestampCorrection = true;
    std::map<int, std::map<int, Long64_t>> TimeOffset;
    std::map<int, Long64_t>                BoardTimeOffset;
    BoardTimeOffset[0] = 0;
    BoardTimeOffset[1] = 0;

    // Board 0
    TimeOffset[1][0]   = 0;
    TimeOffset[1][16]  = 0;
    TimeOffset[1][32]  = 0;
    TimeOffset[1][48]  = 0;
    TimeOffset[1][1]   = 0;
    TimeOffset[1][17]  = 0;
    TimeOffset[1][33]  = 0;
    TimeOffset[1][49]  = 0;
    TimeOffset[1][2]   = 0;
    TimeOffset[1][18]  = 0;
    TimeOffset[1][34]  = 0;
    TimeOffset[1][50]  = 0;
    TimeOffset[1][3]   = 0;
    TimeOffset[1][19]  = 0;
    TimeOffset[1][35]  = 0;
    TimeOffset[1][51]  = 0;
    TimeOffset[1][4]   = 0;
    TimeOffset[1][20]  = 0;
    TimeOffset[1][36]  = 0;
    TimeOffset[1][52]  = 0;
    TimeOffset[1][5]   = 0;
    TimeOffset[1][21]  = 0;
    TimeOffset[1][37]  = 0;
    TimeOffset[1][53]  = 0;
    TimeOffset[1][6]   = 0;
    TimeOffset[1][22]  = 0;
    TimeOffset[1][38]  = 0;
    TimeOffset[1][54]  = 0;
    TimeOffset[1][7]   = 0;
    TimeOffset[1][23]  = 0;
    TimeOffset[1][39]  = 0;
    TimeOffset[1][55]  = 0;
    TimeOffset[1][8]   = 0;
    TimeOffset[1][24]  = 0;
    TimeOffset[1][40]  = 0;
    TimeOffset[1][56]  = 0;
    TimeOffset[1][9]   = 0;
    TimeOffset[1][25]  = 0;
    TimeOffset[1][41]  = 0;
    TimeOffset[1][57]  = 0;
    TimeOffset[1][10]  = 0;
    TimeOffset[1][26]  = 0;
    TimeOffset[1][42]  = 0;
    TimeOffset[1][58]  = 0;
    TimeOffset[1][11]  = 0;
    TimeOffset[1][27]  = 0;
    TimeOffset[1][43]  = 0;
    TimeOffset[1][59]  = 0;
    TimeOffset[1][12]  = 0;
    TimeOffset[1][28]  = 0;
    TimeOffset[1][44]  = 0;
    TimeOffset[1][60]  = 0;
    TimeOffset[1][13]  = 0;
    TimeOffset[1][29]  = 0;
    TimeOffset[1][45]  = 0;
    TimeOffset[1][61]  = 0;
    TimeOffset[1][14]  = 0;
    TimeOffset[1][30]  = 0;
    TimeOffset[1][46]  = 0;
    TimeOffset[1][62]  = 0;
    TimeOffset[1][15]  = 0;
    TimeOffset[1][31]  = 0;
    TimeOffset[1][47]  = 0;
    TimeOffset[1][63]  = 0;
    for(int i{0}; i<64; i++)
      TimeOffset[1][i] = 63360.900;
    // Board 1
    TimeOffset[0][0]   = 0;
    TimeOffset[0][1]   = -62676;
    TimeOffset[0][2]   = 0;
    TimeOffset[0][3]   = 0;
    TimeOffset[0][4]   = 0;
    TimeOffset[0][5]   = 0;
    TimeOffset[0][6]   = 0;
    TimeOffset[0][7]   = 0;
    TimeOffset[0][8]   = 0;
    //TimeOffset[0][9]   = 0;
    //TimeOffset[0][10]  = 0;
    //TimeOffset[0][11]  = 0;
    //TimeOffset[0][12]  = 0;
    //TimeOffset[0][13]  = 0;
    //TimeOffset[0][14]  = 0;
    //TimeOffset[0][15]  = 0;

    if(argc < 3) {
        std::cout << "\nERROR: not enough parameters!" << std::endl;
        std::cout << "CaenToTreeMaster inputFile outputFile [max_events]"
                  << std::endl;
        return -1;
    }

    std::map<int, std::map<int, std::deque<DataIn>>> dataInBuff;
 //   std::unordered_map<int<std::unordered_map<int,unsigned long long>> chanFirstTS;
    std::deque<DataIn>                               timeSortBuff;
    DataIn                                           dataIn;
    DataOut                                          dataOut;

    const int         InBuffMaxSize     = 100000000;
    const int         SortedBuffMaxSize = 50000000;

    // Time coincidence window for building of events, in TS units ( 500TS =
    // 1ns )
    const int         TimeCoincWindow   = 500*5000;

    const std::string outputfile        = argv[2];
    const std::string inputfile         = argv[1];
    int               it_max            = -1;
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

    if(SauronOn) {
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
        outTree->Branch("SAURONCfd", &dataOut.SAURONCfd,
                               "SAURONCfd[nbSAURON]/F");
        outTree->Branch("SAURONImax", &dataOut.SAURONImax,
                               "SAURONImax[nbSAURON]/F");
        outTree->Branch("SAURONPid", &dataOut.SAURONPid,
                               "SAURONPid[nbSAURON]/F");
    }

    if(OscarOn) {
        outTree->Branch("nbOSCAR", &dataOut.nbOSCAR, "nbOSCAR/I");
        outTree->Branch("TSOSCAR", &dataOut.TSOSCAR, "TSOSCAR/l");
        outTree->Branch("TSOSCAR_Hit", &dataOut.TSOSCAR_Hit,
                               "TSOSCAR_Hit[nbOSCAR]/l");
        outTree->Branch("OSCARChannel", &dataOut.OSCARChannel,
                               "OSCARChannel[nbOSCAR]/s");
        outTree->Branch("OSCARBoard", &dataOut.OSCARBoard,
                               "OSCARBoard[nbOSCAR]/s");
        outTree->Branch("OSCAREnergy", &dataOut.OSCAREnergy,
                               "OSCAREnergy[nbOSCAR]/F");
        outTree->Branch("OSCARTime", &dataOut.OSCARTime,
                               "OSCARTime[nbOSCAR]/F");
        outTree->Branch("OSCARCfd", &dataOut.OSCARCfd, "OSCARCfd[nbOSCAR]/F");
        outTree->Branch("OSCARImax", &dataOut.OSCARImax,
                               "OSCARImax[nbOSCAR]/F");
        outTree->Branch("OSCARPid", &dataOut.OSCARPid, "OSCARPid[nbOSCAR]/F");
    }

    if(ExssideOn) {
        outTree->Branch("nbEXSSiDe", &dataOut.nbEXSSiDe, "nbEXSSiDe/I");
        outTree->Branch("TSEXSSiDe", &dataOut.TSEXSSiDe, "TSEXSSiDe/l");
        outTree->Branch("TSEXSSiDe_Hit", &dataOut.TSEXSSiDe_Hit,
                               "TSEXSSiDe_Hit[nbEXSSiDe]/l");
        outTree->Branch("EXSSiDeChannel", &dataOut.EXSSiDeChannel,
                               "EXSSiDeChannel[nbEXSSiDe]/s");
        outTree->Branch("EXSSiDeBoard", &dataOut.EXSSiDeBoard,
                               "EXSSiDeBoard[nbEXSSiDe]/s");
        outTree->Branch("EXSSiDeEnergy", &dataOut.EXSSiDeEnergy,
                               "EXSSiDeEnergy[nbEXSSiDe]/F");
        outTree->Branch("EXSSiDeTime", &dataOut.EXSSiDeTime,
                               "EXSSiDeTime[nbEXSSiDe]/F");
        outTree->Branch("EXSSiDeCfd", &dataOut.EXSSiDeCfd,
                               "EXSSiDeCfd[nbEXSSiDe]/F");
    }

    if(McpOn) {
        outTree->Branch("nbMCP", &dataOut.nbMCP, "nbMCP/I");
        outTree->Branch("TSMCP", &dataOut.TSMCP, "TSMCP/l");
        outTree->Branch("TSMCP_Hit", &dataOut.TSMCP_Hit,
                               "TSMCP_Hit[nbMCP]/l");
        outTree->Branch("MCPChannel", &dataOut.MCPChannel,
                               "MCPChannel[nbMCP]/s");
        outTree->Branch("MCPBoard", &dataOut.MCPBoard,
                               "MCPBoard[nbMCP]/s");
        outTree->Branch("MCPEnergy", &dataOut.MCPEnergy,
                               "MCPEnergy[nbMCP]/F");
        outTree->Branch("MCPTime", &dataOut.MCPTime,
                               "MCPTime[nbMCP]/F");
    }

    int board0{99}, chan0{99};
    int count{1};
    ULong64_t T0;
    T0 = std::numeric_limits<ULong64_t>::max();

    ULong64_t nEntries = inTree->GetEntries();
    if(nEntries == 0) {
        std::cerr << "\nERROR: number of entries in " << inputfile
                  << " is zero" << std::endl;
        return -1;
    }
    if(it_max != -1 && nEntries > static_cast<ULong64_t>(it_max))
        nEntries = static_cast<ULong64_t>(it_max);

    ULong64_t minTS;
    ULong64_t nrEvts    = 0;
    ULong64_t iteration = 0;
    std::cout<<"Entries = "<<nEntries<<std::endl;
    // read input data
    while(iteration < nEntries) 
    {
        if(iteration % 200000 == 0)
            std::cout << "\r" << iteration << "\t" << timeSortBuff.size()
                      << "\t\t\t\t" << nrEvts << "\t\t\t\tREADING DATA" << std::flush;
       
        inTree->GetEntry(iteration);
        if(doTimestampCorrection)
            dataIn.Timestamp += TimeOffset[dataIn.Board][dataIn.Channel]
                              + BoardTimeOffset[dataIn.Board];
      
        // ***********************************************
        dataInBuff[dataIn.Board][dataIn.Channel].push_back(dataIn);


        while(dataInBuff[dataIn.Board][dataIn.Channel].size()
              > InBuffMaxSize)
        {
            minTS = std::numeric_limits<ULong64_t>::max();
            std::pair<int, int> minBrdChan;

            for(auto& iter : dataInBuff) {
                for(auto& it : iter.second) {
                    if(it.second.size() == 0) continue;
                    if(it.second.front().Timestamp < minTS) {
                        minTS             = it.second.front().Timestamp;
                        minBrdChan.first  = it.second.front().Board;
                        minBrdChan.second = it.second.front().Channel;
                    }
                }
            }
            if(count!=0){
                T0 = minTS;
                board0 = minBrdChan.first;
                chan0 = minBrdChan.second;
                std::cout<<std::endl;
                std::cout<<"Min TS = "<<T0<<" for Board "<<board0<<" and Channel "<<chan0<<std::endl;
                count=0;
            }
            timeSortBuff.push_back(
                dataInBuff[minBrdChan.first][minBrdChan.second].front());
            dataInBuff[minBrdChan.first][minBrdChan.second].pop_front();
            
            if(timeSortBuff.size() > SortedBuffMaxSize) {
                if(SauronOn) {
                    dataOut.nbSAURON = 0;
                    dataOut.TSSAURON = timeSortBuff[0].Timestamp;

                    while(timeSortBuff[0].Timestamp
                          < dataOut.TSSAURON + TimeCoincWindow) {
                        if(timeSortBuff[0].Timestamp < dataOut.TSSAURON) {
                            std::cerr
                                << "\nERROR: time sorting was not "
                                          "successfull, please increase buffer size"
                                << std::endl;
                            std::cerr << "timeSortBuff[0].Timestamp "
                                      << timeSortBuff[0].Timestamp << "\t"
                                      << "dataOut.TSSAURON "
                                      << dataOut.TSSAURON << std::endl;
                            std::cerr << "nrEvts " << nrEvts << std::endl;
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
                            std::cout << "\nWARNING: time sorted buffer was "
                                                "emptied in the reading phase, time "
                                                "sorted buffer size may be too small"
                                      << std::endl; 
                        }
                    }
                }

                if(OscarOn) {
                    dataOut.nbOSCAR = 0;
                    dataOut.TSOSCAR = timeSortBuff[0].Timestamp;

                    while(timeSortBuff[0].Timestamp
                          < dataOut.TSOSCAR + TimeCoincWindow) {
                        if(timeSortBuff[0].Timestamp < dataOut.TSOSCAR) {
                            std::cerr
                                << "\nERROR: time sorting was not "
                                          "successfull, please increase buffer size"
                                << std::endl;
                            std::cerr << "timeSortBuff[0].Timestamp "
                                      << timeSortBuff[0].Timestamp << "\t"
                                      << "dataOut.TSOSCAR " << dataOut.TSOSCAR
                                      << std::endl;
                            std::cerr << "nrEvts " << nrEvts << std::endl;
                            return -1;
                        }

                        dataOut.nbOSCAR++;
                        dataOut.TSOSCAR_Hit[dataOut.nbOSCAR - 1]
                            = timeSortBuff[0].Timestamp;
                        dataOut.OSCARChannel[dataOut.nbOSCAR - 1]
                            = timeSortBuff[0].Channel;
                        dataOut.OSCARBoard[dataOut.nbOSCAR - 1]
                            = timeSortBuff[0].Board;
                        dataOut.OSCAREnergy[dataOut.nbOSCAR - 1]
                            = timeSortBuff[0].Energy;

                        timeSortBuff.pop_front();
                        if(timeSortBuff.size() == 0) {
                            std::cout << "\nWARNING: time sorted buffer was "
                                                "emptied in the reading phase, time "
                                                "sorted buffer size may be too small"
                                      << std::endl;
                        }
                    }
                }

                if(ExssideOn && McpOn)
                {
                    dataOut.nbEXSSiDe = 0;
                    dataOut.nbMCP = 0;

                    if(timeSortBuff[0].Board==1)
                    {
                        dataOut.TSEXSSiDe = timeSortBuff[0].Timestamp;

                        while(timeSortBuff[0].Timestamp
                              < dataOut.TSEXSSiDe + TimeCoincWindow) {
                            if(timeSortBuff[0].Timestamp < T0 or timeSortBuff[0].Timestamp < dataOut.TSEXSSiDe) {
                                std::cerr
                                    << "\nERROR: time sorting was not "
                                              "successfull, please increase buffer size"
                                    << std::endl;
                                std::cerr << "timeSortBuff[0].Timestamp "
                                          << timeSortBuff[0].Timestamp << "\t" << "Chan (Board)"<<timeSortBuff[0].Channel
                                          <<" "<<timeSortBuff[0].Board
                                          << "T0 " << T0 
                                          << "\tdataOut.TSEXSSiDe " << dataOut.TSEXSSiDe << std::endl;
                                std::cerr << "nrEvts " << nrEvts << std::endl;
                                return -1;
                            }


                            if(timeSortBuff[0].Board==1){
                                dataOut.nbEXSSiDe++;
                                dataOut.TSEXSSiDe_Hit[dataOut.nbEXSSiDe - 1]
                                    = timeSortBuff[0].Timestamp;
                                dataOut.EXSSiDeTime[dataOut.nbEXSSiDe - 1]
                                    = 0.;//dataOut.TSEXSSiDe_Hit[dataOut.nbEXSSiDe-1] - dataOut.TSEXSSiDe;
                                dataOut.EXSSiDeChannel[dataOut.nbEXSSiDe - 1]
                                    = timeSortBuff[0].Channel;
                                dataOut.EXSSiDeBoard[dataOut.nbEXSSiDe - 1]
                                    = timeSortBuff[0].Board;
                                dataOut.EXSSiDeEnergy[dataOut.nbEXSSiDe - 1]
                                    = timeSortBuff[0].Energy;
    
                                timeSortBuff.pop_front();
                            }else{
                                dataOut.nbMCP++;
                                dataOut.TSMCP_Hit[dataOut.nbMCP - 1]
                                    = timeSortBuff[0].Timestamp;
                                dataOut.MCPTime[dataOut.nbMCP - 1]
				  = 0.;//dataOut.TSMCP_Hit[dataOut.nbMCP-1] - dataOut.TSEXSSiDe;
                                dataOut.MCPChannel[dataOut.nbMCP - 1]
                                    = timeSortBuff[0].Channel;
                                dataOut.MCPBoard[dataOut.nbMCP - 1]
                                    = timeSortBuff[0].Board;
                                dataOut.MCPEnergy[dataOut.nbMCP - 1]
                                    = timeSortBuff[0].Energy;

                                timeSortBuff.pop_front();
                            }
                            if(timeSortBuff.size() == 0) 
                            {
                            std::cout << "\nWARNING: time sorted buffer was "
                                                "emptied in the reading phase, time "
                                                "sorted buffer size may be too small"
                                      << std::endl;
                            break;
                            }
                        }

                        
                    } else if(timeSortBuff[0].Board==0)
                    {

                        dataOut.TSMCP = timeSortBuff[0].Timestamp;

                        while(timeSortBuff[0].Timestamp
                          < dataOut.TSMCP + TimeCoincWindow) {
                            if(timeSortBuff[0].Timestamp < T0 or timeSortBuff[0].Timestamp < dataOut.TSMCP) {
                                std::cerr
                                    << "\nERROR: time sorting was not "
                                              "successfull, please increase buffer size"
                                    << std::endl;
                                std::cerr << "timeSortBuff[0].Timestamp "
                                          << timeSortBuff[0].Timestamp << "\t"
                                          << timeSortBuff[0].Timestamp << "\t" << "Chan (Board)"<<timeSortBuff[0].Channel
                                          <<" "<<timeSortBuff[0].Board
                                        << "T0 " << T0
                                        << "\tdataOut.TSMCP " << dataOut.TSMCP << std::endl;
                                std::cerr << "nrEvts " << nrEvts << std::endl;
                                return -1;
                            }
                            if(timeSortBuff[0].Board==1){
                                    dataOut.nbEXSSiDe++;
                                    dataOut.TSEXSSiDe_Hit[dataOut.nbEXSSiDe - 1]
                                        = timeSortBuff[0].Timestamp;
                                    dataOut.EXSSiDeTime[dataOut.nbEXSSiDe - 1]
				      = 0.;//dataOut.TSEXSSiDe_Hit[dataOut.nbEXSSiDe-1] - dataOut.TSMCP;
                                    dataOut.EXSSiDeChannel[dataOut.nbEXSSiDe - 1]
                                        = timeSortBuff[0].Channel;
                                    dataOut.EXSSiDeBoard[dataOut.nbEXSSiDe - 1]
                                        = timeSortBuff[0].Board;
                                    dataOut.EXSSiDeEnergy[dataOut.nbEXSSiDe - 1]
                                        = timeSortBuff[0].Energy;

                                    timeSortBuff.pop_front();
                            }else{
                                dataOut.nbMCP++;
                                dataOut.TSMCP_Hit[dataOut.nbMCP - 1]
                                    = timeSortBuff[0].Timestamp;
                                dataOut.MCPTime[dataOut.nbMCP - 1]
				  = 0.;//dataOut.TSMCP_Hit[dataOut.nbMCP-1] - dataOut.TSMCP;
                                dataOut.MCPChannel[dataOut.nbMCP - 1]
                                    = timeSortBuff[0].Channel;
                                dataOut.MCPBoard[dataOut.nbMCP - 1]
                                    = timeSortBuff[0].Board;
                                dataOut.MCPEnergy[dataOut.nbMCP - 1]
                                    = timeSortBuff[0].Energy;

                                timeSortBuff.pop_front();
                            }

                            if(timeSortBuff.size() == 0) 
                            {
                            std::cout << "\nWARNING: time sorted buffer was "
                                                "emptied in the reading phase, time "
                                                "sorted buffer size may be too small"
                                      << std::endl;
                            break;
                            }
                        }

                    }
                        
                    
                
                }

                outTree->Fill();
                nrEvts++;
            }
        }
        iteration++;
    }

    // empty time unsorted buffer
    std::cout<<"\n"<<std::endl;
    std::cout<<"Second"<<std::endl;
    std::cout<<"Min TS = "<<T0<<" for Board "<<board0<<" and Channel "<<chan0<<std::endl;
    std::cout<<"Size buffer = "<<dataInBuff[dataIn.Board][dataIn.Channel].size()<<std::endl;
    std::cout<<"Size timeSortBuff = "<<timeSortBuff.size() <<std::endl;
    while(dataInBuff[dataIn.Board][dataIn.Channel].size() > 0) {
        if(timeSortBuff.size() % 200000 == 0)
            std::cout << "\r" << dataInBuff[dataIn.Board][dataIn.Channel].size() << "\t" << timeSortBuff.size()
                      << "\t\t\t\t" << nrEvts << "\t\t\t\tEMPTYING TIME UNSORTED BUFFER"
                      << std::flush;
        // std::cout << iteration << std::endl;

        minTS = std::numeric_limits<ULong64_t>::max();
        std::pair<int, int> minBrdChan;

        for(auto& iter : dataInBuff) {
            for(auto& it : iter.second) {
                if(it.second.size() == 0) continue;
                if(it.second.front().Timestamp < minTS) {
                    minTS             = it.second.front().Timestamp;
                    minBrdChan.first  = it.second.front().Board;
                    minBrdChan.second = it.second.front().Channel;
                }
            }
        }
        if(count!=0){
                T0 = minTS;
                board0 = minBrdChan.first;
                chan0 = minBrdChan.second;
                std::cout<<std::endl;
                std::cout<<"Min TS = "<<T0<<" for Board "<<board0<<" and Channel "<<chan0<<std::endl;
                count=0;
            }

        timeSortBuff.push_back(
            dataInBuff[minBrdChan.first][minBrdChan.second].front());
        dataInBuff[minBrdChan.first][minBrdChan.second].pop_front();

        if(timeSortBuff.size() > SortedBuffMaxSize) {
            if(SauronOn) {
                dataOut.nbSAURON = 0;
                dataOut.TSSAURON = timeSortBuff[0].Timestamp;

                while(timeSortBuff[0].Timestamp
                      < dataOut.TSSAURON + TimeCoincWindow) {
                    if(timeSortBuff[0].Timestamp < dataOut.TSSAURON) {
                        std::cerr << "\nERROR: time sorting was not "
                                            "successfull, please increase buffer size"
                                  << std::endl;
                        std::cerr << "timeSortBuff[0].Timestamp "
                                  << timeSortBuff[0].Timestamp << "\t"
                                  << "dataOut.TSSAURON " << dataOut.TSSAURON
                                  << std::endl;
                        std::cerr << "nrEvts " << nrEvts << std::endl;
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
                        std::cout << "\nWARNING: time sorted buffer was "
                                            "emptied in the reading phase, time "
                                            "sorted buffer size may be too small"
                                  << std::endl;
                    }
                }
            }

            if(OscarOn) {
                dataOut.nbOSCAR = 0;
                dataOut.TSOSCAR = timeSortBuff[0].Timestamp;

                while(timeSortBuff[0].Timestamp
                      < dataOut.TSOSCAR + TimeCoincWindow) {
                    if(timeSortBuff[0].Timestamp < dataOut.TSOSCAR) {
                        std::cerr << "\nERROR: time sorting was not "
                                            "successfull, please increase buffer size"
                                  << std::endl;
                        std::cerr << "timeSortBuff[0].Timestamp "
                                  << timeSortBuff[0].Timestamp << "\t"
                                  << "dataOut.TSOSCAR " << dataOut.TSOSCAR
                                  << std::endl;
                        std::cerr << "nrEvts " << nrEvts << std::endl;
                        return -1;
                    }

                    dataOut.nbOSCAR++;
                    dataOut.TSOSCAR_Hit[dataOut.nbOSCAR - 1]
                        = timeSortBuff[0].Timestamp;
                    dataOut.OSCARChannel[dataOut.nbOSCAR - 1]
                        = timeSortBuff[0].Channel;
                    dataOut.OSCARBoard[dataOut.nbOSCAR - 1]
                        = timeSortBuff[0].Board;
                    dataOut.OSCAREnergy[dataOut.nbOSCAR - 1]
                        = timeSortBuff[0].Energy;

                    timeSortBuff.pop_front();
                    if(timeSortBuff.size() == 0) {
                        std::cout << "\nWARNING: time sorted buffer was "
                                            "emptied in the reading phase, time "
                                            "sorted buffer size may be too small"
                                  << std::endl;
                        break;
                    }
                }
            }

            if(ExssideOn && McpOn)
                {
                    dataOut.nbEXSSiDe = 0;
                    dataOut.nbMCP = 0;

                    if(timeSortBuff[0].Board==1)
                    {
                        dataOut.TSEXSSiDe = timeSortBuff[0].Timestamp;

                        while(timeSortBuff[0].Timestamp
                              < dataOut.TSEXSSiDe + TimeCoincWindow) {
                            if(timeSortBuff[0].Timestamp < T0 or timeSortBuff[0].Timestamp < dataOut.TSEXSSiDe) {
                                std::cerr
                                    << "\nERROR: time sorting was not "
                                              "successfull, please increase buffer size"
                                    << std::endl;
                                std::cerr << "timeSortBuff[0].Timestamp "
                                          << timeSortBuff[0].Timestamp << "\t" << "Chan (Board) "<<timeSortBuff[0].Channel
                                          <<" "<<timeSortBuff[0].Board
                                          << " T0 " << T0 
                                          << "\tdataOut.TSEXSSiDe " << dataOut.TSEXSSiDe << std::endl;
                                std::cerr << "nrEvts " << nrEvts << std::endl;
                                return -1;
                            }


                            if(timeSortBuff[0].Board==1){
                                dataOut.nbEXSSiDe++;
                                dataOut.TSEXSSiDe_Hit[dataOut.nbEXSSiDe - 1]
                                    = timeSortBuff[0].Timestamp;
                                dataOut.EXSSiDeTime[dataOut.nbEXSSiDe - 1]
                                    = 0.;//dataOut.TSEXSSiDe_Hit[dataOut.nbEXSSiDe-1] - dataOut.TSEXSSiDe;

                                dataOut.EXSSiDeChannel[dataOut.nbEXSSiDe - 1]
                                    = timeSortBuff[0].Channel;
                                dataOut.EXSSiDeBoard[dataOut.nbEXSSiDe - 1]
                                    = timeSortBuff[0].Board;
                                dataOut.EXSSiDeEnergy[dataOut.nbEXSSiDe - 1]
                                    = timeSortBuff[0].Energy;
    
                                timeSortBuff.pop_front();
                            }else{
                                dataOut.nbMCP++;
                                dataOut.TSMCP_Hit[dataOut.nbMCP - 1]
                                    = timeSortBuff[0].Timestamp;
				dataOut.MCPTime[dataOut.nbMCP - 1]
				  = 0.;//dataOut.TSMCP_Hit[dataOut.nbMCP-1] - dataOut.TSEXSSiDe;

                                dataOut.MCPChannel[dataOut.nbMCP - 1]
                                    = timeSortBuff[0].Channel;
                                dataOut.MCPBoard[dataOut.nbMCP - 1]
                                    = timeSortBuff[0].Board;
                                dataOut.MCPEnergy[dataOut.nbMCP - 1]
                                    = timeSortBuff[0].Energy;

                                timeSortBuff.pop_front();
                            }
                            if(timeSortBuff.size() == 0) 
                            {
                            std::cout << "\nWARNING: time sorted buffer was "
                                                "emptied in the reading phase, time "
                                                "sorted buffer size may be too small"
                                      << std::endl;
                            break;
                            }
                        }

                        
                    } else if(timeSortBuff[0].Board==0)
                    {

                        dataOut.TSMCP = timeSortBuff[0].Timestamp;

                        while(timeSortBuff[0].Timestamp
                          < dataOut.TSMCP + TimeCoincWindow) {
                            if(timeSortBuff[0].Timestamp < T0 or timeSortBuff[0].Timestamp < dataOut.TSMCP) {
                                std::cerr
                                    << "\nERROR: time sorting was not "
                                              "successfull, please increase buffer size"
                                    << std::endl;
                                std::cerr << "timeSortBuff[0].Timestamp "
                                          << timeSortBuff[0].Timestamp << "\t"
                                          << timeSortBuff[0].Timestamp << "\t" << "Chan (Board) "<<timeSortBuff[0].Channel
                                          <<" "<<timeSortBuff[0].Board
                                        << " T0 " << T0
                                        << "\tdataOut.TSMCP " << dataOut.TSMCP << std::endl;
                                std::cerr << "nrEvts " << nrEvts << std::endl;
                                return -1;
                            }
                            if(timeSortBuff[0].Board==1){
                                    dataOut.nbEXSSiDe++;
                                    dataOut.TSEXSSiDe_Hit[dataOut.nbEXSSiDe - 1]
                                        = timeSortBuff[0].Timestamp;
				    dataOut.EXSSiDeTime[dataOut.nbEXSSiDe - 1]
				      = 0.;//dataOut.TSEXSSiDe_Hit[dataOut.nbEXSSiDe-1] - dataOut.TSMCP;

                                    dataOut.EXSSiDeChannel[dataOut.nbEXSSiDe - 1]
                                        = timeSortBuff[0].Channel;
                                    dataOut.EXSSiDeBoard[dataOut.nbEXSSiDe - 1]
                                        = timeSortBuff[0].Board;
                                    dataOut.EXSSiDeEnergy[dataOut.nbEXSSiDe - 1]
                                        = timeSortBuff[0].Energy;

                                    timeSortBuff.pop_front();
                            }else{
                                dataOut.nbMCP++;
                                dataOut.TSMCP_Hit[dataOut.nbMCP - 1]
                                    = timeSortBuff[0].Timestamp;
				dataOut.MCPTime[dataOut.nbMCP - 1]
				  = 0.;//dataOut.TSMCP_Hit[dataOut.nbMCP-1] - dataOut.TSMCP;

                                dataOut.MCPChannel[dataOut.nbMCP - 1]
                                    = timeSortBuff[0].Channel;
                                dataOut.MCPBoard[dataOut.nbMCP - 1]
                                    = timeSortBuff[0].Board;
                                dataOut.MCPEnergy[dataOut.nbMCP - 1]
                                    = timeSortBuff[0].Energy;

                                timeSortBuff.pop_front();
                            }
                            if(timeSortBuff.size() == 0) 
                            {
                            std::cout << "\nWARNING: time sorted buffer was "
                                                "emptied in the reading phase, time "
                                                "sorted buffer size may be too small"
                                      << std::endl;
                            break;
                            }
                        }

                    }
                   
                }
            


            outTree->Fill();
            nrEvts++;
        }
    }

    //nrEvts=0;
    std::cout<<"\n"<<std::endl;
    std::cout<<"Last"<<std::endl;
    std::cout<<"Size timeSortBuff = "<<timeSortBuff.size()<<std::endl;
    // empty time sorted buffer
    while(timeSortBuff.size() > 0) {
        if(nrEvts % 100000 == 0)
            std::cout << "\r" << dataInBuff[dataIn.Board][dataIn.Channel].size() << "\t" << timeSortBuff.size()
                      << "\t\t\t\t" << nrEvts << "\t\t\t\tEMPTYING TIME SORTED BUFFER"
                      << std::flush;

        if(SauronOn) {
            dataOut.nbSAURON = 0;
            dataOut.TSSAURON = timeSortBuff[0].Timestamp;

            while(timeSortBuff[0].Timestamp
                  < dataOut.TSSAURON + TimeCoincWindow) {
                if(timeSortBuff[0].Timestamp < dataOut.TSSAURON) {
                    std::cerr << "\nERROR: time sorting was not successfull, "
                                        "please increase buffer size"
                              << std::endl;
                    std::cerr << "timeSortBuff[0].Timestamp "
                              << timeSortBuff[0].Timestamp << "\t"
                              << "dataOut.TSSAURON " << dataOut.TSSAURON
                              << std::endl;
                    std::cerr << "nrEvts " << nrEvts << std::endl;
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
                if(timeSortBuff.size() == 0) break;
            }
        }

        if(OscarOn) {
            dataOut.nbOSCAR = 0;
            dataOut.TSOSCAR = timeSortBuff[0].Timestamp;

            while(timeSortBuff[0].Timestamp
                  < dataOut.TSOSCAR + TimeCoincWindow) {
                if(timeSortBuff[0].Timestamp < dataOut.TSOSCAR) {
                    std::cerr << "\nERROR: time sorting was not successfull, "
                                        "please increase buffer size"
                              << std::endl;
                    std::cerr << "timeSortBuff[0].Timestamp "
                              << timeSortBuff[0].Timestamp << "\t"
                              << "dataOut.TSOSCAR " << dataOut.TSOSCAR
                              << std::endl;
                    std::cerr << "nrEvts " << nrEvts << std::endl;
                    return -1;
                }

                dataOut.nbOSCAR++;
                dataOut.TSOSCAR_Hit[dataOut.nbOSCAR - 1]
                    = timeSortBuff[0].Timestamp;
                dataOut.OSCARChannel[dataOut.nbOSCAR - 1]
                    = timeSortBuff[0].Channel;
                dataOut.OSCARBoard[dataOut.nbOSCAR - 1]
                    = timeSortBuff[0].Board;
                dataOut.OSCAREnergy[dataOut.nbOSCAR - 1]
                    = timeSortBuff[0].Energy;

                timeSortBuff.pop_front();
                if(timeSortBuff.size() == 0) break;
            }
        }

        if(ExssideOn && McpOn)
                {
                    dataOut.nbEXSSiDe = 0;
                    dataOut.nbMCP = 0;
                    int br, ch;
                    br = dataIn.Board;
                    ch = dataIn.Channel;
                    if(timeSortBuff[0].Board==1)
                    {
                        dataOut.TSEXSSiDe = timeSortBuff[0].Timestamp;

                        while(timeSortBuff[0].Timestamp
                              < dataOut.TSEXSSiDe + TimeCoincWindow) {
                            if(timeSortBuff[0].Timestamp < T0 or timeSortBuff[0].Timestamp < dataOut.TSEXSSiDe) {
                                std::cerr
                                    << "\nERROR: time sorting was not "
                                              "successfull, please increase buffer size"
                                    << std::endl;
                                std::cerr << "timeSortBuff[0].Timestamp "
                                          << timeSortBuff[0].Timestamp << "\t" << "Chan (Board) "<<timeSortBuff[0].Channel
                                          <<" "<<timeSortBuff[0].Board
                                          << " T0 " << T0 
                                          << "\tdataOut.TSEXSSiDe " << dataOut.TSEXSSiDe 
                                          << " Board " << br <<" Channel "<< ch 
                                          << std::endl;
                                std::cerr << "nrEvts " << nrEvts << std::endl;
                                return -1;
                            }


                            if(timeSortBuff[0].Board==1){
                                dataOut.nbEXSSiDe++;
                                dataOut.TSEXSSiDe_Hit[dataOut.nbEXSSiDe - 1]
                                    = timeSortBuff[0].Timestamp;
				dataOut.EXSSiDeTime[dataOut.nbEXSSiDe - 1]
                                    = 0.;//dataOut.TSEXSSiDe_Hit[dataOut.nbEXSSiDe-1] - dataOut.TSEXSSiDe;

				dataOut.EXSSiDeChannel[dataOut.nbEXSSiDe - 1]
				  = timeSortBuff[0].Channel;
                                dataOut.EXSSiDeBoard[dataOut.nbEXSSiDe - 1]
                                    = timeSortBuff[0].Board;
                                dataOut.EXSSiDeEnergy[dataOut.nbEXSSiDe - 1]
                                    = timeSortBuff[0].Energy;
    
                                timeSortBuff.pop_front();
                            }else{
                                dataOut.nbMCP++;
                                dataOut.TSMCP_Hit[dataOut.nbMCP - 1]
                                    = timeSortBuff[0].Timestamp;
				dataOut.MCPTime[dataOut.nbMCP - 1]
				  = 0.;//dataOut.TSMCP_Hit[dataOut.nbMCP-1] - dataOut.TSEXSSiDe;

				dataOut.MCPChannel[dataOut.nbMCP - 1]
                                    = timeSortBuff[0].Channel;
                                dataOut.MCPBoard[dataOut.nbMCP - 1]
                                    = timeSortBuff[0].Board;
                                dataOut.MCPEnergy[dataOut.nbMCP - 1]
                                    = timeSortBuff[0].Energy;

                                timeSortBuff.pop_front();
                            }
                            if(timeSortBuff.size() == 0) 
                            {
                            std::cout << "\nWARNING: time sorted buffer was "
                                                "emptied in the reading phase, time "
                                                "sorted buffer size may be too small"
                                      << std::endl;
                            break;
                            }

                        }

                        
                    } else if(timeSortBuff[0].Board==0)
                    {

                        dataOut.TSMCP = timeSortBuff[0].Timestamp;

                        while(timeSortBuff[0].Timestamp
                          < dataOut.TSMCP + TimeCoincWindow) {
                            if(timeSortBuff[0].Timestamp < T0 or timeSortBuff[0].Timestamp < dataOut.TSMCP) {
                                std::cerr
                                    << "\nERROR: time sorting was not "
                                              "successfull, please increase buffer size"
                                    << std::endl;
                                std::cerr << "timeSortBuff[0].Timestamp "
                                          << timeSortBuff[0].Timestamp << "\t"
                                          << timeSortBuff[0].Timestamp << "\t" << "Chan (Board) "<<timeSortBuff[0].Channel
                                          <<" "<<timeSortBuff[0].Board
                                        << " T0 " << T0
                                        << "\tdataOut.TSMCP " << dataOut.TSMCP  
                                          << " Board " << br <<" Channel "<< ch 
                                          << std::endl;
                                std::cerr << "nrEvts " << nrEvts << std::endl;
                                return -1;
                            }
                            if(timeSortBuff[0].Board==1){
                                    dataOut.nbEXSSiDe++;
                                    dataOut.TSEXSSiDe_Hit[dataOut.nbEXSSiDe - 1]
                                        = timeSortBuff[0].Timestamp;
				    dataOut.EXSSiDeTime[dataOut.nbEXSSiDe - 1]
                                    = 0.;//dataOut.TSEXSSiDe_Hit[dataOut.nbEXSSiDe-1] - dataOut.TSMCP;

                                    dataOut.EXSSiDeChannel[dataOut.nbEXSSiDe - 1]
                                        = timeSortBuff[0].Channel;
                                    dataOut.EXSSiDeBoard[dataOut.nbEXSSiDe - 1]
                                        = timeSortBuff[0].Board;
                                    dataOut.EXSSiDeEnergy[dataOut.nbEXSSiDe - 1]
                                        = timeSortBuff[0].Energy;

                                    timeSortBuff.pop_front();
                            }else{
                                dataOut.nbMCP++;
                                dataOut.TSMCP_Hit[dataOut.nbMCP - 1]
                                    = timeSortBuff[0].Timestamp;
                                dataOut.MCPChannel[dataOut.nbMCP - 1]
                                    = timeSortBuff[0].Channel;
				dataOut.MCPTime[dataOut.nbMCP - 1]
                                    = 0.;//dataOut.TSMCP_Hit[dataOut.nbMCP-1] - dataOut.TSMCP;

                                dataOut.MCPBoard[dataOut.nbMCP - 1]
                                    = timeSortBuff[0].Board;
                                dataOut.MCPEnergy[dataOut.nbMCP - 1]
                                    = timeSortBuff[0].Energy;

                                timeSortBuff.pop_front();
                            }
                            if(timeSortBuff.size() == 0) 
                            {
                            std::cout << "\nWARNING: time sorted buffer was "
                                                "emptied in the reading phase, time "
                                                "sorted buffer size may be too small"
                                      << std::endl;
                            break;
                            }
                        }

                    }
                     
                }
            

        outTree->Fill();
        nrEvts++;
    }
    std::cout << std::endl;
    std::cout<<"Size timeSortBuff = "<<timeSortBuff.size() <<std::endl;
  
    std::cout << "Read input events:\t" << iteration
              << "\t\tProduced output events:\t" << nrEvts << std::endl;

    output->cd();
    outTree->Write();
    output->Write();
    output->Close();
}
