#include "LocalWorkerNEDA.h"

#include <cstdio>
#include <cstring>
#include <algorithm>

#include "rubuilder/utils/ConfReader.h"

const int   specCal     =  4096;    // adc range
const int   specRaw     = 16384;    // adc range
const int   matPackChan =    10;

using namespace std;

LocalWorkerNEDA::LocalWorkerNEDA(CycleServer& _cServer, int& _tcpPortno, int boardID,
				 int numdoms, int numsubs, int numsamp)
  : LocalWorker(_cServer, _tcpPortno, boardID)
{
  gDaughterClass = "LocalWorkerNEDA";
  calibration.clear();
  Dets = 0;
  Offs = 0;
  Mods = 0;
  PixLen = 0;    // excluded
  HasHeader   = false;
  FirstBlock  = true;
  tstCount = 0;
  fGlobalTSOffset = 0L;
  ctrl_default_neda = NedaCtrl(5,25,2,3,.5,40.,-15.,150,-9,70,7,55,1.0,0.2,0.9,200,0);
  fDomID = 0;
  tstoff = 0;
  specAmpl = nullptr;
  matPSD = nullptr;
  outEvt =  new caenEventNEDA_t;
  ftrace.clear();
  cfdtrace.clear();
  tracenobl.clear();
  integral.clear();
}

// todo: add the delete []
LocalWorkerNEDA::~LocalWorkerNEDA()
{
  cServer.Finish();
}

