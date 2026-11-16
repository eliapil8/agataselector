#include "LocalWorkerPSD.h"

#include <cstdio>
#include <cstring>
#include <algorithm>

#include "rubuilder/utils/ConfReader.h"

const int   specCal     =  4096;    // adc range
const int   specRaw     = 16384;    // adc range
const int   matPackChan =    32;


using namespace std;

LocalWorkerPSD::LocalWorkerPSD(CycleServer& _cServer, int& _tcpPortno, int boardID,
                               int numdoms, int numsubs, int numsamp)
  : LocalWorker(_cServer, _tcpPortno, boardID)
{
  gDaughterClass = "LocalWorkerPSD";
  fOutMagicData = nullptr;
  calibration.clear();
  fEnergyGain = 1.f;
  fGlobalTSOffset = 0L;
  outEvt =  new caenEventPSD_t;
  //spectra
  specAmpl = nullptr;
  matPSD   = nullptr;
  waveAnalyzer = new MWD();
  aTrace = nullptr;
  wspace = nullptr;
}

// todo: add the delete []
LocalWorkerPSD::~LocalWorkerPSD()
{
  delete waveAnalyzer;
  cServer.Finish();
  if(aTrace)
    delete [] aTrace;
  if(wspace)
    delete [] wspace;
}

int LocalWorkerPSD::GetParameters(bool doList, bool partial)
{


  bool        AnaRiseTime;
  int         tsOffs;
  int         graphPort;
  uint16_t    boardID, boardCH, boardTS, boardSampling;
  int16_t     channelID;
  uint16_t    preTrig;
  std::string boardFW;
  std::string boardName;
  std::string histPrefix;
  std::string tmpOutData;
  std::string graphServ;
  std::string confFile;
  int         rdbPort;
  std::string rdbHost;
  rubuilder::utils::ConfReader conf;

  hGroup.destroy();
  conf.Add("Board"            ,"Board description: BoardID, Name, Firmware, number of channel, channel offset for the event builder, nsPerTimestamp, ns per sample",
	   &boardID, &boardName, &boardFW, &boardCH, &boardTS, &boardSampling);
  conf.Add("ServerPort"       ,"tcp port number for the command server (default=16161, no server if <=1000)"
	   , &tcpPortno);
  conf.Add("SaveDataDir"      ,"where to write data and spectra"
	   , &fOutPath);
  conf.Add("SpecPrefix",       "prefix to names of spectra"
	   , &boardID, &histPrefix);
  conf.Add("Verbose",          "verbosity of printouts"
	   , &fVerbose);
  conf.Add("TSOffset",   "Timestamp offset for board X chanel Y",
	   &boardID, &channelID, &tsOffs);
  conf.Add("GraphiteServer", "Graphite Server to send the rates of the boards: Hostname and port",
	   &graphServ, &graphPort);
  conf.Add("OutDataKey", "Output ADF frame key (0xFA0201A2=SPIDER, 0xFA0201A3=DANTE, ...",
	   &boardID, &tmpOutData);
  conf.Add("PreTrigger",       "Pre-trigger time in ns (expected to be common for the full board)",
	   &boardID, &preTrig);
  conf.Add("ConfFilePSA",      "Configuration file for the PSA",
	   &boardID, &confFile);
  conf.Add("AnaRiseTime",      "Enable analysis of the short traces for the particle discrimination",
	   &boardID, &AnaRiseTime);
  conf.Add( "RedisServer"    ,"REDIS Server host and port",
	    &rdbHost, &rdbPort);
  if (doList) { conf.Show(gMotherClass); return 0; }
    
  if(partial) {
    // disable "non-calibration" keywords
    conf.Disable("ServerPort");
    conf.Disable("SaveDataDir");
    conf.Disable("SpecPrefix");
  }
    
  int rv = conf.Read(gMotherClass, fConfigFile);
    
  if (rv != 0)
    return rv;  // error
    
  int which, times;

  which = conf.Find("Board");
  times = conf.Times(which);
  for (int nn = 0; nn < times; nn++) {
    conf.Restore(which, nn);
    if(boardID!=fBoardID)continue;
    filterName.str("");
    fBoardName = boardName;
    filterName << "LF_" << fBoardName;
    ratesMonitor.setName(fBoardName);
    fNsPerSample  = boardSampling;
    fNsPerTStamp  = boardTS;
    nbChannel     = boardCH;
    for(int ch = 0 ; ch < nbChannel ; ++ch){
      lastTS[ch]=0;
      nbEvents[ch]=0;
      nbGoodEvents[ch]=0;
      nbIdleEvents[ch]=0;
    }
  }
    
  which = conf.Find("SpecPrefix");
  times = conf.Times(which);
  for (int nn = 0 ; nn < times ; ++nn){
    conf.Restore(which,nn);
    if(boardID!=fBoardID)continue;
    fHistPfx = histPrefix;
    specAmpl = new MultiHist<uint32_t>(2,nbChannel, 16384);     // [0]-qshort [1]-qlong
    specAmpl->setFileName(fOutPath+fHistPfx+"?Ampl.spec");
    specAmpl->setComment("raw energy");
    hGroup.add(specAmpl);
    matPSD = new MultiHist<uint16_t>(2,nbChannel,1024,1024); //[0]PSD:qlong [1]Time:qlong
    matPSD->setFileName(fOutPath+fHistPfx+"?PSD.matr");
    matPSD->setComment("Long-Short matrix");
    hGroup.add(matPSD);
  }
    
  if(conf.Seen("GraphiteServer")){
    ratesMonitor.setHost(graphServ,graphPort);
  }
    
  which = conf.Find("TSOffset");
  times = conf.Times(which);
  for (int nn = 0; nn < times; nn++) {
    conf.Restore(which, nn);
    if(boardID!=fBoardID) continue;
    if(channelID==-1){
      for(int ch = 0 ; ch < nbChannel ; ++ch){
	tsOffset[ch]=tsOffs;
      }
    }else{
      tsOffset[channelID]=tsOffs;
    }
  }
    
  which = conf.Find("OutDataKey");
  times = conf.Times(which);
  for (int nn = 0; nn < times; nn++) {
    conf.Restore(which, nn);
    if(boardID!=fBoardID)continue;
    outDataKey = OutDataKey(tmpOutData) ;
  }
    
  fPromptPeriod = max(0, fPromptPeriod);
  cServer.SetHistGroup(&hGroup);
  return rv;
}

