///////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////

#include "GlobalFilter.h"

#include <ctime>
#include <csignal>
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

// boost::iostreams added by Alain to be able to read compressed files
#ifdef BOOST
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/algorithm/string.hpp>

boost::iostreams::filtering_istream fin;
#endif

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

string  inpFileName;

#ifdef BOOST
ifstream inpFILE;
#else
FILE*    inpFILE;
#endif

string   inpFileList;
ifstream listStream;

string    outFileName;
FILE     *outFILE = nullptr;
bool      outWrite = false;

string    configFile;

const int bufferSizeInp =   1024*1024;
const int bufferSizeOut = 4*1024*1024;  // make it bigger than inpBuff
uint32_t  inpBuff[bufferSizeInp];
uint32_t  outBuff[bufferSizeOut];
uint32_t  outFrameKey = 0xCA010105;     // enclosing key of output data
uint32_t  outFrameFmt = 0xFA010205;     // enclosing key of output format
int       inpSize;                      // units of int16_t
int       outSize;                      // units of int32_t

int  maxEvents = 0;
int  eventCountInp = 0;
int  eventCountOut = 0;
int  gterminate;

bool listKeys = false;

// callback for SIGINT
void terminate(int sig) {
  gterminate++;
  if(gterminate > 3)
    exit(-1);
}

GlobalFilter* theGlobalFilter = nullptr;

clock_t startTime, stopTime;

void  getparams       (int, char *[]);
void  numparams       (char *, int);
void  listKeysAndExit ();
void  initialize      ();
void  updatePrompt    (ostream &);// cyclic info on terminal or list
int   NextEvent       (uint32_t *inp, int space);
bool  NextFile        ();
int   ProcessInput    (int iLength);                // call GlobalFilter and process its output
bool  WriteEvents     (uint32_t *out, int size);    // write dat in the proper file(s)
int   ScanOutBuffer   (uint32_t *out, int size);    // check integrity of output buffer

