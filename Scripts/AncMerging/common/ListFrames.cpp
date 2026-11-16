///////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////
#include <cstdint>
#include <ctime>
#include <csignal>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <map>
#include <cstdint>
#include "DataFrames.h"

// should be done using std::chrono in c++11

// taken from
// http://stackoverflow.com/questions/17432502/how-can-i-measure-cpu-time-and-wall-clock-time-on-both-linux-windows

#ifdef _WIN32
//  Windows
#include <Windows.h>
double get_wall_time(){
  LARGE_INTEGER time, freq;
  if (!QueryPerformanceFrequency(&freq)){
    return 0;    //  Handle error
  }
  if (!QueryPerformanceCounter(&time)){
    return 0;    //  Handle error
  }
  return (double)time.QuadPart / freq.QuadPart;
}
double get_cpu_time(){
  FILETIME a, b, c, d;
  if (GetProcessTimes(GetCurrentProcess(), &a, &b, &c, &d) != 0) {
    //  Returns total user time.
    //  Can be tweaked to include kernel times as well.
    DWORD dwtt = d.dwLowDateTime | ((unsigned long long)d.dwHighDateTime << 32);
    return double(dwtt) * 0.0000001;
  }
  else{
    return 0;    //  Handle error
  }
}

#else
//  Posix/Linux
#include <sys/time.h>
double get_wall_time(){
  struct timeval time;
  if (gettimeofday(&time, nullptr)){
    return 0;   //  Handle error
  }
  return (double)time.tv_sec + (double)time.tv_usec * .000001;
}
double get_cpu_time(){
  return (double)clock() / CLOCKS_PER_SEC;
}

#endif

using namespace std;


// ALREADY DEFINED IN THE DataFrames.cpp
//const uint32_t keyIdleMask = 0x80000000;   // to be seen!!
//
//struct agataKey {
//   // POD with 5 uint32_t
//   uint32_t size;    // bytes
//   uint32_t key;     //
//   uint32_t evnum;   // AGATA: defined but never used === GALILEO: domain_number/crystal_id info for the EventBuilder
//   uint32_t ts_0;    //
//   uint32_t ts_1;    //
//
//   uint32_t GetBytes() { return  size; }
//   uint32_t GetKey() { return  key; }
//   bool     IsComposite() { return (key & 0xF0000000) == 0xC0000000; }
//   uint32_t GetDomain() { return  evnum & ~keyIdleMask; }
//   bool     IsIdle() { return (evnum & keyIdleMask) != 0; }
//   uint32_t GetEvNum() { return  evnum; }
//   uint64_t GetTstamp() { return (uint64_t(ts_1) << 32) | (uint64_t(ts_0)); }
//};

string  inpFileName;
FILE   *inpFILE;

const int bufferSize = 1000000;
uint32_t  inpBuff[bufferSize];

int      maxEvents = 0;
int      eventCount = 0;
int      printModulo = 0;
int      gterminate = 0;
long     skipBytes = 0;

bool bFull  = true;
bool bCount = false;
bool bTd2TS = false;

struct rMap_t
{
  uint32_t *ptr;
  uint32_t  parent;
  uint32_t  level;
  rMap_t() : ptr(nullptr), parent(0), level(0) {}
  rMap_t(uint32_t *pt, uint32_t  par, uint32_t  lev) : ptr(pt), parent(par), level(lev) {}
};

// taken from AgataSoftware/agapro/filters/GlobalFilter
vector <rMap_t>  rMap;

//std::map<int, std::string> LUT;


// callback for SIGINT
void terminate(int sig) {
  gterminate++;
  if (gterminate > 3)
    exit(-1);
}

void  getparams(int, char *[]);
void  numparams(char *, int);
void  initialize();
int   NextEvent(uint32_t *inp, int space);
uint32_t GetEvtnum(uint32_t *i4v) { return i4v[2]&0xffff; }
uint64_t GetTstamp(uint32_t *i4v) { return (uint64_t)i4v[4] << 32 | (0x00000000FFFFFFFFULL & (uint64_t)i4v[3]); }
uint32_t MapOneFrame(uint32_t *i4Buf, uint32_t level, uint32_t parent);

void  ConvertTd2TS();      // should be moved to another program

