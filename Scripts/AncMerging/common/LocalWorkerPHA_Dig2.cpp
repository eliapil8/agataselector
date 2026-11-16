#include "LocalWorkerPHA_Dig2.h"

#include <cstdio>
#include <cstring>
#include <algorithm>

#include "rubuilder/utils/ConfReader.h"

const int   specCal     =  4096;    // adc range
const int   specRaw     = 16384;    // adc range
const int   matPackChan =    10;

using namespace std;

LocalWorkerPHA_Dig2::LocalWorkerPHA_Dig2(CycleServer& _cServer, int& _tcpPortno, int boardID,
			       int numdoms, int numsubs, int numsamp)
  : LocalWorker(_cServer, _tcpPortno, boardID)
{
  gDaughterClass = "LocalWorkerPHA_Dig2";
  calibration.clear();
  fGlobalTSOffset = 0L;
  fEnergyGain = 1.f;
  HasHeader   = false;
  FirstBlock  = true;
  outEvt = new caenEventPHA_t;
  fLastTS =  0;
  aTrace = nullptr;
  wspace = nullptr;
  specAmpl = nullptr;
  matPSD = nullptr;
  waveAnalyzer = new MWD;
  waveAnalyzer->Reset(128, true);      // user-provided working buffers
  waveAnalyzer->nDcwin     = 128 / 5;     // the first 20% --> check if this is valid
  waveAnalyzer->nSmooth    = 5;
  waveAnalyzer->nWidthT    = 8;
  waveAnalyzer->nDelayCFD  = 6;
  waveAnalyzer->fFractCDF  = 0.25f;
  waveAnalyzer->fThreshTFA = 20;
  waveAnalyzer->fThreshCFD = 0;
  waveAnalyzer->nMinWidthP = 2;
  waveAnalyzer->nMinWidthN = 6;  
  memset(TimeShaper,  0, sizeof(TimeShaper));

}

// todo: add the delete []
LocalWorkerPHA_Dig2::~LocalWorkerPHA_Dig2()
{
  if(aTrace)
    delete [] aTrace;
  if(wspace)
    delete [] wspace;
  for (int ii = 0; ii < 64; ii++) {
    if (TimeShaper[ii]) {
      delete TimeShaper[ii];
      TimeShaper[ii] = nullptr;
    }
  }

  cServer.Finish();
}

