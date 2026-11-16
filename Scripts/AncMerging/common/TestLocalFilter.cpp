///////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "LocalFilter.h"

#include <ctime>
#include <csignal>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <mutex>

using namespace std;

#ifdef GAL_OFFLINE
std::mutex ioMutex;  // for LOCK_COUT
#endif

#if 0   // insert this wherever you want to use ioMutex
#ifdef GAL_OFFLINE
extern std::mutex ioMutex;  // ioMutex is defined in the main module
#define LOCK_COUT std::lock_guard<std::mutex> ioLock(ioMutex);
#else
#define LOCK_COUT
#endif
#endif

const int maxDoms = 128;
int verbose = 0 ;
string   inpFileName;
FILE    *inpFILE;

string   inpFileList;
ifstream listStream;

string   outFileName;
string   outName[maxDoms];
FILE    *outFILE[maxDoms] = {nullptr};

bool outWrite  = false;
bool outUnique = false;
bool outIdles  = true;
bool outGframe = false;

const int bufferSize  = 1024*1024;
const int maxRawEvLen = (1 << 16) - 1;                  // coded with 16 bits
const int maxInpSize  = bufferSize - 2 * maxRawEvLen;   // with some spare

uint32_t  inpBuff[bufferSize];
uint32_t  outBuff[bufferSize];    // two times bigger than inpBuff
int       inpSize;                // units of int16_t
int       outSize;                // units of int32_t

int      domOffset = 0;
uint32_t outMagicData;

bool listKeys = false;
bool isAncillary = false;

uint32_t vmeFileHeader[10];

int  tstCount = 0;
int  maxEvents = 0;
int  eventCount;
int  gterminate;

string       configFile;
LocalFilter* theLocalFilter = nullptr;

clock_t startTime, stopTime;

// callback for SIGINT
void terminate(int sig) {
  gterminate++;
  if(gterminate > 3)
    exit(-1);
}

void  getparams       (int, char *[]);
void  numparams       (char *, int);
void  initialize      ();
void  updatePrompt    (ostream &);// cyclic info on terminal or list
int   NextAggregate   (uint32_t *inp, int space);
void  ShowFilePos     (FILE* pFILE);
bool  NextFile        ();
int   ProcessInput    (int iLength);            // call LocalFilter and process its output
int   CheckOutput     (uint32_t *out, int len); // check integrity of output buffer
bool  WriteOutput     (uint32_t *out, int len); // write data to the proper file(s)

int main(int argc, char *argv[])
{
  getparams(argc, argv);  // parameters from the command line
  initialize();           // process command line arguments

  ////// Create LocalFilter
  theLocalFilter = new LocalFilter();

  ////// Configure LocalFilter
  int errorCode = 0;
  theLocalFilter->process_config(configFile.c_str(), &errorCode);
  if (errorCode) {
    cout << " Configuration error " << endl;
    exit(EXIT_FAILURE);
  }
  //outMagicData = theLocalFilter->GetOutMagicDATA();

  ////// Start LocalFilter
  errorCode = 0;
  theLocalFilter->process_start(100, &errorCode);
  if(errorCode) {
    cout << endl << " Error " << errorCode << " from process_start() " << endl;
    exit(EXIT_FAILURE);
  }

  inpSize = 0;
  outSize = 0;
  eventCount = 0;
  int  evnum = 0;
  int  numBlocks = 0;
  bool done = false;
  bool evok = true;

  gterminate = 0;
  signal(SIGINT, terminate);
  cout << "#\n# Analysis can be stopped by typing CTRL_C\n#" << endl;

  startTime = clock();

  ////////////////////////
  // the execution loop //

  while( !done ) {
    int evSize = NextAggregate( inpBuff + inpSize, bufferSize-inpSize);

    if(evSize <= 0) {
      if( NextFile() )
        evSize = NextAggregate( inpBuff + inpSize, bufferSize-inpSize);
      else
        done = true;
    }

    if(evSize > 0) {
      inpSize += evSize;
      eventCount++;
    }

    done |= gterminate  || (maxEvents>0 && eventCount>=maxEvents);

    if( (inpSize > maxInpSize) || (done && inpSize > 0) ) {
      if( (outSize = ProcessInput(inpSize))  < 0)
	break;
      numBlocks++;
      inpSize = 0;    // reset position of input buffer
    }

    if(done)
      break;
  }

  // call without input to flush parallel buffers
  while ( (outSize = ProcessInput(0)) >  0);

  cerr << endl;

  // end of execution loop //
  ///////////////////////////

  ////// Stop LocalFilter
  errorCode = 0;
  theLocalFilter->process_stop(&errorCode);
  if(errorCode) {
    cout << endl << " Error " << errorCode << " from process_stop() " << endl;
    //exit(EXIT_FAILURE);
  }

  stopTime = clock();
  double realTime = (double)(stopTime - startTime) / CLOCKS_PER_SEC;

  cout << "\n# " << setw(12) << eventCount << " events in " << (int(realTime*10))/10. <<  " seconds " << endl;
  cout << "# " << setw(12) << numBlocks << " calls to process_block\n" << endl;

  if(theLocalFilter) {
    delete theLocalFilter;
  }

  exit(EXIT_SUCCESS);
}