int LocalWorkerPSD::ProcessConfig(const char* fname)
{
  std::cout << gMotherClass + "::process_config() called with file = " << fname << std::endl;
    
  string confString;
  fConfigFile = fname;
    
  if (!fConfigFile.empty()) {          // use defaults, if process_config was not called
    if (!fileExists(fConfigFile)) {
      return 102;              // fatal error because configuration file not present
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
    
  cServer.Start(filterName.str().c_str(), max(fPromptPeriod, 0));
    
  return 0;
}

int LocalWorkerPSD::ProcessStart(uint32_t run)
{
  fRunNumber = run;
  fCountBlocks = 0;
  cServer.Start(filterName.str().c_str(), max(fPromptPeriod, 0));
    
  return 0;
}

int LocalWorkerPSD::ProcessStop()
{
  cServer.Save(true);
    
  cout << "Number of events in board " << fBoardID << " of this run " << cServer.GetCounts() << endl;
  for(auto it=nbEvents.begin();it!=nbEvents.end();++it)
    printf("\t Ch %2i -- %10i -- %10i -- %10i\n",it->first,
	   it->second,nbGoodEvents[it->first],nbIdleEvents[it->first]);
  printStatistics();
  return 0;
}

int LocalWorkerPSD::ProcessBlock(const void *input_buffer,  int input_size,  int packet_ID,
                                 void *output_buffer, int output_size, int &used_output)
{
  uint32_t* inpBuffer   = (uint32_t*)input_buffer;
  uint32_t* outBuffer   = (uint32_t*)output_buffer;
  uint32_t  inpBuffSize = input_size  / sizeof(uint32_t);
    
  //  std::cout << "Receiving data in  " << __FILE__ << std::endl;
  int error_code = 0;
  int outByteSize = used_output;
  int countEvents = 0;
  //
  //  // Output has the format of GALILEO Data
  //  // with header + subData for only one subDomain
  //
  uint32_t outEvtLen = (sizeof(agataKey) + sizeof(subDataPSD_t));
  uint32_t posBuffer         = 0 ; 
  std::bitset<32> decodeField;
  // Aggregate buffer has a header containing different information
  uint32_t aggLength         = inpBuffer[posBuffer++]&0x0FFFFFFF;
  decodeField = inpBuffer[posBuffer++];
  uint32_t board             = project_range<27,31>(decodeField).to_ulong();
  bool boardFailFlag         = decodeField.test(26);
  std::bitset<8> channelMask = project_range<0,7>(decodeField).to_ulong();
  uint32_t aggregateCounter  = inpBuffer[posBuffer++]&0x7FFFFF;
  uint32_t aggregateTimeTag  = inpBuffer[posBuffer++];
       
  uint8_t couples[8]={0,0,0,0,0,0,0,0};
  // we need to reconstruct the channel number from the couple id for the
  // 16 channels boards
  int index = 0 ;
  for(uint16_t bit = 0 ; bit < 8 ; bit++){
    if(channelMask.test(bit)){
      couples[index++]=bit;
    }
  }

  uint32_t coupleAggregateSize = 0 ;
  uint64_t tstamp_10ns         = 0 ;
  uint32_t fineTS              = 0 ;
  int      offsetTS            = 0 ;
  uint32_t flags               = 0 ;
  bool     pur                 = false;
  bool     ovr                 = false;
  bool     lost                = false;
  uint32_t chanNum             = 0 ;

  for(int ch=0; ch < nbChannel ; ch++){  
    if(lastTS[ch]==0) lastTS[ch]=fGlobalTSOffset;
  }

  // here starts the real decoding of the data for the channels
  for(uint8_t cpl = 0 ; cpl < channelMask.count() ; cpl++){
    coupleAggregateSize = inpBuffer[posBuffer++]&0x3FFFFF;
    dataForm.SetDataFormat(inpBuffer[posBuffer++]);
    //dataForm.printDataFormat();
    const uint16_t evtSize = dataForm.evtSize();
    // Depacking the dual channel aggregate

    for(uint16_t evt = 0 ; evt < (coupleAggregateSize-2)/evtSize;evt++){ 
     
      outEvt->theData.cfd = 0;
      flags = 0;
      decodeField = inpBuffer[posBuffer++];
      tstamp  = project_range<0,30>(decodeField).to_ulong();
      chanNum = static_cast<uint32_t>(project_range<31,31>(decodeField).to_ulong()
				      +couples[cpl]*2);
      float trig = 0;
      // if the trace are present we should decode them or at least jump the samples
      if(dataForm.enaTrace()){ 
	std::valarray<float> samples;
	samples.resize(dataForm.numSamples());
	for(uint16_t s = 0 ; s < dataForm.numSamples()/2 ; ++s){
	  decodeField = inpBuffer[posBuffer++];
	  samples[s*2]   = project_range< 0,13>(decodeField).to_ulong() + (rand()%1000)/1000.-0.5;
	  samples[s*2+1] = project_range<16,29>(decodeField).to_ulong() + (rand()%1000)/1000.-0.5;
	}
	float baseline = static_cast<std::valarray<float>>(samples[std::slice(0,20,1)]).sum()/20.;
	samples-=baseline;	
	if(samples.sum()<0) samples *=-1;
	if(aTrace == nullptr)
	  aTrace = new float[samples.size()];
	if(wspace == nullptr)
	  wspace = new float[3 * samples.size()];
	std::copy(begin(samples),end(samples),aTrace);
	float otrig = 0 ;

	trig = waveAnalyzer->TriggerCFD(aTrace, 5, wspace, otrig);
	//	std::cout << "trig = " << trig << " otrig = " << otrig << std::endl; 
	

	//samples.clear();
      }
      // After the trace we have the extra if the extra is present
      if(dataForm.enaExtras()){
	decodeField = inpBuffer[posBuffer++];
	switch(dataForm.confExtras()) {
	case 0: // Extended Time Stamp [31:16] ; baseline*4 [15:0]
	  tstamp = (static_cast<uint64_t>(project_range<16,31>(decodeField).to_ulong())<<31)
	    | tstamp;
	  break;
	case 1: // Extended Time stamp [31:16] ; flags [15:0]
	  // bit[15] Tigger lost 0x40000000
	  // bit[14] Over-range 0x20000000
	  // bit[13] event counter every 1024 events bit up 0x10000000
	  // bit[12] N lost trigger counted 0x08000000
	  lost   = decodeField.test(15);
	  ovr    = decodeField.test(14);
	  tstamp = (static_cast<uint64_t>(project_range<16,31>(decodeField).to_ulong())<<31)
	    | tstamp;
	  break;
	case 2: // Extended Time stamp [31:16] ; Flags [15:10] ; Fine Time Stamp [9:0]
	  // bit[15] Tigger lost 0x40000000
	  // bit[14] Over-range 0x20000000
	  // bit[13] event counter every 1024 events bit up 0x10000000
	  // bit[12] N lost trigger counted 0x08000000
	  lost   = decodeField.test(15);
	  ovr    = decodeField.test(14);
	  fineTS = static_cast<uint16_t>(project_range<0,9>(decodeField).to_ulong());
	  outEvt->theData.cfd = (fNsPerTStamp/1024.* fineTS) / 10; // to have it in 10 ns units
	  tstamp = (static_cast<uint64_t>(project_range<16,31>(decodeField).to_ulong())<<31)
	    | tstamp;
	  break;
	case 4: // Lost Trigger Counter [31:16] ; Total Trigger [15:0]
	  break;
	case 5: // Positive zero crossing [31:16] ; Negative zero crossing [15:0]
	  break;
	case 7: // Debug fixed value;
	  break;
	default:
	  break;
	}
      }
      //	qshort  = static_cast<uint16_t>((inpBuffer[posBuffer+evtSize-1+evtSize*evt+2])&0x7FFF);
      //last word of the buffer has the energies
      decodeField=inpBuffer[posBuffer++];
      pur = decodeField.test(15);
      outEvt->theData.qshort  = static_cast<uint16_t>(project_range< 0,14>(decodeField).to_ulong());
      outEvt->theData.qlong   = static_cast<uint16_t>(project_range<16,31>(decodeField).to_ulong());
         
            
      
           
           

      if(tsOffset.find(chanNum)!=tsOffset.end())
	offsetTS = tsOffset[chanNum];
      else
	offsetTS = 0;

      outEvt->theKey.evnum    = (board << 8 |chanNum );
      tstamp_10ns             = tstamp * fNsPerTStamp / 10 ;
      outEvt->theData.cfd    += (float)(tstamp*fNsPerTStamp - tstamp_10ns*10)/10.;
      tstamp_10ns            += offsetTS + fGlobalTSOffset;
      outEvt->theKey.size     = outEvtLen ;
      outEvt->theKey.key      = outDataKey ;
      outEvt->theKey.ts_0     = uint32_t(tstamp_10ns & 0xFFFFFFFF);         // tstLow
      outEvt->theKey.ts_1     = uint32_t((tstamp_10ns >> 32) & 0x0000FFFF); // tstHigh

      if(HCHECK(specAmpl)){
	specAmpl->IncrQ(0,chanNum,outEvt->theData.qshort/2,0);
	specAmpl->IncrQ(1,chanNum,outEvt->theData.qlong/2,0);
      }
      if(HCHECK(matPSD)){
	matPSD->IncrQ(0,
		      chanNum,
		      outEvt->theData.qshort/matPackChan,
		      ((float)(outEvt->theData.qlong-outEvt->theData.qshort))/outEvt->theData.qlong*1024,
		      0);
	matPSD->IncrQ(1,
		      chanNum,
		      outEvt->theData.qshort/matPackChan,
		      trig*10,
		      0);	
      }
      // Adding PUR to the flags
            
      flags = ovr << 18 | pur << 16;
      // current time in ns
      if(ratesMonitor.values.find(chanNum) == ratesMonitor.values.end())
	ratesMonitor.values[chanNum].Init();
      ratesMonitor.values[chanNum].time = tstamp*fNsPerTStamp;
      ++ratesMonitor.values[chanNum].totalEvents ;
      if( pur ) ++ratesMonitor.values[chanNum].pileEvents ;
      if( ovr ) ++ratesMonitor.values[chanNum].satuEvents ;
      if( lost )  ratesMonitor.values[chanNum].lostEvents += 1024 ;
      // Adding flags to the evt num
      outEvt->theKey.evnum   |= flags        ;
      outEvt->theKey.evnum   |= boardFailFlag;
      ++nbEvents[chanNum];
      countEvents++;
      lastTS[chanNum] = tstamp_10ns;
      if (fLastTS < tstamp_10ns) fLastTS = tstamp_10ns;
      if(outEvt->theData.qlong<1)continue;
      ++nbGoodEvents[chanNum];
      int outEvtSize = SetOutput(outBuffer,output_size-outByteSize,outEvt);
      if(outEvtSize<0){ //output buffer full
	return outEvtSize;
      }
      outByteSize += outEvtSize ;
      outBuffer   += outEvtSize / sizeof(uint32_t) ;
    }
  }
  if(error_code)
    return error_code;
    
  used_output = outByteSize;
  fCountBlocks++;
    
  cServer.Exec(tstamp_10ns, countEvents);
  cServer.Prompt(0, countEvents, uint32_t(outByteSize)*sizeof(uint32_t));
    
  if (cServer.CheckCommand()) {
    if (cServer.CheckCommand(CycleServer::ECommand::kExit))
      return -1000;
    if (cServer.CheckCommand(CycleServer::ECommand::kConfigure))
      GetParameters(false, true);
  }
  return 0;
    
}

int LocalWorkerPSD::SetOutput(uint32_t *outBuff, int availableBytes, const caenEventPSD_t * pE)
{
  // agataKey + numSubs * subData_t
  int outEvLenByte = sizeof(agataKey) + sizeof(subDataPSD_t); // max needed size
  if (outEvLenByte > availableBytes) {
    cout << "LocalWorkerPSD::SetOutput ==> buffer full" << endl;
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
    
  // the full size as defined in subDataPSD_t
  subDataPSD_t pS = pE->theData;
  memcpy(outBuff, &pS, sizeof(subDataPSD_t));
  outEvLenByte += sizeof(subDataPSD_t);                  // recalculate effective value
  outBuff      += sizeof(subDataPSD_t) / sizeof(uint32_t);
    
  psaKey->size = outEvLenByte;
  return outEvLenByte;
}

int LocalWorkerPSD::GenerateIdles(uint64_t largest_ts,
                                  void *output_buffer, int output_size, int &used_output)
{
  uint32_t* outBuffer   = (uint32_t*)output_buffer;
  int outByteSize = used_output;
  uint32_t outEvtLen = (sizeof(agataKey) + sizeof(subDataPSD_t));
  uint64_t newTS = 0;
  for(int ch=0; ch < nbChannel ; ch++){
    if ( lastTS[ch] < fGlobalTSOffset ) lastTS[ch]=fGlobalTSOffset;
    if ( largest_ts - lastTS[ch] < 100000 ) continue;
    //at this point we generate an idle event using the eventIdx to identify them
    newTS = lastTS[ch] + 10000;
    ++nbIdleEvents[ch];
    //    std::cout << "Generating idle for channel " << ch << " of board " << fBoardID
    //	       << " " << __FILE__ << std::endl;
    outEvt->theKey.size  = outEvtLen;
    outEvt->theKey.key   = outDataKey;
    outEvt->theKey.evnum = 0x80000000 | ((fBoardID&0xFF) << 8) | (ch&0xff) ;
    outEvt->theKey.ts_0     = uint32_t(newTS & 0xFFFFFFFF);         // tstLow
    outEvt->theKey.ts_1     = uint32_t((newTS >> 32) & 0x0000FFFF); // tstHigh
    outEvt->theData.qshort  = 0;
    outEvt->theData.qlong   = 0;
    outEvt->theData.cfd     = 0.;
    int outEvtSize = SetOutput(outBuffer,output_size-outByteSize,outEvt);
    if(outEvtSize<0){ //output buffer full
      return outEvtSize;
    }
    outByteSize += outEvtSize ;
    outBuffer   += outEvtSize / sizeof(uint32_t) ;
    lastTS[ch]   = newTS;
  }
  used_output = outByteSize;
  return 0;
}
