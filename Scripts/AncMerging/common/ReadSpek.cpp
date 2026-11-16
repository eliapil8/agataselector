#include "ReadSpek.h"

#include <iostream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <algorithm>

using namespace std;

bool ReadSpek::Open(std::string sName, std::string sForm)
{
  if(specFILE)
    Close();

  if(sForm == "A")
    specFILE = fopen(sName.c_str(), "r");
  else
    specFILE = fopen(sName.c_str(), "rb");
  if(!specFILE) {
    return false;
  }
  specName   = sName;
  specFormat = sForm;  //  should verify validity
  return true;
}

ReadSpek::~ReadSpek()
{
  Close();
  if(specData) {
   delete [] specData;
   specData = nullptr;
  }
}

bool ReadSpek::Close()
{
  if(specFILE)
    fclose(specFILE);
  specFILE = nullptr;
  return true;
}

bool ReadSpek::InitData(int slen)
{
  if(specData) {
   delete [] specData;
   specData = nullptr;
  }

  specLength = slen;
  specData = new float[specLength];
  memset(specData, 0, sizeof(float)*specLength);

  return true;
}

bool ReadSpek::Read(int slen, int ns)
{
  if(!specFILE) {
    return false;   // open it first
  }

  if(!specData || specLength != slen)
    InitData(slen);

  bool readOK;

  if(specFormat ==  "S") {
    readOK = ReadSpek_S(ns, false);
  }
  else if(specFormat ==  "US") {
    readOK = ReadSpek_S(ns, true);
  }
  else if(specFormat ==  "I") {
    readOK = ReadSpek_I(ns, false);
  }
  else if(specFormat ==  "UI") {
    readOK = ReadSpek_I(ns, true);
  }
  else if(specFormat ==  "L") {
    readOK = ReadSpek_L(ns, false);
  }
  else if(specFormat ==  "UL") {
    readOK = ReadSpek_L(ns, true);
  }
  else if(specFormat ==  "F") {
    readOK = ReadSpek_F(ns);
  }
  else if(specFormat ==  "D") {
    readOK = ReadSpek_D(ns);
  }
  else if(specFormat ==  "A") {
    readOK = ReadSpek_A(ns);
  }
  else {
    readOK = false;
  }
  return readOK;
}

bool ReadSpek::ReadSpek_C(int ns, bool uns)
{
  if(uns) {
    long offset = ns*specLength*sizeof(unsigned char);
    if(fseek(specFILE, offset, SEEK_SET) )
      return false;
    unsigned char *iData = new unsigned char[specLength];
    size_t nr = fread(iData, sizeof(unsigned char), specLength, specFILE);
    if(nr < (size_t)specLength) {
      delete [] iData;
      return false;
    }
    for(int ii = 0; ii < specLength; ii++)
      specData[ii] = float(iData[ii]);
    delete [] iData;
    return true;
  }
  else {
    long offset = ns*specLength*sizeof(char);
    if(fseek(specFILE, offset, SEEK_SET) )
      return false;
    char *iData = new char[specLength];
    size_t nr = fread(iData, sizeof(char), specLength, specFILE);
    if(nr < (size_t)specLength) {
      delete [] iData;
      return false;
    }
    for(int ii = 0; ii < specLength; ii++)
      specData[ii] = float(iData[ii]);
    delete [] iData;
    return true;
  }
}

bool ReadSpek::ReadSpek_S(int ns, bool uns)
{
  if(uns) {
    long offset = ns*specLength*sizeof(unsigned short);
    if(fseek(specFILE, offset, SEEK_SET) )
      return false;
    unsigned short *iData = new unsigned short[specLength];
    size_t nr = fread(iData, sizeof(unsigned short), specLength, specFILE);
    if(nr < (size_t)specLength) {
      delete [] iData;
      return false;
    }
    for(int ii = 0; ii < specLength; ii++)
      specData[ii] = float(iData[ii]);
    delete [] iData;
    return true;
  }
  else {
    long offset = ns*specLength*sizeof(short);
    if(fseek(specFILE, offset, SEEK_SET) )
      return false;
    short *iData = new short[specLength];
    size_t nr = fread(iData, sizeof(short), specLength, specFILE);
    if(nr < (size_t)specLength) {
      delete [] iData;
      return false;
    }
    for(int ii = 0; ii < specLength; ii++)
      specData[ii] = float(iData[ii]);
    delete [] iData;
    return true;
  }
}

