
#include <array>
#include <bitset>
#include <cstdint>
#include <deque>
#include <fstream>
#include <iostream>
#include <map>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <time.h>
#include <valarray>
#include <vector>

#include <TFile.h>
#include <TH2.h>
#include <TTree.h>

#include "mwdlib.h"

//#define dbg std::cerr << "DEBUG: in " << __FILE__ << " at line " << __LINE__
//<< std::endl;

const size_t       MAX_OUT_ADF_FILE_SIZE = 4ULL * 1024 * 1024 * 1024; // 4GB
int                outADFFileIndex{0};

FILE*              pFile;
constexpr uint32_t maxboards{32};
constexpr uint32_t maxchannels{64};
char*              buffer{nullptr};
char*              prismaBuffer{nullptr};
bool               mergeWithPrisma{false};
bool               rootWrite{false};
uint32_t           sizeFrame{0};
uint16_t           ancFileCounter(0);
uint16_t           prismaFileCounter(0);
uint32_t           readCountRate[maxboards][maxchannels];
uint32_t           writeCountRate[maxboards][maxchannels];
uint32_t           prismaReadRate;
uint32_t           prismaWriteRate;

uint64_t           tsprisma(0), tsanc(0), eneanc(0);
uint16_t           board_id(0), channel_id(0), mult(0), time_anc(0);

TFile*             fOutput   = nullptr;
TTree*             coincTree = nullptr;

struct Arguments {
    bool        verb         = false;
    bool        write_traces = false;
    unsigned    n_samples    = 100;
    std::string outputRootFileName;
    int64_t     globalAncTsOffset = 0;
    int         nrevts            = 0;
    std::deque<FILE*>
        prismaFiles; // Deque to hold FILE* pointers for --prisma files
    std::deque<FILE*>
        ancillaryFiles; // Deque to hold FILE* pointers for --input files
    std::string         outAdfFileName{"out.adf"};
    std::pair<int, int> plot{-1, -1};
} args;
double coincWindowAnc{500};

template <std::size_t R, std::size_t L, std::size_t N>
std::bitset<N> project_range(std::bitset<N> b) {
    static_assert(R <= L && L <= N, "invalid bitrange");
    b = b << (N - L - 1);       // drop R rightmost bits
    b = b >> (R + (N - L - 1)); // drop L-1 leftmost bits
    return b;
}

uint64_t swapLong(uint64_t X) {
    uint64_t x = X;
    x = (x & 0x00000000FFFFFFFF) << 32 | (x & 0xFFFFFFFF00000000) >> 32;
    x = (x & 0x0000FFFF0000FFFF) << 16 | (x & 0xFFFF0000FFFF0000) >> 16;
    x = (x & 0x00FF00FF00FF00FF) << 8 | (x & 0xFF00FF00FF00FF00) >> 8;
    return x;
}

struct boardSettings {
    uint32_t key{0xFA0201A2};
    bool     isPHA{false};
    bool     slave{false};
    bool     isV1{true};
    uint16_t minMult{1};
    uint32_t fNsPerTStamp{2};
    uint32_t fNsPerSample{2};
    int64_t  tsOffset[64]{0};
    uint16_t nbChannels{16};
};