int LocalWorkerPHA_Dig2::GetParameters(bool doList, bool partial)
{
  bool        AnaRiseTime;
  int         boardID, boardCH, boardTS, boardSampling;
  std::string boardFW;
  std::string histPrefix;
  std::string boardName;
  std::string graphServ;
  uint32_t    graphPort;
  int16_t     channelID;
  float       tsOffs;
  std::string tmpOutData;
  std::string confFile;
  float vVal[8] = { 0 };
  int         rdbPort;
  std::string rdbHost;
  hGroup.destroy();

  rubuilder::utils::ConfReader conf;
  conf.Add("SaveDataDir", 
	   "Where to write spectra"
	   , &fOutPath);
  conf.Add("SpecPrefix",
	   "prefix to names of spectra",
	   &boardID, &histPrefix); 
  conf.Add("Board",  
	   "Board description: BoardID, Name, Firmware, number of channel,  nsPerTimestamp, ns per sample",
	   &boardID, &boardName, &boardFW, &boardCH, &boardTS, &boardSampling);
  conf.Add("TSOffset",   
	   "Timestamp offset for board X chanel Y",
	   &boardID, &channelID, &tsOffs);
  conf.Add("GraphiteServer", "Graphite Server to send the rates of the boards: Hostname and port",
	   &graphServ, &graphPort);
  conf.Add("OutDataKey", 
	   "Output ADF frame key (0xFA0201A2=SPIDER, 0xFA0201A3=DANTE, ...",
	   &boardID, &tmpOutData);
  conf.Add("ConfFilePSA",
	   "Configuration file for the PSA",
	   &boardID, &confFile);
  conf.Add( "RedisServer"    ,"REDIS Server host and port",
	    &rdbHost, &rdbPort);
//  conf.Add("AnaRiseTime",
//	   "apply 2 triangular filters to short traces to determine risetime of signal"
//	   ,&fRise1, &fRise2, &fRiseThreshold, &fRiseXoffs, &fRiseXgain, &fRiseYoffs, &fRiseYgain).SetRequired(3);
  // refine behaviour
  fRise1 = -1;
  fRise2 =  0;
  fRiseThreshold = 4;
  fRiseXoffs = 20;
  fRiseXgain = 0.05;
  fRiseYoffs = 0;
  fRiseYgain = 0.05;
    
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
    fBoardName = boardName;
    filterName << "LF_"<< fBoardName;
    ratesMonitor.setName(fBoardName);
    fNsPerSample  = boardSampling;
    fNsPerTStamp  = boardTS;
    nbChannel     = boardCH;
    
    for(int ch = 0 ; ch < nbChannel ; ++ch){
      lastTS[ch]  =0;
      nbEvents[ch]=0;
      nbGoodEvents[ch]=0;
      nbIdleEvents[ch]=0;
      nbTraces[ch]=1000;
    }
  }  
  which = conf.Find("SpecPrefix");
  times = conf.Times(which);
  for (int nn = 0 ; nn < times ; ++nn){
    conf.Restore(which,nn);
    //    std::cout << boardID << " " << histPrefix << std::endl;
    if(boardID!=fBoardID)continue;
    fHistPfx = histPrefix;
    // maybe a good idea to put back Dino's histograms to have an idea of what is going on
    // we should nevertheless write them on the local disk
    specAmpl = new MultiHist<uint32_t>(nbChannel, 16384);     // [0]-fromShaper [1]-fromTrace
    specAmpl->setFileName(fOutPath+fHistPfx+"?Ampl.spec");
    specAmpl->setComment("raw energy");
    hGroup.add(specAmpl);
    matPSD = new MultiHist<uint16_t>(2,nbChannel,1024,1024); //[0] 2-shaping [1] deriv
    matPSD->setFileName(fOutPath+fHistPfx+"?PSD.matr");
    matPSD->setComment("Imax and PSD matrix");
    hGroup.add(matPSD);
  }
  
  doRise = true;
  //if(conf.Seen("AnaRiseTime")) doRise = true;
  if(conf.Seen("GraphiteServer")){
    ratesMonitor.setHost(graphServ,graphPort);
  }

  which = conf.Find("OutDataKey");
  times = conf.Times(which);
  for (int nn = 0; nn < times; nn++) {
    conf.Restore(which, nn);
    if(boardID!=fBoardID)continue;
    outDataKey = OutDataKey(tmpOutData) ;
    if(outDataKey == keyDataSauron){
      delete outEvt;
      outEvtSauron = new caenEventSauron_t;
    }
  }  

  
  if (rv != 0)
    return rv;    // not worthwhile continuing
  
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

  fPromptPeriod = max(0, fPromptPeriod);
  cServer.SetHistGroup(&hGroup);

  return rv;
}