bool ReadSpek::ReadSpek_I(int ns, bool uns)
{
  if(uns) {
    long offset = ns*specLength*sizeof(unsigned int);
    if(fseek(specFILE, offset, SEEK_SET) )
      return false;
    unsigned int *iData = new unsigned int[specLength];
    size_t nr = fread(iData, sizeof(unsigned int), specLength, specFILE);
    if(nr < (size_t)specLength) {
      delete [] iData;
      return false;
    }
    for(int ii = 0; ii < specLength; ii++)
      specData[ii] = float(iData[ii]);
    delete [] iData;
    return true;
  }
  else {
    long offset = ns*specLength*sizeof(int);
    if(fseek(specFILE, offset, SEEK_SET) )
      return false;
    int *iData = new int[specLength];
    size_t nr = fread(iData, sizeof(int), specLength, specFILE);
    if(nr < (size_t)specLength) {
      delete [] iData;
      return false;
    }
    for(int ii = 0; ii < specLength; ii++)
      specData[ii] = float(iData[ii]);
    delete [] iData;
    return true;
  }
}

bool ReadSpek::ReadSpek_L(int ns, bool uns)
{
  if(uns) {
    long offset = ns*specLength*sizeof(unsigned long long);
    if(fseek(specFILE, offset, SEEK_SET) )
      return false;
    unsigned long long *iData = new unsigned long long[specLength];
    size_t nr = fread(iData, sizeof(unsigned long long), specLength, specFILE);
    if(nr < (size_t)specLength) {
      delete [] iData;
      return false;
    }
    for(int ii = 0; ii < specLength; ii++)
      specData[ii] = float(iData[ii]);
    delete [] iData;
    return true;
  }
  else {
    long offset = ns*specLength*sizeof(long long);
    if(fseek(specFILE, offset, SEEK_SET) )
      return false;
    long long *iData = new long long[specLength];
    size_t nr = fread(iData, sizeof(long long), specLength, specFILE);
    if(nr < (size_t)specLength) {
      delete [] iData;
      return false;
    }
    for(int ii = 0; ii < specLength; ii++)
      specData[ii] = float(iData[ii]);
    delete [] iData;
    return true;
  }
}

bool ReadSpek::ReadSpek_F(int ns)
{
  long offset = ns*specLength*sizeof(float);
  if( fseek(specFILE, offset, SEEK_SET) )
    return false;

  memset(specData, 0, sizeof(float)*specLength);
  size_t nr = fread(specData, sizeof(float), specLength, specFILE);
  if(nr < (size_t)specLength)
    return false;
  else
    return true;
}

bool ReadSpek::ReadSpek_D(int ns)
{
  long offset = ns*specLength*sizeof(double);
  if( fseek(specFILE, offset, SEEK_SET) )
    return false;

  memset(specData, 0, sizeof(float)*specLength);
  double *dData = new double[specLength];
  size_t nr = fread(dData, sizeof(double), specLength, specFILE);
  if(nr < (size_t)specLength) {
    delete [] dData;
    return false;
  }
  else {
    for(int ii = 0; ii < specLength; ii++)
      specData[ii] = float(dData[ii]);
    delete [] dData;
    return true;
  }
}

bool ReadSpek::ReadSpek_A(int ns)
{
  //if( fseek(specFILE, 0, SEEK_SET) ) // rewind the file
  //  return false;
  //ifstream ifs(specFILE);  // constructor not available in g++

  Close();
  ifstream ifs(specName.c_str(), std::ifstream::in);

  memset(specData, 0, sizeof(float)*specLength);
  ns = max(0, ns);

  int skip = specLength*ns;
  int seen = 0;
  string line;

  while( getline(ifs, line) ) {
    //StringTrim(line, "#");
    //if(line.size() == 0)  // empty or commented-out
    //  continue;
    istringstream il(line);
    for(;;) {
      float ff;
      il >> ff;
      if( il.fail() )
        break;
      if(skip > 0)
        skip--;
      else if(seen < specLength) {
        specData[seen++] = ff;
        if(seen == specLength)
          return true;
      }
      if( il.eof() )
        break;
    }
  }
  return (seen > 0);
}

