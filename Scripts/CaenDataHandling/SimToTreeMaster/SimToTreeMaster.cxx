#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <TFile.h>
#include <TRandom.h>
#include <TTree.h>

// to compile: g++ -Wall `root-config --cflags` -o SimToTreeMaster *.cxx
// `root-config --libs`

struct Event {
    // AGATA
    Int_t     nbHits = 0;
    Float_t   hitT[10];
    Float_t   hitE[10];
    Float_t   hitX[10];
    Float_t   hitY[10];
    Float_t   hitZ[10];
    Float_t   hitGX[10];
    Float_t   hitGY[10];
    Float_t   hitGZ[10];
    Int_t     hitId[10];
    Int_t     hitSg[10];
    Int_t     nbCores = 0;
    Int_t     nbHitsperCry[10];
    Int_t     coreId[10];
    Float_t   coreE0[10];
    Float_t   coreE1[10];
    Float_t   coreT0[10];
    ULong64_t coreTS[10];
    Int_t     nbAdd = 0;
    Int_t     AddId[10];
    Float_t   AddE[10];
    Float_t   AddX[10];
    Float_t   AddY[10];
    Float_t   AddZ[10];
    ULong64_t AddTS[10];
    ULong64_t TSHit   = 0;
    Int_t     nbTrack = 0;
    Float_t   trackE[10];
    Float_t   trackX1[10];
    Float_t   trackY1[10];
    Float_t   trackZ1[10];
    Float_t   trackhitE1[10];
    Float_t   trackX2[10];
    Float_t   trackY2[10];
    Float_t   trackZ2[10];
    Float_t   trackhitE2[10];
    Int_t     tracknbHits[10];
    Int_t     trackType[10];
    Float_t   trackFOM[10];
    Float_t   trackT[10];
    Int_t     trackCrystalID[10];
    ULong64_t TStrack  = 0;

    // SAURON
    Int_t     nbSAURON = 0;
    ULong64_t TSSAURON = 0;
    ULong64_t TSSAURON_Hit[10];
    UShort_t  SAURONChannel[10];
    UShort_t  SAURONBoard[10];
    Float_t   SAURONEnergy[10];
    Float_t   SAURONTime[10];
    Float_t   SAURONCfd[10];
    Float_t   SAURONImax[10];
    Float_t   SAURONPid[10];

    // OSCAR
    Int_t     nbOSCAR = 0;
    ULong64_t TSOSCAR = 0;
    ULong64_t TSOSCAR_Hit[10];
    UShort_t  OSCARChannel[10];
    UShort_t  OSCARBoard[10];
    Float_t   OSCAREnergy[10];
    Float_t   OSCARTime[10];
    Float_t   OSCARCfd[10];
    Float_t   OSCARImax[10];
    Float_t   OSCARPid[10];
};

const bool AgataOn  = true;
const bool SauronOn = true;
const bool OscarOn  = true;

void       ClearEvent(Event& ev);