int LocalWorkerPHA_Dig2::ProcessConfig(const char* fname)
{
  cout << gMotherClass + "::process_config() called with file = " << fname << endl;
  
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

int LocalWorkerPHA_Dig2::ProcessStart(uint32_t run)
{
  fRunNumber = run;
  fCountBlocks = 0;
  cServer.Start(filterName.str().c_str(), max(fPromptPeriod, 0));
  return 0;
}

int LocalWorkerPHA_Dig2::ProcessStop()
{
  cServer.Save(true);
  
  std::cout << "Number of events in board " 
	    << fBoardName << " (" <<fBoardID << ") of this run " 
	    << cServer.GetCounts() << std::endl;
  printStatistics();
  
  return 0;
}

int LocalWorkerPHA_Dig2::ProcessBlock(const void *input_buffer,  int input_size,  int packet_ID,
                                   void *output_buffer, int output_size, int &used_output)
{
  uint64_t* inpBuffer   = (uint64_t*)input_buffer;
  uint32_t* outBuffer   = (uint32_t*)output_buffer;
  uint64_t  inpBuffSize = input_size  / sizeof(uint64_t)-1;

  //  std::cout << "Receiving data in  " << __FILE__ << std::endl;
  int error_code = 0;
//  
  int outByteSize = used_output;
  int countEvents = 0;
  uint32_t outEvtLen = (sizeof(agataKey) + sizeof(subDataPHA_t));
  if(outDataKey == keyDataSauron)
    outEvtLen = (sizeof(agataKey) + sizeof(subDataSauron_t));
  
  uint32_t posBuffer = 1;
  std::bitset<64> decodeField;
  std::bitset<64> sample[2];
  uint64_t tstamp_10ns = 0;
  uint64_t tstamp;
  int      offsetTS = 0 ;
  uint16_t board  = (inpBuffer[0]&0xF8000000)>>59;
  uint16_t boardID; // to match the 16 channels for the BU
  uint16_t channel;
  uint16_t chanID ; // again to match the max 16 channels / board ... useful no but don't want to modify the BU
  uint16_t energy ;
  uint16_t fineTS ;
  float    cfd    ;
  float fineTS_2_GTS = 8/1024.;

  while(posBuffer < inpBuffSize){
    decodeField = swapLong(inpBuffer[posBuffer]);
    uint32_t n_words = project_range< 0,31>(decodeField).to_ulong();
    uint32_t headKey = project_range<60,63>(decodeField).to_ulong();
    if(n_words == 4 && headKey == 0x3){ // START BLOCK
      // This is the start run frame we do not really care about it
      posBuffer += n_words;
      printf("Start header posBuffer = %i\n",posBuffer);
      continue;
    }
    if(n_words == 3 && headKey == 0x3){ // STOP BLOCK
      // This is the start run frame we do not really care about it
      posBuffer += n_words;
      printf("Stop block detected ... we should have all the data\n");
      continue;
    }
//printf("Number of words in the aggregate = %i (size buffer = %i) at pos %i\n for board %i",
//	   n_words,inpBuffSize,posBuffer,board);

    // So now we should have aggregate events. 
    // Since we could maybe have more than 1 aggregate / buffer we loop again.
    
    uint32_t posEvent = posBuffer++;
    while (posEvent < posBuffer+n_words){
      if(posEvent==inpBuffSize)break;
      uint32_t flags = 0;
      //if(posAggregate == 0 ) posAggregate++;
      //std::cout << "posEvents = " << posEvent << " inpSize = " << inpBuffSize << std::endl;
      decodeField = swapLong(inpBuffer[++posEvent]);
      channel = project_range<56,62>(decodeField).to_ulong();
      tstamp  = project_range< 0,47>(decodeField).to_ulong();
      //printf("posEvent=%i - %llx - swapped %llx Channel = %i, tstamp = %llu\n",posEvent,inpBuffer[posEvent],decodeField.to_ulong(),channel,tstamp);
      //      if(decodeField.test(55)) std::cout << "Event with statistic only" << std::endl;
      //      std::cout << " --> Event from channel " << channel << " with TStamp = " << tstamp <<  std::endl;
      decodeField = swapLong(inpBuffer[++posEvent]);
      energy = project_range< 0,15>(decodeField).to_ulong();
      fineTS = project_range<16,25>(decodeField).to_ulong();
      cfd = fineTS*fineTS_2_GTS;
      tstamp_10ns = (tstamp * 8) / 10;
      cfd += (tstamp * 8 - tstamp_10ns *10);
      // bits 42 to 61 are flags (42-49: high priority, 50-61 low priority) 
      //page 51 of rev6
      bool pur     = decodeField.test(42);
      if( pur )  ++ratesMonitor.values[channel].pileEvents ;
      bool purGard = decodeField.test(43);
      bool inpSat  = decodeField.test(44);
      if( inpSat ) ++ratesMonitor.values[channel].satuEvents ;
      bool trpSat  = decodeField.test(46);
      bool sftTrig = decodeField.test(55);
      bool glbTrig = decodeField.test(54);
      bool extTrig = decodeField.test(53);
      bool slfTrig = decodeField.test(56);
      flags = decodeField[44]<<18 | decodeField[46] << 17 | decodeField[42] << 16;
      if (!slfTrig){ // considered as an idle event for us
	//	std::cout << "Global trigger event for board " << fBoardID << std::endl;
	flags |= 0x80000000;
	++nbIdleEvents[channel];
      }
      bool wavePresent = decodeField.test(62);
      //if(wavePresent)std::cout << "Wave are present? " << ((wavePresent)? "yes" : "no") << std::endl;
//      std::cout << std::dec << "Energy = " << energy << " fineTS = " << fineTS << std::endl;
      //extra
      bool lastExtra = decodeField.test(63);
      uint16_t extraType=0;
      if(wavePresent){
	uint16_t ap1_Type, ap1_Mult, ap2_Type, ap2_Mult;
	bool ap1_Signed, ap2_Signed;
	uint16_t dp_Type[4];	
	uint16_t triggerThres ;
	uint16_t downSamp    ; 
	while(!lastExtra){
	  decodeField = swapLong(inpBuffer[++posEvent]);
	  lastExtra = decodeField.test(63);
	  extraType = project_range<60,62>(decodeField).to_ulong();
	  //	  std::cout << "extra type = " << extraType << " at posEvent = " << posEvent << std::endl;
	  switch(extraType){
	  case 0: // waveform configuration
	    ap1_Type     = project_range< 0, 2>(decodeField).to_ulong();
	    ap1_Signed   = decodeField.test(3);
	    ap1_Mult     = project_range< 4, 5>(decodeField).to_ulong();
	    ap2_Type     = project_range< 6, 8>(decodeField).to_ulong();
	    ap2_Signed   = decodeField.test(9);
	    ap2_Mult     = project_range<10,11>(decodeField).to_ulong();
	    dp_Type[0]   = project_range<12,15>(decodeField).to_ulong();
	    dp_Type[1]   = project_range<16,19>(decodeField).to_ulong();
	    dp_Type[2]   = project_range<20,23>(decodeField).to_ulong();
	    dp_Type[3]   = project_range<24,27>(decodeField).to_ulong();
	    triggerThres = project_range<28,43>(decodeField).to_ulong();
	    downSamp     = project_range<44,45>(decodeField).to_ulong();
//	    printf("Analog probe 1: %i is signed %i mult %i\n",ap1_Type,ap1_Signed,ap1_Mult);
//	    printf("Analog probe 2: %i is signed %i mult %i\n",ap2_Type,ap2_Signed,ap2_Mult);
//	    printf("Digital probe 0: %i, 1: %i, 2: %i, 3: %i\n",dp_Type[0],dp_Type[1],dp_Type[2],dp_Type[3]);
	    break;
	  case 1: // dead time
	    //uint64_t deadTime = project_range<0,47>(decodeField).to_ulong();
	    break;
	  case 2: // counter
//	    uint32_t savedCount = project_range< 0,23>(decodeField).to_ulong();
//	    uint32_t triggCount = project_range<24,47>(decodeField).to_ulong();
	    break;
	  default:
	    std::cerr << __LINE__ << ": Type not implemented yet: " << extraType << std::endl;
	  }
	  //posBuff++;
	}
	decodeField = swapLong(inpBuffer[++posEvent]);
	// This is the number of 64 bit words ... the trace will be twice as long;
	uint16_t nbSamples_word = project_range< 0,11>(decodeField).to_ulong();
	//std::cout << "Samples to be read: " << nbSamples_word*2 << std::endl;
	std::valarray<float> probe1;
	probe1.resize(nbSamples_word*2);
	int sampTrig = 0;
	for(uint16_t samp = 0 ; samp<nbSamples_word ; ++samp){
	  decodeField = swapLong(inpBuffer[++posEvent]);
	  sample[0] = project_range< 0,31>(decodeField).to_ulong();
	  sample[1] = project_range<32,63>(decodeField).to_ulong();
	  if(ap1_Type == 0x0){
	    //Should take what was done for GALTRACE in the LocalFilter of GALILEO
	    for(short sub = 0 ;sub < 2;++sub){
	      if(sample[sub].test(14))sampTrig = samp*2+sub;
	      probe1[samp*2+sub] = project_range< 0,13>(sample[sub]).to_ulong() + (rand()%1000)/1000. - 0.5;
	    }
	  }else if(ap2_Type == 0x0){
	    for(short sub = 0 ;sub < 2;++sub){
	      probe1[samp*2+sub] = project_range<0,13>(sample[sub]).to_ulong() + (rand()%1000)/1000. - 0.5;
	    }
	  }
 	}
	if(probe1.sum()<0) probe1 *=-1;
	float baseline = static_cast<std::valarray<float>>(probe1[std::slice(0,20,1)]).sum()/20.;
	probe1-=baseline;	
	if(aTrace == nullptr)
	  aTrace = new float[probe1.size()];
	if(wspace == nullptr)
	  wspace = new float[3 * probe1.size()];
	std::copy(begin(probe1),end(probe1),aTrace);
	// CFD
	float otrig = 0 ;
	float trig = waveAnalyzer->TriggerCFD(aTrace,50, wspace, otrig);
	//	std::cout << "otrig = " << otrig << " trig: " <<trig <<" sampleTrig = " << sampTrig << std::endl;
	if(outDataKey == keyDataSauron){
	  outEvtSauron->theData.off_cfd = otrig;
	}
	if (TimeShaper[channel] == nullptr && nbTraces[channel]>0) {
	  std::ostringstream on;
	  on << fOutPath + fHistPfx << "__1000-4-"
	     << probe1.size() << "-F__timing-"
	     << setfill('0') << setw(2) << channel << ".dat";
	  TimeShaper[channel] = new Append<float>(on.str(), 4*probe1.size());
	}
	if(nbTraces[channel]>0){
	  float *fw0 = wspace;
	  float *fw1 = fw0 + probe1.size();
	  float *fw2 = fw1 + probe1.size();
	  TimeShaper[channel]->add(aTrace,probe1.size());
	  TimeShaper[channel]->add(fw0,probe1.size());
	  TimeShaper[channel]->add(fw1,probe1.size());
	  TimeShaper[channel]->add(fw2,probe1.size());
	  nbTraces[channel]--;
	}
	// Rise time
	if(outDataKey == keyDataSauron){
	  if(doRise){
	    outEvtSauron->theData.off_cfd = otrig;
	    float yMax1 = waveAnalyzer->TriangAmpli(aTrace,   4, wspace);    // shaping 1
	    float yMax2 = waveAnalyzer->TriangAmpli(aTrace,  10, wspace);    // shaping 2
	    outEvtSauron->theData.imax    = waveAnalyzer->TriangAmpli(aTrace, -4, wspace);    // shaping 2
	    outEvtSauron->theData.psd     = yMax2-yMax1;
	  } else {
	    outEvtSauron->theData.imax    = 0.;
	    outEvtSauron->theData.psd     = 0.;
	  }
	}
 	//if(samples.sum()>0){
	  //float baseline = static_cast<std::valarray<float>>(samples[std::slice(0,30,1)]).sum()/20.;
	  //samples -= baseline;
	  //if (samples.sum()<0) samples *=-1; //invert negative signals
//	  for(uint16_t samp = 0 ; samp<nbSamples_word*2 ; ++samp){
//	    std::cout << "samples["<<samp<<"] = " << samples[samp] << std::endl;
//	  }
	      
	  //}
      } // end wave present
      //Filling the output event, here for the BU to work without modification we modify 
      // channel and board to match the 16 channel structure. 
      if(tsOffset.find(channel)!=tsOffset.end())
	offsetTS = tsOffset[channel];
      else
	offsetTS = 0;
      boardID = fBoardID + channel/16;
      //std::cout << "board = " << fBoardID << " boardID = " << boardID << " (channel = " << channel << ")\n)";
      chanID  = channel%16;
      int outEvtSize = 0;

      if(outDataKey!=keyDataSauron){
	outEvt->theKey.size     = outEvtLen              ; 
	tstamp_10ns            += offsetTS + fGlobalTSOffset;
	outEvt->theKey.key      = outDataKey             ;                      // default is data:ranc1
	outEvt->theKey.evnum    = (boardID << 8 | chanID);
	outEvt->theKey.evnum   |= flags;
	outEvt->theKey.ts_0     = uint32_t(tstamp_10ns & 0xFFFFFFFF);         // tstLow
	outEvt->theKey.ts_1     = uint32_t((tstamp_10ns >> 32) & 0x0000FFFF); // tstHigh
	if(slfTrig){
	  outEvt->theData.energy  = energy;
	  outEvt->theData.cfd     += static_cast<uint16_t>(std::floor(cfd));	
	}else{
	  outEvt->theData.energy  = 0;
	  outEvt->theData.cfd     = 0;
	}
	if(HCHECK(specAmpl) && slfTrig)
	  specAmpl->IncrQ(channel,energy/4,0); // energy is between 0 and 65536 so we divide by 4
	
	
	outEvtSize = SetOutput(outBuffer,output_size-outByteSize,outEvt);
      }else{
	outEvtSauron->theKey.size     = outEvtLen              ; 
	tstamp_10ns            += offsetTS + fGlobalTSOffset;
	outEvtSauron->theKey.key      = outDataKey             ;                      // default is data:ranc1
	outEvtSauron->theKey.evnum    = (boardID << 8 | chanID);
	outEvtSauron->theKey.evnum   |= flags;
	outEvtSauron->theKey.ts_0     = uint32_t(tstamp_10ns & 0xFFFFFFFF);         // tstLow
	outEvtSauron->theKey.ts_1     = uint32_t((tstamp_10ns >> 32) & 0x0000FFFF); // tstHigh
	if(slfTrig){
	  outEvtSauron->theData.energy  = energy;
	  outEvtSauron->theData.cfd     += static_cast<uint16_t>(std::floor(cfd));	
	}else{
	  outEvtSauron->theData.energy  = 0;
	  outEvtSauron->theData.cfd     = 0;
	  outEvtSauron->theData.imax    = 0.;	
	  outEvtSauron->theData.psd     = 0.;	

	}
	if(HCHECK(matPSD) && doRise){
	      matPSD->IncrQ(0,
			    channel,
			    static_cast<uint16_t>(std::floor(outEvtSauron->theData.energy)/64),
			    static_cast<uint16_t>(std::floor(outEvtSauron->theData.imax)),
			    0);
	      matPSD->IncrQ(1,
			    channel,
			    static_cast<uint16_t>(std::floor(outEvtSauron->theData.energy)/64),
			    static_cast<uint16_t>(std::floor(outEvtSauron->theData.psd)),
			    0);
	      
	    }

	if(HCHECK(specAmpl) && slfTrig)
	  specAmpl->IncrQ(channel,energy/4,0); // energy is between 0 and 65536 so we divide by 4
	
	
	outEvtSize = SetOutput(outBuffer,output_size-outByteSize,outEvtSauron);

      }
      ++nbEvents[channel];
      countEvents++;
      if(slfTrig)
	++nbGoodEvents[channel];
      lastTS[channel] = tstamp_10ns;
      if (fLastTS < tstamp_10ns) fLastTS = tstamp_10ns;

      if(outEvtSize<0){ //output buffer full
	//	  std::cout << "Returning to LF" << std::endl;
	return outEvtSize;
      }
      outByteSize += outEvtSize ;
      outBuffer   += outEvtSize / sizeof(uint32_t) ;
      //	std::cout << "End of event at posBuffer = " << posEvent 
//		  << "posAggregate = " << posAggregate << std::endl;
      //posAggregate = posEvent-posBuffer;
    }
    //  std::cout << "End of the aggregate!" << std::endl;
    posBuffer += (posEvent-posBuffer);
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

int LocalWorkerPHA_Dig2::SetOutput(uint32_t *outBuff, int availableBytes, const caenEventPHA_t * pE)
{
  // agataKey + numSubs * subData_t
  int outEvLenByte = sizeof(agataKey) + sizeof(subDataPHA_t); // max needed size
  
  agataKey* psaKey = (agataKey *)outBuff;
  outEvLenByte = sizeof(agataKey);                      // recalculate effective value
  
  // hand-made agata key
  psaKey->size  = pE->theKey.size;
  psaKey->key   = pE->theKey.key;
  psaKey->ts_0  = pE->theKey.ts_0;
  psaKey->ts_1  = pE->theKey.ts_1;
  psaKey->evnum = pE->theKey.evnum;
  if ((sizeof(agataKey) + sizeof(subDataPHA_t)) > availableBytes) {
    std::cout << "LocalWorkerPHA_Dig2::SetOutput ==> buffer full (needed/available): " << outEvLenByte << " " << availableBytes << std::endl;
    printf("Board = %i Channel %i, ts_0 = 0x%08X ts_1 = 0x%08X\n",
	   pE->theKey.key&0xFF00 >> 8,
	   pE->theKey.key&0x00FF,
	   pE->theKey.ts_0,
	   pE->theKey.ts_1);
    return availableBytes - outEvLenByte;
  }  
  
  outBuff += sizeof(agataKey) / sizeof(uint32_t);       // immediatly after agataKey
  
  // the full size as defined in subData_t
  
  subDataPHA_t pS = pE->theData; 
  memcpy(outBuff, &pS, sizeof(subDataPHA_t));
  outEvLenByte += sizeof(subDataPHA_t);                  // recalculate effective value
  outBuff += sizeof(subDataPHA_t) / sizeof(uint32_t);
  
//  if(psaKey->evnum>>31 == 1)
//     printf("PHA SIZE IDLE = %i\n",outEvLenByte);
  psaKey->size = outEvLenByte;
  return outEvLenByte;
}

int LocalWorkerPHA_Dig2::SetOutput(uint32_t *outBuff, int availableBytes, const caenEventSauron_t * pE)
{
  // agataKey + numSubs * subData_t
  int outEvLenByte = sizeof(agataKey) + sizeof(subDataSauron_t); // max needed size
  
  agataKey* psaKey = (agataKey *)outBuff;
  outEvLenByte = sizeof(agataKey);                      // recalculate effective value
  
  // hand-made agata key
  psaKey->size  = pE->theKey.size;
  psaKey->key   = pE->theKey.key;
  psaKey->ts_0  = pE->theKey.ts_0;
  psaKey->ts_1  = pE->theKey.ts_1;
  psaKey->evnum = pE->theKey.evnum;
  if ((sizeof(agataKey) + sizeof(subDataSauron_t)) > availableBytes) {
    std::cout << "LocalWorkerPHA_Dig2::SetOutput ==> buffer full (needed/available): " << outEvLenByte << " " << availableBytes << std::endl;
    printf("Board = %i Channel %i, ts_0 = 0x%08X ts_1 = 0x%08X\n",
	   pE->theKey.key&0xFF00 >> 8,
	   pE->theKey.key&0x00FF,
	   pE->theKey.ts_0,
	   pE->theKey.ts_1);
    return availableBytes - outEvLenByte;
  }  
  
  outBuff += sizeof(agataKey) / sizeof(uint32_t);       // immediatly after agataKey
  
  // the full size as defined in subData_t
  
  subDataSauron_t pS = pE->theData; 
  memcpy(outBuff, &pS, sizeof(subDataSauron_t));
  outEvLenByte += sizeof(subDataSauron_t);                  // recalculate effective value
  outBuff += sizeof(subDataSauron_t) / sizeof(uint32_t);
  
  psaKey->size = outEvLenByte;
  return outEvLenByte;
}


int LocalWorkerPHA_Dig2::GenerateIdles(uint64_t largest_ts,
                                   void *output_buffer, int output_size, int &used_output)
{
  return 0;
}