struct dataFormatPSD {
    std::bitset<32> dataFormat;
    void            SetDataFormat(uint32_t form) { dataFormat = form; }
    bool            enaDT() const { return dataFormat.test(31); }
    bool            enaCharge() const { return dataFormat.test(30); }
    bool            enaTS() const { return dataFormat.test(29); }
    bool            enaExtras() const { return dataFormat.test(28); }
    bool            enaTrace() const { return dataFormat.test(27); }
    uint8_t         confExtras() const {
                return static_cast<uint8_t>(
            project_range<24, 26, 32>(dataFormat).to_ulong());
    }
    uint8_t confAnaProbe() const {
        return static_cast<uint8_t>(
            project_range<22, 23, 32>(dataFormat).to_ulong());
    }
    uint8_t confDigProbe1() const {
        return static_cast<uint8_t>(
            project_range<19, 21, 32>(dataFormat).to_ulong());
    }
    uint8_t confDigProbe2() const {
        return static_cast<uint8_t>(
            project_range<16, 18, 32>(dataFormat).to_ulong());
    }
    uint16_t numSamples() const {
        return static_cast<uint16_t>(
                   project_range<0, 15, 32>(dataFormat).to_ulong())
             * 8;
    }
    uint16_t evtSize() const {
        uint16_t evtSize = 0;
        //		if(enaDT())     evtSize+=1;
        if(enaCharge()) evtSize += 1;
        if(enaTS()) evtSize += 1;
        if(enaExtras()) evtSize += 1;
        if(enaTrace()) evtSize += numSamples() / 2;
        return evtSize;
    }
    void printDataFormat() {
        printf("Data format (%lu): ", dataFormat.to_ulong());
        std::cout << dataFormat << std::endl;
        printf(" --- Dual Trace [31] are %s\n",
               ((enaDT()) ? "enabled" : "disabled"));
        printf(" --- Charge [30] are %s\n",
               ((enaCharge()) ? "enabled" : "disabled"));
        printf(" --- TimeTag [29] are %s\n",
               ((enaTS()) ? "enabled" : "disabled"));
        printf(" --- Extras [28] are %s\n",
               ((enaExtras()) ? "enabled" : "disabled"));
        if(enaExtras())
            printf(" --- Extras configuration [26:24] set to %i \n",
                   confExtras());
        printf(" --- Trace [27] are %s\n",
               ((enaTrace()) ? "enabled" : "disabled"));
        printf(" --- Event length %i 32-bit words\n", evtSize());
    }
};
struct dataFormatPHA {
    std::bitset<32> dataFormat;
    void            SetDataFormat(uint32_t form) { dataFormat = form; }
    bool            enaDT() const { return dataFormat.test(31); }
    bool            enaEnergy() const { return dataFormat.test(30); }
    bool            enaTS() const { return dataFormat.test(29); }
    bool            enaExtras2() const { return dataFormat.test(28); }
    bool            enaTrace() const { return dataFormat.test(27); }
    bool            enaAnalog() const {
                   if(confAnaProbe1() == 0)
            return true;
        else if(confAnaProbe2() == 0)
            return true;
        else
            return false;
    }
    uint8_t confExtras() const {
        return static_cast<uint8_t>(
            project_range<24, 26, 32>(dataFormat).to_ulong());
    }
    uint8_t confAnaProbe1() const {
        return static_cast<uint8_t>(
            project_range<22, 23, 32>(dataFormat).to_ulong());
    }
    uint8_t confAnaProbe2() const {
        return static_cast<uint8_t>(
            project_range<20, 21, 32>(dataFormat).to_ulong());
    }
    uint8_t confDigProbe() const {
        return static_cast<uint8_t>(
            project_range<16, 19, 32>(dataFormat).to_ulong());
    }
    uint16_t numSamples() const {
        return static_cast<uint16_t>(
                   project_range<0, 15, 32>(dataFormat).to_ulong())
             * 8;
    }
    uint16_t evtSize() const {
        uint16_t evtSize = 0;
        // SP
        //		if(enaDT()) evtSize+=1;
        // end SP
        if(enaEnergy()) evtSize += 1;
        if(enaTS()) evtSize += 1;
        if(enaExtras2()) evtSize += 1;
        if(enaTrace()) evtSize += numSamples() / 2;
        return evtSize;
    }
    void printDataFormat() {
        printf("Data format (%lu): ", dataFormat.to_ulong());
        std::cout << dataFormat << std::endl;
        printf(" --- Dual Trace [31] are %s\n",
               ((enaDT()) ? "enabled" : "disabled"));
        printf(" --- Energy [30] are %s\n",
               ((enaEnergy()) ? "enabled" : "disabled"));
        printf(" --- TimeTag [29] are %s\n",
               ((enaTS()) ? "enabled" : "disabled"));
        printf(" --- Extras [28] are %s\n",
               ((enaExtras2()) ? "enabled" : "disabled"));
        if(enaExtras2())
            printf(" --- Extras configuration [26:24] set to %i \n",
                   confExtras());
        printf(" --- Trace [27] are %s\n",
               ((enaTrace()) ? "enabled" : "disabled"));
        printf(" --- Traces length = %i samples", numSamples());
        printf(" --- Event length %i 32-bit words\n", evtSize());
        if(enaAnalog())
            printf(" --- EnaAnalog enabled\n");
        else
            printf(" --- EnaAnalog disabled\n");
        if(enaTrace()) printf(" --- Probe1 nr %i\n", confAnaProbe1());
    }
};