int        main(int argc, char* argv[]) {
    const double       SAURON_RESOLUTION = 0; // keV
    const double       OSCAR_RESOLUTION  = 0; // keV

    const unsigned int EventTimePeriod
        = 1000000; // simulated event rate is 1.0 / EventTimePeriod

    if(argc < 3) {
        std::cout << "Error: not enough parameters!" << std::endl;
        std::cout << "SimToTreeMaster inputFile outputFile [max_events]"
                  << std::endl;
        return 1;
    }

    Event event;
    ClearEvent(event);

    const std::string rootoutput = argv[2];
    const std::string inputfile  = argv[1];
    int               it_max     = -1;
    if(argc >= 4) it_max = atoi(argv[3]);

    TFile* output  = new TFile(rootoutput.c_str(), "recreate");
    TTree* theTree = new TTree("TreeMaster", "TreeMaster");

    // Create branches in the TTree
    if(AgataOn) {
        theTree->Branch("nbHits", &event.nbHits, "nbHits/I");
        theTree->Branch("hitT", &event.hitT, "hitT[nbHits]/F");
        theTree->Branch("hitE", &event.hitE, "hitE[nbHits]/F");
        theTree->Branch("hitX", &event.hitX, "hitX[nbHits]/F");
        theTree->Branch("hitY", &event.hitY, "hitY[nbHits]/F");
        theTree->Branch("hitZ", &event.hitZ, "hitZ[nbHits]/F");
        theTree->Branch("hitGX", &event.hitGX, "hitGX[nbHits]/F");
        theTree->Branch("hitGY", &event.hitGY, "hitGY[nbHits]/F");
        theTree->Branch("hitGZ", &event.hitGZ, "hitGZ[nbHits]/F");
        theTree->Branch("hitId", &event.hitId, "hitId[nbHits]/I");
        theTree->Branch("hitSg", &event.hitSg, "hitSg[nbHits]/I");
        theTree->Branch("nbCores", &event.nbCores, "nbCores/I");
        theTree->Branch("nbHitsperCry", &event.nbHitsperCry,
                               "nbHitsperCry[nbCores]/I");
        theTree->Branch("coreId", &event.coreId, "coreId[nbCores]/I");
        theTree->Branch("coreE0", &event.coreE0, "coreE0[nbCores]/F");
        theTree->Branch("coreE1", &event.coreE1, "coreE1[nbCores]/F");
        theTree->Branch("coreT0", &event.coreT0, "coreT0[nbCores]/F");
        theTree->Branch("coreTS", &event.coreTS, "coreTS[nbCores]/l");
        theTree->Branch("nbAdd", &event.nbAdd, "nbAdd/I");
        theTree->Branch("AddId", &event.AddId, "AddId[nbAdd]/I");
        theTree->Branch("AddE", &event.AddE, "AddE[nbAdd]/F");
        theTree->Branch("AddX", &event.AddX, "AddX[nbAdd]/F");
        theTree->Branch("AddY", &event.AddY, "AddY[nbAdd]/F");
        theTree->Branch("AddZ", &event.AddZ, "AddZ[nbAdd]/F");
        theTree->Branch("AddTS", &event.AddTS, "AddTS[nbAdd]/l");
        theTree->Branch("TSHit", &event.TSHit, "TSHit/l");
        theTree->Branch("nbTrack", &event.nbTrack, "nbTrack/I");
        theTree->Branch("trackE", &event.trackE, "trackE[nbTrack]/F");
        theTree->Branch("trackX1", &event.trackX1, "trackX1[nbTrack]/F");
        theTree->Branch("trackY1", &event.trackY1, "trackY1[nbTrack]/F");
        theTree->Branch("trackZ1", &event.trackZ1, "trackZ1[nbTrack]/F");
        theTree->Branch("trackhitE1", &event.trackhitE1,
                               "trackhitE1[nbTrack]/F");
        theTree->Branch("trackX2", &event.trackX2, "trackX2[nbTrack]/F");
        theTree->Branch("trackY2", &event.trackY2, "trackY2[nbTrack]/F");
        theTree->Branch("trackZ2", &event.trackZ2, "trackZ2[nbTrack]/F");
        theTree->Branch("trackhitE2", &event.trackhitE2,
                               "trackhitE2[nbTrack]/F");
        theTree->Branch("tracknbHits", &event.tracknbHits,
                               "tracknbHits[nbTrack]/I");
        theTree->Branch("trackType", &event.trackType, "trackType[nbTrack]/I");
        theTree->Branch("trackFOM", &event.trackFOM, "trackFOM[nbTrack]/F");
        theTree->Branch("trackT", &event.trackT, "trackT[nbTrack]/F");
        theTree->Branch("trackCrystalID", &event.trackCrystalID,
                               "trackCrystalID[nbTrack]/I");
        theTree->Branch("TStrack", &event.TStrack, "TStrack/l");
    }

    if(SauronOn) {
        theTree->Branch("nbSAURON", &event.nbSAURON, "nbSAURON/I");
        theTree->Branch("TSSAURON", &event.TSSAURON, "TSSAURON/l");
        theTree->Branch("TSSAURON_Hit", &event.TSSAURON_Hit,
                               "TSSAURON_Hit[nbSAURON]/l");
        theTree->Branch("SAURONChannel", &event.SAURONChannel,
                               "SAURONChannel[nbSAURON]/s");
        theTree->Branch("SAURONBoard", &event.SAURONBoard,
                               "SAURONBoard[nbSAURON]/s");
        theTree->Branch("SAURONEnergy", &event.SAURONEnergy,
                               "SAURONEnergy[nbSAURON]/F");
        theTree->Branch("SAURONTime", &event.SAURONTime,
                               "SAURONTime[nbSAURON]/F");
        theTree->Branch("SAURONCfd", &event.SAURONCfd,
                               "SAURONCfd[nbSAURON]/F");
        theTree->Branch("SAURONImax", &event.SAURONImax,
                               "SAURONImax[nbSAURON]/F");
        theTree->Branch("SAURONPid", &event.SAURONPid,
                               "SAURONPid[nbSAURON]/F");
    }

    if(OscarOn) {
        theTree->Branch("nbOSCAR", &event.nbOSCAR, "nbOSCAR/I");
        theTree->Branch("TSOSCAR", &event.TSOSCAR, "TSOSCAR/l");
        theTree->Branch("TSOSCAR_Hit", &event.TSOSCAR_Hit,
                               "TSOSCAR_Hit[nbOSCAR]/l");
        theTree->Branch("OSCARChannel", &event.OSCARChannel,
                               "OSCARChannel[nbOSCAR]/s");
        theTree->Branch("OSCARBoard", &event.OSCARBoard,
                               "OSCARBoard[nbOSCAR]/s");
        theTree->Branch("OSCAREnergy", &event.OSCAREnergy,
                               "OSCAREnergy[nbOSCAR]/F");
        theTree->Branch("OSCARTime", &event.OSCARTime, "OSCARTime[nbOSCAR]/F");
        theTree->Branch("OSCARCfd", &event.OSCARCfd, "OSCARCfd[nbOSCAR]/F");
        theTree->Branch("OSCARImax", &event.OSCARImax, "OSCARImax[nbOSCAR]/F");
        theTree->Branch("OSCARPid", &event.OSCARPid, "OSCARPid[nbOSCAR]/F");
    }

    std::string        aLine;
    std::istringstream oneline;
    std::string        detType;
    double             Ene1, Ene2, posX, posY, posZ;
    int                detNum;
    std::ifstream      input;
    input.open(inputfile.c_str());

    int    iteration = 0;

    int    pSide, nSide, sauronDet; // sauron variables
    int    oscarChan, oscarDet;     // oscar variables

    char   dummyC;
    double dummyF;

    std::cout << std::endl;

    if(input.is_open()) {
        while(input.good()) {
            if(iteration > it_max && it_max != -1) break;

            std::getline(input, aLine);
            if(aLine.empty()) continue;
            std::istringstream oneline(aLine);

            // std::cout << iteration << " || " << aLine << std::endl;

            if(aLine[0] == '#') { // new event
                theTree->Fill();

                iteration++;
                if(iteration % 5000 == 0)
                    std::cout << "\r" << iteration << std::flush;
                // std::cout << iteration << std::endl;

                // if(event.nbTrack>0 && event.nbSAURON>0) std::cout <<
                // event.TStrack << "\t" << event.TSSAURON << std::endl;

                ClearEvent(event);

                oneline >> dummyC >> dummyF >> dummyF >> dummyF >> posX >> posY
                    >> posZ >> Ene1 >> detType;
                if(detType != "ION") {
                    std::cerr << "Something wrong dude with " << detType
                              << std::endl;
                    std::cerr << "the line = " << aLine << std::endl;
                    break;
                }

                continue;
            }

            oneline >> detNum >> Ene1 >> Ene2 >> posX >> posY >> posZ
                >> detType;

            if(detType == "AGATA") { // tracked Gamma
                event.nbTrack++;
                event.trackE[event.nbTrack - 1]  = Ene2 * 1000;
                event.trackX1[event.nbTrack - 1] = posX * 10;
                event.trackY1[event.nbTrack - 1] = posY * 10;
                event.trackZ1[event.nbTrack - 1] = posZ * 10;
                event.TStrack = iteration * EventTimePeriod + event.nbTrack;
            } else if(detType == "CoreEAGATA") { // core energy
                event.nbCores++;
                event.coreId[event.nbCores - 1] = detNum;
                event.coreE0[event.nbCores - 1] = Ene1 * 1000;
                event.coreTS[event.nbCores - 1]
                    = iteration * EventTimePeriod + event.nbCores;
            } else if(detType == "SAURON" && SauronOn) { // SAURON hit
                sauronDet = detNum / 256;
                pSide     = (detNum % 256) % 16;
                nSide     = (detNum % 256) / 16;

                // pSideHit
                event.nbSAURON++;
                event.TSSAURON_Hit[event.nbSAURON - 1]
                    = iteration * EventTimePeriod + event.nbSAURON;
                event.SAURONChannel[event.nbSAURON - 1]
                    = (nSide / 4) * 16 + pSide;
                event.SAURONBoard[event.nbSAURON - 1] = 2 * sauronDet;
                event.SAURONEnergy[event.nbSAURON - 1]
                    = gRandom->Gaus(Ene1, SAURON_RESOLUTION / 1000.);

                // nSideHit
                event.nbSAURON++;
                event.TSSAURON_Hit[event.nbSAURON - 1]
                    = iteration * EventTimePeriod + event.nbSAURON;
                event.SAURONChannel[event.nbSAURON - 1] = nSide;
                event.SAURONBoard[event.nbSAURON - 1] = 2 * sauronDet + 1;
                event.SAURONEnergy[event.nbSAURON - 1]
                    = gRandom->Gaus(Ene1, SAURON_RESOLUTION / 1000.);

                event.TSSAURON = iteration * EventTimePeriod + event.nbSAURON;
            } else if(detType == "Oscar" && OscarOn) { // SAURON hit
                oscarDet  = detNum / 100;
                oscarChan = detNum % 100;

                // pSideHit
                event.nbOSCAR++;
                event.TSOSCAR_Hit[event.nbOSCAR - 1]
                    = iteration * EventTimePeriod + event.nbOSCAR;
                event.OSCARChannel[event.nbOSCAR - 1] = oscarChan;
                event.OSCARBoard[event.nbOSCAR - 1]   = 2 * oscarDet;
                event.OSCAREnergy[event.nbOSCAR - 1]
                    = gRandom->Gaus(Ene1, OSCAR_RESOLUTION / 1000.);

                event.TSOSCAR = iteration * EventTimePeriod + event.nbOSCAR;
            } else {
                std::cerr << "Something wrong with " << detType << std::endl;
                std::cerr << "the line = " << aLine << std::endl;
                // std::cerr <<  detNum << "\t" << Ene1 << Ene2 << posX << posY
                // << posZ << std::endl;
            }
        }
    }

    std::cout << std::endl;

    theTree->Write();
    output->Write();
    output->Close();

    return 0;
}