void updatePrompt(ostream &oo)
{
  const uint64_t onesecond = 100000000;

  stopTime = clock();
  double realTime = (double)(stopTime - startTime) / CLOCKS_PER_SEC;

  oo << "# " << setw(12) << eventCount;

  //oo << "  " << theEventNumber
  //  << "  " << theTimeStamp/onesecond
  //  << "." << setfill('0') << setw(8) << theTimeStamp%onesecond << setfill(' ')
  //  << "  " << fixed << setprecision(2) << realTime << scientific << setprecision(6);

  oo << flush;
}

void getparams(int argc, char *argv[])
{
  int   argn;
  char *cmd;
  int   ok;

  ok = (argc > 1) ? 1 : 0;
  argn = 1;

  while(ok == 1 && argn < argc) {
    cmd = argv[argn++];
    ok  = 1;
    if     (!strcmp(cmd, "-h") || !strcmp(cmd, "--help")) {
      ok = -1;
    }
    else if(!strcmp(cmd, "-k") || !strcmp(cmd, "--keys")) {
      listKeys = true;
      return;
    }
    else if(!strcmp(cmd, "-if")) {
      numparams(cmd, argc-argn-1);
      inpFileName = argv[argn++];
    }
    else if(!strcmp(cmd, "-ifl")) {
      numparams(cmd, argc-argn-1);
      inpFileList = argv[argn++];
    }
    else if(!strcmp(cmd, "-of")) {
      numparams(cmd, argc-argn-1);
      outFileName = argv[argn++];
    }
    else if (!strcmp(cmd, "-ogf")) {
      outGframe = true;
    }
    else if (!strcmp(cmd, "-unique")) {
      outUnique = true;
    }
    else if(!strcmp(cmd, "-noidle")) {
      outIdles = false;
    }
    else if(!strcmp(cmd, "-ne")) {
      numparams(cmd, argc-argn-1);
      maxEvents = atoi(argv[argn++]);
    }
    else if(!strcmp(cmd, "-cfg")) {
      numparams(cmd, argc-argn-1);
      configFile = argv[argn++];
    }
    else if(!strcmp(cmd, "-off")) {
      numparams(cmd, argc-argn-1);
      domOffset = atoi(argv[argn++]);
    }
    else if(!strcmp(cmd, "-tsn")) {
      numparams(cmd, argc-argn-1);
      tstCount = atoi(argv[argn++]);
    }
    else if(!strcmp(cmd, "-anc")) {
      isAncillary = true;
    }
    else if(!strcmp(cmd, "-v")) {
      numparams(cmd, argc-argn-1);
      verbose = atoi(argv[argn++]);
    }
    else {
      ok = 0;
    }
  }

  if(ok < 1) {
    if(ok == 0 && argc > 1) cout << "\nInvalid switch " << argv[argn-1] << endl;
    cout << "\nUsage: " << argv[0] << " -[...]" << endl << endl;
    cout << "  -h               print this list" << endl;
    cout << "  -k               print the list of keywords accepted by LocalFilter and exit" << endl ;
    cout << "  -if    file      name of input file" << endl;
    cout << "  -ifl   file      file with the list of input files to be analysed in sequence" << endl;
    cout << "  -of    file      name of output file (will be suffixed with domain number)" << endl;
    cout << "  -ogf             surround output events with a merger 0xCA010103 frame" << endl;
    cout << "  -unique          all events in the same file (no suffix to output file name)" << endl;
    cout << "  -noidle          discard idles from the output file(s)" << endl;
    cout << "  -ne    nn        number of events to analyze [" << maxEvents << "] (0 == all)" << endl;
    cout << "  -cfg   file      configuration file" << endl;
    cout << "  -off   offset    offset to add to domainID while writing the data" << endl;
    cout << "  -tsn   nn        number of timestamps to list on cout" << endl;
    cout << "  -v     nn        verbose level" << std::endl;
    cout << " " << endl;
    if(argc > 1 || ok < 0)
      exit(-1);
  }

  // echo the command line
  time_t ltime;
  time(&ltime);
  cout << "# " << endl << "# " << ctime(&ltime);
  cout << "# " << argv[0] << endl;
  cout << "# ";
  for(int nn = 1; nn < argc; nn++)
    cout << " " << argv[nn];
  cout << endl << "# " << endl;
}