int main(int argc, char *argv[])
{
  getparams(argc, argv);        // parameters from the command line

  if (listKeys)
    listKeysAndExit();

  initialize();                 // initialize parameters related to digitisers and detectors

  gterminate = 0;
  signal(SIGINT, terminate);
  cout << "#\n# Analysis can be stopped by typing CTRL_C\n#" << endl;

  const int maxRawEvLen = 8192;                         // rather arbitrary
  const int maxInpSize = bufferSizeInp - 2 * maxRawEvLen;   // with some spare

  int  evnum = 0;
  bool evok = true;
  startTime = clock();

  int error_code;

  ////// START //////
  error_code = 0;
  theGlobalFilter->process_start(100, &error_code);
  if (error_code) {
    cout << endl << " Error " << error_code << " from process_start() " << endl;
    exit(EXIT_FAILURE);
  }

  ////////////////////////
  // the execution loop //

  inpSize = 0;
  outSize = 0;
  eventCountInp = 0;
  eventCountOut = 0;
  int numBlocks = 0;
  bool done = false;

  uint64_t oldTst = 0, newTst = 0;
  int evtOffset = 0;
  int numEvt = 0;

  while (!done) {

    int evSize = NextEvent(inpBuff + inpSize, bufferSizeInp - inpSize);
  //std::cout << std::dec << "Event size = " << evSize << std::endl;

#if 0
    // list adf-key and DeltaTstamp
    cout << setw(10) << numEvt << setw(12) << evtOffset;
    for (int ii = 0; ii < 5; ii++)
      cout << setw(12) << inpBuff[inpSize + ii];
    newTst = (uint64_t(inpBuff[inpSize + 4]) << 32) | (0x00000000FFFFFFFFULL & uint64_t(inpBuff[inpSize + 3]));
    if (oldTst)
      cout << setw(16) << newTst - oldTst;
    cout << endl;
    oldTst = newTst;
    evtOffset += evSize;
    numEvt++;
#endif

    if (evSize <= 0) {
      if (NextFile())
        evSize = NextEvent(inpBuff + inpSize, bufferSizeInp - inpSize);
      else
        done = true;
    }

    if (evSize > 0) {
      inpSize += evSize;
      eventCountInp++;
    }

    done |= gterminate || (maxEvents > 0 && eventCountInp >= maxEvents);

#if 0
    if ((inpSize > maxInpSize) || (done && inpSize > 0)) {
      int used_size_of_output_buffer = 0;
      int error_code = 0;
      int packet_ID = 0;
      theGlobalFilter->process_block(
        (void*)inpBuff, inpSize * sizeof(uint32_t), packet_ID,
        (void*)outBuff, bufferSize * sizeof(uint32_t),
        &used_size_of_output_buffer, &error_code);
      if (error_code) {
        cout << "\n process_block error " << error_code << endl;
        break;
      }
      numBlocks++;
      outSize = used_size_of_output_buffer / sizeof(uint32_t);
      if (ScanOutBuffer(outBuff, outSize) < 0)
        break;    // scanning error
      if (outWrite && outSize > 0) {
        if (!WriteEvents(outBuff, outSize))
          break;
      }
      inpSize = 0;    // reset position of input buffer
    }
#else
    if ((inpSize > maxInpSize) || (done && inpSize > 0)) {
    //std::cout << "inpSize= " <<inpSize<< std::endl;
      if ((outSize = ProcessInput(inpSize)) < 0) {
      //std::cout<< "Warning outsize = " << outSize << std::endl;
        break;
      }
    //std::cout << "outSize= " << outSize << std::endl;
      numBlocks++;
    //std::cout << "NumBlocks= " << numBlocks << std::endl;
      inpSize = 0;    // reset position of input buffer
    }
#endif

    if (done)
      break;
  }

  // call without input to flush parallel buffers
  do {
    outSize = ProcessInput(0);
  } while (outSize > 0);

  cerr << "# " << endl;

  // end of execution loop //
  ///////////////////////////

  ////// STOP //////
  error_code = 0;
  theGlobalFilter->process_stop(&error_code);
  if (error_code) {
    cout << endl << " Error " << error_code << " from process_stop() " << endl;
    //exit(EXIT_FAILURE);
  }
  if (outWrite) {
    fclose(outFILE);
    cout << "# Closed output file " << outFileName  << endl;
  }

  stopTime = clock();
  double realTime = (double)(stopTime - startTime) / CLOCKS_PER_SEC;

  cout << "# " << endl;
  cout << "# " << setw(12) << eventCountInp << " input  events in " << (int(realTime*10))/10. <<  " seconds " << endl;
  cout << "# " << setw(12) << eventCountOut << " output events produced by " << endl;
  cout << "# " << setw(12) <<     numBlocks << " calls  to process_block" << endl;
  cout << "#" << endl;

  //if(theGlobalFilter) {
  //  delete theGlobalFilter;
  //}

  exit(EXIT_SUCCESS);
}