int main(int argc, char *argv[])
{
  getparams(argc, argv);        // parameters from the command line

  gterminate = 0;
  signal(SIGINT, terminate);
  cout << "#\n# Analysis can be stopped by typing CTRL_C\n#" << endl;

  if (bTd2TS) {
    ConvertTd2TS();   // this should not be done in this program
    exit(EXIT_SUCCESS);
  }

  initialize();                 // initialize parameters related to digitisers and detectors

  int  evnum = 0;
  bool evok = true;

  int error_code = 0;
  uint64_t oldTst = 0;
  int64_t evtOffset = 0;
  if (printModulo < 2) printModulo = 0;
  eventCount = 0;

  double cpuTime0 = get_cpu_time();
  double wallTime0 = get_wall_time();

  if (bCount) {
    cout << "#      counting outer frames ... " << endl;
  }
  else {
    if (!bFull)
      cout << "#      event     bOffset     bSize  0xKeyADF   domain        timeStamp     dTstamp    keyADFname" << endl;
    else
      cout << "#      event     bOffset     bSize  0xKeyADF   domain   (rel)timeStamp     dTstamp    keyADFname" << endl;
  }

  ////////////////////////
  // the execution loop //

  while (true) {

    int evSize = NextEvent(inpBuff, bufferSize);
    if (evSize <= 0)
      break;

    if (bCount) {
      eventCount++;
      if (gterminate || (maxEvents > 0 && eventCount >= maxEvents))
        break;
      continue;
    }

    uint32_t frameLen = inpBuff[0];
    uint32_t frameKey = inpBuff[1];
    uint64_t frameTst = GetTstamp(inpBuff);

    // list main adf-key and DeltaTstamp
    if ((printModulo == 0) || (eventCount%printModulo == 0)) {
      cout << setw(12) << eventCount << setw(12) << evtOffset * 4;
      cout << setw(10) << frameLen;
      cout << hex << setw(10) << frameKey << dec;
      cout << " " << setw(10) << (inpBuff[2] & keyDomainMask);
      cout << " " << setw(16) << frameTst;
      if (oldTst == 0)
        cout << setw(15) << " ";
      else if (frameTst >= oldTst)
        cout << " " << setw(11) << frameTst - oldTst << "   ";
      else
        cout << " " << setw(11) << -int64_t(oldTst - frameTst) << " * ";

      // if (LUT.find(frameKey) != LUT.end())
      cout << OutDataType(frameKey);//LUT[frameKey];
      cout << endl;
    }
    rMap.clear();
    uint32_t lenTot = 4 * MapOneFrame(inpBuff, 0, 0);     // recursively map frames
    if (lenTot != frameLen) {                             // should not happen
      cout << " ERROR: Mapped buffer length is " << lenTot
	   << "   Expected value is " << frameLen
	   << "  (fOffs = " << evtOffset * 4 << " )" << endl;
    }

    size_t rMapLen = rMap.size();
    if (bFull && ((printModulo == 0) || (eventCount%printModulo == 0))) {
      // list inner keys
      for (size_t nn = 1; nn < rMapLen; nn++) {    // skipping the outer frame which was already printed above
        uint32_t * i4Buf = rMap[nn].ptr;
        uint32_t keyFrame = i4Buf[1];
        cout << setw(34) << i4Buf[0] << hex << setw(10) << i4Buf[1] << dec;
        cout << " " << setw(10) << GetEvtnum(i4Buf);
        if (frameTst)
          cout << " " << setw(16) << GetTstamp(i4Buf) - frameTst; // re timestamp of outermost frame
        else
          cout << setw(17) << " ";
        //if (LUT.find(keyFrame) != LUT.end()) {
	cout << setw(16) << " ";
	for (uint32_t ii = 0; ii < rMap[nn].level; ii++)
	  cout << ".";
	cout << OutDataType(keyFrame);//LUT[keyFrame];
	//}
      cout << endl;
    }
  }

  evtOffset += evSize;
  oldTst = frameTst;


  eventCount++;

  if (gterminate || (maxEvents > 0 && eventCount >= maxEvents))
    break;

}

// end of execution loop //
///////////////////////////

cerr << "# " << endl;

double cpuTime = get_cpu_time() - cpuTime0;
double secTime = get_wall_time() - wallTime0;

cout << "#\n# " << setw(12) << eventCount << " events in " << (int(secTime * 100)) / 100. << " seconds" << endl;

cout << "#" << endl;

exit(EXIT_SUCCESS);
}

