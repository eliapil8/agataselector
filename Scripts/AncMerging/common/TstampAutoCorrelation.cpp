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
#include <string>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <map>

using namespace std;

const int      adfKeyLength = 5;
const int      adfTotLength = 0;
const int      adfDomainID  = 2;
const int      adfTimestamp = 3;
const uint32_t adfMaskIdles = 0x80000000;

const int  ggpKeyLength = 16;
const int  ggpTimestamp = 3;
const int  ggpTotLength = 7;
const int  ggpDomainID  = 8;
const int  ggpStatusPos = 9;
const uint16_t ggpMaskIdles = 0x0008;
const uint16_t ggpMaskPiled = 0x0001;

string  inpFileName;
int     format = 0;           // 0-binary 1-ascii 2-adf 3-ggp
int     domain = -1;          // take everything (ggp stuff)
bool    isTstamp  = false;    // data is timestamps or timestamp_diff
bool    wTstdiffA = false;    // ascii
bool    wTstdiffB = false;    // UI32
bool    wTstampA  = false;    // ascii
bool    wTstampB  = false;    // UI64
bool    wTstampEA = false;    // UI64 , E=1  to test Luciano's triggers
bool    rejectIdles = true;   // reject idle events (ggp stuff)
bool    rejectPiled = false;  // reject pileup events (ggp stuff)

uint64_t tStampOffset = 0;

vector<uint64_t> vTstamp;

size_t  maxEvents = 0;

int  tstScale = 100;
int  maxLag   = 1000000;

// callback for SIGINT
int  gterminate;
void terminate(int sig) {
  gterminate++;
  if(gterminate > 3)
    exit(-1);
}

clock_t startTime, stopTime;

void  getparams  (int, char *[]);
void  numparams  (char *, int);
bool  readBinary ();
bool  readAscii  ();
bool  readADF    ();
bool  readGGP    ();
uint64_t adfGetTstamp(uint32_t *i4v) { return (uint64_t)i4v[adfTimestamp+1] << 32 | (0x00000000FFFFFFFFULL & (uint64_t)i4v[adfTimestamp]); }
uint64_t ggpGetTstamp(uint16_t *i2v) { return (uint64_t(i2v[ggpTimestamp+2]) << 32) | (uint64_t(i2v[ggpTimestamp+1]) << 16) | (uint64_t(i2v[ggpTimestamp])); }