struct agataKey {
    // POD with 5 uint32_t
    uint32_t size  = 0; // in bytes
    uint32_t key   = 0; //
    uint32_t evnum = 0; // AGATA: defined but never used === GALILEO:
    // domain_number/crystal_id info for the EventBuilder
    uint32_t ts_0  = 0; //
    uint32_t ts_1  = 0; //

    uint32_t GetBytes() const { return size; }
    uint32_t GetEvnum() const { return evnum; }
    uint64_t GetTstamp() const {
        return (uint64_t(ts_1) << 32) | (uint64_t(ts_0));
    }
    // agataKey() { memset(this, 0, sizeof(agataKey)); }
};

struct subDataPHA_t {
    uint16_t energy = 0;
    uint16_t cfd    = 0; // in ps
};
struct caenEventPHA_t {
    agataKey     theKey;
    subDataPHA_t theData;
};

struct subDataPSD_t {
    uint16_t energy = 0;
    uint16_t qshort = 0;
    float    cfd    = 0; // in ps
};
struct caenEventPSD_t {
    agataKey     theKey;
    subDataPSD_t theData;
};

struct internalData {
    uint64_t           timestamp = 0;
    uint32_t           key       = 0;
    uint32_t           cfd       = 0; // in ps
    uint32_t           evnum     = 0;
    float              cfd_off   = 0;
    float              psd       = 0;
    float              imax      = 0;
    uint16_t           energy    = 0;
    uint16_t           board     = 0;
    uint16_t           channel   = 0;
    uint16_t           qshort    = 0;
    std::vector<float> samples;
};

bool     isEof() { return feof(args.ancillaryFiles.back()); }
void     timeSortEvent(const internalData&);
void     depleteBuffers();

uint32_t getSizeFrame();
bool     getData(const uint32_t& aggregateSize);

uint32_t getSizeFrame() {
    // if(!sizeFrame){
    //	delete sizeFrame;
    //	sizeFrame = new uint32_t;
    // }
    uint32_t sizeFrame = 0;

    if(args.ancillaryFiles.size() == 0) {
        return 0;
    } else if(fread(&sizeFrame, 1, sizeof(uint32_t),
                    args.ancillaryFiles.front())) {
        // printf("sizeFrame in getSize = %08X\n",sizeFrame);
        fseek(args.ancillaryFiles.front(), -4, SEEK_CUR);
        return sizeFrame;
    } else {
        fclose(args.ancillaryFiles.front());
        std::cout << "End of anc file " << ancFileCounter << std::endl;
        args.ancillaryFiles.pop_front();
        ancFileCounter++;
        return getSizeFrame();
    }
}

bool getData(const uint32_t& aggregateSize) {
    // printf("Trying to read %i\n",aggregateSize);
    size_t elements_read = fread(buffer, sizeof(uint32_t), aggregateSize,
                                 args.ancillaryFiles.front());
    if(elements_read == aggregateSize) return true;

    fclose(args.ancillaryFiles.front());
    std::cout << "End of anc file " << ancFileCounter << std::endl;
    args.ancillaryFiles.pop_front();
    ancFileCounter++;
    if(args.ancillaryFiles.size() == 0) return false;
    elements_read
        += fread(buffer + sizeof(uint32_t) * elements_read, sizeof(uint32_t),
                 aggregateSize - elements_read, args.ancillaryFiles.front());
    if(elements_read == aggregateSize)
        return true;
    else {
        std::cout << "Aggregate not read completely" << std::endl;
        return false;
    }
}

int                                          channelsBuffersSize{50000};
std::map<uint16_t, std::deque<internalData>> eventContainer;
std::deque<internalData>                     timeSorted;

void                                         decode_PHA_v1(uint32_t* event);
void                                         decode_PSD_v1(uint32_t* event);
void                                         decode_PHA_v2(uint32_t* event);
void                                         decode_PSD_v2(uint32_t* event);
uint64_t                                     fGlobalOffset;
std::array<std::array<MWD, maxchannels>, maxboards> waveAnalyzer;
TTree*                                              fTree;
TH2D*                                               hitpatternPHA;
TH2D*                                               hitpatternPSD;