void ClearEvent(Event& ev) {
    if(AgataOn) {
        // ev.nbHits = 0;
        ev.TStrack = 0;
        // ev.TSHit = 0;
        ev.nbTrack = 0;
        // ev.nbAdd = 0;
        ev.nbCores = 0;
        for(int i = 0; i < 10; i++) {
            // ev.hitT[i] = 0;
            // ev.hitE[i] = 0;
            // ev.hitX[i] = 0;
            // ev.hitY[i] = 0;
            // ev.hitZ[i] = 0;
            // ev.hitGX[i] = 0;
            // ev.hitGY[i] = 0;
            // ev.hitGZ[i] = 0;
            // ev.hitId[i] = 0;
            // ev.hitSg[i] = 0;
            // ev.nbHitsperCry[i] = 0;
            ev.coreId[i]  = 0;
            ev.coreE0[i]  = 0;
            // ev.coreE1[i] = 0;
            // ev.coreT0[i] = 0;
            ev.coreTS[i]  = 0;
            // ev.AddId[i] = 0;
            // ev.AddE[i] = 0;
            // ev.AddX[i] = 0;
            // ev.AddY[i] = 0;
            // ev.AddZ[i] = 0;
            // ev.AddTS[i] = 0;
            ev.trackE[i]  = 0;
            ev.trackX1[i] = 0;
            ev.trackY1[i] = 0;
            ev.trackZ1[i] = 0;
            // ev.trackhitE1[i] = 0;
            // ev.trackX2[i] = 0;
            // ev.trackY2[i] = 0;
            // ev.trackZ2[i] = 0;
            // ev.trackhitE2[i] = 0;
            // ev.tracknbHits[i] = 0;
            // ev.trackType[i] = 0;
            // ev.trackFOM[i] = 0;
            // ev.trackT[i] = 0;
            // ev.trackCrystalID[i] = 0;
        }
    }

    if(SauronOn) {
        ev.nbSAURON = 0;
        ev.TSSAURON = 0;
        for(int i = 0; i < 10; i++) {
            ev.TSSAURON_Hit[i]  = 0;
            ev.SAURONChannel[i] = 0;
            ev.SAURONBoard[i]   = 0;
            ev.SAURONEnergy[i]  = 0;
            // ev.SAURONTime[i] = 0;
            // ev.SAURONCfd[i] = 0;
            // ev.SAURONImax[i] = 0;
            // ev.SAURONPid[i] = 0;
        }
    }

    if(OscarOn) {
        ev.nbOSCAR = 0;
        ev.TSOSCAR = 0;
        for(int i = 0; i < 10; i++) {
            ev.TSOSCAR_Hit[i]  = 0;
            ev.OSCARChannel[i] = 0;
            ev.OSCARBoard[i]   = 0;
            ev.OSCAREnergy[i]  = 0;
            // ev.OSCARTime[i] = 0;
            // ev.OSCARCfd[i] = 0;
            // ev.OSCARImax[i] = 0;
            // ev.OSCARPid[i] = 0;
        }
    }
}
