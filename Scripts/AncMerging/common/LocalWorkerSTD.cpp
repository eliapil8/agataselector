#include "LocalWorkerSTD.h"

#include <cstdio>
#include <cstring>
#include <algorithm>

#include "rubuilder/utils/ConfReader.h"

const int   specCal     =  4096;    // adc range
const int   specRaw     = 16384;    // adc range
const int   matPackChan =    10;

using namespace std;

LocalWorkerSTD::LocalWorkerSTD(CycleServer& _cServer, int& _tcpPortno, int boardID,
			       int numdoms, int numsubs, int numsamp)
  : LocalWorker(_cServer, _tcpPortno, boardID)
{
  gDaughterClass = "LocalWorkerSTD";
  //  fOutMagicData  = keyDataV1730;
  fOutMagicData = nullptr;
  calibration.clear();
  Dets = 0;
  Offs = 0;
  Mods = 0;
  PixLen = 0;    // excluded
  v1730_Ener  = nullptr;
  v1730_Ampl  = nullptr;
  v1730_H2D   = nullptr;
  InpDataFILE = nullptr;
  fEnergyGain = 1.f;
  HasHeader   = false;
  FirstBlock  = true;
  outEvt = new caenEventSTD_t;
  tstCount = 0;
  galTS  = 0;
  fDomID = 0;
  tstoff = 0;
  fDomOutOffset = 0;
  v1730MaxChans=16;
  fOutMagicData = new uint32_t[v1730MaxChans];
}

// todo: add the delete []
LocalWorkerSTD::~LocalWorkerSTD()
{
  cServer.Finish();
}

int LocalWorkerSTD::GetParameters(bool doList, bool partial)
{
  float    cTstamp(0);
  uint32_t adcSlot(0), adcNumChan(32);
  
  uint32_t calBoard(0), calChan(0);
  float    calThresh(0), calOffs(0), calGain(1.f);
  bool     isAncillary(false);
  unsigned long long tsOffset(0);
  int   chSync(-1);
  int         rdbPort;
  std::string rdbHost;  
  rubuilder::utils::ConfReader conf;
  conf.Add("AncV1730",         "input data is from V1730 for ancillary devices"
                               , &isAncillary);
  conf.Add("Domains",          "ID of first domain and offset to remap ID on the output "
                               , &fDomOutOffset);
  conf.Add("ServerPort",       "tcp port number for the command server (default=16161, no server if <=1 000)"
                               , &tcpPortno);
  conf.Add("SaveDataDir",      "where to write data and spectra"
                               , &fOutPath);
  conf.Add("SpecPrefix",       "prefix to names of spectra"
                               , &fHistPfx);
  conf.Add("TstampCal",        "detector and constant_positive_offset applied to its timestamp"
                               , &cTstamp);
  conf.Add("ListTimeStamps",   "number of timestamps to list on cout"
                               , &tstCount);
  conf.Add("Boards",           "Number of V1730 boards"
                               , &adcSlot, &adcNumChan);
  conf.Add("EnergyCal",        "Noard Channel Threshold Offset Gain"
                               , &calBoard, &calChan, &calThresh, &calOffs, &calGain);
  conf.Add("MatSize",          "Size of the individual matrix in the total 2D correlation matrix (default=0 == no matrix)"
                               , &PixLen);
  conf.Add("TSGlobalOffset",   "Global Offset on the TimeStamp"
                               , &tsOffset);
  conf.Add("EnergyGain",       "Scaling factor for energy spectra"
                               , &fEnergyGain);
  //Added by AG for the synchronization of the DAQ with the V1730
  conf.Add("Verbose",          "verbosity of printouts"
                               , &fVerbose);
  conf.Add( "RedisServer"    ,"REDIS Server host and port",
	    &rdbHost, &rdbPort);  
  // refine behaviour
  
  if (doList) { conf.Show(gMotherClass); return 0; }
  
  if(partial) {
    // disable "non-calibration" keywords
    conf.Disable("AncV1730");
    conf.Disable("Domains");
    conf.Disable("ServerPort");
    conf.Disable("SaveDataDir");
    conf.Disable("SpecPrefix");
    conf.Disable("NumBoards");
    conf.Disable("MatSize");
  }
  
  int rv = conf.Read(gMotherClass, fConfigFile);
  
  if (rv != 0)
    return rv;  // error
  
  int which, times;
  
  
  // vme stuff for NW
  if (!partial) {
    Offs = 0xFFFFFFFF;
    Mods = 0;
    which = conf.Find("Boards");
    times = conf.Times(which);
    for (int nn = 0; nn < times; nn++) {
      conf.Restore(which, nn);
      if (adcNumChan < 1 || adcNumChan > v1730MaxChans) {
        cout << "==> ERROR decoding V1730: Invalid Channel " << adcNumChan << endl;
        rv = 200; break;
      }
      Mods++;
      Offs = std::min(Offs, adcSlot*v1730MaxChans);
    }
    if (rv != 0)
      return rv;    // not worthwhile continuing
    
  }
  
  which = conf.Find("EnergyCal");
  times = conf.Times(which);
  for (int nn = 0; nn < times; nn++) {
    conf.Restore(which, nn);
    if (calChan < 0 || calChan >= v1730MaxChans) {
      cout << "==> ERROR decoding v1730Cal: Invalid Channel " << calChan << endl;
      rv = 200; break;
    }
    calibration[calBoard*16+calChan].SetPar(calThresh,calOffs,calGain);
  }
  
  // ("TstampCal",         "detector and constant_positive_offset applied to its timestamp", &ctstDom, &cTstamp);
  if (conf.Times("TstampCal")) {
    tstoff = int(cTstamp + 1000.5) - 1000;
  }
  fPromptPeriod = max(0, fPromptPeriod);
  return rv;
}

