///////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////

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

#include "Misc.h"
#include "DataFrames.h"

using namespace std;

const int maxDoms = 256;

string  inpFileName;
FILE   *inpFILE;

string   inpFileList;
ifstream listStream;

string    outFileName;
string    outName[maxDoms];
FILE     *outFILE[maxDoms] = {nullptr};
bool      outUnique = false;
bool      outIdles  = true;

const int promptModMin = 100000;
int       promptModulo = 0;
int       promptFactor = 1;

const int bufferSize = 1000000;
uint16_t  inpBuff[bufferSize];

int  numDoms   = maxDoms;
int  domOffs   = 0;
int  maxEvents = 0;
int  eventCount;

uint32_t adfTag    = 0;         // !=0 to add an adfkey to the output events
bool     adfKeep   = false;     // data content not written
uint64_t timeStamp = 0;         // calculated only if adfTag

struct dataHeader {
  // POD with 16 uint16_t
  uint16_t key;
  uint16_t evn_0;
  uint16_t evn_1;
  uint16_t ts_0;
  uint16_t ts_1;
  uint16_t ts_2;
  uint16_t num_samples;
  uint16_t total_length;
  uint16_t crystal_id;
  uint16_t status; // bit_0=Pile-up; bit_1=0 bit_2=0; bit_3=Idle-bit; bit_4...bit_15=0
  uint16_t spare[6];

  uint32_t GetEvnum  () {return (uint32_t(evn_1)<<16) | (uint32_t(evn_0));}
  uint64_t GetTstamp () {return (uint64_t(ts_2 )<<32) | (uint64_t(ts_1 )<<16) | (uint64_t(ts_0));}
  uint16_t GetStatus () {return (status&0xF);}  // keep only the 4 defined bits
  uint16_t GetNumSamp() {return num_samples;}
  bool     IsData()  {return (key&inpMagicMask) == inpMagicData;}
  bool     IsWave()  {return (key&inpMagicMask) == inpMagicWave;}
  bool     IsSpec()  {return (key&inpMagicMask) == inpMagicSpec;}
  bool     IsIdle()  {return ((status&inpIdleMask) != 0);}
  bool     IsPileUp(){return ((status&inpPileMask) != 0);}
};

int  gterminate;

bool listKeys = false;

// callback for SIGINT
void terminate(int sig) {
  gterminate++;
  if(gterminate > 3)
    exit(-1);
}

clock_t startTime, stopTime;

void  getparams       (int, char *[]);
void  numparams       (char *, int);
void  initialize      ();
void  updatePrompt    (ostream &);// cyclic info on terminal or list
int   NextEvent       (uint16_t *inp, int space);
bool  NextFile        ();
bool  WriteEvent      (uint16_t *inp, int size, int dom);    // write event to the proper file(s)