const uint32_t keyDataVME      = 0xFA0201A0; // adf::data:ranc0
const uint32_t keyDataPrisma   = 0xFA0201A1; // adf::data:ranc1
const uint32_t keyDataSpider   = 0xFA0201A2; // adf::data:ranc2
const uint32_t keyDataDante    = 0xFA0201A3; // adf::data:ranc3
const uint32_t keyDataEuclides = 0xFA0201A4; // adf::data:ranc4
const uint32_t keyDataLaBr     = 0xFA0201A5; // adf::data:ranc5
const uint32_t keyDataSauron   = 0xFA0201A6; // adf::data:ranc6
const uint32_t keyDataOscar    = 0xFA0201A7; // adf::data:ranc7

uint32_t       getKey(std::string name) {
          if(name == "spider") {
              return keyDataSpider;
    } else if(name == "dante") {
              return keyDataDante;
    } else if(name == "euclides") {
              return keyDataEuclides;
    } else if(name == "labr") {
              return keyDataLaBr;
    } else if(name == "sauron") {
              return keyDataSauron;
    } else if(name == "oscar") {
              return keyDataOscar;
    } else {
              std::cerr << "Warning: Unknown ancillary name, please check your "
                                 "configuration file\n"
                        << "Possible names (case sensitive):\n"
                        << "\t- spider\n"
                        << "\t- dante\n"
                        << "\t- euclides\n"
                        << "\t- labr\n"
                        << "\t- sauron\n"
                        << "\t- oscar\n"
                        << std::endl;
              std::cerr << "Received name is " << name << std::endl;
              std::exit(-10);
    }
}

struct softTrigger {
    uint32_t keyType;
    uint16_t minFold;
    bool     onlyCoinc;
};

// ROOT OUTPUT IDENTICAL TO THE ONE YOU WOULD GET WITH THE AGAPRO PACKAGE
// SO YOU CAN USE THE SAME ANALYSIS CODE THAN FOR AGATA ROOT OUTPUT
const uint16_t MaxHit   = 128;
const uint16_t MaxTrace = 5000;

Int_t          nbSPIDER;
UShort_t       SPIDERChannelId[MaxHit];
UShort_t       SPIDERBoardId[MaxHit];
float          SPIDEREnergy[MaxHit];
float          SPIDERTime[MaxHit];
ULong64_t      TSSPIDERHit[MaxHit];
ULong64_t      TSSPIDER;
float          SPIDERTrace[MaxHit][MaxTrace];
void           createBranchSpider() {
              if(fTree != nullptr) {
                  fTree->Branch("nbSPIDER", &nbSPIDER, "nbSPIDER/I");
                  fTree->Branch("SPIDERBoard", SPIDERBoardId, "SPIDERBoard[nbSPIDER]/s");
                  fTree->Branch("SPIDERChannel", SPIDERChannelId,
                                "SPIDERChannel[nbSPIDER]/s");
                  fTree->Branch("SPIDEREnergy", SPIDEREnergy,
                                "SPIDEREnergy[nbSPIDER]/F");
                  fTree->Branch("SPIDERTime", SPIDERTime, "SPIDERTime[nbSPIDER]/F");
                  fTree->Branch("TSSPIDER_Hit", TSSPIDERHit, "TSSPIDER_Hit[nbSPIDER]/l");
                  fTree->Branch("TSSPIDER", &TSSPIDER, "TSSPIDER/l");
                  if(args.write_traces) {
                      fTree->Branch("SPIDERTrace", &SPIDERTrace,
                                    Form("SPIDERTrace[nbSPIDER][%i]/F", args.n_samples));
        }
    } else {
                  std::cerr << "Warning: Output root tree is not defined! Spider branch "
                                         "will not be created"
                            << std::endl;
    }
}