void numparams(char * cmd, int nn)
{
  if(nn >= 0) return;
  nn = -nn;
  printf("error decoding command %s ==> %d parameter(s) missing \n", cmd, nn);
  exit(-1);
}

// initializations after reading the command line
void initialize()
{
  if (listKeys) {
    theLocalFilter = new LocalFilter();
    theLocalFilter->ShowKeywords();
    exit(EXIT_SUCCESS);
  }

  if( !inpFileList.empty() ) {
    listStream.open(inpFileList.c_str() );
    if(!listStream.good()) {
      cout << endl << " Error opening " << inpFileList << endl;
      exit(EXIT_FAILURE);
    }
    cout << "# Reading " << inpFileList << endl;
    string line, extra;
    inpFileName.clear();
    while(getline(listStream, line)) {
      if( !stringSplit(line, inpFileName, extra) )
        continue;   // empty line
      cout << "# Data file is : " << line << endl;
      break;
    }
    if( inpFileName.empty() ) {
      cout << endl << " Error reading " << inpFileList << endl;
      exit(EXIT_FAILURE);
    }
  }

  inpFILE  = fopen(inpFileName.c_str(), "rb");
  if(!inpFILE) {
    cout << " Error opening " << inpFileName << endl;
    exit(EXIT_FAILURE);
  }else{
    std::cout << "Input file " << inpFileName.c_str() << " opened!" << std::endl;
    //    ReadFileHeader()
  }

  if( !outFileName.empty() ) {
    outWrite = true;
  }

  if( !configFile.empty() ) {
    // should do a proper check that it is a path-only name (better if inside the class ??)
    if(configFile=="." || configFile=="..") {
      configFile = configFile+"/LocalFilter.conf";
    }
    else if(configFile[configFile.size()-1] == '/') {
      configFile = "LocalFilter.conf";
    }
  }

}

bool NextFile()
{
  cout << "\n# Done " << inpFileName << endl;
  fclose(inpFILE);
  inpFILE = nullptr;
  inpFileName.clear();
  if( inpFileList.empty() )
    return false;
  // multifile
  string line, extra;
  while(getline(listStream, line)) {
    if( !stringSplit(line, inpFileName, extra) )
      continue;   // empty line
    cout << "# Next " << line << endl;
    break;
  }

  if( !inpFileName.empty() )
    inpFILE  = fopen(inpFileName.c_str(), "rb");

  return (inpFILE != nullptr);
}

int ReadFileHeader(uint32_t *inp){

  return 1;
}

