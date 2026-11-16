#include "LocalFilter.h"

#include "LocalWorker.h"
#include "LocalWorkerSTD.h"
#include "LocalWorkerPHA.h"
#include "LocalWorkerPHA_Dig2.h"
#include "LocalWorkerPSD.h"
#include "LocalWorkerNEDA.h"
#include "rubuilder/utils/ConfReader.h"


using namespace std;

#ifndef GAL_OFFLINE
LocalFilter::LocalFilter(xdaq::ApplicationStub *s) :
  SuperApplication::SuperApplication(s),
  logger_( getApplicationLogger() ),
#else
  LocalFilter::LocalFilter() :
#endif
  gMotherClass("LocalFilter")
{
  workers.clear();
  fPackBlocks = 0;
  fPackNumber = 0;
  fPackedSize = 0;
  fPackedMaxSize = 0;
  fPacked = nullptr;
  //syncFrame = new syncData_t;
  tcpPortno = 16161;
  nbBuffer=0;
  redis_server_ = nullptr;  
}

LocalFilter::~LocalFilter()
{
  if (workers.size()>0)
    workers.clear();
  if (fPacked)
    delete[] fPacked;
  //delete[] syncFrame;
}

void LocalFilter::process_config(const char* fname, int *error_code)
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
  srand((unsigned) time(NULL));
  bool        AnaRiseTime;
  int         boardID, boardCH, boardTS, boardSampling;
  int         channelID, tsOffs;
  int         graphPort;
  int         rdbPort;
  std::string boardFW;
  std::string boardName;
  std::string confFile;
  std::string fOutPath;
  std::string graphServ;
  std::string histPrefix;
  std::string tmpOutData;
  std::string rdbHost;

  rubuilder::utils::ConfReader conf;


  conf.Add("Board",      "Board description: BoardID, Name, Firmware, number of channel, channel offset for the event builder, nsPerTimestamp, ns per sample",
	   &boardID, &boardName, &boardFW, &boardCH, &boardTS, &boardSampling);
  conf.Add("SaveDataDir",      "where to write data and spectra"
	   , &fOutPath);
  conf.Add("SpecPrefix",       "prefix to names of spectra",
	   &boardID, &histPrefix);
  conf.Add("TSOffset",   "Timestamp offset for board X chanel Y",
	   &boardID, &channelID, &tsOffs);
  conf.Add("GraphiteServer", "Graphite Server to send the rates of the boards: Hostname and port",
	   &graphServ, &graphPort);
  conf.Add("OutDataKey", "Output ADF frame key (0xFA0201A2=SPIDER, 0xFA0201A3=DANTE, ...",
	   &boardID, &tmpOutData);
  conf.Add("ConfFilePSA",      "Configuration file for the PSA",
	   &boardID, &confFile);
  conf.Add("AnaRiseTime",      "Enable analysis of the short traces for the particle discrimination",
	   &boardID, &AnaRiseTime);
  conf.Add( "RedisServer"    ,"REDIS Server host and port",
	    &rdbHost, &rdbPort);
  int rv = conf.Read(gMotherClass, fConfigFile);
  if (rv != 0)
    return;  // error

  if(!conf.Seen("Board")){
    std::cerr << "Fatal Error: " << fConfigFile << " MUST contain at least one board description" << std::endl;
    *error_code = 102; // Fatal error because the keyword "Board" MUST be present
  }
  int which, times;
  which = conf.Find("Board");
  times = conf.Times(which);

   if( conf.Seen("RedisServer") ) {
     struct timeval timeout = {1,500000};
     redis_server_ = redisConnectWithTimeout(rdbHost.c_str(),
					     rdbPort,
					     timeout);
     if ( redis_server_ == nullptr || redis_server_->err ){
       if ( redis_server_ ){
	 std::cerr << "Connection error on the REDIS database: " 
		   << redis_server_->errstr << std::endl;
	 redisFree(redis_server_);
       }else{
	 std::cerr << "Connection error: can't allocate REDIS cotext" << std::endl;
       }
     }
   }

  for(int nn = 0 ; nn < times ; ++nn){
    conf.Restore(which, nn);
    tcpPortno = tcpPortno+boardID;
    std::cout << "board " << boardID 
	      << " is using the FW " << boardFW 
	      << " Filter control on tcp port: " << tcpPortno << std::endl; 
    if(boardFW=="DPP_PSD"){
      workers[boardID] = new LocalWorkerPSD(cServer[boardID],tcpPortno,boardID);
    }else if(boardFW=="DPP_PHA"){
      workers[boardID] = new LocalWorkerPHA(cServer[boardID],tcpPortno,boardID);
    }else if(boardFW=="STD"){
      workers[boardID] = new LocalWorkerSTD(cServer[boardID],tcpPortno,boardID);
    }else if(boardFW=="NEDA"){
      workers[boardID] = new LocalWorkerNEDA(cServer[boardID],tcpPortno,boardID);
    }else if(boardFW=="DPP_PHA_DIG2"){
      workers[boardID] = new LocalWorkerPHA_Dig2(cServer[boardID],tcpPortno,boardID);
    }
    cServer[boardID].TcpLaunch(tcpPortno);   // with the port number from worker::GetParameters
  }
  for(auto it = workers.begin(); it != workers.end(); ++it){
    *error_code != it->second->ProcessConfig(fname);
  }
  //fPackBlocks = worker->GetPackBlocks();
  fPackNumber = 0;
  fPackedSize = 0;
  firstBlock=true;

}