Int_t     nbDANTE;
UShort_t  DANTEChannelId[MaxHit];
UShort_t  DANTEBoardId[MaxHit];
float     DANTEEnergy[MaxHit];
float     DANTETime[MaxHit];
ULong64_t TSDANTEHit[MaxHit];
ULong64_t TSDANTE;
float     DANTETrace[MaxHit][MaxTrace];
void      createBranchDante() {
         if(fTree != nullptr) {
             fTree->Branch("nbDANTE", &nbDANTE, "nbDANTE/I");
             fTree->Branch("DANTEBoard", DANTEBoardId, "DANTEBoard[nbDANTE]/s");
             fTree->Branch("DANTEChannel", DANTEChannelId,
                           "DANTEChannel[nbDANTE]/s");
             fTree->Branch("DANTEEnergy", DANTEEnergy, "DANTEEnergy[nbDANTE]/F");
             fTree->Branch("DANTETime", DANTETime, "DANTETime[nbDANTE]/F");
             fTree->Branch("TSDANTE_Hit", TSDANTEHit, "TSDANTE_Hit[nbDANTE]/l");
             fTree->Branch("TSDANTE", &TSDANTE, "TSDANTE/l");
             if(args.write_traces) {
                 fTree->Branch("DANTETrace", &DANTETrace,
                               Form("DANTETrace[nbDANTE][%i]/F", args.n_samples));
        }
    } else {
             std::cerr << "Warning: Output root tree is not defined! Dante branch "
                               "will not be created"
                       << std::endl;
    }
}

Int_t     nbEUCLIDES;
UShort_t  EUCLIDESChannelId[MaxHit];
UShort_t  EUCLIDESBoardId[MaxHit];
float     EUCLIDESEnergy[MaxHit];
float     EUCLIDESTime[MaxHit];
ULong64_t TSEUCLIDESHit[MaxHit];
ULong64_t TSEUCLIDES;
float     EUCLIDESTrace[MaxHit][MaxTrace];
void      createBranchEuclides() {
         if(fTree != nullptr) {
             fTree->Branch("nbEUCLIDES", &nbEUCLIDES, "nbEUCLIDES/I");
             fTree->Branch("EUCLIDESBoard", EUCLIDESBoardId,
                           "EUCLIDESBoard[nbEUCLIDES]/s");
             fTree->Branch("EUCLIDESChannel", EUCLIDESChannelId,
                           "EUCLIDESChannel[nbEUCLIDES]/s");
             fTree->Branch("EUCLIDESEnergy", EUCLIDESEnergy,
                           "EUCLIDESEnergy[nbEUCLIDES]/F");
             fTree->Branch("EUCLIDESTime", EUCLIDESTime,
                           "EUCLIDESTime[nbEUCLIDES]/F");
             fTree->Branch("TSEUCLIDES_Hit", TSEUCLIDESHit,
                           "TSEUCLIDES_Hit[nbEUCLIDES]/l");
             fTree->Branch("TSEUCLIDES", &TSEUCLIDES, "TSEUCLIDES/l");
             if(args.write_traces) {
                 fTree->Branch(
                     "EUCLIDESTrace", &EUCLIDESTrace,
                     Form("EUCLIDESTrace[nbEUCLIDES][%i]/F", args.n_samples));
        }
    } else {
             std::cerr << "Warning: Output root tree is not defined!  branch will "
                               "not be created"
                       << std::endl;
    }
}

Int_t     nbLABR;
UShort_t  LABRChannelId[MaxHit];
UShort_t  LABRBoardId[MaxHit];
float     LABREnergy[MaxHit];
float     LABRQshort[MaxHit];
float     LABRTime[MaxHit];
ULong64_t TSLABRHit[MaxHit];
ULong64_t TSLABR;
float     LABRTrace[MaxHit][MaxTrace];
void      createBranchLaBr() {
         if(fTree != nullptr) {
             fTree->Branch("nbLABR", &nbLABR, "nbLABR/I");
             fTree->Branch("LABRBoard", LABRBoardId, "LABRBoard[nbLABR]/s");
             fTree->Branch("LABRChannel", LABRChannelId, "LABRChannel[nbLABR]/s");
             fTree->Branch("LABREnergy", LABREnergy, "LABREnergy[nbLABR]/F");
             fTree->Branch("LABRQshort", LABRQshort, "LABRQshort[nbLABR]/F");
             fTree->Branch("LABRTime", LABRTime, "LABRTime[nbLABR]/F");
             fTree->Branch("TSLABR_Hit", TSLABRHit, "TSLABR_Hit[nbLABR]/l");
             fTree->Branch("TSLABR", &TSLABR, "TSLABR/l");
             if(args.write_traces) {
                 fTree->Branch("LABRTrace", &LABRTrace,
                               Form("LABRTrace[nbLABR][%i]/F", args.n_samples));
        }
    } else {
             std::cerr << "Warning: Output root tree is not defined! LaBr branch "
                               "will not be created"
                       << std::endl;
    }
}