int main(int argc, char *argv[])
{
  getparams(argc, argv);        // parameters from the command line

  bool ok = format==0 ? readBinary()
          : format==1 ? readAscii()
          : format==2 ? readADF()
          : format==3 ? readGGP()
          : false;
  if (!ok)
    exit(EXIT_FAILURE);

  size_t lenTstamp = vTstamp.size();
  cout << setw(15) << lenTstamp << " timestamps" << endl;
  if (lenTstamp < 2) {
    cout << "Too short " << endl;
    exit(EXIT_FAILURE);
  }
  cout << setw(15) << vTstamp[0] << " first" << endl;
  cout << setw(15) << vTstamp[lenTstamp-1] << " last" << endl;
  uint64_t rtime = vTstamp[lenTstamp - 1] - vTstamp[0];
  cout << setw(7) << (vTstamp[lenTstamp - 1] - vTstamp[0])/100000000ULL << "         seconds" << endl;

  if (tStampOffset) {
    for (size_t nn = 0; nn < lenTstamp; nn++)
      vTstamp[nn] -= tStampOffset;
    cout << setw(15) << tStampOffset << " subtracted offset" << endl;
  }

  if (wTstdiffA) {
    string oFname = inpFileName + ".tstdiff.ascii";
    ofstream os(oFname.c_str());
    if (!os) {
      cout << "Error opening file " << oFname << endl;
      exit(EXIT_FAILURE);
    }
    os << 0 << "\n";
    for (size_t nn = 1; nn < vTstamp.size(); nn++)
      os << vTstamp[nn] - vTstamp[nn - 1] << "\n";
    os.close();   // could also let it go out of scope
    cout << "\nTimestamp differences written to " << oFname << endl << endl;
  }

  if (wTstdiffB) {
    string oFname = inpFileName + ".tstdiff.UI32";
    ofstream os(oFname.c_str(), ios::binary);
    if (!os) {
      cout << "Error opening file " << oFname << endl;
      exit(EXIT_FAILURE);
    }
    uint32_t td = 0;
    os.write((char *)(&td), sizeof(td));
    for (size_t nn = 1; nn < vTstamp.size(); nn++) {
      td = uint32_t(vTstamp[nn] - vTstamp[nn - 1]);
      os.write((char *)(&td), sizeof(td));
    }
    os.close();   // could also let it go out of scope
    cout << "\nTimestamp differences written to " << oFname << endl << endl;
  }

  if (wTstampA) {
    string oFname = inpFileName + ".tstamp.ascii";
    ofstream os(oFname.c_str());
    if (!os) {
      cout << "Error opening file " << oFname << endl;
      exit(EXIT_FAILURE);
    }
    for (size_t nn = 0; nn < vTstamp.size(); nn++)
      os << vTstamp[nn] << "\n";
    os.close();   // could also let it go out of scope
    cout << "\nTimestamps written to " << oFname << endl << endl;
  }

  if (wTstampB) {
    string oFname = inpFileName + ".tstamp.UI64";
    ofstream os(oFname.c_str(), ios::binary);
    if (!os) {
      cout << "Error opening file " << oFname << endl;
      exit(EXIT_FAILURE);
    }
    for (size_t nn = 0; nn < vTstamp.size(); nn++) {
      uint64_t ts = vTstamp[nn];
      os.write((char *)(&ts), sizeof(ts));
    }
    os.close();   // could also let it go out of scope
    cout << "\nTimestamps written to " << oFname << endl << endl;
  }

  if (wTstampEA) {
    string oFname = inpFileName + ".TE.ascii";
    ofstream os(oFname.c_str());
    if (!os) {
      cout << "Error opening file " << oFname << endl;
      exit(EXIT_FAILURE);
    }
    for (size_t nn = 0; nn < vTstamp.size(); nn++)
      os << vTstamp[nn] << " 1\n";
    os.close();   // could also let it go out of scope
    cout << "\nTimestamp  and Energy written to " << oFname << endl << endl;
  }

  cout << "Step = " << tstScale << "   Lag = " << maxLag << endl;
  if (maxLag < 1) {
    exit(EXIT_SUCCESS);
  }

  uint32_t* pAuto = new uint32_t[maxLag]; memset(pAuto, 0, maxLag*sizeof(uint32_t));

  size_t n1Max = lenTstamp-1;
  uint64_t tstMax = vTstamp[n1Max];
  if (tstMax > maxLag) {
    tstMax = tstMax - maxLag;
    for (; n1Max > 0; n1Max--) {
      if (vTstamp[n1Max] < tstMax)
        break;
    }
  }

  gterminate = 0;
  signal(SIGINT, terminate);
  cout << "#\n# Analysis can be stopped by typing CTRL_C\n#" << endl << flush;

  // brute force
  for (size_t n1 = 0; n1 < n1Max; n1++) {
    if ((n1 % 10000) == 0 && n1)
      cout << setw(8) << n1 << endl;
    pAuto[0]++;
    uint64_t tStamp = vTstamp[n1];
    for (size_t n2 = n1 + 1; n2 < lenTstamp; n2++) {
      int td = int((vTstamp[n2] - tStamp)/tstScale);
      if (td < 0 || td >= maxLag)
        break;
      pAuto[td]++;
    }
    if (gterminate)
      break;
  }

  string outFileName = inpFileName + ".corr";
  FILE* outFILE = fopen(outFileName.c_str(), "wb");
  if (!outFILE) {
    cout << "Error opening " << outFileName << endl;
    exit(EXIT_FAILURE);
  }

  cout << "Writing " << outFileName << " __" << maxLag << "-UI__" << endl;
  size_t nw = fwrite(pAuto, sizeof(uint32_t), maxLag, outFILE);
  fclose(outFILE);

  delete[] pAuto;

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
    else if (!strcmp(cmd, "-bd")) {
      numparams(cmd, argc - argn - 1);
      inpFileName = argv[argn++];
      format = 0;
      isTstamp = false;
    }
    else if (!strcmp(cmd, "-bt")) {
      numparams(cmd, argc - argn - 1);
      inpFileName = argv[argn++];
      format = 0;
      isTstamp = true;
    }
    else if (!strcmp(cmd, "-ad")) {
      numparams(cmd, argc - argn - 1);
      inpFileName = argv[argn++];
      format = 1;
      isTstamp = false;
    }
    else if (!strcmp(cmd, "-at")) {
      numparams(cmd, argc - argn - 1);
      inpFileName = argv[argn++];
      format = 1;
      isTstamp = true;
    }
    else if (!strcmp(cmd, "-adf")) {
      numparams(cmd, argc - argn - 1);
      inpFileName = argv[argn++];
      format = 2;
      isTstamp = true;
    }
    else if (!strcmp(cmd, "-ggp")) {
      numparams(cmd, argc - argn - 1);
      inpFileName = argv[argn++];
      format = 3;
      isTstamp = true;
    }
    else if (!strcmp(cmd, "-dom")) {
      numparams(cmd, argc - argn - 1);
      domain= atoi(argv[argn++]);
    }
    else if (!strcmp(cmd, "-pur")) {
      rejectPiled = true;
    }
    else if (!strcmp(cmd, "-wda")) {
      wTstdiffA = true;
    }
    else if (!strcmp(cmd, "-wdb")) {
      wTstdiffB = true;
    }
    else if (!strcmp(cmd, "-wta")) {
      wTstampA = true;
    }
    else if (!strcmp(cmd, "-wtb")) {
      wTstampB = true;
    }
    else if (!strcmp(cmd, "-wtea")) {
      wTstampEA = true;
    }
    else if (!strcmp(cmd, "-n")) {
      numparams(cmd, argc - argn - 1);
      maxEvents = atoi(argv[argn++]);
    }
    else if (!strcmp(cmd, "-T0")) {
      numparams(cmd, argc - argn - 1);
      tStampOffset = atoll(argv[argn++]);
    }
    else if (!strcmp(cmd, "-step")) {
      numparams(cmd, argc - argn - 1);
      tstScale = atoi(argv[argn++]);
    }
    else if (!strcmp(cmd, "-lag")) {
      numparams(cmd, argc - argn - 1);
      maxLag = atoi(argv[argn++]);
    }
    else {
      ok = 0;
    }
  }

  if (ok < 1) {
    if (ok == 0 && argc > 1) cout << "\nInvalid switch " << argv[argn - 1] << endl;
    cout << "\nUsage: " << argv[0] << " -[...]" << endl << endl;
    cout << "  -h              print this list" << endl;
    cout << "  -bd   file      input file containing tstdiff__UI32 values" << endl;
    cout << "  -bt   file      input file containing tstamps__UI64 values" << endl;
    cout << "  -ad   file      input file containing tstdiff__ascii values" << endl;
    cout << "  -at   file      input file containing tstamps__ascii values" << endl;
    cout << "  -adf  file      input file containing adf frames" << endl;
    cout << "  -ggp  file      input file containing ggp raw frames" << endl;
    cout << "  -dom  nn        select domain for ggp or adf-galileo data [-1 == all]" << endl;
    cout << "  -pur            reject pileup events for ggp data" << endl;
    cout << "  -n    nn        number of events to analyze [" << maxEvents << "] (0 == all)" << endl;
    cout << "  -T0   nn        subtract this value from timestamps" << endl;
    cout << "  -wda            write timestamp differences to file.tstdiff as ascii" << endl;
    cout << "  -wdb            write timestamp differences to file.tstdiff as UI32" << endl;
    cout << "  -wta            write timestamp to file.tstamp as ascii" << endl;
    cout << "  -wtb            write timestamp to file.tstamp as UI64" << endl;
    cout << "  -wtea           write timestamp and energy (==1) to file.tstener as ascii" << endl;
    cout << "  -step           scale down factor for timestamps [100]" << endl;
    cout << "  -lag            maximum lag of autocorrelation [1000000]" << endl;
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
  for (int nn = 1; nn < argc; nn++)
    cout << " " << argv[nn];
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

bool readBinary()
{
  FILE* inpFILE = fopen(inpFileName.c_str(), "rb");
  if (!inpFILE) {
    cout << "Error opening " << inpFileName << endl;
    return false;
  }


  if (isTstamp) {
    uint64_t tStamp = 0;
    while (true) {
      size_t nr = fread(&tStamp, sizeof(tStamp), 1, inpFILE);
      if (nr != 1)
        break;
      vTstamp.push_back(tStamp);
      if (maxEvents && vTstamp.size() >= maxEvents)
        break;
    }
  }
  else {
    uint64_t tStamp = 0;
    uint32_t dDiffs = 0;
    // tstfirst in seconds, assuming 100 Ms/s
    size_t nr = fread(&dDiffs, sizeof(dDiffs), 1, inpFILE);
    if (nr != 1) {
      cout << "Error reading " << inpFileName << endl;
      exit(EXIT_FAILURE);
    }
    tStamp = uint64_t(dDiffs) * 100000000;
    while (true) {
      size_t nr = fread(&dDiffs, sizeof(dDiffs), 1, inpFILE);
      if (nr != 1)
        break;
      tStamp += dDiffs;
      vTstamp.push_back(tStamp);
      if (maxEvents && vTstamp.size() >= maxEvents)
        break;
    }
  }

  fclose(inpFILE);
  return true;

}

bool readAscii()
{
  FILE* inpFILE = fopen(inpFileName.c_str(), "r");
  if (!inpFILE) {
    cout << "Error opening " << inpFileName << endl;
    return false;
  }

  if (isTstamp) {
    uint64_t tStamp = 0;
    while (true) {
      size_t nr = fscanf(inpFILE, "%llu", &tStamp);
      if (nr != 1)
        break;
      vTstamp.push_back(tStamp);
      if (maxEvents && vTstamp.size() >= maxEvents)
        break;
    }
  }
  else {
    uint64_t tStamp = 0;
    uint32_t dDiffs = 0;
    size_t nr = fscanf(inpFILE, "%llu", &tStamp);
    if (nr != 1) {
      cout << "Error reading " << inpFileName << endl;
      exit(EXIT_FAILURE);
    }
    vTstamp.push_back(tStamp);
    while (true) {
      size_t nr = fscanf(inpFILE, "%uld", &dDiffs);
      if (nr != 1)
        break;
      tStamp += dDiffs;
      vTstamp.push_back(tStamp);
      if (maxEvents && vTstamp.size() >= maxEvents)
        break;
    }
  }

  fclose(inpFILE);
  return true;
}

// only outer frame
bool readADF()
{
  FILE* inpFILE = fopen(inpFileName.c_str(), "rb");
  if (!inpFILE) {
    cout << "Error opening " << inpFileName << endl;
    return false;
  }

  uint32_t akey[adfKeyLength];
  uint64_t tStamp = 0;
  while (true) {
    size_t nr = fread(akey, sizeof(uint32_t), adfKeyLength, inpFILE);
    if (nr != adfKeyLength)
      break;
    if ((akey[adfDomainID] & adfMaskIdles) == 0) {        // skip idles
      int dom = domain < 0 ? domain : akey[adfDomainID];  // check domain
      if (dom == domain) {
        tStamp = adfGetTstamp(akey);
        vTstamp.push_back(tStamp);
        if (maxEvents && vTstamp.size() >= maxEvents)
          break;
      }
    }
    size_t rest = akey[adfTotLength] - sizeof(akey);      // bytes
    if (rest)
      fseek(inpFILE, rest, SEEK_CUR);  // skip rest
  }

  fclose(inpFILE);
  return true;
}

bool  readGGP()
{
  FILE* inpFILE = fopen(inpFileName.c_str(), "rb");
  if (!inpFILE) {
    cout << "Error opening " << inpFileName << endl;
    return false;
  }

  uint16_t gkey[ggpKeyLength];
  uint16_t statusMask = 0;
  if (rejectIdles) statusMask |= ggpMaskIdles;
  if (rejectPiled) statusMask |= ggpMaskPiled;
  uint64_t tStamp = 0;
  while (true) {
    size_t nr = fread(gkey, sizeof(uint16_t), ggpKeyLength, inpFILE);
    if (nr != ggpKeyLength)
      break;
    if ((gkey[ggpStatusPos] & statusMask) == 0) {         // skip idles+piled
      int dom = domain < 0 ? domain : gkey[ggpDomainID];  // check domain
      if (dom == domain) {
        tStamp = ggpGetTstamp(gkey);
        vTstamp.push_back(tStamp);
        if (maxEvents && vTstamp.size() >= maxEvents)
          break;
      }
    }
    size_t rest = gkey[ggpTotLength] - ggpKeyLength;      // short
    if (rest)
      fseek(inpFILE, rest*sizeof(uint16_t), SEEK_CUR);    // skip rest
  }

  return true;
}
