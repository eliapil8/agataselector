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

// to compile: g++ -Wall `root-config --cflags` -o SimToTreeMaster *.cxx `root-config --libs`

struct DataOut {
  // Detector1 structure
  Int_t     nbDetector1 = 0;
  ULong64_t TSDetector1 = 0;
  ULong64_t TSDetector1_Hit[100];
  UShort_t  Detector1Channel[100];
  UShort_t  Detector1Board[100];
  Float_t   Detector1Energy[100];
  Float_t   Detector1Time[100];
  Float_t   Detector1Cfd[100];
  Float_t   Detector1Imax[100];
  Float_t   Detector1Pid[100];
  
  // Detector1 structure
  Int_t     nbDetector2 = 0;
  ULong64_t TSDetector2 = 0;
  ULong64_t TSDetector2_Hit[100];
  UShort_t  Detector2Channel[100];
  UShort_t  Detector2Board[100];
  Float_t   Detector2Energy[100];
  Float_t   Detector2Time[100];
  Float_t   Detector2Cfd[100];
  Float_t   Detector2Imax[100];
  Float_t   Detector2Pid[100];
  
};

struct DataIn {
    UShort_t  Channel   = 0;
    ULong64_t Timestamp = 0;
    UShort_t  Board     = 0;
    UShort_t  Energy    = 0;
    UInt_t    Flags     = 0;
};

void treeOut(int detnr, std::string sdet1, std::string sdet2, TTree *tree, DataOut &dOut){

      
        tree->Branch(Form("nb%s",sdet1.c_str()), &dOut.nbDetector1, Form("nb%s/I",sdet1.c_str()));
        tree->Branch(Form("TS%s",sdet1.c_str()), &dOut.TSDetector1, Form("TS%s/l",sdet1.c_str()));
        tree->Branch(Form("TS%s_Hit",sdet1.c_str()), &dOut.TSDetector1_Hit,
                           Form("TS%s_Hit[nb%s]/l",sdet1.c_str(),sdet1.c_str()));
        tree->Branch(Form("%sChannel",sdet1.c_str()), &dOut.Detector1Channel,
                           Form("%sChannel[nb%s]/s",sdet1.c_str(),sdet1.c_str()));
        tree->Branch(Form("%sBoard",sdet1.c_str()), &dOut.Detector1Board,
                           Form("%sBoard[nb%s]/s",sdet1.c_str(),sdet1.c_str()));
        tree->Branch(Form("%sEnergy",sdet1.c_str()), &dOut.Detector1Energy,
                           Form("%sEnergy[nb%s]/F",sdet1.c_str(),sdet1.c_str()));
        tree->Branch(Form("%sTime",sdet1.c_str()), &dOut.Detector1Time,
                           Form("%sTime[nb%s]/F",sdet1.c_str(),sdet1.c_str()));
        tree->Branch(Form("%sCfd",sdet1.c_str()), &dOut.Detector1Cfd,
                           Form("%sCfd[nb%s]/F",sdet1.c_str(),sdet1.c_str()));
        if(sdet1=="SAURON" or sdet1 == "OSCAR"){
            tree->Branch(Form("%sImax",sdet1.c_str()), &dOut.Detector1Imax,
                               Form("%sImax[nb%s]/F",sdet1.c_str(),sdet1.c_str()));
            tree->Branch(Form("%sPid",sdet1.c_str()), &dOut.Detector1Pid,
                               Form("%sPid[nb%s]/F",sdet1.c_str(),sdet1.c_str()));
        }
        if(detnr>1){
            
            tree->Branch(Form("nb%s",sdet2.c_str()), &dOut.nbDetector2, Form("nb%s/I",sdet2.c_str()));
            tree->Branch(Form("TS%s",sdet2.c_str()), &dOut.TSDetector2, Form("TS%s/l",sdet2.c_str()));
            tree->Branch(Form("TS%s_Hit",sdet2.c_str()), &dOut.TSDetector2_Hit,
                               Form("TS%s_Hit[nb%s]/l",sdet2.c_str(),sdet2.c_str()));
            tree->Branch(Form("%sChannel",sdet2.c_str()), &dOut.Detector2Channel,
                               Form("%sChannel[nb%s]/s",sdet2.c_str(),sdet2.c_str()));
            tree->Branch(Form("%sBoard",sdet2.c_str()), &dOut.Detector2Board,
                               Form("%sBoard[nb%s]/s",sdet2.c_str(),sdet2.c_str()));
            tree->Branch(Form("%sEnergy",sdet2.c_str()), &dOut.Detector2Energy,
                               Form("%sEnergy[nb%s]/F",sdet2.c_str(),sdet2.c_str()));
            tree->Branch(Form("%sTime",sdet2.c_str()), &dOut.Detector2Time,
                               Form("%sTime[nb%s]/F",sdet2.c_str(),sdet2.c_str()));
            tree->Branch(Form("%sCfd",sdet2.c_str()), &dOut.Detector2Cfd,
                               Form("%sCfd[nb%s]/F",sdet2.c_str(),sdet2.c_str()));
            if(sdet2=="SAURON" or sdet2=="OSCAR"){
                tree->Branch(Form("%sImax",sdet2.c_str()), &dOut.Detector2Imax,
                                   Form("%sImax[nb%s]/F",sdet2.c_str(),sdet2.c_str()));
                tree->Branch(Form("%sPid",sdet2.c_str()), &dOut.Detector2Pid,
                                   Form("%sPid[nb%s]/F",sdet2.c_str(),sdet2.c_str()));
            }   


        }
        
};