Int_t     nbSAURON;
UShort_t  SAURONChannelId[MaxHit];
UShort_t  SAURONBoardId[MaxHit];
float     SAURONEnergy[MaxHit];
float     SAURONTime[MaxHit];
float     SAURONCfd[MaxHit];
float     SAURONImax[MaxHit];
float     SAURONPid[MaxHit];
ULong64_t TSSAURONHit[MaxHit];
ULong64_t TSSAURON;
float     SAURONTrace[MaxHit][MaxTrace];
void      createBranchSauron() {
         if(fTree != nullptr) {
             fTree->Branch("nbSAURON", &nbSAURON, "nbSAURON/I");
             fTree->Branch("SAURONBoard", SAURONBoardId, "SAURONBoard[nbSAURON]/s");
             fTree->Branch("SAURONChannel", SAURONChannelId,
                           "SAURONChannel[nbSAURON]/s");
             fTree->Branch("SAURONEnergy", SAURONEnergy,
                           "SAURONEnergy[nbSAURON]/F");
             fTree->Branch("SAURONTime", SAURONTime, "SAURONTime[nbSAURON]/F");
             fTree->Branch("SAURONCfd", SAURONCfd, "SAURONCfd[nbSAURON]/F");
             fTree->Branch("SAURONImax", SAURONImax, "SAURONImax[nbSAURON]/F");
             fTree->Branch("SAURONPid", SAURONPid, "SAURONPid[nbSAURON]/F");
             fTree->Branch("TSSAURON_Hit", TSSAURONHit, "TSSAURON_Hit[nbSAURON]/l");
             fTree->Branch("TSSAURON", &TSSAURON, "TSSAURON/l");
             if(args.write_traces) {
                 fTree->Branch("SAURONTrace", &SAURONTrace,
                               Form("SAURONTrace[nbSAURON][%i]/F", args.n_samples));
        }
    } else {
             std::cerr << "Warning: Output root tree is not defined! Sauron branch "
                               "will not be created"
                       << std::endl;
    }
}

Int_t     nbOSCAR;
UShort_t  OSCARChannelId[MaxHit];
UShort_t  OSCARBoardId[MaxHit];
float     OSCAREnergy[MaxHit];
float     OSCARTime[MaxHit];
ULong64_t TSOSCARHit[MaxHit];
ULong64_t TSOSCAR;
float     OSCARTrace[MaxHit][MaxTrace];
void      createBranchOscar() {
         if(fTree != nullptr) {
             fTree->Branch("nbOSCAR", &nbOSCAR, "nbOSCAR/I");
             fTree->Branch("OSCARBoard", OSCARBoardId, "OSCARBoard[nbOSCAR]/s");
             fTree->Branch("OSCARChannel", OSCARChannelId,
                           "OSCARChannel[nbOSCAR]/s");
             fTree->Branch("OSCAREnergy", OSCAREnergy, "OSCAREnergy[nbOSCAR]/F");
             fTree->Branch("OSCARTime", OSCARTime, "OSCARTime[nbOSCAR]/F");
             fTree->Branch("TSOSCAR_Hit", TSOSCARHit, "TSOSCAR_Hit[nbOSCAR]/l");
             fTree->Branch("TSOSCAR", &TSOSCAR, "TSOSCAR/l");
             if(args.write_traces) {
                 fTree->Branch("OSCARTrace", &OSCARTrace,
                               Form("OSCARTrace[nbOSCAR][%i]/F", args.n_samples));
        }
    } else {
             std::cerr << "Warning: Output root tree is not defined! Oscar branch "
                               "will not be created"
                       << std::endl;
    }
}

std::vector<internalData> coincData;
void                      fillTree();
void                      resetTreeEvents();
