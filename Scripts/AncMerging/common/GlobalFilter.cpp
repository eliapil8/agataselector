#include "GlobalFilter.h"

#include "GlobalWorker.h"

using namespace std;

#ifndef GAL_OFFLINE
GlobalFilter::GlobalFilter(xdaq::ApplicationStub *s) :
  SuperApplication::SuperApplication(s),
  logger_( getApplicationLogger() ),
#else
GlobalFilter::GlobalFilter() :
#endif
  worker(nullptr),
  gMotherClass("GlobalFilter")
{
  fPackBlocks = 0;
  fPackNumber = 0;
  fPackedSize = 0;
  fPackedMaxSize = 0;
  fPacked = nullptr;
  tcpPortno = 16161;
}

// todo: add the delete []
GlobalFilter::~GlobalFilter()
{
  if (worker)
    delete worker;
  if (fPacked)
    delete[] fPacked;
}

void GlobalFilter::process_config(const char* fname, int *error_code)
{
  fConfigFile = fname;

  if (fConfigFile.empty()) {
    cout << "Fatal Error:  ConfigFile  MUST be given" << endl;
    *error_code = 101;              // fatal error because configuration file not given
    return;
  }

  cout << gMotherClass + "::process_config() called with file = " << fConfigFile << endl;
  *error_code = 0;

  if (!fileExists(fConfigFile)) {
    cout << gMotherClass + "::process_config() " << fConfigFile << " does not exist " << endl;
    *error_code = 102;              // fatal error because configuration file not present
    return;
  }

  string confString;
  int rv = getKeyFromFile(fConfigFile, "Detectors", confString);
  if(rv == 2) {
    cout << "Fatal Error: " << fConfigFile << " MUST contain the keyword 'Detectors numDoms numSubs' " << endl;
    *error_code = 102; // Fatal error because the keyword "Detectors" MUST be present
    return;
  }

  worker = new GlobalWorker(cServer, tcpPortno);

  *error_code = worker->ProcessConfig(fname);

  fPackBlocks = worker->GetPackBlocks();
  fPackNumber = 0;
  fPackedSize = 0;

  cServer.TcpLaunch(tcpPortno);   // with the port noumber from worker::GetParameters
}

void GlobalFilter::ShowKeywords()
{
  if (worker) {
    worker->GetParameters(true);
    return;
  }

  worker = new GlobalWorker(cServer, tcpPortno);
  worker->GetParameters(true);

  delete worker;
  worker = nullptr;

  return;
}

bool GlobalFilter::PackInputBlocks(void *input_buffer, int input_size)
{
  if (fPackBlocks < 2) {
    // no packing, use input buffers
    fPacked = (char *)input_buffer;
    fPackedSize = input_size;
    return true;    // done by definition
  }

  if ((fPackedMaxSize - fPackedSize) < input_size) {
    char *oldPacked = fPacked;
    fPackedMaxSize += input_size + 1024;
    fPacked = new char[fPackedMaxSize];
    if (fPackedSize)
      memcpy(fPacked, oldPacked, fPackedSize);
    if (oldPacked)
      delete[] oldPacked;
  }

  if (input_size) {
    memcpy(fPacked + fPackedSize, input_buffer, input_size);
    fPackedSize += input_size;
  }

  return (++fPackNumber >= fPackBlocks);
}


void GlobalFilter::process_start(uint32_t run, int *error_code)
{
  *error_code = worker->ProcessStart(run);
}

void GlobalFilter::process_stop ( int *error_code )
{
  printf("start of %s\n of %s",__func__,__FILE__);
  *error_code = worker->ProcessStop();
  printf("ProcessStop ended with error = %i",*error_code);
  //delete worker;
  worker = nullptr;
  printf("end of %s\n of %s",__func__,__FILE__);
}

void GlobalFilter::process_block(
  void *input_buffer, int input_size, int packet_ID,
  void *output_buffer, int output_size, int *used_size_of_output_buffer,
  int *error_code)
{
  //cout << "input_size: " << input_size << "    packet_ID: " << packet_ID << endl;
  int errorCode = 0;
  int usedSize = 0;

  if (fPackBlocks < 2) {
    // no packing
    errorCode = worker->ProcessBlock(input_buffer,  input_size,  packet_ID,
                                     output_buffer, output_size, usedSize);
  }
  else {
    // pack fPackBlocks input blocks
    bool done = input_size==0 || PackInputBlocks(input_buffer, input_size);
    if (done) {
      errorCode = worker->ProcessBlock(fPacked, fPackedSize, packet_ID,
                                       output_buffer, output_size, usedSize);
      fPackNumber = 0;
      fPackedSize = 0;
    }
  }

  *used_size_of_output_buffer = usedSize;
  *error_code = errorCode;
}
#ifndef GAL_OFFLINE
/*
 * Provides the factory method for the instantiation of RU applications.
 * XDAQ_INSTANTIATOR_IMPL() is a macro that implements the actual entry point for creating the application object at run-time-
 *-when the application is loaded into a running XDAQ process.
 * The macro takes a single parameter, the C++ class name of the XDAQ application.
 */
XDAQ_INSTANTIATOR_IMPL(GlobalFilter)
#endif