int LocalWorkerSTD::ProcessConfig(const char* fname)
{
  cout << gMotherClass + "::process_config() called with file = " << fname << endl;
  
  string confString;
  fConfigFile = fname;
  
  if (!fConfigFile.empty()) {          // use defaults, if process_config was not called
    if (!fileExists(fConfigFile)) {
      return 102;              // fatal error because configuration file not present
    }
    int rvAga = getKeyFromFile(fConfigFile, "AncV1730", confString);
    if (rvAga != 0) {
      cout << "Fatal Error: " << fConfigFile << " MUST contain the keyword 'AncV1730' " << endl;
      return 102; // Fatal error because the keyword "Ancillary" MUST be present
    }
    
    // VME/AGAVA
    int error_code = GetParameters();
    if (error_code) {
      return error_code;
    }
    
    
    
    
    fScaleTime = 0;
    if (fScaleDown > 0)
      fScaleTime = (tScaleMask + 1) / fScaleDown;
  }
  
  cServer.SetCommandFile(fConfigFile + ".live");
#ifdef GAL_OFFLINE
  cServer.AcceptExit(); // enable ECommand::kExit
#endif
  cServer.Start("LF_V1730", max(fPromptPeriod, 0));
  
  return 0;
}

int LocalWorkerSTD::ProcessStart(uint32_t run)
{
  
  
  fRunNumber = run;
  
  fCountBlocks = 0;
  //cServer.Start(gMotherClass, max(fPromptPeriod, 0));
  cServer.Start("LF_V1730", max(fPromptPeriod, 0));
  
  if (InpDataName.length() > 0) {
    ostringstream oss;
    oss << InpDataName << "_r" << fRunNumber;
    InpDataNameFull = oss.str();
    InpDataFILE = fopen((fOutPath + InpDataNameFull).c_str(), "wb");
    if (!InpDataFILE) {
      cout << "ERROR opening " << fOutPath + InpDataNameFull << endl;
      return 400;
    }
    if (HasHeader) {
      size_t nw = fwrite(FileHeader, sizeof(uint32_t), 10, InpDataFILE);
      if (nw != 10) {
        cout << "ERROR writing vmeFileHeader to original ancillary data" << endl;
        return 410;
      }
    }
    FirstBlock = true;
  }
  
  return 0;
}

int LocalWorkerSTD::ProcessStop()
{
  cServer.Save(true);
  
  cout << "Number of V1730 events of this run " << cServer.GetCounts() << endl;
  if (InpDataFILE) {
    cout << " Closing V1730 original data file " << InpDataNameFull << endl;
    fclose(InpDataFILE);
    InpDataFILE = nullptr;
  }
  
  
  return 0;
}

int LocalWorkerSTD::ProcessBlock(
				 const void *input_buffer,  int input_size,  int packet_ID,
				 void *output_buffer, int output_size, int &used_output)
{
  uint32_t* inpBuffer   = (uint32_t*)input_buffer;
  uint32_t* outBuffer   = (uint32_t*)output_buffer;
  uint32_t  inpBuffSize = input_size  / sizeof(uint32_t);
  
  //  std::cout << "Receiving data in  " << __FILE__ << std::endl;
  int error_code = 0;
  
  
  if (InpDataFILE) {
    size_t nw = fwrite(inpBuffer, sizeof(uint32_t), inpBuffSize, InpDataFILE);
    if (nw != inpBuffSize) {
      cout << "ERROR writing original V1730 data" << endl;
      return 400;
    }
  }
  
  int outByteSize = used_output;
  int countEvents = 0;
  uint64_t tStamp_10ns = 0;
  if(error_code)
    return error_code;
  
  used_output = outByteSize;
  fCountBlocks++;
  
  cServer.Exec(tStamp_10ns, countEvents);
  cServer.Prompt(0, countEvents, uint32_t(outByteSize)*sizeof(uint32_t));
  
  if (cServer.CheckCommand()) {
    if (cServer.CheckCommand(CycleServer::ECommand::kExit))
      return -1000;
    if (cServer.CheckCommand(CycleServer::ECommand::kConfigure))
      GetParameters(false, true);
  }
  return 0;
  
}

int LocalWorkerSTD::SetOutput(uint32_t *outBuff, int availableBytes, const caenEventSTD_t * pE)
{
  // agataKey + numSubs * subData_t
  int outEvLenByte = sizeof(agataKey) + sizeof(subData_t); // max needed size
  if (outEvLenByte > availableBytes) {
    cout << "LocalWorkerSTD::SetOutput ==> buffer full" << endl;
    return availableBytes - outEvLenByte;
  }
  
  agataKey* psaKey = (agataKey *)outBuff;
  outEvLenByte = sizeof(agataKey);                      // recalculate effective value
  
  // hand-made agata key
  psaKey->size  = pE->theKey.size;
  psaKey->key   = pE->theKey.key;
  psaKey->ts_0  = pE->theKey.ts_0;
  psaKey->ts_1  = pE->theKey.ts_1;
  psaKey->evnum = pE->theKey.evnum;
  
  
  outBuff += sizeof(agataKey) / sizeof(uint32_t);       // immediatly after agataKey
  
  // the full size as defined in subData_t
  subData_t pS = pE->theData;
  memcpy(outBuff, &pS, sizeof(subData_t));
  outEvLenByte += sizeof(subData_t);                  // recalculate effective value
  outBuff += sizeof(subData_t) / sizeof(uint32_t);
  
  psaKey->size = outEvLenByte;
  return outEvLenByte;
}


int LocalWorkerSTD::GenerateIdles(uint64_t largest_ts,
				  void *output_buffer, int output_size, int &used_output)
{
  return 0;
}