int main(int argc, char *argv[])
{
  getparams(argc, argv);        // parameters from the command line

  initialize();                 // initialize parameters related to digitisers and detectors

  gterminate = 0;
  signal(SIGINT, terminate);
  cout << "#\n# Analysis can be stopped by typing CTRL_C\n#" << endl << flush;

  const int maxRawEvLen = (1<<16) - 1;                   // coded with 16 bits
  const int maxInpSize  = bufferSize - 2*maxRawEvLen;    // with some spare

  int  evnum = 0;
  bool evok  = true;
  startTime  = clock();

  if(promptModulo <= 0)
    promptModulo = promptModMin;

  eventCount = 0;
  bool done = false;

  ////////////////////////
  // the execution loop //

  while( !done ) {
    // periodic printouts     --> now in LocalFilter::process_block
    if(eventCount && !(eventCount % promptModulo)) {
      cerr << "\r"; updatePrompt(cerr);
    }

    int evSize = NextEvent( inpBuff, bufferSize);

    if(evSize <= 0) {
      if( NextFile() )
        evSize = NextEvent( inpBuff, bufferSize);
      else
        done = true;
    }

    if(evSize > 0) {
      eventCount++;
    }

    done |= gterminate  || (maxEvents>0 && eventCount>=maxEvents);

    if(done)
      break;
  }
  cerr << "# " << endl;

  // end of execution loop //
  ///////////////////////////

  stopTime = clock();
  double realTime = (double)(stopTime - startTime) / CLOCKS_PER_SEC;

  cout << "#\n# " << setw(12) << eventCount << " events in " << (int(realTime*100))/100 <<  " seconds " << endl;
  cout << "#" << endl;

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

void
getparams(int argc, char *argv[])
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
    else if(!strcmp(cmd, "-unique")) {
      outUnique = true;
    }
    else if(!strcmp(cmd, "-noidle")) {
      outIdles = false;
    }
    else if(!strcmp(cmd, "-nd")) {
      numparams(cmd, argc-argn-1);
      numDoms = atoi(argv[argn++]);
    }
    else if(!strcmp(cmd, "-do")) {
      numparams(cmd, argc-argn-1);
      domOffs = atoi(argv[argn++]);
    }
    else if(!strcmp(cmd, "-ne")) {
      numparams(cmd, argc-argn-1);
      maxEvents = atoi(argv[argn++]);
    }
    else if(!strcmp(cmd, "-pr")) {
      numparams(cmd, argc-argn-1);
      promptFactor = atoi(argv[argn++]);
    }
    else if(!strcmp(cmd, "-adf")) {
      numparams(cmd, argc-argn-2);
      adfTag  = strtoul(argv[argn++], nullptr, 0);
      adfKeep = atoi(argv[argn++]) != 0;
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
    cout << "  -unique          all events in the same file (no suffix to output file name)" << endl;
    cout << "  -noidle          remove idles from the output file(s)" << endl;
    cout << "  -ne    nn        number of events to analyze [" << maxEvents << "] (0 == all)" << endl;
    cout << "  -nd    nn        number of domains [" << numDoms << "]" << endl ;
    cout << "  -do    nn        offset applied to domainID [" << domOffs << "]" << endl ;
    cout << "  -pr    nn        scale down factor for prompt lines [" << promptFactor << "]" << endl;
    cout << "  -adf   key keep  key!=0 to add an adf frame to the data; keep==0 to write only the frame [" << adfTag << " " << adfKeep << "]" << endl;
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

void
numparams(char * cmd, int nn)
{
  if(nn >= 0) return;
  nn = -nn;
  printf("error decoding command %s ==> %d parameter(s) missing \n", cmd, nn);
  exit(-1);
}

// initializations after reading the command line
void initialize()
{
  if( outFileName.empty() ) {
    cout << endl << " Output file name cannot be empty " << endl;
    exit(EXIT_FAILURE);
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
  }


}

bool NextFile()
{
  cout << "\nuint16_t *inp# Done " << inpFileName << endl;
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

 // "space" and return value in units of uint16_t
int NextEvent(uint16_t *inp, int space)
{
  if( space < sizeof(dataHeader)/sizeof(uint16_t) )
    return -1;

  const int lhead = sizeof(dataHeader)/sizeof(uint16_t);

  size_t llh = fread(inp, sizeof(uint16_t), lhead, inpFILE);
  if(llh != lhead)
    return -2;

  int evlen = 0;  // units of uint16_t
  if(inp[0]==inpMagicData || inp[0]==inpMagicWave) {
    evlen = inp[7];
  }
  else if(inp[0]==inpMagicSpec) {
    evlen = inp[3]+lhead; // ?
  }

  int rest  = evlen - lhead;
  if(rest <= 0)
    return -5;

  size_t lld = fread(inp+lhead, sizeof(uint16_t), rest, inpFILE);
  if(lld != rest)
    return -6;

  dataHeader* head = (dataHeader*)inp;
  head->crystal_id -= domOffs;        // mind what you are doing
  if(outIdles || head->status == 0)  { // if outIdles, write irrespective of state
    if(adfTag)
      timeStamp = head->GetTstamp();
    WriteEvent(inp, evlen, head->crystal_id);
  }

  return evlen;
}

bool WriteEvent(uint16_t *out, int evSize, int evTag)
{
  if(evTag < 0 || evTag >= numDoms)
    return true; // ignore it

  if(outUnique)
    evTag = 0;
  if(outFILE[evTag] == nullptr) {
    ostringstream oss;
    oss << outFileName;
    if(!outUnique)
      oss << "_" << setw(3) << setfill('0') << evTag;
    outName[evTag] = oss.str();
    outFILE[evTag] = fopen(outName[evTag].c_str(), "wb");
    if(!outFILE[evTag]) {
      cout << "Error opening " << outName[evTag] << endl;
      exit(EXIT_FAILURE);
    }
  }
  if(adfTag == 0) {
    // original data
    size_t nw = fwrite(out, sizeof(uint16_t), evSize, outFILE[evTag]);
    if(nw != evSize) {
      cout << "\n# Error writing output file " << outName[evTag] << endl;
      return false;
    }
    return true;
  }

  // data into an adf frame
  uint32_t outBuffer[5];
  uint32_t outEvLen = 0;
  if(adfKeep) {
    outEvLen = evSize*sizeof(uint16_t);                               // bytes
    outEvLen = (outEvLen + sizeof(uint32_t) - 1) / sizeof(uint32_t); // uprounded words
  }

  outBuffer[0] = (outEvLen+5)*sizeof(uint32_t);                 // total length
  outBuffer[1] = adfTag;                                        // user defined
  outBuffer[2] = evTag;                                         // corrected domID
  outBuffer[3] = uint32_t( timeStamp        & 0xFFFFFFFFULL);   // tstLow
  outBuffer[4] = uint32_t((timeStamp >> 32) & 0xFFFFFFFFULL);   // tstHigh
  {
    size_t nw = fwrite(outBuffer, sizeof(uint32_t), 5, outFILE[evTag]);
    if(nw != 5) {
      cout << "\n# Error writing output file " << outName[evTag] << endl;
      return false;
    }
  }
  if(outEvLen > 0) {
    // original data
    size_t nw = fwrite(out, sizeof(uint32_t), outEvLen, outFILE[evTag]);
    if(nw != outEvLen) {
      cout << "\n# Error writing output file " << outName[evTag] << endl;
      return false;
    }
  }
  return true;
}