void LocalFilter::ShowKeywords()
{
  workers[0] = new LocalWorkerSTD(cServer[0], tcpPortno,16,16);
  workers[0]->GetParameters(true);
  delete workers[0];
  workers[0] = nullptr;

  workers[0] = new LocalWorkerPHA(cServer[0], tcpPortno,16,16);
  workers[0]->GetParameters(true);
  delete workers[0];
  workers[0] = nullptr;
  
  workers[0] = new LocalWorkerPSD(cServer[0], tcpPortno,16,16);
  workers[0]->GetParameters(true);
  delete workers[0];
  workers[0] = nullptr;

  workers[0] = new LocalWorkerNEDA(cServer[0], tcpPortno,16,16);
  workers[0]->GetParameters(true);
  delete workers[0];
  workers[0] = nullptr;

  workers[0] = new LocalWorkerPHA_Dig2(cServer[0], tcpPortno,16,16);
  workers[0]->GetParameters(true);
  delete workers[0];
  workers[0] = nullptr;

  return;
}

bool LocalFilter::PackInputBlocks(void *input_buffer, int input_size)
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

void LocalFilter::process_start(uint32_t run, int *error_code)
{
  for(auto it = workers.begin() ; it != workers.end() ; ++it){
    *error_code |= it->second->ProcessStart(run);
    
  }
}

void LocalFilter::process_stop(int *error_code)
{
  for(auto it = workers.begin() ; it != workers.end() ; ++it){
    *error_code |= it->second->ProcessStop();
    delete it->second;
    it->second = nullptr;
  }
}

