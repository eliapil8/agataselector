#include "LocalWorkerPSD_Dig2.h"

#include <cstdio>
#include <cstring>
#include <algorithm>

#include "rubuilder/utils/ConfReader.h"

const int   specCal     =  4096;    // adc range
const int   specRaw     = 16384;    // adc range
const int   matPackChan =    10;

using namespace std;

LocalWorkerPSD_Dig2::LocalWorkerPSD_Dig2(CycleServer& _cServer, int& _tcpPortno, int boardID,
                                         int numdoms, int numsubs, int numsamp)
: LocalWorker(_cServer, _tcpPortno, boardID)
{
    gDaughterClass = "LocalWorkerPSD_Dig2";
    fOutMagicData = nullptr;
    calibration.clear();
    fGlobalTSOffset = 0L;
    HasHeader   = false;
    FirstBlock  = true;
    outEvt = new caenEventPSD_t;
    tstCount = 0;
    fDomID = 0;
    tstoff = 0;
    fDomOutOffset = 0;
    maxChans=64;
    fOutMagicData = new uint32_t[16];
    fLastTS =  0;
    
}

// todo: add the delete []
LocalWorkerPSD_Dig2::~LocalWorkerPSD_Dig2()
{
    cServer.Finish();
}

int LocalWorkerPSD_Dig2::GetParameters(bool doList, bool partial)
{
    float    cTstamp(0);
    uint32_t adcSlot(0), adcNumChan(32);
    std::string histPrefix;
    uint32_t calBoard(0), calChan(0);
    float    calThresh(0), calOffs(0), calGain(1.f);
    bool     isAncillary(false);
    int   chSync(-1);
    uint16_t boardID, boardFW, boardCH, boardTS, boardSampling;
    std::string boardName;
    std::string graphServ;
    uint32_t graphPort;
    int16_t channelID;
    float tsOffs;
    std::string tmpOutData;
    uint16_t preTrig;
    std::string confFile;
  int         rdbPort;
  std::string rdbHost;
    hGroup.destroy();
    
    rubuilder::utils::ConfReader conf;
    conf.Add("SaveDataDir",      "where to write spectra"
             , &fOutPath);
    conf.Add("SpecPrefix",       "prefix to names of spectra",
             &boardID, &histPrefix);
    conf.Add("Board",      "Board description: BoardID, Name, Firmware, number of channel, nsPerTimestamp, ns per sample",
             &boardID, &boardName, &boardFW, &boardCH, &boardTS, &boardSampling);
    conf.Add("EnergyCal",  "Energy calibration for board X chanel Y",
             &boardID, &channelID);
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
  conf.Add( "RedisServer"    ,"REDIS Server host and port",
	    &rdbHost, &rdbPort);    
    // refine behaviour
    
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
            
        }
    }
    which = conf.Find("SpecPrefix");
    times = conf.Times(which);
    for (int nn = 0 ; nn < times ; ++nn){
        conf.Restore(which,nn);
        if(boardID!=fBoardID)continue;
        fHistPfx = histPrefix;
        // maybe a good idea to put back Dino's histograms to have an idea of what is going on
        // we should nevertheless write them on the local disk
        SpecAmpl = new MultiHist<uint32_t>(2,nbChannel, 16384);      // [0]-qshort [1]-qlong
        SpecAmpl->setFileName(fOutPath+fHistPfx+"?Ampl.spec");
        SpecAmpl->setComment("raw energy");
        hGroup.add(SpecAmpl);
    }
    
    
    if(conf.Seen("GraphiteServer")){
        ratesMonitor.setHost(graphServ,graphPort);
    }
    
    which = conf.Find("OutDataKey");
    times = conf.Times(which);
    for (int nn = 0; nn < times; nn++) {
        conf.Restore(which, nn);
        if(boardID!=fBoardID)continue;
        outDataKey = OutDataKey(tmpOutData) ;
    }
    
    
    for(int nn = 0 ; nn < v1730MaxChans ; nn++)
        fOutMagicData[nn] = keyDataV1730;
    
    
    if (rv != 0)
        return rv;    // not worthwhile continuing
    
    
    
    which = conf.Find("EnergyCal");
    times = conf.Times(which);
    for (int nn = 0; nn < times; nn++) {
        conf.Restore(which, nn);
        if(boardID!=fBoardID) continue;
        if (calChan < 0 || calChan >= v1730MaxChans) {
            cout << "==> ERROR decoding v1730Cal: Invalid Channel " << calChan << endl;
            rv = 200; break;
        }
        calibration[calBoard*16+calChan].SetPar(calThresh,calOffs,calGain);
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
    
    fPromptPeriod = max(0, fPromptPeriod);
    cServer.SetHistGroup(&hGroup);
    
    return rv;
}