int LocalWorkerNEDA::GetParameters(bool doList, bool partial)
{
  float    cTstamp(0);
  uint32_t adcSlot(0), adcNumChan(32);
  std::string histPrefix;
  uint32_t calBoard(0), calChan(0);
  float    calThresh(0), calOffs(0), calGain(1.f);
  bool     isAncillary(false);
  int   chSync(-1);
  int tsOffs;
  uint16_t boardID, boardFW, boardCH, boardTS, boardSampling;
  std::string boardName;
  uint16_t channelID;
  std::string graphServ;
  int  graphPort;
  std::string tmpOutData;
  uint16_t preTrig;
  std::string confFile;
  rubuilder::utils::ConfReader conf;
  int         rdbPort;
  std::string rdbHost;
  hGroup.destroy();
  conf.Add("Board",      "Board description: BoardID, Name, Firmware, number of channel, nsPerTimestamp, ns per sample",
	   &boardID, &boardName, &boardFW, &boardCH, &boardTS, &boardSampling);
  conf.Add("ServerPort",       "tcp port number for the command server (default=16161, no server if <=1000)"
	   , &tcpPortno);
  conf.Add("SaveDataDir",      "where to write data and spectra"
	   , &fOutPath);
  conf.Add("SpecPrefix",       "prefix to names of spectra"
	   , &boardID, &histPrefix);
  conf.Add("EnergyCal",        "Noard Channel Threshold Offset Gain"
	   , &calBoard, &calChan, &calThresh, &calOffs, &calGain);
  //Added by AG for the synchronization of the DAQ with the V1730
  conf.Add("Verbose",          "verbosity of printouts"
	   , &fVerbose);
  conf.Add("TSOffset",         "Timestamp offset for board X chanel Y",
	   &boardID, &channelID, &tsOffs);
  conf.Add("PreTrigger",       "Pre-trigger time in ns (expected to be common for the full board)",
	   &boardID, &preTrig);
  conf.Add("ConfFilePSA",      "Configuration file for the PSA",
	   &boardID, &confFile);
  conf.Add("GraphiteServer", "Graphite Server to send the rates of the boards: Hostname and port",
	   &graphServ, &graphPort);
  conf.Add("OutDataKey", "Output ADF frame key (0xFA0201A2=SPIDER, 0xFA0201A3=DANTE, ...",
	   &boardID, &tmpOutData);  
  conf.Add( "RedisServer"    ,"REDIS Server host and port",
	    &rdbHost, &rdbPort);
  if (doList) { conf.Show(gMotherClass); return 0; }
  
  if(partial) {
    // disable "non-calibration" keywords
    conf.Disable("Domains");
    conf.Disable("ServerPort");
    conf.Disable("SaveDataDir");
    conf.Disable("SpecPrefix");
    conf.Disable("NumBoards");
  }
  
  int rv = conf.Read(gMotherClass, fConfigFile);
  
  if (rv != 0)
    return rv;  // error
  

  Offs = 0xFFFFFFFF;
  Mods = 0;

  if (rv != 0)
    return rv;    // not worthwhile continuing

  int which, times;
  
  which = conf.Find("Board");
  times = conf.Times(which);
  for (int nn = 0; nn < times; nn++) {
    conf.Restore(which, nn);
    if(boardID!=fBoardID)continue;
    filterName.str("");
    filterName << "LF_" << boardName;
    ratesMonitor.setName(boardName);
    fNsPerSample  = boardSampling;
    fNsPerTStamp  = boardTS;
    nbChannel     = boardCH;
    for(int ch = 0 ; ch < boardCH ; ++ch){
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
    specAmpl = new MultiHist<uint32_t>(4,nbChannel, 16384);     // [0]-qshort [1]-qlong [2]-qfast [3]-qslow
    specAmpl->setFileName(fOutPath+fHistPfx+"?Ampl.spec");
    specAmpl->setComment("raw energy");
    hGroup.add(specAmpl);
    matPSD = new MultiHist<uint16_t>(3,nbChannel,1024,1024); //[0] psd_qlong [1] cc_light [2] irt_light
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

  which = conf.Find("PreTrigger");
  times = conf.Times(which);
  for (int nn = 0; nn < times; nn++) {
    conf.Restore(which, nn);
    if(boardID!=fBoardID) continue;
    fBoardPreTrigger = preTrig;
  }

  which = conf.Find("ConfFilePSA");
  times = conf.Times(which);
  for (int nn = 0; nn < times; nn++) {
    conf.Restore(which, nn);
    if(boardID!=fBoardID) continue;
    std::ifstream confPSA ;
    confPSA.open(confFile);
    if(!confPSA.is_open()){
      std::cerr << "Error: Cannot find the configuration file for board " << fBoardID 
		<< "\n Please check the path " << confFile << std::endl;
    }else{
      std::string aLine;
      NedaCtrl one_ctrl = {5,25,2,3,.5,40.,-15.,150,-9,70,7,55,1.0,0.2,0.9,200,0};
      while(std::getline(confPSA,aLine)){
	if(aLine.empty())continue;
	if(aLine[0]=='#' || aLine[0]=='/') continue;
	std::stringstream oneLine (aLine);
	int tmp_board(0) ,tmp_channel(0);
	oneLine >> tmp_board >> tmp_channel
		>> one_ctrl.BaselineStopsNBeforeLEStart
		>> one_ctrl.LEthreshold
		>> one_ctrl.ZCintegration
		>> one_ctrl.CFDdelay
		>> one_ctrl.CFDfraction
		>> one_ctrl.CFDthreshold
		>> one_ctrl.CFDCrossingLine
		>> one_ctrl.CFDDiffThreshold
		>> one_ctrl.StartInt
		>> one_ctrl.EnergyGate
		>> one_ctrl.LengthFast
		>> one_ctrl.LengthSlow
		>> one_ctrl.LightCalibration	  
		>> one_ctrl.LowerIRT
	  	>> one_ctrl.HigherIRT
		>> one_ctrl.StartExclude
	  	>> one_ctrl.WidthExclude;
	if(tmp_board != fBoardID) continue;
	ctrl[tmp_channel]=one_ctrl;

      }
    }
  }

  which = conf.Find("OutDataKey");
  times = conf.Times(which);
  for (int nn = 0; nn < times; nn++) {
    conf.Restore(which, nn);
    if(boardID!=fBoardID)continue;
    outDataKey = OutDataKey(tmpOutData) ;
  }  
  
  which = conf.Find("EnergyCal");
  times = conf.Times(which);
  for (int nn = 0; nn < times; nn++) {
    conf.Restore(which, nn);
    if (calChan < 0 || calChan >= nbChannel) {
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
  cServer.SetHistGroup(&hGroup);
  return rv;
}

int LocalWorkerNEDA::ProcessConfig(const char* fname)
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

int LocalWorkerNEDA::ProcessStart(uint32_t run)
{
  
  
  fRunNumber = run;
  fCountBlocks = 0;
  cServer.Start(filterName.str().c_str(), max(fPromptPeriod, 0));
    
  return 0;
}

int LocalWorkerNEDA::ProcessStop()
{
  cServer.Save(true);
  
  std::cout << "Number of events in board " << fBoardID << " of this run " << cServer.GetCounts() << std::endl;
  printStatistics();
  
  return 0;
}

int LocalWorkerNEDA::ProcessBlock(const void *input_buffer,  int input_size,  int packet_ID,
				  void *output_buffer, int output_size, int &used_output)
{
  uint32_t* inpBuffer   = (uint32_t*)input_buffer;
  uint32_t* outBuffer   = (uint32_t*)output_buffer;
  uint32_t  inpBuffSize = input_size  / sizeof(uint32_t);
  
  //  std::cout << "Receiving data in  " << __FILE__ << std::endl;
  int error_code = 0;
  int outByteSize = used_output;
  int countEvents = 0;
  uint32_t posBuffer = 0;
  //  
  //  // Output has the format of GALILEO Data
  //  // with header + subData for only one subDomain
  //  
  uint32_t outEvtLen = (sizeof(agataKey) + sizeof(subDataNEDA_t));
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

  for(int ch=0; ch < nbChannel ; ch++){  
    if(lastTS[ch]==0)lastTS[ch]=fGlobalTSOffset;
  }

  uint32_t coupleAggregateSize = 0 ;
  uint64_t tstamp_10ns         = 0;
  uint32_t fineTS              = 0;
  uint32_t extras              = 0 ;
  int      offsetTS            = 0 ;
  bool     pur                 = false;
  bool     ovr                 = false;
  bool     lost                = false;
  uint16_t chanNum             = 0;
  std::bitset<32> flags        = 0;

  // here starts the real decoding of the data for the channels 
  for(uint8_t cpl = 0 ; cpl < channelMask.count() ; cpl++){
    coupleAggregateSize = inpBuffer[posBuffer++]&0x3FFFFF;
    dataForm.SetDataFormat(inpBuffer[posBuffer++]);
    const uint16_t evtSize = dataForm.evtSize();
    if(evtSize==0){
      dataForm.printDataFormat();
      break;
    }
    //    dataForm.printDataFormat();
    for(uint16_t evt = 0 ; evt < (coupleAggregateSize-2)/evtSize;evt++){
      flags   = 0;
      decodeField = inpBuffer[posBuffer++];
      tstamp  = project_range<0,30>(decodeField).to_ulong();
      chanNum = static_cast<uint32_t>(project_range<31,31>(decodeField).to_ulong()
				      +couples[cpl]*2);
      if(dataForm.enaTrace()){ // Here we get the trace from the data frame to pass it to the ProcessOneEvent
	bool trigPos  = false;
	bool startInt = false;
	ftrace.clear();
	int LEStart  = 0;
	int INTStart = 0;
	//	ftrace.resize(dataForm.numSamples());
	for (uint16_t samp=0 ; samp<dataForm.numSamples()/2 ; samp++){
	  decodeField = inpBuffer[posBuffer++];
	  ftrace.push_back(static_cast<uint16_t>(project_range< 0,13>(decodeField).to_ulong()));
	  ftrace.push_back(static_cast<uint16_t>(project_range<16,29>(decodeField).to_ulong()));
	  if(!trigPos){
	    if ( decodeField.test(14) ) {LEStart = 2*samp; trigPos=true;}
	    else if ( decodeField.test(30) ) {LEStart = 2*samp+1; trigPos=true;}
	  }
	  if(!startInt){
	    if ( decodeField.test(15) ) {INTStart = 2*samp; startInt=true;}
	    else if ( decodeField.test(31) ) {INTStart = 2*samp+1; startInt=true;}
	  }
	}
	if(LEStart==0) LEStart=64;
	NedaCtrl *current_ctrl = &ctrl_default_neda;
	if(ctrl.find(chanNum)!=ctrl.end()){
	  current_ctrl = &ctrl[chanNum];
	}
	
	ProcessOneEvent( ftrace,
			 LEStart/2,
			 res,
			 *current_ctrl,
			 cfdtrace,
			 tracenobl,
			 integral );
	
		
	// First get the best time
	// HERE WE HAVE THE ISSUE THAT THE TS IN CAEN IS GIVEN BY THE CFD. 
	// THE "CFD" VALUE CALCULATED BY THE FPGA IS THE INTERPOLATION BETWEEN THE SAMPLES 
	// TO GET THE PRECISE ZERO CROSSING 
	// THE COMPARISON CANNOT BE MADE AS IT WAS DONE FOR THE NUMEXO2 
	// --> we pass the pre-trigger time to the filter so that we can calculate the 
	// ZC position with respect to the beginning of the waveform 

	outEvt->theData.qfast   = res.fIntegralFast;
	outEvt->theData.qslow   = res.fIntegralSlow;
	outEvt->theData.light   = res.fEnergy;
	outEvt->theData.intrise = res.fIntegratedRisetime;
	outEvt->theData.nbzero  = res.fNumberOfzc;
	outEvt->theData.flags   = 0;
	if(HCHECK(specAmpl)){
	  specAmpl->IncrQ(2,chanNum,static_cast<uint16_t>(std::floor(outEvt->theData.qfast/2)),0);
	  specAmpl->IncrQ(3,chanNum,static_cast<uint16_t>(std::floor(outEvt->theData.qslow/2)),0);
	}
	if(HCHECK(matPSD)){
	  float psd = (res.fIntegralSlow-res.fIntegralFast)/res.fEnergy*1024;
	  matPSD->IncrQ(1,
			chanNum,
			static_cast<uint16_t>(std::floor(outEvt->theData.light/matPackChan)),
			static_cast<uint16_t>(std::floor(psd)),
			0);
	  matPSD->IncrQ(2,
			chanNum,
			static_cast<uint16_t>(std::floor(outEvt->theData.light/matPackChan)),
			static_cast<uint16_t>(std::floor(res.fIntegratedRisetime)),
			0);
	}
      }
      //	qshort  = static_cast<uint16_t>((inpBuffer[startingPos+evtSize-1+evtSize*evt+2])&0x7FFF);
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
	  ovr    = decodeField.test(14);
	  lost   = decodeField.test(12);
	  flags  = static_cast<uint32_t>((extras&0x0000F000)<<15);
	  tstamp = (static_cast<uint64_t>(project_range<16,31>(decodeField).to_ulong())<<31)
	    | tstamp;
	  break;
	case 2: // Extended Time stamp [31:16] ; Flags [15:10] ; Fine Time Stamp [9:0]
	  // bit[15] Tigger lost 0x40000000
	  // bit[14] Over-range 0x20000000
	  // bit[13] event counter every 1024 events bit up 0x10000000
	  // bit[12] N lost trigger counted 0x08000000
	  flags  = static_cast<uint32_t>((extras&0x0000F000)<<15);
	  fineTS = static_cast<uint16_t>(project_range<0,9>(decodeField).to_ulong());
	  outEvt->theData.cfd = (fNsPerTStamp/1.024* fineTS) ; // to have it in ps units

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

      decodeField = inpBuffer[posBuffer++];
      pur     = decodeField.test(15);
      outEvt->theData.cfd = 0.;
      
      

      if(tsOffset.find(chanNum)!=tsOffset.end())
	offsetTS = tsOffset[chanNum];
      else
	offsetTS = 0;
      //      std::cout << fBoardID << " " <<  outEvt->theKey.evnum << " " << offsetTS << std::endl;
      outEvt->theKey.evnum    = (board << 8 |chanNum );
      // we convert the CAEN clock to the 10ns GTS one. This will be needed for the 
      // coincidence with diamant later on
      // the higher precision of the the CAEN timestamp is saved in the CFD 
      tstamp_10ns             = tstamp * fNsPerTStamp / 10 ;
      outEvt->theData.cfd    += (float)(tstamp*fNsPerTStamp - tstamp_10ns*10)*1000.;
      // THis is to go back to the GTS Clock for the coincidence with DIAMANT later on
      tstamp_10ns            += offsetTS + fGlobalTSOffset;
      if(outEvt->theData.cfd> 9999.9999){
	outEvt->theData.cfd  -= 10000. ;
	tstamp_10ns          +=     1  ;
      }
      //      std::cout << __func__ << ":" << fGlobalTSOffset << std::endl;
      outEvt->theKey.size     = outEvtLen ; 
      outEvt->theKey.key      = 0xFA020101;     

      outEvt->theKey.ts_0     = uint32_t(tstamp_10ns & 0xFFFFFFFF);         // tstLow
      outEvt->theKey.ts_1     = uint32_t((tstamp_10ns >> 32) & 0x0000FFFF); // tstHigh

      //to randomize those two:
      outEvt->theData.qshort  = static_cast<float>(project_range< 0,14>(decodeField).to_ulong());
      outEvt->theData.qlong   = static_cast<float>(project_range<16,31>(decodeField).to_ulong());
      outEvt->theData.flags   = 0x1234; 

      if(HCHECK(specAmpl)){
	specAmpl->IncrQ(0,chanNum,outEvt->theData.qshort/2,0);
	specAmpl->IncrQ(1,chanNum,outEvt->theData.qlong/2,0);
      }
      if(HCHECK(matPSD)){
	float psd = (outEvt->theData.qlong-outEvt->theData.qshort)/outEvt->theData.qlong * 1024.;
	matPSD->IncrQ(0,
		      chanNum,
		      static_cast<uint16_t>(std::floor(outEvt->theData.qlong/matPackChan)),
		      static_cast<uint16_t>(std::floor(psd)),
		      0);
      }

      if(!dataForm.enaTrace()){
	outEvt->theData.light   = outEvt->theData.qlong;
	outEvt->theData.intrise = 0.f;
	outEvt->theData.nbzero  = 1;
	outEvt->theData.flags   = 0;
      }
      // Adding PUR to the flags 
      //    flags  |= (((inpBuffer[startingPos+evtSize-1+evtSize*evt+2])&0x8000)<<11);
      // current time in ns
      if(ratesMonitor.values.find(chanNum) == ratesMonitor.values.end())
	ratesMonitor.values[chanNum].Init();
      ratesMonitor.values[chanNum].time = tstamp*fNsPerTStamp;
      ++ratesMonitor.values[chanNum].totalEvents ;
      if( pur ) ++ratesMonitor.values[chanNum].pileEvents ;
      if( ovr ) ++ratesMonitor.values[chanNum].satuEvents ;
      if( lost ) ratesMonitor.values[chanNum].lostEvents += 1024 ;
      // Adding flags to the evt num
      //    outEvt->theKey.evnum   |= flags        ;
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

int LocalWorkerNEDA::SetOutput(uint32_t *outBuff, int availableBytes, const caenEventNEDA_t * pE)
{
  // agataKey + numSubs * subData_t
  int outEvLenByte = sizeof(agataKey) + sizeof(subDataNEDA_t); // max needed size
  if (outEvLenByte > availableBytes) {
    cout << "LocalWorkerNEDA::SetOutput ==> buffer full" << endl;
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
  
  // the full size as defined in subDataNEDA_t
  subDataNEDA_t pS = pE->theData;
  memcpy(outBuff, &pS, sizeof(subDataNEDA_t));
  outEvLenByte += sizeof(subDataNEDA_t);                  // recalculate effective value
  outBuff      += sizeof(subDataNEDA_t) / sizeof(uint32_t);
  //  if(psaKey->evnum>>31 == 1)
  //     printf("NEDA SIZE IDLE = %i\n",outEvLenByte);
  psaKey->size = outEvLenByte;
  return outEvLenByte;
}


int LocalWorkerNEDA::GenerateIdles(uint64_t largest_ts,
				   void *output_buffer, int output_size, int &used_output)
{
  uint32_t* outBuffer   = (uint32_t*)output_buffer;
  int outByteSize = used_output;
  uint32_t outEvtLen = (sizeof(agataKey) + sizeof(subDataNEDA_t));  
  uint64_t newTS = 0;
  for(int ch=0; ch < nbChannel ; ch++){  
    if ( lastTS[ch] < fGlobalTSOffset ) lastTS[ch]=fGlobalTSOffset;
    if ( largest_ts - lastTS[ch] < 500000 )continue;
    //at this point we generate an idle event using the eventIdx to identify them       
    ++nbIdleEvents[ch];
    newTS = lastTS[ch] + 200000; // Adding at least 101 TS to be larger than the building window
    //    std::cout << "Generating idle for channel " << ch << " of board " << fBoardID
    //	      << " with TS = 0x" << std::hex << newTS << std::dec << std::endl;
    outEvt->theKey.size      = outEvtLen;
    outEvt->theKey.key       = 0xFA020101;
    outEvt->theKey.evnum     = 0x80000000 | ((fBoardID&0xFF) << 8) | ch&0xff ;
    outEvt->theKey.ts_0      = uint32_t(newTS & 0xFFFFFFFF);         // tstLow
    outEvt->theKey.ts_1      = uint32_t((newTS >> 32) & 0x0000FFFF); // tstHigh
    outEvt->theData.qshort   = 0.;
    outEvt->theData.qlong    = 0.;
    outEvt->theData.qfast    = 0.;
    outEvt->theData.qslow    = 0.;
    outEvt->theData.light    = 0.;
    outEvt->theData.cfd      = 0.;
    outEvt->theData.intrise  = 0.;
    outEvt->theData.nbzero   = 0 ;
    outEvt->theData.flags    = 0 ;
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


namespace
{
  void curve2d( double *x, double *y, int32_t length, double &a, double &b, double &c)
  {
    double sx = 0;
    double sy = 0;
    double sxx = 0;
    double sxy = 0;
    double sxxx = 0;
    double sxxy = 0;
    double sxxxx = 0;
		
    /* // 2- WIN OF 3%? PERF
       for(int32_t i = 0; i < length; i++)
       {
       sx += x[i];
       sy += y[i];
       sxx += x[i] * x[i];
       sxy += x[i] * y[i];
       sxxx += x[i] * x[i] * x[i];
       sxxy += x[i] * x[i] * y[i];
       sxxxx += x[i] * x[i] * x[i] * x[i];
       }*/
		
    double x2;
    for(int32_t i = 0; i < length; i++)
      {
	x2 = x[i] * x[i];
			
	sx += x[i];
	sy += y[i];
	sxx += x2;
	sxy += x[i] * y[i];
	sxxx += x2 * x[i];
	sxxy += x2 * y[i];
	sxxxx += x2 * x2;
      }
		
    double delta = -sxxxx*sxx*length +
      sxxxx*sx*sx +
      sxxx*sxxx*length -
      2*sxxx*sxx*sx +
      sxx*sxx*sxx;
		
    c = ( -sxxy*sxxx*sx +
	  sxxy*sxx*sxx +
	  sxy*sxxxx*sx -
	  sxy*sxxx*sxx -
	  sy*sxxxx*sxx +
	  sy*sxxx*sxxx ) / delta;
		
    b = ( sxxy*sxxx*length -
	  sxxy*sxx*sx -
	  sxy*sxxxx*length +
	  sxy*sxx*sxx +
	  sy*sxxxx*sx -
	  sy*sxxx*sxx ) / delta;
		
    a = ( -sxxy*sxx*length +
	  sxxy*sx*sx +
	  sxy*sxxx*length -
	  sxy*sxx*sx -
	  sy*sxxx*sx +
	  sy*sxx*sxx ) / delta;
  }
	
  /// Cubic interpolation with 4 points C2 (victor)
  double SplineUp(double *y, double zco)
  {
    double val = 0.0;
    double k0 = (-12.*y[3] +72.*y[2] +18.*y[1] -42.*y[0])/90.0;
    double k1 = (+12.*y[0] -72.*y[1] +18.*y[2] +42.*y[3])/90.0;
		
    int32_t xp = 0;
    bool pass=false;
    double yn = y[1];
    double yp = y[2];
		
    const int32_t bin = 512;
    double st1 = 1./(double)bin;
		
    for(int32_t k=1;k<bin+1;k++)
      {
	double tval = st1*k;
	double spline = (1.-tval)*y[1] + tval*y[2] + tval*(1.-tval)*( (k0-(y[2]-y[1]))*(1.-tval) +(y[2]-y[1] -k1)*tval );

	if(!pass)
	  {
	    if(spline > zco) { xp=k ; pass=true; yp = spline;}
	    else yn = spline;
	  }
      }
		
    if(pass) val = st1*( (zco-yn)/(yp-yn) +((double)xp -1.0) );
    else val = 1.0 - 0.5*st1;
		
    return val;
  }
	
  // Same for the LE (pass threshold downwards)
  double SplineDown(double *y, double zco)
  {
    double val = 0.0;
    double k0 = (-12.*y[3] +72.*y[2] +18.*y[1] -42.*y[0])/90.0;
    double k1 = (+12.*y[0] -72.*y[1] +18.*y[2] +42.*y[3])/90.0;
		
    int32_t xp = 0;
    bool pass=false;
    double yp = y[1];
    double yn = y[2];
		
    const int32_t bin = 512;
    double st1 = 1./(double)bin;
		
    for(int32_t k=1;k<bin+1;k++)
      {
	double tval = st1*k;
	double spline = (1.-tval)*y[1] + tval*y[2] + tval*(1.-tval)*( (k0-(y[2]-y[1]))*(1.-tval) +(y[2]-y[1] -k1)*tval );
			
	if(!pass)
	  {
	    if(spline < zco){ xp=k ; pass=true; yn = spline;}
	    else yp = spline;
	  }
      }
		
    if(pass) val = st1*( (zco-yp)/(yn-yp) +((double)xp -1.0) );
    else val = 1.0 -0.5*st1;
		
    return val;
  }
}

void LocalWorkerNEDA::ProcessOneEvent(
				      const std::vector<uint16_t> &fTrace,
				      const int fLEStart,
				      NedaStruct &results,
				      const NedaCtrl &ctrl,
				      std::vector<double> &cfdtrace,
				      std::vector<double> &traceNoBL,
				      std::vector<double> &integral )
{
  uint32_t fTrace_size = fTrace.size();
	
  // Get base line
  results.fSlopeBaseline     = 0;
  results.fInterceptBaseline = 0;
  results.fEnergy            = 0;
  results.fIntegralFast      = 0;
  results.fIntegralSlow      = 0;
  cfdtrace.resize( 3 * fTrace_size );
	
  for(uint32_t i = 0; i < fTrace_size - ctrl.ZCintegration; i++) {
    unsigned int intSamples = 0.;
    for(uint32_t j=i ; j<i+ctrl.ZCintegration ; j++)
      intSamples += fTrace[j];
    cfdtrace[i + 2 * fTrace_size] = static_cast<double>(intSamples) / static_cast<double>(ctrl.ZCintegration);
  }
	
  // Here count Zero Crossings, should be one
  results.fNumberOfzc = 0;
  bool passed_threshold = false;
	
  for(int32_t i = 2 * fTrace_size; i < 3 * fTrace_size; i++)
    {
      if( passed_threshold && cfdtrace[i - 1] * cfdtrace[i] < 0 )
	{
	  results.fNumberOfzc++;
	  passed_threshold = false; // Rearm zero-crossing check
	}
      else if(cfdtrace[i] < -ctrl.CFDthreshold)
	{
	  passed_threshold = true;
	}
    }
	
  uint32_t length = 0;
	
  if(fLEStart < 70 && fLEStart > ctrl.BaselineStopsNBeforeLEStart)
    length = fLEStart - ctrl.BaselineStopsNBeforeLEStart;
  else
    length = 20;
	
  traceNoBL.resize(fTrace_size);
	
  bool passed_le = false;
  //Very simple correction of the baseline
  double average_value = 0;
  for(unsigned int j=0; j<length; j++) {
    average_value+=fTrace[j];
  }
  average_value = average_value/length;
  results.fSlopeBaseline = 0;
  results.fInterceptBaseline  = average_value;
  for(uint32_t j = 0; j < fTrace_size; j++){
    traceNoBL[j] =  static_cast<double>(fTrace[j]) - (results.fInterceptBaseline + j * results.fSlopeBaseline );
    if( traceNoBL[j] < -ctrl.LEthreshold && !passed_le ){
      results.fLE = j;
      passed_le = true;
    }
  }
  
  // Determination of the LE position as the FPGA is doing it using a 250 MHz sampling
  results.LEtimeC = ( results.fLE - 4 ) + SplineDown( &traceNoBL[ results.fLE - 2 ], -ctrl.LEthreshold );
  //  printf("Leading edge position = %i, interpolation is %f\n",results.fLE,results.LEtimeC);
  // CFD
  bool passed_zc = false;
  passed_threshold = false;
  results.zc = 0;
	
  integral.resize(fTrace_size);

  for(uint32_t i = 0; i < ctrl.CFDdelay; i++)
    cfdtrace[i] = 0.0;
	
  for(uint32_t i = ctrl.CFDdelay; i < fTrace_size; i++)
    {
      cfdtrace[i] = ctrl.CFDfraction * traceNoBL[i];
      cfdtrace[i] -= traceNoBL[i - ctrl.CFDdelay];
    }
	
  int32_t CFDStart = (results.fLE > 3)*(results.fLE-3) +3;
  for(uint32_t i = CFDStart; i < fTrace_size-3; i++)
    {
      double shapediff = -cfdtrace[i-3] - 2.*cfdtrace[i-2] - 3.*cfdtrace[i-1] + 3.*cfdtrace[i] + 2.*cfdtrace[i+1] + cfdtrace[i+2];
		
      if(cfdtrace[i] < -ctrl.CFDthreshold){
	passed_threshold = true;
	//	std::cout << "Passed CFD Threshold :)" << std::endl;
      }/*else{
	 std::cout << i << " "  << cfdtrace[i] << " > " << -ctrl.CFDthreshold<<"\n";
	 }*/
		
      if(cfdtrace[i] > ctrl.CFDCrossingLine // before, the threshold was acting as a zcrossing line
	 && passed_threshold
	 && shapediff > ctrl.CFDDiffThreshold
	 && !passed_zc
	 && i > 5)
	{
	  //	  std::cout << "Threshold passed hehehe" << std::endl;
	  results.zc = i;
	  passed_zc = true;
	  continue; // Skip checking zero crossing when just passed threshold
	}
    }
  
  results.CFDtimeC = -1;
  results.CFDtimeQ = -1;
  results.CFDtimeL = -1;
  results.CFDtimeI = -1;
  //  printf("results.zc = %i\n", results.zc);
  if(results.zc > 10 && results.zc < 230)
    {
      // Cubic interpolation for the CFD
      results.CFDtimeC = (results.zc-1) + SplineUp(&cfdtrace[results.zc-2], ctrl.CFDCrossingLine);
      // Fit quadratic to 4 pts around zc
      std::vector<double> xs;
		
      for(int32_t i = results.zc - 2; i < results.zc + 2; i++)
	{
	  xs.push_back(i);
	}
		
      double a,b,c;
		
		
      curve2d( &xs[0], &cfdtrace[results.zc-2], 4, a, b, c );
		
      double SR = b * b / (4. * a * a) - c / a;
		
      if(SR >= 0)
	{
	  double t1 = -b / (2. * a) + std::sqrt(SR);
	  double t2 = -b / (2. * a) - std::sqrt(SR);

	  if( fabs(t1 - results.zc) <= fabs(t2 - results.zc) )
	    {
	      results.CFDtimeQ=t1;
	    }
	  else
	    {
	      results.CFDtimeQ=t2;
	    }
	}
		
      // Linear fit, 4 pts around zc
      double sx = 0;
      double sy = 0;
      double sxx = 0;
      double sxy = 0;
      length = 4;
		
      for(uint32_t j = 0; j < length; j++)
	{
	  sy += cfdtrace[results.zc - 2 + j];
	  sx += xs[j];
	  sxx += xs[j] * xs[j];
	  sxy += cfdtrace[results.zc - 2 + j] * xs[j];
	}
      double delta = 0.;
      delta = length*sxx - sx*sx;
		
      a = ( length*sxy - sx*sy ) / delta;
      b = ( sxx*sy - sx*sxy ) / delta;
		
      results.CFDtimeL = ( a != 0 ? -b/a : -1 );
		
      // Linear interpolation i-1 i
      double dy = cfdtrace[results.zc] - cfdtrace[results.zc - 1];
		
      results.CFDtimeI = ( dy != 0 ? results.zc - 1 - cfdtrace[results.zc - 1] / dy : -1 );
    }
  if(results.zc > 0)
    {
      // Integrate energy (trapizoidal integration)
      for(int32_t i = 0; i < results.zc + ctrl.StartInt; i++)
	integral[i] = 0;
		
      int32_t atp = 0;
      double maxI = -1; // used to keep track of the max value in integral[]
      int32_t stoppoint = ( results.zc/*+ctrl.StartInt*/+ctrl.EnergyGate < traceNoBL.size() ? results.zc/*+ctrl.StartInt*/+ctrl.EnergyGate : traceNoBL.size() );
		
      int32_t startpoint = results.zc + ctrl.StartInt;
		
      if(startpoint < 0)
	startpoint = 0;
		
      for(int32_t i = startpoint; i < stoppoint; i++, atp++)
	{
	  if( !( atp == 0 || atp == ctrl.EnergyGate - 1 ) )
	    results.fEnergy -= traceNoBL[i];
	  else
	    results.fEnergy -= 0.5*traceNoBL[i]; // This is "trap part" of int32_t.

	  integral[i] = results.fEnergy;

	  if( integral[i] > maxI )
	    maxI = integral[i];
	}
		
      for(int32_t i = stoppoint; i < traceNoBL.size(); i++)
	{
	  integral[i] = integral[stoppoint - 1];

	  if(integral[i] > maxI)
	    maxI = integral[i];
	}
		
      // Find TimeToFraction
      std::vector<double>::iterator just_after =
	std::upper_bound( integral.begin(), integral.end(), maxI * ctrl.HigherIRT );
		
      std::vector<double>::iterator just_before = just_after - 1;
		
      // Just_after points to the array first array element larger than
      // the wanted fraction, so just_before points to the elment just before :-)
		
      // interpolate value
      double frac_part = ( maxI*ctrl.HigherIRT - *just_before) / (*just_after - *just_before);
		
      // This is iterator arithmetics, just_before-integral.begin() is just the
      // "array index" so the sum is the wanted time
		
      results.fIntegratedRisetime = (just_before - integral.begin()) + frac_part - results.CFDtimeC + 5; // The "5" is just an offset
		
      // Integration
      atp = 0;
      stoppoint = ( results.zc + ctrl.LengthFast < traceNoBL.size() ?
		    results.zc + ctrl.LengthFast :
		    traceNoBL.size() );
      //      std::cout << "start integration at = " << results.zc + ctrl.StartInt 
      //		<< " (FYI LengthFast = " << ctrl.LengthFast << ") "
      //		<< "stop integration at = " << stoppoint << std::endl;
      for(int32_t i = results.zc + ctrl.StartInt; i < stoppoint; i++, atp++)
	{
	  if (i > ctrl.StartExclude && i < ctrl.StartExclude + ctrl.WidthExclude) continue;
	  if( !( atp == 0 || atp == ctrl.LengthFast - 1 ) )
	    results.fIntegralFast -= traceNoBL[i];
	  else
	    results.fIntegralFast -= 0.5 * traceNoBL[i]; // This is "trap part" of int32_t.
	}
		
      atp = 0;
      stoppoint = ( results.zc + ctrl.LengthFast + ctrl.LengthSlow < traceNoBL.size() ?
		    results.zc + ctrl.LengthFast + ctrl.LengthSlow :
		    traceNoBL.size() );
		
      for(int32_t i = results.zc + ctrl.LengthFast; i < stoppoint; i++, atp++)
	{
	  if (i > ctrl.StartExclude && i < ctrl.StartExclude + ctrl.WidthExclude) continue;
	  if( !( atp == 0 || atp == ctrl.LengthSlow - 1 ) )
	    results.fIntegralSlow -= traceNoBL[i];
	  else
	    results.fIntegralSlow -= 0.5 * traceNoBL[i]; // This is "trap part" of int32_t.
	}
    }
}