int        main(int argc, char* argv[])
{
    bool                                   doTimestampCorrection = true;
    std::map<int, std::map<int, Long64_t>> TimeOffset;
    std::map<int, Long64_t>                BoardTimeOffset;
    BoardTimeOffset[0] = 0;
    BoardTimeOffset[1] = 0;

    // Board 0

    for(int i{0}; i<64; i++)
      TimeOffset[0][i] = 0;//-150000;//-50000;//63360.900;

    //    TimeOffset[0][1] = -180000;
      // TimeOffset[0][4] = -30000;
      //TimeOffset[0][8] = -190000;

    // MCP2
    TimeOffset[0][1] = -69827;
    TimeOffset[0][2] = -182199;
    TimeOffset[0][3] = -176345;
    TimeOffset[0][7] = -133830;
      // Board 1
      //TimeOffset[1][1]   = -62676;

    
    if(argc < 3) {
        std::cout << "\nERROR: not enough parameters!" << std::endl;
        std::cout << "CaenToTreeMaster inputFile outputFile DET1_NAME it_max [DET2_NAME] [board_number_first_detector (detNr>1)] [board_num second det] [ch first det]"
                  << std::endl;
        return -1;
    }

    std::map<int, std::map<int, std::deque<DataIn>>> dataInBuff;
 //   std::unordered_map<int<std::unordered_map<int,unsigned long long>> chanFirstTS;
    std::deque<DataIn>                               timeSortBuff;
    DataIn                                           dataIn;
    DataOut                                          dataOut;

    const int         InBuffMaxSize     = 10000000;
    const int         SortedBuffMaxSize = 9000000;

    // Time coincidence window for building of events, in TS units ( 500TS =
    // 1ns )
    const int         TimeCoincWindow   = 500*300;

    const std::string outputfile        = argv[2];
    const std::string inputfile         = argv[1];
    int               it_max            = -1;
    int               firstbrd          = 0;
    std::string det1name                = argv[3];
    std::string det2name                = "";

    bool firstdet = false;
    unsigned int secondbrd;
    int  detNr     = 1;

    if(argc >= 6){
      it_max = atoi(argv[4]);
      det2name              = argv[5];
      detNr                 =2;
      firstbrd = atoi(argv[6]);
      secondbrd = atoi(argv[7]);
      
    }
    
    std::vector<unsigned int> ch1;
    if(argc >8){
      for(int jj=8; jj<argc; jj++)
	ch1.emplace_back(atoi(argv[jj]));			 
    }
    
    bool twoBoards = true;  
    if(firstbrd == secondbrd) twoBoards = false;
    
    
    
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

    // TREE Branches definitions
    
    treeOut(detNr,det1name,det2name,outTree,dataOut);
    
    // ---------------------------------

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
                if(detNr==1) {
                    dataOut.nbDetector1 = 0;
                    dataOut.TSDetector1 = timeSortBuff[0].Timestamp;

                    while(timeSortBuff[0].Timestamp
                          < dataOut.TSDetector1 + TimeCoincWindow) {
                        if(timeSortBuff[0].Timestamp < dataOut.TSDetector1) {
                            std::cerr
                                << "\nERROR: time sorting was not "
                                          "successfull, please increase buffer size"
                                << std::endl;
                            std::cerr << "timeSortBuff[0].Timestamp "
                                      << timeSortBuff[0].Timestamp << "\t"
                                      << Form("dataOut.TS%s ",det1name.c_str())
                                      << dataOut.TSDetector1 << std::endl;
                            std::cerr << "nrEvts " << nrEvts << std::endl;
                            return -1;
                        }

                        dataOut.nbDetector1++;
                        dataOut.TSDetector1_Hit[dataOut.nbDetector1 - 1]
                            = timeSortBuff[0].Timestamp;
                        dataOut.Detector1Channel[dataOut.nbDetector1 - 1]
                            = timeSortBuff[0].Channel;
                        dataOut.Detector1Board[dataOut.nbDetector1 - 1]
                            = timeSortBuff[0].Board;
                        dataOut.Detector1Energy[dataOut.nbDetector1 - 1]
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

              
                else if(detNr==2)
                {
                    dataOut.nbDetector1 = 0;
                    dataOut.nbDetector2 = 0;
		    
		    unsigned int ch0 = timeSortBuff[0].Channel;
		    for( const auto &t : ch1){

		      if(ch0 == t){
			firstdet = true;
			break;
		      }
		      else firstdet = false;
		    }
		    
                    if((!twoBoards && timeSortBuff[0].Board==firstbrd) || (twoBoards && firstdet))
                    {
                        dataOut.TSDetector1 = timeSortBuff[0].Timestamp;

                        while(timeSortBuff[0].Timestamp
                              < dataOut.TSDetector1 + TimeCoincWindow) {
                            if(timeSortBuff[0].Timestamp < T0 or timeSortBuff[0].Timestamp < dataOut.TSDetector1) {
                                std::cerr
                                    << "\nERROR: time sorting was not "
                                              "successfull, please increase buffer size"
                                    << std::endl;
                                std::cerr << "timeSortBuff[0].Timestamp "
                                          << timeSortBuff[0].Timestamp << "\t" << "Chan (Board)"<<timeSortBuff[0].Channel
                                          <<" "<<timeSortBuff[0].Board
                                          << "T0 " << T0 
                                          << Form("\tdataOut.TS%s ",det1name.c_str()) << dataOut.TSDetector1 << std::endl;
                                std::cerr << "nrEvts " << nrEvts << std::endl;
                                return -1;
                            }


                            
			    unsigned int ch01 = timeSortBuff[0].Channel;
			    for( const auto &t : ch1){
			      
			      if(ch01 == t)
				{
				  firstdet = true;
				  break;
				}
			      else firstdet = false;
			    }
			    
			    if((!twoBoards && timeSortBuff[0].Board==firstbrd) || (twoBoards && firstdet))
			      {
                                dataOut.nbDetector1++;
                                dataOut.TSDetector1_Hit[dataOut.nbDetector1 - 1]
                                    = timeSortBuff[0].Timestamp;
                                dataOut.Detector1Time[dataOut.nbDetector1 - 1]
                                    = 0.;
                                dataOut.Detector1Channel[dataOut.nbDetector1 - 1]
                                    = timeSortBuff[0].Channel;
                                dataOut.Detector1Board[dataOut.nbDetector1 - 1]
                                    = timeSortBuff[0].Board;
                                dataOut.Detector1Energy[dataOut.nbDetector1 - 1]
                                    = timeSortBuff[0].Energy;
    
                                timeSortBuff.pop_front();
                            }else{
                                dataOut.nbDetector2++;
                                dataOut.TSDetector2_Hit[dataOut.nbDetector2 - 1]
                                    = timeSortBuff[0].Timestamp;
                                dataOut.Detector2Time[dataOut.nbDetector2 - 1]
				                    = 0.;
                                dataOut.Detector2Channel[dataOut.nbDetector2 - 1]
                                    = timeSortBuff[0].Channel;
                                dataOut.Detector2Board[dataOut.nbDetector2 - 1]
                                    = timeSortBuff[0].Board;
                                dataOut.Detector2Energy[dataOut.nbDetector2 - 1]
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

                        
                    } else
                    {

                        dataOut.TSDetector2 = timeSortBuff[0].Timestamp;

                        while(timeSortBuff[0].Timestamp
                          < dataOut.TSDetector2 + TimeCoincWindow) {
                            if(timeSortBuff[0].Timestamp < T0 or timeSortBuff[0].Timestamp < dataOut.TSDetector2) {
                                std::cerr
                                    << "\nERROR: time sorting was not "
                                              "successfull, please increase buffer size"
                                    << std::endl;
                                std::cerr << "timeSortBuff[0].Timestamp "
                                          << timeSortBuff[0].Timestamp << "\t"
                                          << timeSortBuff[0].Timestamp << "\t" << "Chan (Board)"<<timeSortBuff[0].Channel
                                          <<" "<<timeSortBuff[0].Board
                                        << "T0 " << T0
                                        << Form("\tdataOut.TS%s ",det2name.c_str()) << dataOut.TSDetector2 << std::endl;
                                std::cerr << "nrEvts " << nrEvts << std::endl;
                                return -1;
                            }
                            unsigned int ch01 = timeSortBuff[0].Channel;
			    for( const auto &t : ch1){
			      
			     if(ch01 == t)
				{
				  firstdet = true;
				  break;
				}
			     
			      else firstdet = false;
			    }
			    
			    if((!twoBoards && timeSortBuff[0].Board==firstbrd) || (twoBoards && firstdet))
			      {
                            
                                    dataOut.nbDetector1++;
                                    dataOut.TSDetector1_Hit[dataOut.nbDetector1 - 1]
                                        = timeSortBuff[0].Timestamp;
                                    dataOut.Detector1Time[dataOut.nbDetector1 - 1]
				                        = 0.;
                                    dataOut.Detector1Channel[dataOut.nbDetector1 - 1]
                                        = timeSortBuff[0].Channel;
                                    dataOut.Detector1Board[dataOut.nbDetector1 - 1]
                                        = timeSortBuff[0].Board;
                                    dataOut.Detector1Energy[dataOut.nbDetector1 - 1]
                                        = timeSortBuff[0].Energy;

                                    timeSortBuff.pop_front();
                            }else{
                                dataOut.nbDetector2++;
                                dataOut.TSDetector2_Hit[dataOut.nbDetector2 - 1]
                                    = timeSortBuff[0].Timestamp;
                                dataOut.Detector2Time[dataOut.nbDetector2 - 1]
				                    = 0.;
                                dataOut.Detector2Channel[dataOut.nbDetector2 - 1]
                                    = timeSortBuff[0].Channel;
                                dataOut.Detector2Board[dataOut.nbDetector2 - 1]
                                    = timeSortBuff[0].Board;
                                dataOut.Detector2Energy[dataOut.nbDetector2 - 1]
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
            if(detNr==1) {
                    dataOut.nbDetector1 = 0;
                    dataOut.TSDetector1 = timeSortBuff[0].Timestamp;

                    while(timeSortBuff[0].Timestamp
                          < dataOut.TSDetector1 + TimeCoincWindow) {
                        if(timeSortBuff[0].Timestamp < dataOut.TSDetector1) {
                            std::cerr
                                << "\nERROR: time sorting was not "
                                          "successfull, please increase buffer size"
                                << std::endl;
                            std::cerr << "timeSortBuff[0].Timestamp "
                                      << timeSortBuff[0].Timestamp << "\t"
                                      << Form("dataOut.TS%s ",det1name.c_str())
                                      << dataOut.TSDetector1 << std::endl;
                            std::cerr << "nrEvts " << nrEvts << std::endl;
                            return -1;
                        }

                        dataOut.nbDetector1++;
                        dataOut.TSDetector1_Hit[dataOut.nbDetector1 - 1]
                            = timeSortBuff[0].Timestamp;
                        dataOut.Detector1Channel[dataOut.nbDetector1 - 1]
                            = timeSortBuff[0].Channel;
                        dataOut.Detector1Board[dataOut.nbDetector1 - 1]
                            = timeSortBuff[0].Board;
                        dataOut.Detector1Energy[dataOut.nbDetector1 - 1]
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

              
                else if(detNr==2)
                {
                    dataOut.nbDetector1 = 0;
                    dataOut.nbDetector2 = 0;

		    unsigned int ch0 = timeSortBuff[0].Channel;
		    for( const auto &t : ch1){
		      
		      if(ch0 == t) {
			firstdet = true;
			break;
		      }
		      else firstdet = false;
		    }
		    
                    if((!twoBoards && timeSortBuff[0].Board==firstbrd) || (twoBoards && firstdet))
		      {
                      
                        dataOut.TSDetector1 = timeSortBuff[0].Timestamp;

                        while(timeSortBuff[0].Timestamp
                              < dataOut.TSDetector1 + TimeCoincWindow) {
                            if(timeSortBuff[0].Timestamp < T0 or timeSortBuff[0].Timestamp < dataOut.TSDetector1) {
                                std::cerr
                                    << "\nERROR: time sorting was not "
                                              "successfull, please increase buffer size"
                                    << std::endl;
                                std::cerr << "timeSortBuff[0].Timestamp "
                                          << timeSortBuff[0].Timestamp << "\t" << "Chan (Board)"<<timeSortBuff[0].Channel
                                          <<" "<<timeSortBuff[0].Board
                                          << "T0 " << T0 
                                          << Form("\tdataOut.TS%s ",det1name.c_str()) << dataOut.TSDetector1 << std::endl;
                                std::cerr << "nrEvts " << nrEvts << std::endl;
                                return -1;
                            }


                            unsigned int ch01 = timeSortBuff[0].Channel;
			    for( const auto &t : ch1){
			      
			      if(ch01 == t){
				firstdet = true;
				break;
			      }
			      else firstdet = false;
			    }
			    
			    if((!twoBoards && timeSortBuff[0].Board==firstbrd) || (twoBoards && firstdet))
			      {
				
                                dataOut.nbDetector1++;
                                dataOut.TSDetector1_Hit[dataOut.nbDetector1 - 1]
                                    = timeSortBuff[0].Timestamp;
                                dataOut.Detector1Time[dataOut.nbDetector1 - 1]
                                    = 0.;
                                dataOut.Detector1Channel[dataOut.nbDetector1 - 1]
                                    = timeSortBuff[0].Channel;
                                dataOut.Detector1Board[dataOut.nbDetector1 - 1]
                                    = timeSortBuff[0].Board;
                                dataOut.Detector1Energy[dataOut.nbDetector1 - 1]
                                    = timeSortBuff[0].Energy;
    
                                timeSortBuff.pop_front();
                            }else{
                                dataOut.nbDetector2++;
                                dataOut.TSDetector2_Hit[dataOut.nbDetector2 - 1]
                                    = timeSortBuff[0].Timestamp;
                                dataOut.Detector2Time[dataOut.nbDetector2 - 1]
				                    = 0.;
                                dataOut.Detector2Channel[dataOut.nbDetector2 - 1]
                                    = timeSortBuff[0].Channel;
                                dataOut.Detector2Board[dataOut.nbDetector2 - 1]
                                    = timeSortBuff[0].Board;
                                dataOut.Detector2Energy[dataOut.nbDetector2 - 1]
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

                        
                    } else
                    {

                        dataOut.TSDetector2 = timeSortBuff[0].Timestamp;

                        while(timeSortBuff[0].Timestamp
                          < dataOut.TSDetector2 + TimeCoincWindow) {
                            if(timeSortBuff[0].Timestamp < T0 or timeSortBuff[0].Timestamp < dataOut.TSDetector2) {
                                std::cerr
                                    << "\nERROR: time sorting was not "
                                              "successfull, please increase buffer size"
                                    << std::endl;
                                std::cerr << "timeSortBuff[0].Timestamp "
                                          << timeSortBuff[0].Timestamp << "\t"
                                          << timeSortBuff[0].Timestamp << "\t" << "Chan (Board)"<<timeSortBuff[0].Channel
                                          <<" "<<timeSortBuff[0].Board
                                        << "T0 " << T0
                                        << Form("\tdataOut.TS%s ",det2name.c_str()) << dataOut.TSDetector2 << std::endl;
                                std::cerr << "nrEvts " << nrEvts << std::endl;
                                return -1;
                            }
                            unsigned int ch01 = timeSortBuff[0].Channel;
			    for( const auto &t : ch1){
			      
			      if(ch01 == t){
				firstdet = true;
				break;
			      }
			      else firstdet = false;
			    }
			    
			    if((!twoBoards && timeSortBuff[0].Board==firstbrd) || (twoBoards && firstdet))
			      {
				
                                    dataOut.nbDetector1++;
                                    dataOut.TSDetector1_Hit[dataOut.nbDetector1 - 1]
                                        = timeSortBuff[0].Timestamp;
                                    dataOut.Detector1Time[dataOut.nbDetector1 - 1]
				                        = 0.;
                                    dataOut.Detector1Channel[dataOut.nbDetector1 - 1]
                                        = timeSortBuff[0].Channel;
                                    dataOut.Detector1Board[dataOut.nbDetector1 - 1]
                                        = timeSortBuff[0].Board;
                                    dataOut.Detector1Energy[dataOut.nbDetector1 - 1]
                                        = timeSortBuff[0].Energy;

                                    timeSortBuff.pop_front();
                            }else{
                                dataOut.nbDetector2++;
                                dataOut.TSDetector2_Hit[dataOut.nbDetector2 - 1]
                                    = timeSortBuff[0].Timestamp;
                                dataOut.Detector2Time[dataOut.nbDetector2 - 1]
				                    = 0.;
                                dataOut.Detector2Channel[dataOut.nbDetector2 - 1]
                                    = timeSortBuff[0].Channel;
                                dataOut.Detector2Board[dataOut.nbDetector2 - 1]
                                    = timeSortBuff[0].Board;
                                dataOut.Detector2Energy[dataOut.nbDetector2 - 1]
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

        if(detNr==1) {
                    dataOut.nbDetector1 = 0;
                    dataOut.TSDetector1 = timeSortBuff[0].Timestamp;

                    while(timeSortBuff[0].Timestamp
                          < dataOut.TSDetector1 + TimeCoincWindow) {
                        if(timeSortBuff[0].Timestamp < dataOut.TSDetector1) {
                            std::cerr
                                << "\nERROR: time sorting was not "
                                          "successfull, please increase buffer size"
                                << std::endl;
                            std::cerr << "timeSortBuff[0].Timestamp "
                                      << timeSortBuff[0].Timestamp << "\t"
                                      << Form("dataOut.TS%s ",det1name.c_str())
                                      << dataOut.TSDetector1 << std::endl;
                            std::cerr << "nrEvts " << nrEvts << std::endl;
                            return -1;
                        }

                        dataOut.nbDetector1++;
                        dataOut.TSDetector1_Hit[dataOut.nbDetector1 - 1]
                            = timeSortBuff[0].Timestamp;
                        dataOut.Detector1Channel[dataOut.nbDetector1 - 1]
                            = timeSortBuff[0].Channel;
                        dataOut.Detector1Board[dataOut.nbDetector1 - 1]
                            = timeSortBuff[0].Board;
                        dataOut.Detector1Energy[dataOut.nbDetector1 - 1]
                            = timeSortBuff[0].Energy;

                        timeSortBuff.pop_front();
                        if(timeSortBuff.size() == 0) {
                            std::cout << "\nTime sorted buffer was "
                                                "emptied"
                                      << std::endl;
			    break;
                        }
                    }
                }

              
                else if(detNr==2)
                {
                    dataOut.nbDetector1 = 0;
                    dataOut.nbDetector2 = 0;

		    unsigned int ch0 = timeSortBuff[0].Channel;
		    for( const auto &t : ch1){
		      
		      if(ch0 == t){
			firstdet = true;
			break;
		      }
		      else firstdet = false;
		    }
		    
                    if((!twoBoards && timeSortBuff[0].Board==firstbrd) || (twoBoards && firstdet))
		      {
			
                        dataOut.TSDetector1 = timeSortBuff[0].Timestamp;

                        while(timeSortBuff[0].Timestamp
                              < dataOut.TSDetector1 + TimeCoincWindow) {
                            if(timeSortBuff[0].Timestamp < T0 or timeSortBuff[0].Timestamp < dataOut.TSDetector1) {
                                std::cerr
                                    << "\nERROR: time sorting was not "
                                              "successfull, please increase buffer size"
                                    << std::endl;
                                std::cerr << "timeSortBuff[0].Timestamp "
                                          << timeSortBuff[0].Timestamp << "\t" << "Chan (Board)"<<timeSortBuff[0].Channel
                                          <<" "<<timeSortBuff[0].Board
                                          << "T0 " << T0 
                                          << Form("\tdataOut.TS%s ",det1name.c_str()) << dataOut.TSDetector1 << std::endl;
                                std::cerr << "nrEvts " << nrEvts << std::endl;
                                return -1;
                            }


                            unsigned int ch01 = timeSortBuff[0].Channel;
			    for( const auto &t : ch1){
			      
			      if(ch01 == t){
				firstdet = true;
				break;
			      }
			      else firstdet = false;
			    }
			    
			    if((!twoBoards && timeSortBuff[0].Board==firstbrd) || (twoBoards && firstdet))
			      {
                    
                                dataOut.nbDetector1++;
                                dataOut.TSDetector1_Hit[dataOut.nbDetector1 - 1]
                                    = timeSortBuff[0].Timestamp;
                                dataOut.Detector1Time[dataOut.nbDetector1 - 1]
                                    = 0.;
                                dataOut.Detector1Channel[dataOut.nbDetector1 - 1]
                                    = timeSortBuff[0].Channel;
                                dataOut.Detector1Board[dataOut.nbDetector1 - 1]
                                    = timeSortBuff[0].Board;
                                dataOut.Detector1Energy[dataOut.nbDetector1 - 1]
                                    = timeSortBuff[0].Energy;
    
                                timeSortBuff.pop_front();
                            }else{
                                dataOut.nbDetector2++;
                                dataOut.TSDetector2_Hit[dataOut.nbDetector2 - 1]
                                    = timeSortBuff[0].Timestamp;
                                dataOut.Detector2Time[dataOut.nbDetector2 - 1]
				                    = 0.;
                                dataOut.Detector2Channel[dataOut.nbDetector2 - 1]
                                    = timeSortBuff[0].Channel;
                                dataOut.Detector2Board[dataOut.nbDetector2 - 1]
                                    = timeSortBuff[0].Board;
                                dataOut.Detector2Energy[dataOut.nbDetector2 - 1]
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

                        
                    } else
                    {

                        dataOut.TSDetector2 = timeSortBuff[0].Timestamp;

                        while(timeSortBuff[0].Timestamp
                          < dataOut.TSDetector2 + TimeCoincWindow) {
                            if(timeSortBuff[0].Timestamp < T0 or timeSortBuff[0].Timestamp < dataOut.TSDetector2) {
                                std::cerr
                                    << "\nERROR: time sorting was not "
                                              "successfull, please increase buffer size"
                                    << std::endl;
                                std::cerr << "timeSortBuff[0].Timestamp "
                                          << timeSortBuff[0].Timestamp << "\t"
                                          << timeSortBuff[0].Timestamp << "\t" << "Chan (Board)"<<timeSortBuff[0].Channel
                                          <<" "<<timeSortBuff[0].Board
                                        << "T0 " << T0
                                        << Form("\tdataOut.TS%s ",det2name.c_str()) << dataOut.TSDetector2 << std::endl;
                                std::cerr << "nrEvts " << nrEvts << std::endl;
                                return -1;
                            }
                            unsigned int ch01 = timeSortBuff[0].Channel;
			    for( const auto &t : ch1){
			      
			      if(ch01 == t){
				firstdet = true;
				break;
			      }
			      else firstdet = false;
			    }
			    
			    if((!twoBoards && timeSortBuff[0].Board==firstbrd) || (twoBoards && firstdet))
			      {
                    
                                    dataOut.nbDetector1++;
                                    dataOut.TSDetector1_Hit[dataOut.nbDetector1 - 1]
                                        = timeSortBuff[0].Timestamp;
                                    dataOut.Detector1Time[dataOut.nbDetector1 - 1]
				                        = 0.;
                                    dataOut.Detector1Channel[dataOut.nbDetector1 - 1]
                                        = timeSortBuff[0].Channel;
                                    dataOut.Detector1Board[dataOut.nbDetector1 - 1]
                                        = timeSortBuff[0].Board;
                                    dataOut.Detector1Energy[dataOut.nbDetector1 - 1]
                                        = timeSortBuff[0].Energy;

                                    timeSortBuff.pop_front();
                            }else{
                                dataOut.nbDetector2++;
                                dataOut.TSDetector2_Hit[dataOut.nbDetector2 - 1]
                                    = timeSortBuff[0].Timestamp;
                                dataOut.Detector2Time[dataOut.nbDetector2 - 1]
				                    = 0.;
                                dataOut.Detector2Channel[dataOut.nbDetector2 - 1]
                                    = timeSortBuff[0].Channel;
                                dataOut.Detector2Board[dataOut.nbDetector2 - 1]
                                    = timeSortBuff[0].Board;
                                dataOut.Detector2Energy[dataOut.nbDetector2 - 1]
                                    = timeSortBuff[0].Energy;

                                timeSortBuff.pop_front();
                            }

                            if(timeSortBuff.size() == 0) 
                            {
                            std::cout << "\nTime sorted buffer was "
                                                "emptied"
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