int NextAggregate(uint32_t *inp, int space)
{
  uint32_t* inpBuffer = (uint32_t*)inp;
  size_t llh = fread(inpBuffer, sizeof(uint32_t), 1, inpFILE);
  uint32_t inpAggLen = 0 ;
  if((inpBuffer[0]&0xF0000000) == 0xA0000000){
    inpAggLen = inpBuffer[0]&0x0FFFFFFF;
    if(verbose>10)
      std::cout << "Aggregate detected of size " << inpAggLen << std::endl;
  }else{
    inpAggLen = inpBuffer[0];
  }
  if(verbose>10)
    printf("Aggregate length = %08X\n",inpAggLen);
  size_t lld = fread(inpBuffer+1,sizeof(uint32_t),inpAggLen-1,inpFILE);
  if(lld != inpAggLen-1){
    size_t pos = ftell(inpFILE);
    cout << "\nError in NextAggregate: (3) requested " << inpAggLen << "(uint32_t)  obtained " << lld;
    ShowFilePos(inpFILE);
    return -4;
  }
  if(verbose>100){
    std::cout << "--- NEW EVENT ---" << std::endl;
    for(uint32_t j= 0 ; j < inpAggLen ; j++){
      printf("%5i %08X\n",j,inpBuffer[j]);
    }
  }
  return inpAggLen;
}

void ShowFilePos(FILE* pFILE)
{
  size_t pos = ftell(inpFILE);
  cout << "   @filepos " << pos;
  if (feof(inpFILE))
    cout << " == EOF";
  cout << endl;
}

int ProcessInput(int iLength)
{
  int used_size_of_output_buffer = 0;
  int error_code = 0;
  int packet_ID = 0;
  theLocalFilter->process_block(
				(void*)inpBuff, iLength*sizeof(uint32_t), packet_ID,
				(void*)outBuff, bufferSize*sizeof(uint32_t),
				&used_size_of_output_buffer, &error_code);
  if (error_code) {
    cout << "\n LocalFilter::process_block error " << error_code << endl;
    return -1;
  }

  int oLength = used_size_of_output_buffer / sizeof(uint32_t);
  if (oLength == 0)
    return 0;

  int nFrames = CheckOutput(outBuff, oLength);
  //if( nFrames < 0 )
  //  return -2;    // scanning error

  if (outWrite && oLength > 0) {
    if( !WriteOutput(outBuff, oLength) ) {
      cout << "\n TestLocalFilter::WriteOutput error " << endl;
      return -3;
    }
  }

  return oLength;
}

int CheckOutput(uint32_t *out, int totLen)  // len is in units of uint32_t
{
  uint32_t* outBuff = out;
  int numFrames = 0;
  int restLen = totLen;
  while (restLen > 4) { // minimum length is header-only
    int evSize = outBuff[0]/sizeof(uint32_t);
    outBuff += evSize;
    restLen -= evSize;
    ++numFrames;
  }
  return numFrames;
}

bool WriteOutput(uint32_t *out, int totLen)
{
  uint32_t* outBuff = out;
  int numFrames = 0;
  int restLen = totLen;
  while (restLen > 4) { // minimum length is header-only
    agataKey* dataKey = (agataKey *)outBuff;
    int evSize = outBuff[0] / sizeof(uint32_t);
    int evTag = dataKey->evnum;
    if (outIdles) evTag &= keyDomainMask;   // switch off the idle bit so that domain re-enters range
    if (evTag >= 0 && evTag < maxDoms) {
      if (outUnique)
        evTag = 0;
      if (outFILE[evTag] == nullptr) {
        ostringstream oss;
        oss << outFileName;
        if (!outUnique)
          oss << "_" << setw(2) << setfill('0') << evTag + domOffset;
        outName[evTag] = oss.str();
        outFILE[evTag] = fopen(outName[evTag].c_str(), "wb");
        if (!outFILE[evTag]) {
          cout << "Error opening " << outName[evTag] << endl;
          exit(EXIT_FAILURE);
        }
      }
      dataKey->evnum += domOffset;
      if (outGframe) {
        agataKey gKey;
        memcpy(&gKey, dataKey, sizeof(agataKey));
        gKey.size = dataKey->size + 20;
        gKey.key = keyEventMerged;
        gKey.evnum = 1;
        size_t nw = fwrite(&gKey, sizeof(agataKey), 1, outFILE[evTag]);
        if (nw != 1) {
          cout << "\n# Error writing output file " << outName[evTag] << endl;
          return false;
        }
      }
      size_t nw = fwrite(outBuff, sizeof(uint32_t), evSize, outFILE[evTag]);
      if (nw != evSize) {
        cout << "\n# Error writing output file " << outName[evTag] << endl;
        return false;
      }
    }
    outBuff += evSize;
    restLen -= evSize;
    ++numFrames;
  }
  return true;
}