int LocalWorkerPSD_Dig2::ProcessConfig(const char* fname)
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

int LocalWorkerPSD_Dig2::ProcessStart(uint32_t run)
{
    fRunNumber = run;
    fCountBlocks = 0;
    cServer.Start(filterName.str().c_str(), max(fPromptPeriod, 0));
    return 0;
}

int LocalWorkerPSD_Dig2::ProcessStop()
{
    cServer.Save(true);
    
    std::cout << "Number of events in board "
    << fBoardName << " (" <<fBoardID << ") of this run "
    << cServer.GetCounts() << std::endl;
    printStatistics();
    
    return 0;
}

int LocalWorkerPSD_Dig2::ProcessBlock(const void *input_buffer,  int input_size,  int packet_ID,
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
    uint32_t outEvtLen = (sizeof(agataKey) + sizeof(subDataPSD_t));
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
    uint16_t qlong  ;
    uint16_t qshort ;
    uint16_t fineTS ;
    float    cfd    ;
    float fineTS_2_GTS = 8/1024.;
    
    while(posBuffer < inpBuffSize){
        decodeField = swapLong(inpBuffer[posBuffer]);
        uint32_t n_words = project_range< 0,31>(decodeField).to_ulong();
        uint32_t headKey = project_range<60,63>(decodeField).to_ulong();
        if(n_words == 0x3 && headKey == 0x3){
            // This is the start run frame we do not really care about it
            posBuffer += n_words;
            printf("Start header posBuffer = %i\n",posBuffer);
            continue;
        }
        //    printf("Number of words in the aggregate = %i (size buffer = %i) at pos %i\n",
        //	   n_words,inpBuffSize,posBuffer);
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
            //if(verbose>10)std::cout << " --> Event from channel " << channel << " with TStamp = " << tstamp <<  std::endl;
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
            //      if(verbose>10)std::cout << "ware are present? " << ((wavePresent)? "yes" : "no") << std::endl;
            //std::cout << std::dec << "Energy = " << energy << " fineTS = " << fineTS << std::endl;
            //extra
            bool lastExtra = decodeField.test(63);
            uint16_t extraType=0;
            if(wavePresent){
                uint16_t ap1_Type, ap1_Mult, ap2_Type, ap2_Mult;
                bool ap1_Signed, ap2_Signed;
                uint16_t dp_Type[4];
                uint16_t triggerThres ;
                uint16_t downSamp    ;
                uint64_t deadTime;
                uint32_t savedCounter;
                while(!lastExtra){
                    decodeField = swapLong(inpBuffer[++posEvent]);
                    lastExtra = decodeField.test(63);
                    extraType = project_range<60,62>(decodeField).to_ulong();
                    //	std::cout << "extra type = " << extraType << " at posEvent = " << posEvent << std::endl;
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
                            break;
                        case 1: // dead time
                            deadTime = project_range<0,47>(decodeField).to_ulong();
                            break;
                        case 2: // counter
                            savedCounter = project_range< 0,23>(decodeField).to_ulong();
                            triggCounter = project_range<24,47>(decodeField).to_ulong();
                            break;
                        default:
                            std::cerr << __LINE__ << ": Type not implemented yet: " << extraType << std::endl;
                    }
                    //posBuff++;
                }
                decodeField = swapLong(inpBuffer[posEvent]);
                // This is the number of 64 bit words ... the trace will be twice as long;
                uint16_t nbSamples_word = project_range< 0,11>(decodeField).to_ulong();
                //	std::cout << "Samples to be read: " << nbSamples_word << std::endl;
                for(uint16_t samp = 0 ; samp<nbSamples_word ; ++samp){
                    decodeField = swapLong(inpBuffer[++posEvent]);
                    sample[0] = project_range< 0,31>(decodeField);
                    sample[1] = project_range<32,63>(decodeField);
                    if(ap1_Type == 0x0 && doWaveAnalysis){
                        //Should take what was done for GALTRACE in the LocalFilter of GALILEO
                        // REMEMBER TO DO IT ALAIN!
                    }
                }
            }
            //Filling the output event, here for the BU to work without modification we modify
            // channel and board to match the 16 channel structure.
            if(tsOffset.find(channel)!=tsOffset.end())
                offsetTS = tsOffset[channel];
            else
                offsetTS = 0;
            boardID = fBoardID + channel/16;
            //std::cout << "board = " << fBoardID << " boardID = " << boardID << " (channel = " << channel << ")\n)";
            chanID  = channel%16;
            outEvt->theKey.size     = outEvtLen              ;
            tstamp_10ns            += offsetTS + fGlobalTSOffset;
            outEvt->theKey.key      = outDataKey             ;                      // default is data:ranc1
            outEvt->theKey.evnum    = (boardID << 8 | chanID);
            outEvt->theKey.evnum   |= flags;
            outEvt->theKey.ts_0     = uint32_t(tstamp_10ns & 0xFFFFFFFF);         // tstLow
            outEvt->theKey.ts_1     = uint32_t((tstamp_10ns >> 32) & 0x0000FFFF); // tstHigh
            if(slfTrig){
                outEvt->theData.qlong   = qlong;
                outEvt->theData.qshort  = qshort;
                outEvt->theData.cfd    += static_cast<uint16_t>(std::floor(cfd));
            }else{
                outEvt->theData.qlong   = 0;
                outEvt->theData.qshort  = 0;
                outEvt->theData.cfd     = 0;
            }
            if(HCHECK(SpecAmpl) && slfTrig){
                SpecAmpl->IncrQ(0,channel,qshort/2,0); // energy is between 0 and 65536 so we divide by 4
                SpecAmpl->IncrQ(1,channel,qlong/2,0); // energy is between 0 and 65536 so we divide by 4
            }
            ++nbEvents[channel];
            countEvents++;
            //if(outEvt->theData.energy<2)continue;
            if(slfTrig)
                ++nbGoodEvents[channel];
            lastTS[channel] = tstamp_10ns;
            if (fLastTS < tstamp_10ns) fLastTS = tstamp_10ns;
            int outEvtSize = SetOutput(outBuffer,output_size-outByteSize,outEvt);
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
    //  std::cout << "End of the buffer" << std::endl;
    // Aggregate buffer has a header containing different information
    
    
    //  uint32_t coupleAggregateSize = 0 ;
    
    
    
    
    
    //  uint32_t fineTS=0;
    //  uint32_t extras2 = 0 ;
    //  std::bitset<16> extras  = 0;
    //  int      offsetTS = 0 ;
    //  uint32_t flags = 0;
    //  bool pur = false;
    //  bool satu = false;
    //  bool lost = false;
    //  bool trap_sat = false;
    //  uint32_t chanNum = 0;
    //  float cfd=0.;
    //  // here starts the real decoding of the data for the channels
    //  for(uint8_t cpl = 0 ; cpl < channelMask.count() ; cpl++){
    //    coupleAggregateSize = inpBuffer[startingPos]&0x3FFFFF;
    //    dataForm.SetDataFormat(inpBuffer[startingPos+1]);
    //    //    dataForm.printDataFormat();
    //    const uint16_t evtSize = dataForm.evtSize();
    //    if(evtSize==0){
    //      dataForm.printDataFormat();
    //      break;
    //    }
    //    for(uint16_t evt = 0 ; evt < (coupleAggregateSize-2)/evtSize;evt++){
    //	pur     = static_cast<bool>(((inpBuffer[startingPos+evtSize-1+evtSize*evt+2])&0x8000)>>15);
    //	cfd = 0.;
    //	tstamp  = static_cast<uint64_t>((inpBuffer[startingPos+2+evtSize*evt])&0x7FFFFFFF);
    //	if(dataForm.enaExtras()){
    //	  extras  = static_cast<uint16_t>(((inpBuffer[startingPos+evtSize-1+evtSize*evt+2])&0x1F0000)>>16);
    //	  satu     = extras[ 4];
    //	  lost     = extras[ 5];
    //	  pur      = extras[ 9];
    //	  trap_sat = extras[10];
    //	  extras2 = inpBuffer[startingPos+evtSize-2+evtSize*evt+2];
    //	  switch(dataForm.confExtras()) {
    //	  case 0: // Extended Time Stamp [31:16] ; baseline*4 [15:0]
    //	    tstamp = (static_cast<uint64_t>(extras2 & 0xFFFF0000)<<15) | (uint64_t)tstamp;
    //	    break;
    //	  case 1: // Extended Time stamp [31:16] ; flags [15:0]
    //	    tstamp = (static_cast<uint64_t>(extras2 & 0xFFFF0000)<<15) | (uint64_t)tstamp;
    //	    break;
    //	  case 2: // Extended Time stamp [31:16] ; Flags [15:10] ; Fine Time Stamp [9:0]
    //	    fineTS = static_cast<uint16_t>(extras2&0x3FF);
    //	    outEvt->theData.cfd = fNsPerTStamp/1.024* fineTS; // to have it in ps units
    //	    tstamp = (static_cast<uint64_t>(extras2 & 0xFFFF0000)<<15) | (uint64_t)tstamp;
    //	    break;
    //	  case 4: // Lost Trigger Counter [31:16] ; Total Trigger [15:0]
    //	    break;
    //	  case 5: // Positive zero crossing [31:16] ; Negative zero crossing [15:0]
    //	    break;
    //	  case 7: // Debug fixed value;
    //	    break;
    //	  default:
    //	    break;
    //	  }
    //	}
    //	//	statsData.time[cpl] = tstamp*2/pow( 10, 9 );   // must be in ns
    //	chanNum    = static_cast<uint32_t>(((inpBuffer[startingPos+2+evtSize*evt]&0x80000000)>>31)
    //					   +couples[cpl]*2);
    //
    //	if(tsOffset.find(chanNum)!=tsOffset.end())
    //	  offsetTS = tsOffset[chanNum];
    //	else
    //	  offsetTS = 0;
    //	outEvt->theKey.size     = outEvtLen              ;
    //	outEvt->theKey.key      = outDataKey             ;                      // default is data:ranc1
    //	tstamp_10ns             = tstamp * fNsPerTStamp / 10 ;
    //	cfd                    += (tstamp*fNsPerTStamp - tstamp_10ns*10)*1000; // in ps no to loose precision when using uint16_t
    //	tstamp_10ns            += offsetTS + fGlobalTSOffset;
    //	//	std::cout << __func__ << ":" << fGlobalTSOffset << std::endl;
    //	outEvt->theData.cfd     += static_cast<uint16_t>(std::floor(cfd));
    //	if(outEvt->theData.cfd>10000){
    //	  outEvt->theData.cfd -= 10000 ;
    //	  tstamp_10ns         +=     1 ;
    //	}
    //
    //	outEvt->theKey.evnum    = chanNum + fDomOutOffset;
    //	outEvt->theKey.ts_0     = uint32_t(tstamp_10ns & 0xFFFFFFFF);         // tstLow
    //	outEvt->theKey.ts_1     = uint32_t((tstamp_10ns >> 32) & 0x0000FFFF); // tstHigh
    //	outEvt->theData.energy  = static_cast<uint16_t>((inpBuffer[startingPos+evtSize-1+evtSize*evt+2])&0x7FFF);
    //
    //
    //	// Adding PUR to the flag
    //	flags  = extras[0]<<19 | satu << 18 | trap_sat << 17 | pur << 16;
    //	if(ratesMonitor.values.find(chanNum) == ratesMonitor.values.end())
    //	  ratesMonitor.values[chanNum].Init();
    //	ratesMonitor.values[chanNum].time = tstamp*fNsPerTStamp;
    //	++ratesMonitor.values[chanNum].totalEvents ;
    //	if( pur )  ++ratesMonitor.values[chanNum].pileEvents ;
    //	if( satu ) ++ratesMonitor.values[chanNum].satuEvents ;
    //	if( lost ) ratesMonitor.values[chanNum].lostEvents += 1024;
    //	// N is set from bits[17:16] of register 0x1nA0 (default value 1024)
    //	// Adding flags to the evt num
    //	outEvt->theKey.evnum    = (board << 8 | chanNum);
    //	outEvt->theKey.evnum   |= flags        ;
    //	outEvt->theKey.evnum   |= boardFailFlag;
    //	++nbEvents[chanNum];
    //	countEvents++;
    //	if(outEvt->theData.energy<2)continue;
    //	++nbGoodEvents[chanNum];
    //	lastTS[chanNum] = tstamp_10ns;
    //	if (fLastTS < tstamp_10ns) fLastTS = tstamp_10ns;
    //	int outEvtSize = SetOutput(outBuffer,output_size-outByteSize,outEvt);
    //	if(outEvtSize<0){ //output buffer full
    //	  std::cout << "Returning to LF" << std::endl;
    //	  return outEvtSize;
    //	}
    //	outByteSize += outEvtSize ;
    //	outBuffer   += outEvtSize / sizeof(uint32_t) ;
    //
    //      }
    //
    //      startingPos+=coupleAggregateSize;
    //
    //
    //  }
    //
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

int LocalWorkerPSD_Dig2::SetOutput(uint32_t *outBuff, int availableBytes, const caenEventPHA_t * pE)
{
    // agataKey + numSubs * subData_t
    int outEvLenByte = sizeof(agataKey) + sizeof(subDataPSD_t); // max needed size
    
    
    agataKey* psaKey = (agataKey *)outBuff;
    outEvLenByte = sizeof(agataKey);                      // recalculate effective value
    
    // hand-made agata key
    psaKey->size  = pE->theKey.size;
    psaKey->key   = pE->theKey.key;
    psaKey->ts_0  = pE->theKey.ts_0;
    psaKey->ts_1  = pE->theKey.ts_1;
    psaKey->evnum = pE->theKey.evnum;
    if ((sizeof(agataKey) + sizeof(subDataPSD_t)) > availableBytes) {
        std::cout << "LocalWorkerPSD_Dig2::SetOutput ==> buffer full (needed/available): " << outEvLenByte << " " << availableBytes << std::endl;
        printf("Board = %i Channel %i, ts_0 = 0x%08X ts_1 = 0x%08X\n",
               pE->theKey.key&0xFF00 >> 8,
               pE->theKey.key&0x00FF,
               pE->theKey.ts_0,
               pE->theKey.ts_1);
        return availableBytes - outEvLenByte;
    }
    
    outBuff += sizeof(agataKey) / sizeof(uint32_t);       // immediatly after agataKey
    
    // the full size as defined in subData_t
    subDataPSD_t pS = pE->theData;
    memcpy(outBuff, &pS, sizeof(subDataPSD_t));
    outEvLenByte += sizeof(subDataPSD_t);                  // recalculate effective value
    outBuff += sizeof(subDataPSD_t) / sizeof(uint32_t);
    //  if(psaKey->evnum>>31 == 1)
    //     printf("PHA SIZE IDLE = %i\n",outEvLenByte);
    psaKey->size = outEvLenByte;
    return outEvLenByte;
}

int LocalWorkerPSD_Dig2::GenerateIdles(uint64_t largest_ts,
                                       void *output_buffer, int output_size, int &used_output)
{
    // idles for the dig2 are directly generated on the board
    // so there is nothing to do here
    return 0;
}