#if 0
// imported from Misc
bool ReadSpek::StringTrim(std::string& str, const std::string& cmt)
{
  StringTrim(str);

  size_t ll = str.length();
  if(ll <= 0)
    return false;

  // check for comment substring
  size_t nn = str.find_first_of(cmt);
  if(nn == string::npos)
    return false;

  // and remove it to the end of line
  str = str.substr(0, nn);

  // and remove possible white space at the end
  StringTrim(str);

  return false;
}

void ReadSpek::StringTrim(std::string &str)
{
  size_t ll = str.length();
  // get rid of empty  strings
  if(ll <= 0)
    return;
  // get rid of the CR character
  // (e.g. files written on Windows (\r\n) and read on Unix(\n) )
  if(str[ll-1] == '\r') {
    str = str.substr(0, ll-1);
    ll--;
  }
  // discard if now empty
  if(ll <= 0)
    return;

  size_t ii = str.find_first_not_of(" \t");
  // contains only white space
  if(ii == string::npos) {
    str.clear();
    return;
  }

  // remove white space at the beginning
  if(ii > 0) {
    str = str.substr(ii);
    ll = str.length();
  }

  ii = str.find_last_not_of(" \t");
  // remove white space at the end
  if(ii < ll-1)
    str = str.substr(0, ii+1);
}
#endif

// Taken from TkT NameSpecDecode
// excluding the special cases (UA, UT, WT2, ...) where part of the info has to be extracted from the data

bool SpecNameDecode (std::string sName, std::string &sForm, int& slen)
{
  vector<int> vv;
  bool ok = SpecNameDecode (sName, sForm, vv);
  slen = (ok&&vv.size()>0) ? vv[vv.size()-1] : 0;
  return ok;
}

bool SpecNameDecode (std::string sName, std::string &sForm, std::vector<int>& vv)
{
  sForm.clear();
  vv.clear();

  // dire/Psa__2-10-100-100-US__XYZR.matr

  size_t sdir = sName.find_last_of("/");
  if(sdir == string::npos) sdir = 0;
  size_t sep1 = sName.find("__", sdir);
  if(sep1 == string::npos)
    return false;
  size_t sep2 = sName.find("__", sep1+2);
  if(sep2 == string::npos)
    return false;
  size_t sdot = sName.find(".", sep1);
  if(sdot != string::npos) {
    if(sdot < sep2 && sdot > sep1)    // not in the middle of the code
      return false;
  }

  string sCoded = sName.substr(sep1+2, sep2-sep1-2);
  size_t dash = sCoded.find_last_of('-');
  if(dash == string::npos) {
    sForm = sCoded;
  }
  else {
    // format must be present as last item
    sForm  = sCoded.substr(dash+1);
    sCoded = sCoded.substr(0, dash);
    // now decode the other tokens
    if(sCoded[0] == '-')
      sCoded = sCoded.substr(1);
    while( sCoded.size() ) {
      vv.push_back(atoi(sCoded.c_str()));
      size_t pos = sCoded.find("-");
      if(pos != std::string::npos)
        sCoded = sCoded.substr(pos+1);
      else
        break;
    }
  }

  //if(vv.size() == 1) {
  //  vv.insert(vv.begin(), 1);     // in TkT, add numK=1 to simplify life of caller
  //}

  for(size_t nn = 0; nn < vv.size(); nn++) {
    if(vv[nn] < 1)
      vv[nn] = 1;
  }

  return true;
}

bool SpecNameEncode(std::string& sName, std::string sForm, int slen)
{
  vector<int> vv;
  vv.push_back(slen);
  return SpecNameEncode (sName, sForm, vv);
}

bool SpecNameEncode(std::string& sName, std::string sForm, std::vector<int> vlen)
{
  size_t special = sName.find_first_of("?");
  if(special == string::npos)
    return false;

  string fn1 = sName.substr(0,special);
  string fn2 = sName.substr(special+1);
  std::ostringstream sfn;
  for(size_t nn = 0; nn < vlen.size(); nn++) {
    sfn << vlen[nn] << "-";
  }
  sName = fn1 + "__" + sfn.str() + sForm + "__" + fn2;
  return true;
}