void getparams(int argc, char *argv[])
{
  int   argn;
  char *cmd;
  int   ok;

  ok = (argc > 1) ? 1 : 0;
  argn = 1;

  while (ok == 1 && argn < argc) {
    cmd = argv[argn++];
    ok = 1;
    if (!strcmp(cmd, "-h") || !strcmp(cmd, "--help")) {
      ok = -1;
    }
    else if (!strcmp(cmd, "-f")) {
      numparams(cmd, argc - argn - 1);
      inpFileName = argv[argn++];
    }
    else if (!strcmp(cmd, "-n")) {
      numparams(cmd, argc - argn - 1);
      maxEvents = atoi(argv[argn++]);
    }
    else if (!strcmp(cmd, "-m")) {
      numparams(cmd, argc - argn - 1);
      printModulo = atoi(argv[argn++]);
    }
    else if (!strcmp(cmd, "-j")) {
      numparams(cmd, argc - argn - 1);
      skipBytes = atol(argv[argn++]);
    }
    else if (!strcmp(cmd, "-c")) {
      bFull = false;
    }
    else if (!strcmp(cmd, "-C")) {
      bFull = false;
      bCount = true;
    }
    else if (!strcmp(cmd, "-d2t")) {
      bTd2TS = true;
    }
    else {
      ok = 0;
    }
  }

  if (ok < 1) {
    if (ok == 0 && argc > 1) cout << "\nInvalid switch " << argv[argn - 1] << endl;
    cout << "\nUsage: " << argv[0] << " -[...]" << endl << endl;
    cout << "  -h              print this list" << endl;
    cout << "  -f    file      name of input file" << endl;
    cout << "  -n    ne        number of events to analyze [" << maxEvents << "] (0 == all)" << endl;
    cout << "  -m    mm        printout modulo [1] (0 == all)" << endl;
    cout << "  -j    nb        skip the first nb bytes of the input file" << endl;
    cout << "  -c              compact view, showing only the outermost frame" << endl;
    cout << "  -C              only count outermost frames" << endl;
    cout << "  -d2t            take a tstdiff binary file and print ascii timestamps" << endl;
    cout << " " << endl;
    if (argc > 1 || ok < 0)
      exit(-1);
  }

  if (inpFileName.size() == 0) {
    cout << "Name of input file MUST be given \n" << endl;
    exit(EXIT_FAILURE);
  }

  // echo the command line
  time_t ltime;
  time(&ltime);
  cout << "# " << endl << "# " << ctime(&ltime);
  cout << "# " << argv[0] << endl;
  cout << "# ";
  for (int nn = 1; nn < argc; nn++) {
    cout << " " << argv[nn];
  }
  cout << endl << "# " << endl;
}

void
numparams(char * cmd, int nn)
{
  if (nn >= 0) return;
  nn = -nn;
  printf("error decoding command %s ==> %d parameter(s) missing \n", cmd, nn);
  exit(-1);
}