void updatePrompt(ostream &oo)
{
  const uint64_t onesecond = 100000000;

  stopTime = clock();
  double realTime = (double)(stopTime - startTime) / CLOCKS_PER_SEC;

  oo << "# " << setw(12) << eventCountInp;

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
    else if(!strcmp(cmd, "-ne")) {
      numparams(cmd, argc-argn-1);
      maxEvents = atoi(argv[argn++]);
    }
    else if(!strcmp(cmd, "-cfg")) {
      numparams(cmd, argc-argn-1);
      configFile = argv[argn++];
    }
    else {
      ok = 0;
    }
  }

  if(ok < 1) {
    if(ok == 0 && argc > 1) cout << "\nInvalid switch " << argv[argn-1] << endl;
    cout << "\nUsage: " << argv[0] << " -[...]" << endl << endl;
    cout << "  -h               print this list" << endl;
    cout << "  -k               print the list of keywords accepted by GlobalFilter and exit" << endl ;
    cout << "  -if    file      name of input file" << endl;
    cout << "  -ifl   file      file with the list of input files to be analysed in sequence" << endl;
    cout << "  -of    file      name of output file" << endl;
    cout << "  -ne    nn        number of events to analyze [" << maxEvents << "] (0 == all)" << endl;
    cout << "  -cfg   file      configuration file" << endl;
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
  if( !inpFileList.empty() ) {
    listStream.open(inpFileList.c_str() );
    if(!listStream.good()) {
      cout << endl << " Error opening " << inpFileList << endl;
      exit(EXIT_FAILURE);
    }
    string line, extra;
    inpFileName.clear();
    while(getline(listStream, line)) {
      if( !stringSplit(line, inpFileName, extra) )
        continue;   // empty line
      cout << line;
      break;
    }
    if( inpFileName.empty() ) {
      cout << endl << " Error reading " << inpFileList << endl;
      exit(EXIT_FAILURE);
    }
  }

#ifdef BOOST
  inpFILE.open(inpFileName.c_str(), ios_base::in | ios::binary);
  if(!inpFILE.is_open()) {
#else
  inpFILE = fopen(inpFileName.c_str(),"rb");
  if (!inpFILE) {
#endif
    cout << "Error opening " << inpFileName << endl;
    exit(EXIT_FAILURE);
  }

#ifdef BOOST
  bool compressed_data = false;
  if (boost::algorithm::iends_with(inpFileName, ".gz")) {
    fin.push(boost::iostreams::gzip_decompressor());
    compressed_data = true;
  } else if (boost::algorithm::iends_with(inpFileName, ".bz2")) {
    fin.push(boost::iostreams::bzip2_decompressor());
    compressed_data = true;
  } else if (boost::algorithm::iends_with(inpFileName, ".tar")) {
    compressed_data = true;
    //nothing to do
  } else if(boost::algorithm::iends_with(inpFileName,"i1")) {
    //No decompression filter needed
  } else if(boost::algorithm::iends_with(inpFileName,"adf")) {
    //No decompression filter needed
  } else {
    cerr << "Unknown file suffix: " << inpFileName << endl;
    std::exit(10);
  }
  fin.push(inpFILE);
  if(compressed_data) {
    char file_header[512];
    fin.read(file_header, 512);
  }
#endif
  
  if( !outFileName.empty() ) {
    outWrite = true;
    outFILE  = fopen(outFileName.c_str(), "wb");
    if(!outFILE) {
      cout << "Error opening " << outFileName << endl;
      exit(EXIT_FAILURE);
    }
  }

  // create GlobalFilter
  theGlobalFilter = new GlobalFilter();

  //// preset to command line values to be able to run without configuration file

  if( !configFile.empty() ) {
    // should do a proper check that it is a path-only name (better if inside the class ??)
    if(configFile=="." || configFile=="..") {
      configFile = configFile+"/GlobalFilter.conf";
    }
    else if(configFile[configFile.size()-1] == '/') {
      configFile = "GlobalFilter.conf";
    }
  }

  // configure GlobalFilter
  int errorCode = 0;
  theGlobalFilter->process_config(configFile.c_str(), &errorCode);
  if(errorCode) {
    cout << "Configuration error " << endl;
    exit(EXIT_FAILURE);
  }

}

void listKeysAndExit()
{
  theGlobalFilter = new GlobalFilter();
  theGlobalFilter->ShowKeywords();
  exit(EXIT_SUCCESS);
}

 // "space" and return value in units of uint32_t
int NextEvent(uint32_t *inp, int space)
{
  if( space < sizeof(agataKey)/sizeof(uint32_t) )
    return -1;

#ifdef BOOST
  // Part for the compressed data should work also for uncompressed data but this has to be tested
  fin.read((char*)(inp), sizeof(uint32_t));
  if(fin.gcount() != sizeof(uint32_t)) {
    return -1;
  }
  if(inp[0] == 0)
    return -2;
  int bytes_to_read  = inp[0] - sizeof(uint32_t);
  if(bytes_to_read <= 0)
    return -3;
  fin.read((char*)inp+sizeof(uint32_t), bytes_to_read);
  
  if(fin.gcount() != bytes_to_read) {
    std::cout << "\ngcount = " << fin.gcount() << "  bytes_to_read = " << bytes_to_read;
    std::cout << "  inp = " << std::hex << inp << std::dec << std::endl;
    return -3;
  }
  return inp[0]/sizeof(uint32_t);
#else
  size_t llh = fread(inp, sizeof(agataKey), 1, inpFILE);  
  if(llh != 1)
    return -2;

  int evlen = inp[0]/sizeof(uint32_t);                    // total event length in I4
  int rest  = evlen - sizeof(agataKey)/sizeof(uint32_t);  // to be read

  size_t lld = fread(inp+sizeof(agataKey)/sizeof(uint32_t), sizeof(uint32_t), rest, inpFILE);
  if(lld != rest)
    return -3;
  return evlen;
#endif
}

bool NextFile()
{
  cout << "\n# Done " << inpFileName << endl;

#ifdef BOOST
  inpFILE.close();
#else
   inpFILE = nullptr;
#endif

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
#ifdef BOOST

    inpFILE.open(inpFileName.c_str(),ios_base::in | ios::binary);
  return (inpFILE.is_open());
#else
    inpFILE  = fopen(inpFileName.c_str(), "rb");
  return (inpFILE != nullptr);
#endif
}

int ProcessInput(int iLength)
{
  int used_size_of_output_buffer = 0;
  int error_code = 0;
  int packet_ID = 0;
  theGlobalFilter->process_block(
    (void*)inpBuff, iLength * sizeof(uint32_t), packet_ID,
    (void*)outBuff, bufferSizeOut * sizeof(uint32_t),
    &used_size_of_output_buffer, &error_code);
  if (error_code) {
    cout << "\n GlobalFilter::process_block error " << error_code << endl;
    return -1;
  }

  int oLength = used_size_of_output_buffer / sizeof(uint32_t);
  if (oLength == 0)
    return 0;

  int nFrames = ScanOutBuffer(outBuff, oLength);
  if( nFrames < 0 ) {
    cout << "\n TestGlobalFilter::ScanOutBuffer error " << endl;
    return -2;    // scanning error
  }
  eventCountOut += nFrames;
  //cout << "  nFrames " << setw(6) << nFrames << setw(10) << eventCountOut << endl;

  if (outWrite && oLength > 0) {
    if (!WriteEvents(outBuff, oLength)) {
      cout << "\n TestGlobalFilter::WriteEvents error " << endl;
      return -3;
    }
  }

  return oLength;
}

bool WriteEvents(uint32_t *out, int size)
{
  size_t nw = fwrite(outBuff, sizeof(uint32_t), size, outFILE);
  if(nw != size) {
    cout << "\n# Error writing output file " << outFileName << endl;
    return false;
  }
  return true;
}

int ScanOutBuffer(uint32_t *out, int size)  // size is in units of uint32_t
{
  uint32_t* outBuff = out;
  int numFrames = 0;
  while (size > 4) { // minimum length is header-only
    agataKey* dataKey = (agataKey *)outBuff;
    if (dataKey->key != outFrameKey) {
      if (!numFrames == 0 || dataKey->key != outFrameFmt) {
        cerr << " \nError scanning output buffer after " << numFrames << " frames"
	     << std::hex << dataKey->key <<endl;
        return -1;
      }
    }
    int evSize = outBuff[0]/sizeof(uint32_t);
    outBuff += evSize;
    size -= evSize;
    ++numFrames;
  }
  return numFrames;
}

