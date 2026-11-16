#include "LocalWorker.h"

#include <cstring>

using namespace std;

LocalWorker::LocalWorker(CycleServer& _cServer, int& _tcpPortno, int boardID)
  : cServer(_cServer), tcpPortno(_tcpPortno)
{
  gMotherClass   = "LocalFilter";  // traditional name used by prompts
  gDaughterClass = "LocalWorker";
  fBoardID    = boardID;
  fPackBlocks = 0;
  fOutMagicData = 0;
  fRunNumber = 1;
  fCountBlocks = 0;
  fPromptPeriod = 1;
  fDomFirst = 0;
  fDomOutOffset = 0; 
  filterName.str("");
  fVerbose = false;
}


void LocalWorker::printStatistics()
{
  printf("Statistics for Board %i (All Events, Good Events, IdleEvents): \n",fBoardID);
  for(auto it=nbEvents.begin();it!=nbEvents.end();++it)
    printf("\t Ch %2i -- %10i -- %10i -- %10i\n",it->first,
	   it->second,nbGoodEvents[it->first],nbIdleEvents[it->first]);

  nbEvents.clear();
  nbGoodEvents.clear();
  nbIdleEvents.clear();
}

void LocalWorker::SetTimeStampOffset(uint32_t tsHi, uint32_t tsLo){
  fGlobalTSOffset = (uint64_t(tsHi)<<32)|(uint64_t(tsLo));
}