// initializations after reading the command line
void initialize()
{
  inpFILE = fopen(inpFileName.c_str(), "rb");
  if (!inpFILE) {
    cout << "Error opening " << inpFileName << endl;
    exit(EXIT_FAILURE);
  }
  if (skipBytes > 0)
    fseek(inpFILE, skipBytes, SEEK_SET);

  // derived from AgataSoftware/lib/ADF.conf
  // We will use the DataFrames class to get this
  //   LUT[0xFA010101] = "data:crystal";
  //   LUT[0xFA010111] = "data:ccrystal";
  //   LUT[0xFA010102] = "data:gasp";
  //   LUT[0xFA010103] = "data:tc";
  //   LUT[0xFA010104] = "data:merger";
  //   LUT[0xFA010105] = "data:tracked";
  //   LUT[0xFA0201A0] = "data:vme";
  //   LUT[0xFA0201A1] = "data:euclides";
  //   LUT[0xFA0201A2] = "data:nwdigital";
  //   LUT[0xFA0201A3] = "data:labr";
  //   LUT[0xFA0201A4] = "data:spider";
  //   LUT[0xFA0201A5] = "data:trace";
  //   LUT[0xFA0201A6] = "data:v1730";
  //   LUT[0xFA0201A7] = "data:rfd";
  //   LUT[0xFA0201A8] = "data:ranc8";
  //   LUT[0xFA0201A9] = "data:ranc9";
  //   LUT[0xFA001100] = "meta:eof";
  //   LUT[0xFA011101] = "meta:sync";
  //   LUT[0xFA011102] = "meta:vertex";
  //   LUT[0xFA000200] = "conf:global";
  //   LUT[0xFA010201] = "conf:crystal";
  //   LUT[0xFA010202] = "conf:psa";
  //   LUT[0xFA010203] = "conf:eb";
  //   LUT[0xFA010204] = "conf:merger";
  //   LUT[0xFA010205] = "conf:tracked";
  //   LUT[0xFA0202A0] = "conf:ranc0";
  //   LUT[0xFA0202A1] = "conf:ranc1";
  //   LUT[0xFA0202A2] = "conf:ranc2";
  //   LUT[0xFA010301] = "algo:crystal";
  //   LUT[0xFA010311] = "algo:ccrystal";
  //   LUT[0xFA010302] = "algo:psa";
  //   LUT[0xFA010303] = "algo:eb";
  //   LUT[0xFA010304] = "algo:merger";
  //   LUT[0xFA010305] = "algo:tracked";
  //   LUT[0xCA000100] = "event:data";
  //   LUT[0xCA010101] = "event:data:crystal";
  //   LUT[0xCA010102] = "event:data:psa";
  //   LUT[0xCA010103] = "event:data:tracked";
  //   LUT[0xCA010104] = "event:data:labr";
   
}

// "space" and return value in units of uint32_t
int NextEvent(uint32_t *inp, int space)
{
  if (space < sizeof(agataKey) / sizeof(uint32_t))
    return -1;

  size_t llh = fread(inp, sizeof(agataKey), 1, inpFILE);
  if (llh != 1)
    return -2;

  int evlen = inp[0] / sizeof(uint32_t);                      // total event length in I4
  int rest = evlen - sizeof(agataKey) / sizeof(uint32_t);     // to be read

  size_t lld = fread(inp + sizeof(agataKey) / sizeof(uint32_t), sizeof(uint32_t), rest, inpFILE);
  if (lld != rest)
    return -3;

  return evlen;
}

// taken from AgataSoftware/agapro/filters/GlobalFilter
uint32_t MapOneFrame(uint32_t * i4Buf, uint32_t level, uint32_t parent)
{
  rMap.push_back(rMap_t(i4Buf, parent, level));

  uint32_t lenFrame4 = i4Buf[0] / 4;
  uint32_t keyFrame  = i4Buf[1];

  if ((keyFrame & 0xF0000000) != 0xC0000000)
    return lenFrame4;   // not a composite frame

  // recurse as composite
  parent = rMap.size()-1;
  uint32_t lenKey4 = 5;
  uint32_t lenTot4 = lenKey4;
  uint32_t i4Next  = lenKey4;
  while (i4Next < lenFrame4) {  // scan the contained frames
    uint32_t len4 = MapOneFrame(i4Buf + i4Next, level + 1, parent);
    lenTot4 += len4;
    i4Next  += len4;
  }

  return lenTot4;
}

// convert a binary list of uint32_t timestamp differences in an ascii list of timestamps
void  ConvertTd2TS()
{
  inpFILE = fopen(inpFileName.c_str(), "rb");
  if (!inpFILE) {
    cout << "Error opening " << inpFileName << endl;
    exit(EXIT_FAILURE);
  }

  uint64_t tStamp = 0;
  uint32_t dDiffs = 0;
  size_t count = 0;
  size_t nr;
  nr = fread(&dDiffs, sizeof(dDiffs), 1, inpFILE);
  if (nr != 1)
    goto errexit;
  tStamp = uint64_t(dDiffs) * 100000000;
  while (true) {
    nr = fread(&dDiffs, sizeof(dDiffs), 1, inpFILE);
    if (nr != 1)
      break;
    tStamp += dDiffs;
    cout << setw(10) << count++ << setw(15) << tStamp << endl;
    if (gterminate)
      break;
  }

 errexit:
  if (count == 0) {
    cout << "Error reading" << inpFileName << endl;
    exit(EXIT_FAILURE);
  }
  else {
    cout << endl;
    return;
  }
}