void LocalFilter::process_block (
				 void *input_buffer,  int input_size,  int packet_ID,
				 void *output_buffer, int output_size, int *used_size_of_output_buffer,
				 int  *error_code )
{
//  LOG4CPLUS_INFO( logger_,
//		  "In LocalFilter::process_block with input size = " << input_size << std::endl);
  if(firstBlock){
    LOG4CPLUS_INFO( logger_,
                   "In LocalFilter::process_block with input size = " << input_size << std::endl);
    FILE *fileI; fileI = fopen("/onlineANCILLARY/xDaqCurrentRun/Data/caen_digitizer/firstBlockLF.caendat","wb");
    fwrite(input_buffer,1,input_size,fileI);
    fclose(fileI);
    firstBlock=false;
  }
  uint32_t* inpBuff = (uint32_t*)input_buffer;
  int errorCode = 0;
  int usedSize = 0;
  int readSize = 0;
  int pos=0;
  int boardId = 0 ;
  unsigned char *splitBlock = new unsigned char[input_size];
  uint64_t localTS = 0;
  largestTS =0;
  while( readSize!=input_size ){   
    if((inpBuff[pos]>>24)!=0xa0 ){ // This key is only for Dig1 we have to add a key for dig2
      printf("Warning key is not a real board aggregate: 0x%08X! I have read %d over %d bytes\n",
	     inpBuff[pos],readSize,input_size);
      while((inpBuff[pos]>>24)!=0xa0){
	printf("- skipping word %08X\n",inpBuff[pos]);
	pos++;
	readSize += static_cast<int>(sizeof(uint32_t));
	if(readSize==input_size) {std::cout << "breaking because end of buffer" << std::endl;break;}
      }
    }
    int nbInt = inpBuff[pos]&0x07FFFFFF;
    if(nbInt*static_cast<int>(sizeof(uint32_t)) > input_size){
      LOG4CPLUS_INFO( logger_,"Warning buffer size not sufficient to copy the full event!\n"
		      << " - Requested size = " << nbInt*sizeof(uint32_t) << "\n"
		      << " - Available size = " <<2048*2048 <<"\n"
		      << " - inpBuff[pos]= " << std::hex << inpBuff[pos] << std::dec <<"\n"
		      << " - Number of lword to be read " << nbInt <<"\n"
		      << " - size to read: " << nbInt*4 <<"  B " <<"\n"
		      << " - readSize = " << readSize << " " << "input_size = "<< input_size  <<"\n"
		      << " - inpBuf[1] = " << inpBuff[pos+1] << "\n");
      break;
    }
    memcpy(splitBlock,
	   ((char*)input_buffer)+readSize,
	   nbInt*sizeof(uint32_t));
    readSize += nbInt*static_cast<int>(sizeof(uint32_t));

    if(inpBuff[pos+1]==0xFA0201A0){ // BUFFER/FILE HEADER   
      // Since we have the header we get the TS offset of the AGAVA if it is present
      LOG4CPLUS_INFO( logger_,
		      "We have the header with the time stamp of the agava "
		      << "setting the time stamp offset for " << workers.size() << " boards.\n");

      for(auto it = workers.begin(); it!=workers.end(); ++it){
	if(inpBuff[pos+3]!=0xFAFAFAFA)
	  it->second->SetTimeStampOffset(inpBuff[pos+3], // tsOffset_hi
					 inpBuff[pos+4]); // tsOffset_lo
	else
	  it->second->SetTimeStampOffset(0, // tsOffset_hi
					 1); // tsOffset_lo
      }
      LOG4CPLUS_INFO( logger_, 
		      "GTS Timestamp Offset set to 0x: " 
		      << std::hex << inpBuff[pos+3] << inpBuff[pos+4] << std::dec);
      pos+=nbInt;
    }else{
      boardId = (inpBuff[pos+1]&0xF8000000)>>27;
      pos += nbInt;
     
      if(workers.find(boardId)==workers.end()){
	std::cerr << "BoardId = " << boardId << " does not have a LocalFilter ... " << std::endl;
	continue;
      }
      if(workers[boardId]->GetTSOffset()==0){
	redisReply *reply; 
	reply = (redisReply*)redisCommand(redis_server_,
					  "GET ancTS");

	uint64_t tsOff = std::strtoull(reply->str,NULL,16);
	freeReplyObject(reply);
	if(tsOff==0xFAFAFAFACACACACA) tsOff=1;
	LOG4CPLUS_INFO( logger_, 
			"GTS Timestamp Offset set to 0x:" << std::hex 
			<< tsOff << "using redis!" << std::dec);
	  
	workers[boardId]->SetTimeStampOffset((uint32_t)(tsOff>>32),         //tsOffset_hi
					     (uint32_t)(tsOff&0xFFFFFFFF)); //tsOffset_lo
      }

      errorCode = workers[boardId]->ProcessBlock((void*)splitBlock,
						 nbInt*sizeof(uint32_t),  
						 packet_ID,
						 (char*)output_buffer+usedSize,
						 output_size,
						 usedSize);

      localTS = workers[boardId]->GetLastTStamp();
      if(largestTS<localTS)largestTS = localTS;
    }
    //
  }
  
  for(auto it  = workers.begin(); it!=workers.end(); ++it){
    it->second->GenerateIdles(largestTS,
			      (char*)output_buffer+usedSize,
			      output_size,
			      usedSize);
  }
  
  delete [] splitBlock;
  
  if(usedSize > output_size)
    printf("Error! Used size (%i) is larger than output_size (%i)\n",usedSize,output_size);
  
  *used_size_of_output_buffer = usedSize;
  *error_code = errorCode;
  //  std::cout << "End of " << __func__ << std::endl;
}

#ifndef GAL_OFFLINE
/*
 * Provides the factory method for the instantiation of RU applications.
 * XDAQ_INSTANTIATOR_IMPL() is a macro that implements the actual entry point for creating the application object at run-time-
 *-when the application is loaded into a running XDAQ process.
 * The macro takes a single parameter, the C++ class name of the XDAQ application.
 */
XDAQ_INSTANTIATOR_IMPL(LocalFilter)
#endif

