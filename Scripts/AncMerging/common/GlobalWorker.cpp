
#include "GlobalWorker.h"
#include "rubuilder/utils/ConfReader.h"
#include "rubuilder/utils/Misc.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <climits>

const int   specSizeTstD  = 10000;        // in microseconds
const int   specSizeEner  = 16384;
const int   specSizeTime  = 1000;         // for delta T spectra
const int   specTimeGain  = 10;           // tsamp --> ns
const int   matSizeEner   = 1000;
const int   matSizeTime   = 1000;
const int   specOffset    = 0;
const int   matPackChan   = 5;

const int   domainTDtot   = 62;           // where to put event-tstdiff

//// this part to be removed if not using a stright-line fit to fit T0 of traces
//const int   trigChansCC  = 5;     // normally 4
//const float trigFraction = .05f;  // start fitting stright line at this fraction of max amplitude (but not lower than trigLevel1)
//const float trigLevel1   = 20.f;  // 25 keV ~5 sigma of noise ==> should be calculated as a running average of the second threshold
////const float trigLevel1   = 10.f;  // lowered to 10 keV because of the gaussian smoothing performend before fitting the stright line

const int   tGain      = 1/*00*/;         // to increase tScale and the length of time spectra in a consistent way
const float tScale     = tGain*10.f;      // tScale=10 ==> 1ns/ch
const int   specLenT0  = tGain*1000;
const int   specLenT1  = tGain*specSizeTime;
const int   specLenT2  = 400;

const float GE_ESUM_MAX = 1000000;      // 1 GeV --> Spec_GeES generated if (fGeEsumMax < GE_ESUM_MAX)

using namespace std;

GlobalWorker::GlobalWorker(CycleServer& _cServer, int& _tcpPortno, int numdoms, int numsubs)
  : cServer(_cServer), tcpPortno(_tcpPortno)
{
  gMotherClass = "GlobalWorker";
  gDaughterClass = "GlobalWorker";
  
  fOutMagicData = 0;
  fRunNumber = 1;
  fCountBlocks = 0;
  fPromptPeriod = 1;
  fDomFirst = 0;
  fDomOutOffset = 0;
  fVerbose = false;
  
  numDoms  = 0;
  numSubs  = 0;
  
  fExcludeProcessing = false;
  
  fPackBlocks = 0;
  fNumThreads = 0;
  
  theDoms = nullptr;
  
#ifdef USE_SPEC_BASE
  memset(fAverBase, 0, sizeof(fAverBase)); // running average of baseline
  memset(pAverBase, 0, sizeof(pAverBase)); // Append spectra of the baseline running average
#endif
  
  evtMagicKeyInp = keyEventMerged;   // merged-frame key
  evtMagicKeyOut = keyEventMerged;   // merged-frame key
  simpleFrame = false;
  
  fNumDoms = 1;
  fNumSubs = 1;
  fNumSamp = 100;
  
  InitConfig(numdoms, numsubs);
  
  fPromptPeriod = 1;
  fCountBlocks  = 0;
  
  fGeEnerMin = -FLT_MAX;  // keV
  fGeEnerMax =  FLT_MAX;
  fGeEnergyGate = false;
  
  tacGateGeAC[0] = 1;     // time window of Ge-AC coincidence used for Compton suppression
  tacGateGeAC[1] = 0;     // [0]>[1] means no Compton suppression
  fGeSuppression = false;
  
  fGeCalibration = false;
  
  fTacMin = INT_MIN;      // ns
  fTacMax = INT_MAX;
  
  fEnergyGain = 1.f;
  fTacGain    = 1.f;
  
  fGeEsum = 0;
  fGeEsumMin = 0;       // keV
  fGeEsumMax = 100000000000.0f; 
  fDomsMatrGGii = 0;    // number of domains for MatrGGii
  
  fHistPfx = "Hist";
  
  nSpectra = 0;
  specZero = 0;
  InitSpectra(0, 0);    // nullptr all spectra
  
  fSizeMatrGG = 0;      // 0 ==> nomatrix
  fGainMatrGG = 1;
  
  evtTstamp    = 0;
  evtTstampOld = 0;
  memset(minFoldDets, 0, sizeof(minFoldDets));     // minimum requested
  minNumTypes = 1;
  theTriggers.clear();
  fEventTrigger.init();
  //fEventTrigger = new theTrigger_t();
  //fEventTrigger->init();
  
  fRunNumber = 100; // ??
  
  for (int ii = 0; ii < kDetTypes; ii++) {
    datPartial[ii].reset(false);
    nbInpPars[ii] = maxSubPars; //Default set to all parameters are passed
  }
  
  memset(inpPartial, false, sizeof(inpPartial));
  memset(outPartial, false, sizeof(outPartial));
  
  inpHasVME = false;
  outHasVME = false;
  writeFormatFrame = true;
  
  std::fill_n(arrayMap, 256, -1);
  //memset(arrayMap, -1, sizeof(arrayMap));
  
  // defaults to GALILEO (+EUCLIDES)
  std::fill_n(arrayMap,    32, kDetGe);
  //arrayMap[63] = ???
  //std::fill_n(arrayMap+64, 64, kDetSi);

  //memset(arrayMap +   0, kDetGe, 32);
  //memset(arrayMap +  64, kDetSi, 64);
  realMaxType = kDetGe;
  realMaxType = max(realMaxType, 1); // VME is seen as germanium #63 ??
  fVerbose = false;
}

// todo: add the delete []
GlobalWorker::~GlobalWorker()
{
#ifdef USE_SPEC_BASE
  for (int ii = 0; ii < 32; ii++) {
    if (pAverBase[ii]) {
      delete pAverBase[ii];
      pAverBase[ii] = nullptr;
    }
  }
#endif
  cServer.Finish();
}

int GlobalWorker::ProcessBlock(const void *input_buffer,  int input_size,  int packet_ID,
                               void *output_buffer, int output_size, int & used_output)
{
  int error = MW_ProcessBlock(input_buffer,  input_size,  packet_ID,
                              output_buffer, output_size, used_output);
  if (error)
    return error;
  
  cServer.Exec(MW_Tstamp(), MW_ValidEvents());
  cServer.Prompt(0, MW_ValidEvents(), used_output);
  
  if (cServer.CheckCommand()) {
    if (cServer.CheckCommand(CycleServer::ECommand::kExit))
      return -1000;
    if (cServer.CheckCommand(CycleServer::ECommand::kConfigure))
      GetParameters(false, true);
  }
  
  return 0;
  
}

int GlobalWorker::ProcessScan(userData_t *pData)
{
  auto inpBuffer  = pData->GetInpBuffer();
  auto input_size = pData->GetInpSize();
  auto hQueue     = pData->GetQueue();    // MultiHist increment queue
  
  int inpByteSize = 0;
  int frameBytes  = 0;
  
  // scan all events
  int countEvents = 0;
  for (; inpByteSize < input_size; inpByteSize += frameBytes) {
    uint32_t* ptrEvent = inpBuffer + inpByteSize / sizeof(uint32_t);
    
    agataKey* frameKey = (agataKey *)ptrEvent;
    frameBytes = frameKey->size;
    
    if (frameKey->key != evtMagicKeyInp)           // treat only "Merger" (0xCA010103) frames
      continue;
    
    // look into the event (same structure as SetInput(..) )
    
    int restBytes = frameBytes - ( simpleFrame ? 0 : sizeof(agataKey) );
    int restIndex = simpleFrame ? 0 : sizeof(agataKey) / sizeof(uint32_t);
    int domBytes  = 0;
    
    bool updateTstamp = true;
    int  fragments = 0;
    for(; restBytes >= sizeof(agataKey);  restBytes -= domBytes,  restIndex += domBytes / sizeof(uint32_t)) {
      
      agataKey* domKey = (agataKey *)(ptrEvent + restIndex);
      domBytes = domKey->size;
      uint32_t domain = 0;
      int detType = 0;
      
      if (domKey->key == keyDataVME) {          // VME data (0xFA0201A0)
        //handled first due to fixed domDataVME=63
        detType = kDetVME;
        domain = domDataVME;
      }
      else {                                    // ggp data
        // Totally ignore evBuilder subkeys because all info is in domID
        // Non-default detectors (NWdigital, Spider and Trace usually enter as SI)
        if ((0xFF000000 & domKey->key) == keyEventMask) {
          domBytes = sizeof(agataKey);
          continue;
        }

        if ( domKey->key != keyDataEuclides        // 0xFA0101A1
	     && domKey->key != keyDataLaBr         // 0xFA0101A3
	     && domKey->key != keyDataSpider       // 0xFA0101A4
	     && domKey->key != keyDataSauron       // 0xFA0102A5
	     ){      // 0xFA0102A6
          continue;
        }

        domain = domKey->GetDomain();            // idle bit is NOT masked off, so idles are off range
        if (domain >= numDoms)
          continue;
        detType = DetectorType(domain);
        if (detType < 0 || detType > realMaxType)
          continue;
      }
      
      if (updateTstamp) {   // only once and only if the event contains real data
        evtTstampOld = evtTstamp;
        evtTstamp = frameKey->GetTstamp();
        updateTstamp = false;
      }
      
      if (HCHECKE(Spec_EvTS)) {
        int detts = int(domKey->GetTstamp() - evtTstamp);
        Spec_EvTS->IncrQ(detType, detts, hQueue);
      }
      domData_t* pD = theDoms + domain;                     // data is copied into the proper place ???
      pD->domain = domain;
      pD->UpdateCounts();
      pD->UpdateTstamp(domKey->GetTstamp());
      if (HCHECKE(SpecTstD)) {                              // done here because NW analysis not inplemented
        int tstdiff = int(pD->ElapsedTstamp() / 100);       // microseconds
        int specID = domain - specZero;
        SpecTstD->IncrQ(specID, tstdiff, hQueue);
      }
      fragments++;
    }   //  for(; restBytes >= sizeof(agataKey); ...
    
    if (fragments) {
      evtTstampOld = evtTstamp;
      evtTstamp = frameKey->GetTstamp();
      if (HCHECKE(SpecTstD) /* && evtTstampOld*/) {           // global rate (of events with actual data)
        int tstdiff = int(evtTstamp - evtTstampOld);
        SpecTstD->IncrQ(domainTDtot, tstdiff / 100, hQueue);  // discarded by ->Incr(...) if value is off-range
      }
    }
    
    countEvents++;
  } // for (; inpByteSize < input_size; ...
  
  return countEvents;
}

int GlobalWorker::ProcessData(userData_t *pData)
{
  auto inpBuffer   = pData->GetInpBuffer();
  auto outBuffer   = pData->GetOutBuffer();
  auto ptrEvent    = pData->GetPtrEvent();
  auto input_size  = pData->GetInpSize();
  auto output_size = pData->GetOutSize();
  auto hQueue      = pData->GetQueue();    // MultiHist increment queue
  
  int countEvents = 0;
  int validEvents = 0;
  int inpByteSize = 0;
  int outByteSize = 0;
  int errorCode   = 0;
  uint64_t tstEvt = 0;
  
  if (FormatFrameKey(inpBuffer)) {
    int inpEvLenByte = FormatFrameGet(inpBuffer);
    inpByteSize += inpEvLenByte;
    inpBuffer   += inpEvLenByte / sizeof(uint32_t);
  }
  
  if (writeFormatFrame) {
    int outEvLenByte = FormatFramePut(outBuffer, output_size);
    outByteSize += outEvLenByte;
    outBuffer   += outEvLenByte / sizeof(uint32_t);
    writeFormatFrame = false;
  }
  
  while(inpByteSize < input_size) {
    
    ////// 0-input
    int inpEvLenByte = SetInput(ptrEvent, inpBuffer); // missing test
    if(inpEvLenByte < 0) {
      errorCode = inpEvLenByte;                       // format error
      break;
    }
    
    countEvents++;
    
    ///// 1-Filter event
    FilterDetsGe(ptrEvent, hQueue);
    
    if (Spec_GeES && ptrEvent->validEvent) {
      FilterGeEsum(ptrEvent, hQueue);
    }
    
    ///// 2-validate and analyze event
    bool doOutput =
      ptrEvent->validEvent
      && ValidateEventMult(ptrEvent)                  // multiplicity stuff
      && AnalyzeEvent (ptrEvent, hQueue);             // so far, this always returns true
    ptrEvent->validEvent = doOutput;
    
    ////// 3-output
    if(doOutput) {
      validEvents++;
      int outEvLenByte = SetOutput(ptrEvent, outBuffer, output_size-outByteSize, inpBuffer, inpEvLenByte);
      if(outEvLenByte <= 0) {
        errorCode = outEvLenByte;                     // output buffer full
        break;
      }
      outByteSize += outEvLenByte;
      outBuffer   += outEvLenByte/sizeof(uint32_t);
    }
    
    inpByteSize += inpEvLenByte;
    inpBuffer   += inpEvLenByte/sizeof(uint32_t);
    tstEvt = ptrEvent->tstamp;
  }
  
  pData->SetResults(outByteSize, countEvents, validEvents, tstEvt);
  return 0;
}

void GlobalWorker::FilterDetsGe(galEvent_t * pE, unsigned char hQueue)
{
  if (fGeCalibration) {
    for (int nn = 0; nn < pE->numDetsGGP; nn++) {
      theDets_t* pD = pE->ggpDets + nn;
      if (pD->detType != kDetGe || !pD->detValid)
        continue;
      int        id   = pD->domain;
      subData_t *pS   = pD->theSubs;
      float oldEnergy = pS->eEnergy;
      calData_t *pC   = theDoms[id].calibs;
      float newEnergy = pC->Ecal(oldEnergy); // RECALIBRATION !!
      pS->eEnergy = newEnergy;
    }
  }
  
  if (fGeEnergyGate) {
    for (int nn = 0; nn < pE->numDetsGGP; nn++) {
      theDets_t* pD = pE->ggpDets + nn;
      if (pD->detType != kDetGe || !pD->detValid)
        continue;
      //int        id = pD->domain;
      subData_t *pS = pD->theSubs;
      float energy = pS->eEnergy;
      if (energy < fGeEnerMin || energy > fGeEnerMax) {
        pD->detValid = false;
        pE->evtFoldDets[kDetGe]--;
        pE->numDetsEVT--;
      }
    }
  }
  
  if (fGeSuppression) {
    // re-check coincidence sub[0] -- sub[1]
    for (int nn = 0; nn < pE->numDetsGGP; nn++) {
      theDets_t* pD = pE->ggpDets + nn;
      if (pD->detType != kDetGe || !pD->detValid)
        continue;
      //int        id = pD->domain;
      subData_t *pS = pD->theSubs;
      if (fNumSubs > 1) {
        if(pS[0].trTime > 0 && pS[1].trTime > 0) {
          float tacGeAc = (pS[1].trTime - pS[0].trTime)*specTimeGain + +specLenT0*0.5f;
          if ((tacGeAc >= tacGateGeAC[0]) && (tacGeAc <= tacGateGeAC[1])) {
            pD->detValid = false;
            pE->evtFoldDets[kDetGe]--;
            pE->numDetsEVT--;
          }
        }
      }
    }
  }
  
  if (fTacMin > INT_MIN && fTacMax < INT_MAX) {
    for (int nn1 = 0; nn1 < pE->numDetsGGP; nn1++) {
      if (!pE->ggpDets[nn1].detValid || pE->ggpDets[nn1].detType != kDetGe)
        continue;
      theDets_t* pD1 = pE->ggpDets + nn1;
      //int        iD1 = pD1->domain;
      subData_t *pS1 = pD1->theSubs;
      for (int nn2 = nn1 + 1; nn2 < pE->numDetsGGP; nn2++) {
        if (!pE->ggpDets[nn2].detValid || pE->ggpDets[nn2].detType != kDetGe)
          continue;
        theDets_t* pD2 = pE->ggpDets + nn2;
        //int        iD2 = pD2->domain;
        subData_t *pS2 = pD2->theSubs;
        int   dts = (int)(pD2->tstamp - pD1->tstamp);
        float dtf = pS2[0].trTime - pS1[0].trTime;
        float dtt = tScale*(dts + dtf);
        int   tac = int(dtt*fTacGain + specLenT1*0.5f);
        if (tac >= fTacMin && tac <= fTacMax) {
          pD1->tacValid = true;
          pD2->tacValid = true;
        }
      }
    }
    
    for (int nn = 0; nn < pE->numDetsGGP; nn++) {
      theDets_t* pD = pE->ggpDets + nn;
      if (pD->detType != kDetGe || !pD->detValid)
        continue;
      if (!pD->tacValid) {
        pD->detValid = false;
        pE->evtFoldDets[kDetGe]--;
        pE->numDetsEVT--;
      }
    }
  }
  return;
}

void GlobalWorker::FilterGeEsum(galEvent_t * pE, unsigned char hQueue)
{
  fGeEsum = 0;
  if (!pE->validEvent)
    return;
  
  if (fGeEsumMax >= GE_ESUM_MAX)
    return;

  int fold = 0;
  for (int nn = 0; nn < pE->numDetsGGP; nn++) {
    if (pE->ggpDets[nn].detType == kDetGe && pE->ggpDets[nn].detValid) {
      fGeEsum += pE->ggpDets[nn].theSubs[0].eEnergy;
      fold++;
    }
  }
  int iesum = int(fGeEsum);
  
  if (fold >= 1 && HCHECK(Spec_GeES)) {
    Spec_GeES->IncrQ(0,    0, iesum, hQueue);  // total;
    Spec_GeES->IncrQ(0, fold, iesum, hQueue);
  }
  
  if (fold == 2 && HCHECK(Spec_GeESii)) {
    for (int nn1 = 0; nn1 < pE->numDetsGGP; nn1++) {
      theDets_t* pD1 = pE->ggpDets + nn1;
      if (!pD1->detValid || pD1->detType != kDetGe)
        continue;
      int        iD1 = pD1->domain;
      subData_t *pS1 = pD1->theSubs;
      for (int nn2 = 0; nn2 < pE->numDetsGGP; nn2++) {
        if (nn2 == nn1)
          continue;
        theDets_t* pD2 = pE->ggpDets + nn2;
        if (!pD2->detValid || pD2->detType != kDetGe)
          continue;
        int        iD2 = pD2->domain;
        subData_t *pS2 = pD2->theSubs;
        
        float esum = pS1[0].eEnergy + pS2[0].eEnergy;
        Spec_GeESii->IncrQ(iD1, iD2, int(esum), hQueue);
      }
    }
  }
  
  pE->validEvent = (fGeEsum >= fGeEsumMin && fGeEsum <= fGeEsumMax);
  if (!pE->validEvent)
    return;
  
  if (HCHECK(Spec_GeES)) {
    Spec_GeES->IncrQ(1,    0, iesum, hQueue);
    Spec_GeES->IncrQ(1, fold, iesum, hQueue);
  }
  
  if (fold == 2 && HCHECK(Matr_GGii)) {
    for (int nn1 = 0; nn1 < pE->numDetsGGP - 1; nn1++) {
      theDets_t* pD1 = pE->ggpDets + nn1;
      if (!pD1->detValid
	  || pD1->detType != kDetGe
	  || pD1->domain >= uint32_t(fDomsMatrGGii))
        continue;
      int        iD1 = pD1->domain;
      subData_t *pS1 = pD1->theSubs;
      for (int nn2 = nn1 + 1; nn2 < pE->numDetsGGP; nn2++) {
        theDets_t* pD2 = pE->ggpDets + nn2;
        if (!pD2->detValid
	    || pD2->detType != kDetGe
	    || pD2->domain >= uint32_t(fDomsMatrGGii))
          continue;
        int        iD2 = pD2->domain;
        subData_t *pS2 = pD2->theSubs;

        int ie1 = int(pS1[0].eEnergy*0.5);
        int ie2 = int(pS2[0].eEnergy*0.5);
        Matr_GGii->IncrQ(iD1, iD2, ie1, ie2, hQueue);
        Matr_GGii->IncrQ(iD2, iD1, ie2, ie1, hQueue);   // increment the index-symmetric matrix
      }
    }
  }
}

void GlobalWorker::FilterDetsSi(galEvent_t * pE, unsigned char hQueue)
{
  return;
}

bool GlobalWorker::ValidateEventMult(const galEvent_t * pE)
{
  if (pE->numDetsEVT < 1)
    return false;
  
  if(fTriggerCond==0) // Validate all condition
    return true;
  
  //const int * evtFoldDets = pE->evtFoldDets;
  int countTypes = 0;
  fEventTrigger.setTrigger(pE->evtFoldDets);
  //  for (int kd = 0; kd <= realMaxType; kd++) {
  //    fEventTrigger.setTrigger(kd, evtFoldDets[kd]);
  ////    std::cout << "Set the trigger on detType = " << kd
  ////        << " to " << evtFoldDets[kd] << std::endl;
  //    //  if (minFoldDets[kd] > 0 && evtFoldDets[kd] < minFoldDets[kd]) {
  //    //continue;             // type not counted as it has less than the requested fold
  //    //   return false;         // detector type has less than the requested fold
  //    //}
  //    //if (evtFoldDets[kd] > 0)
  //    // countTypes++;         // count also types not requested to have a minimum fold
  //  }
  //Now we have a structure with the trigger comparable to the one of the configuration file
  int  numTriggers = (int)theTriggers.size();
  for (int nn = 0; nn < numTriggers; nn++) {
    if (theTriggers[nn].compareTo(fEventTrigger))
      return true;
  }
  //for(auto it=theTriggers.begin();it!=theTriggers.end();it++){
  //  if(it->second->compareTo(*fEventTrigger)){
  //    validateEvt=true;
  //  }
  //}
  //return (countTypes >= minNumTypes);
  return false;
}

bool GlobalWorker::AnalyzeEvent(const galEvent_t * pE, unsigned char hQueue)
{
  if (fExcludeProcessing) {
    return true;
  }
  

  
  return true;
}

int GlobalWorker::HistoSingles(const galEvent_t * pE, unsigned char hQueue)
{
  return 0;
}

int GlobalWorker::HistoEvents(const galEvent_t * pE, unsigned char hQueue)
{
 
  return 0;
}

int GlobalWorker::SetInput(galEvent_t* pE, const uint32_t * inpBuff)
{
  pE->reset();
  
  agataKey* frameKey = (agataKey *)inpBuff;
  int frameBytes = frameKey->size;
  
  if (frameKey->key != evtMagicKeyInp)    // only for "Merger" (0xCA010103) frames
    return frameBytes;
  
  pE->tstamp = frameKey->GetTstamp();
  
  int restBytes = frameBytes;
  int restIndex = 0;
  if (!simpleFrame) {
    restBytes -= sizeof(agataKey);
    restIndex  = sizeof(agataKey) / sizeof(uint32_t);
  }
  int domBytes = 0;
  
  for (; restBytes >= sizeof(agataKey); restBytes -= domBytes, restIndex += domBytes / sizeof(uint32_t) ) {
    
    agataKey* domKey = (agataKey *)(inpBuff + restIndex);
    domBytes = domKey->size;

    if (domBytes < sizeof(agataKey))
      return -frameBytes;

    if (domBytes == sizeof(agataKey))
      continue;

    /////////////////////////////////////////////////////////////////
    // VME data (0xFA0201A0) handled first due to fixed domDataVME=63 << TO BE GENERALIZED
    if (domKey->key == keyDataVME) {    // do only event-stats and ignore the data frame
      int16_t detType = kDetVME;
      theDets_t* pD = pE->vmeDets /*+ pE->numDetsVME*/;   // as long as there is only one VME
      pD->tstamp   = domKey->GetTstamp();
      pD->domain   = domDataVME;                            // forced to "63u"
      pD->detAddr  = restIndex;                            // in the original frame
      pD->detType  = detType;
      pD->detValid = true;
      pE->numDetsVME = 1;                                 // as long as there is only one VME
      pE->evtFoldDets[detType]++;
      continue;
    }
    
    ////////////////////////////////////////////////////
    // ggp data
    
    // Totally ignore evBuilder subkeys because all info is in domID
    // Non-default detectors (NWdigital, Spider and Trace usually enter as SI)
    if ((0xFF000000 & domKey->key) == keyEventMask) {
      domBytes = sizeof(agataKey);
      continue;
    }

    if (domKey->IsIdle()){                    // ignore idles
      //      std::cout << "Ignoring IDLES" << std::endl;
      continue;
    }
    //  proceed only for the given types (interpreting all them as keyDataGGP ???)
    if(domKey->key == keyDataVME) continue;

    auto domain = domKey->GetDomain();
    if (domain >= numDoms)                  // ignore out-of-range domains
      continue;

    int detType = DetectorType(domain);
    if (detType < 0 || detType > realMaxType)
      continue;                             // ignore out-of-range sub-arrays
    
    theDets_t* pD = pE->ggpDets + pE->numDetsGGP; // data is copied into the proper place

    uint32_t detSubs = 2;//domKey->GetNbSub();  // Get the number of subdomains directly from the agataKey;
    if (detSubs == 0)                       // empty 
      continue;

    bool allSubs = (detSubs == numSubs);
    pD->clear(numSubs);                     // all subs are zeroed

    size_t payload = domKey->size - sizeof(agataKey);       // bytes
    size_t subSize = ((payload / detSubs) / sizeof(uint32_t)) * sizeof(uint32_t); // rounded to int4
    size_t lenRead = std::min(subSize, sizeof(subData_t));  // in case event is longer than subSize_t
    size_t numRead = lenRead / sizeof(float);
    int    indData = restIndex + sizeof(agataKey) / sizeof(uint32_t);
    int    nPars   = sizeof(subData_t) / sizeof(float);
    float *pPars   = (float *)pD->theSubs;

    if (inpPartial[detType]) {
      // to be verified
      float *pInps = (float *)(inpBuff + indData);
      bool  *pInpPars = datPartial[detType].fInpPars;
      for (int ns = 0; ns < detSubs; ns++) {
        int iSub = (allSubs ||
		    (detType != kDetSauron && detType != kDetTC)) ? ns : inpBuff[indData + 4];
        int np = nPars * iSub;
        for (int ip = 0; ip < nPars; ip++, np++) {
          if (pInpPars[np]) {
            pPars[np] = *pInps++;
            indData++;
          }
          else if (ip == 1 || ip == 4) {   // 1 4  energies set to their amplitudes
            pPars[np] = pPars[np - 1];
          }
        }
        pD->notZero[ns] = iSub;
      }
    }
    else {   // non-partial data
      float *pPars = (float *)pD->theSubs;
      for (auto ns = 0u; ns < detSubs; ns++) {
        int iSub = (allSubs ||
		    (detType != kDetSauron && detType != kDetTC)) ? ns : inpBuff[indData + 4];
        memcpy(pPars + iSub * numRead, inpBuff + indData, lenRead);
        indData += nPars;
        pD->notZero[ns] = iSub;
      }
    }
    pD->tstamp = domKey->GetTstamp();
    pD->domain = domain;
    pD->detAddr = restIndex;
    pD->detType = (int16_t)detType;
    pD->detValid = true;
    pE->numDetsGGP++;
    pE->evtFoldDets[detType]++;
  } //  for (; restBytes >= sizeof(agataKey); ...
  
  pE->validEvent = true;
  pE->numDetsEVT = pE->numDetsVME + pE->numDetsGGP;
  return frameBytes;
}

// pass input event to output
int  GlobalWorker::SetOutput(const galEvent_t * pE, uint32_t *outBuff, int availableBytes, const uint32_t * oriData, int numBytes)
{
  if (availableBytes < numBytes) {
    cout << "GlobalWorker::SetOutput ==> buffer full" << endl;
    return availableBytes - numBytes;
  }
  
  // all the rest is <= numBytes: therefore, no need to check again
  
  // write outer key and selected inner data
  int outIndex = 0;                   // uint32_t
  int outToken = 0;                   // bytes
  int outAvail = availableBytes;      // bytes
  
  outToken = sizeof(agataKey);        // global agatakey
  memcpy(outBuff, oriData, outToken);
  outIndex += outToken / sizeof(uint32_t);
  outAvail -= outToken;
  
  int validDets = 0;    // count number of valid detectors in output event
  
  // for NW_VME copy the whole input frame.
  if (pE->numDetsVME && pE->vmeDets->detValid) {
    theDets_t* pD = pE->vmeDets;            // just one
    int detAddr = pD->detAddr;
    outToken = oriData[detAddr];            // the first word of agataKey
    memcpy(outBuff + outIndex, oriData + detAddr, outToken);
    outIndex += outToken / sizeof(uint32_t);
    outAvail -= outToken;
    validDets++;
  }
  
  // for ggp data: take parameters pE->ggpDets[nn].theSubs
  for (int nn = 0; nn < pE->numDetsGGP; nn++) {
    theDets_t* pD = pE->ggpDets+nn;
    if (!pD->detValid)
      continue;
    
    int detAddr = pD->detAddr;    // re original buffer oData
    int domain  = pD->domain;     // data in dDom
    
    outToken = sizeof(agataKey);
    memcpy(outBuff + outIndex, oriData + detAddr, outToken);
    uint32_t detSubs = (outBuff[outIndex+2]&0x3f000000)>>24;
    bool allSubs = detSubs == numSubs;
    int detType = DetectorType(domain);
    
    if (outPartial[detType]) {
      std::cout << "detType == " << detType << std::endl;
      bool *outPars = datPartial[detType].fOutPars;
      uint32_t * pO = outBuff + outIndex + sizeof(agataKey) / sizeof(uint32_t);
      int nump = 0;
      for (auto ns = 0u; ns < detSubs; ns++) {
        int iSub = (allSubs ||
		    (detType != kDetSauron && detType != kDetTC)) ? ns : pD->notZero[ns];
        uint32_t * pV = (uint32_t *)(pD->theSubs+iSub);
        int nv = iSub * maxSubPars;
        for (int iv = 0u; iv < maxSubPars; iv++) {
          if (outPars[nv++]) {
            pO[nump++] = pV[iv];
          }
        }
      }
      outToken += nump * sizeof(uint32_t);
    }
    else {
      // copy all
      size_t lenWrite = detSubs*sizeof(subData_t);
      memcpy(outBuff + (outIndex + outToken / sizeof(uint32_t)), pD->theSubs, lenWrite);
      outToken += (int)lenWrite;
    }
    outBuff[outIndex] = outToken;     // adjust frame length
    outIndex += outToken / sizeof(uint32_t);
    outAvail -= outToken;
    validDets++;
  }   // loop on totDets
  if (validDets == 0) {
    return 0;
  }
  
  int outUsed = outIndex*sizeof(uint32_t);
  outBuff[0] = outUsed;
  outBuff[1] = evtMagicKeyOut;
  outBuff[2] = validDets;
  return outUsed;
}

int  GlobalWorker::FormatFramePut(uint32_t * outBuff, int availableBytes)
{
  if (availableBytes < 200)  // should make a more realistic test
    return 0;
  
  agataKey* fmtKey = (agataKey *)outBuff;
  uint32_t  outEvLenByte = (uint32_t)sizeof(agataKey);
  
  // hand-made agata key
  fmtKey->size  = outEvLenByte;
  fmtKey->key   = keyEventFormat;
  fmtKey->evnum = 0;
  fmtKey->ts_0  = 0;
  fmtKey->ts_1  = 0;
  
  uint32_t outIndex = outEvLenByte / sizeof(uint32_t);
  
  int subsystems = 0;
  
  for (int detType = 0; detType <= realMaxType; detType++) {
    if (detType == kDetVME) {
      if (outHasVME) {    // put descriptor only if explicitly mentioned
        int ns = 0;
        uint32_t outBits = 0;
        outBits |= detType << 24;
        outBits |= ns << 16;
        outBuff[outIndex++] = outBits;
        subsystems++;
        continue;
      }
    }
    if (outPartial[detType]) {
      uint32_t indPars = 0;
      for (auto ns = 0u; ns < numSubs; ns++) { // should consider different numSubs
	// hard coded version to limit the numbers of subdomains
        if(detType != kDetSauron && detType != kDetTC && ns>1)break;
	if(detType == kDetTC && ns>3) break; // hard coded limit for the 
        uint32_t outBits = 0;
        for (size_t ii = 0; ii < 8; ii++) {               // should be number of parameters
          uint32_t set = datPartial[detType].fOutPars[indPars++] ? 1 : 0;
          outBits |= set << ii;
        }
        outBits |= detType << 24;
        outBits |= ns << 16;
        outBuff[outIndex++] = outBits;
      }
      subsystems++;
    }
  }
  
  int outUsed = outIndex*sizeof(uint32_t);
  outBuff[0] = outUsed;
  
  return (subsystems>0) ? outUsed : 0;
}

int  GlobalWorker::FormatFrameGet(const uint32_t * inpBuff) {
  if (!FormatFrameKey(inpBuff))
    return 0;
  
  uint32_t keyPos = (uint32_t)(sizeof(agataKey)/sizeof(uint32_t));
  uint32_t keyEnd = (uint32_t)(inpBuff[0] / sizeof(uint32_t));
  
  bool cleared[kDetTypes]; memset(cleared, false, sizeof(cleared));
  
  while (keyPos < keyEnd) {
    uint32_t key = inpBuff[keyPos++];
    uint32_t det = (key & 0xFF000000) >> 24;
    uint32_t sub = (key & 0x00FF0000) >> 16;
    uint32_t val = (key & 0x0000FFFF);
    
    if (det > uint32_t(realMaxType)) {
      cout << "Unexpected detector-type  %u in FormatFrameGet()" << endl;
      for (uint32_t nn = 0; nn < keyEnd; nn++) {
        cout << setw(4) << nn << "  "
	     << setw(8) << hex << setfill('0') << inpBuff[nn] << dec << setfill(' ')
	     << ((nn + 1 == keyPos) ? " *" : "  ") << endl;
      }
      return inpBuff[0];
    }
    
    if (det != kDetVME) {
      bool  *ppp = datPartial[det].fInpPars;
      size_t sss = sizeof(datPartial[det].fInpPars);
      inpPartial[det] = true;
      
      if (!cleared[det]) {
        memset(ppp, false, sss);
        cleared[det] = true;
      }
      
      ppp += sub * 8;
      uint32_t bit = 1;
      for (int bb = 0; bb < 8; bb++) {
        ppp[bb] = (val & bit) != 0;
        bit <<= 1;
      }
    }
    outPartial[det] = true;
  }
  
  return inpBuff[0];
}

int GlobalWorker::ProcessConfig(const char* fname)
{
  cout << gMotherClass + "::process_config() called with file = " << fname << endl;
  
  fConfigFile = fname;
  if (!fConfigFile.empty()) {       // use defaults, if process_config was not called
    if (!fileExists(fConfigFile)) {
      return  102;                  // fatal error because configuration file not present
    }
    
    // GetParameters uses class storage (e.g. theDoms[dom].calibs[sub].*) that depends on numDoms and numDets
    // Therefore we must (re)intitialize the configuration according to "Detectors nd ns" given in fConfigFile
    string confString;
    int rv = getKeyFromFile(fConfigFile, "Detectors", confString);
    if (rv == 2) {
      std::cout << "Fatal Error: "
		<< fConfigFile << " MUST contain the keyword 'Detectors numDoms numSubs' "
		<< std::endl;
      return 102; // Fatal error because the keyword "Detectors" MUST be present`
    }
    istringstream il(confString);
    il >> fNumDoms >> fNumSubs;
    if (il.fail()) {
      return 103; // Fatal error because the two parameters must be given
    }
    InitConfig(fNumDoms, fNumSubs);
    
    // now we can read the parameters
    rv = GetParameters();
    if (rv) {
      //cout << "Error " << rv << " reading " << fConfigFile << endl;
      return rv;
    }
    
  }
  
  //////////// initialize //////////
  std::cout << "Initializing partial input" << std::endl;
  // description of partial input
  for (int detType = 0; detType <= realMaxType; detType++) {
    string &strInp = datPartial[detType].strInpPars;
    bool   *parInp = datPartial[detType].fInpPars;
    bool ispartial = false;
    if (strInp.size()) {
      memset(parInp, true, sizeof(datPartial[detType].fInpPars));
      int np = 0;
      for (size_t ii = 0; ii < strInp.size(); ii++) {
        if (strInp[ii] == '0') {
          parInp[np++] = false;
          ispartial = true;
        }
        if (strInp[ii] == '1')
          parInp[np++] = true;
        // ignore all other values
      }
    }
    inpPartial[detType] = ispartial;
  }

  std::cout << "Initializing partial output" << std::endl;
  // description of partial output
  for (int detType = 0; detType <= realMaxType; detType++) {
    string &strOut = datPartial[detType].strOutPars;
    bool   *parOut = datPartial[detType].fOutPars;
    bool ispartial = false;
    if (strOut.size()) {
      memset(parOut, true, sizeof(datPartial[detType].fOutPars));
      int np = 0;
      for (size_t ii = 0; ii < strOut.size(); ii++) {
        if (strOut[ii] == '0') {
          parOut[np++] = false;
          ispartial = true;
        }
        if (strOut[ii] == '1')
          parOut[np++] = true;
        // ignore all other values
      }
      
      if(strOut[strOut.size()-1]=='_') {
        printf("String ends up with _ we should repeat the mask for %i times\n",
	       numSubs/(strOut.size()/9));
        for(uint16_t ns = 0 ; ns < numSubs ; ns++){
          for (size_t ii = 0; ii < strOut.size()-1; ii++) {
	    if(strOut[ii] == '_') {
	      // Added for repeating more than 1 subdomain
	      ns++;
	    }
            if (strOut[ii] == '0') {
              parOut[np++] = false;
            }
            if (strOut[ii] == '1')
              parOut[np++] = true;
          }
        }
      }
      outPartial[detType] = ispartial;
    }
  }
  //cout << "Initializing spectra" << endl;
  InitSpectra(nSpectra, specZero);
  
  // read calibrations
  
  ////////////////////////////////////////////////////////////////////
  // set-up for parallel execution (to be done even if fNumThreads==0)
  int numBlocks = MW_Reset(fNumThreads);          // even if not using threads
  for (int nn = 0; nn < numBlocks; nn++) {        // generate numBlocks events
    MW_NewEvent(nn, new galEvent_t(numDoms, numSubs));
  }
  ////////////////////////////////////////////////////////////////////
  
  cServer.SetCommandFile(fConfigFile + ".live");
#ifdef GAL_OFFLINE
  cServer.AcceptExit(); // enable ECommand::kExit
#endif
  cServer.Start("GF", max(fPromptPeriod, 0));
  
  //cout << "Done ProcessConfig" << endl;
  return 0;
}

int GlobalWorker::ProcessStart(uint32_t run)
{
  fCountBlocks = 0;
  fRunNumber = run;
  writeFormatFrame = true;
  //cServer.Start(gMotherClass, max(fPromptPeriod, 0));
  cServer.Start("GF", max(fPromptPeriod, 0));
  return 0;
}

int  GlobalWorker::ProcessStop ()
{
  cServer.Save(true);
  
  // print statistics
  cout << "GlobalWorker Statistics of this run" << dec << endl;
  for(auto nn = 0u; nn < numDoms; nn++) {
    if(!theDoms[nn].GetCounts()) continue;
    int dty = (nn == domDataVME) ? kDetVME : DetectorType(nn);
    std::cout << " dom-" << setw(3) << setfill('0') << nn
	      << setfill(' ') << setw(10) << theDoms[nn].GetCounts()
	      << " {" << dty << "} ";
    int maxstat = 0;
    for(int jj = 0; jj < 16; jj++) {
      if(theDoms[nn].GetCounts(jj)>0)
        maxstat = jj;
    }
    if(maxstat) {
      cout << " ";
      for(int jj = 1; jj <= maxstat; jj++) {
	std::cout << " " << setw(6) << theDoms[nn].GetCounts(jj);
      }
    }
    cout << endl;
  }
  printf("END of %s\n",__func__);
  return 0;
}

void GlobalWorker::InitConfig(int numdoms, int numsubs)
{
  numdoms = max(1, numdoms);
  numsubs = max(1, numsubs);
  
  if(numdoms != numDoms || numsubs != numSubs) {
    if(theDoms) {
      for(auto nd = 0u; nd < numDoms; nd++) {
        if(theDoms[nd].values)
          delete [] theDoms[nd].values;
        if(theDoms[nd].calibs)
          delete [] theDoms[nd].calibs;
      }
      delete [] theDoms;
      theDoms = nullptr;
    }
    
    //if(ggpDoms) delete [] ggpDoms;
    //ggpDoms = nullptr;
    
    //if (allDets)
    //  delete[] allDets;
    //allDets = nullptr;
    
    numDoms = numdoms;
    numSubs = numsubs;
    
    theDoms = new domData_t[numDoms];
    for(auto nd = 0u; nd < numDoms; nd++) {
      theDoms[nd].domain = nd;
      theDoms[nd].values = new subData_t[numSubs];
      theDoms[nd].calibs = new calData_t[numSubs];
    }
    
    //ggpDoms = new int [numDoms];
    //memset(ggpDoms, 0, numDoms*sizeof(int));
    
    //allDets = new theDets_t[numDoms];
    //memset(allDets, 0, numDoms*sizeof(theDets_t));
  }
  
}

void GlobalWorker::InitSpectra(int num, int zero)
{


}

int GlobalWorker::GetParameters(bool doList, bool partial)
{
  int    xDom(0), xSub(0);
  float  vVal[6] = {0};
  string keyEventInp;                 // to override the const merger key
  string keyEventOut;                 // to override the const merger key
  //int    xCond(0), minFold(0); // added to have more flexibility on the trigger
  int    minFold[kDetTypes] = { 0 };  // added to have more flexibility on the trigger
  string htag, hcmd;
  int    which, times;
  int    aType(0);
  int    dFrom(0), dTo(0);
  string strDet, strPars;

  rubuilder::utils::ConfReader conf;
  conf.Add("Detectors",         "number of domains and number of channels/domain"
	   , &fNumDoms, &fNumSubs);
  conf.Add("ArrayType",         "detType domFrom domTo (detType: Galileo, TC, Euclides, Spider, VME,..."
	   , &strDet, &dFrom, &dTo).SetRequired(3);
  conf.Add("Spectra",           "number of spectra, domain of first spectrum"
	   , &nSpectra, &specZero);
  conf.Add("PackBlocks",        "number of input blocks packed toghether before calling the analysis "
	   , &fPackBlocks);
  conf.Add("Threads",           "number of threads to use to parallelize the analysis "
	   , &fNumThreads);
  conf.Add("ServerPort",        " tcp port number for the command server (default=16161, no server if <=1000)"
	   , &tcpPortno);
  conf.Add("KeyEventInp",       "key of outer composite frame (hex)"
	   , &keyEventInp);
  conf.Add("KeyEventOut",       "key of outer composite frame (hex)"
	   , &keyEventOut);
  conf.Add("InpParams",         "DetType (string) and a string of 0/1 describing the partial-input  parameters for DetType"
	   , &strDet, &strPars);
  conf.Add("OutParams",         "DetType (string) and a string of 0/1 describing the partial-output parameters for DetType"
	   , &strDet, &strPars);
  conf.Add("OutHasVME",         "keyword to declare the presence of VME (e.g.Nwall) data on the output file"
	   , &outHasVME);  // special case
  conf.Add("Prompt",            "prompt line repetition time (sec)"
	   , &fPromptPeriod);
  conf.Add("ExcludeProcessing", "exclude all processing, passing out only empty events"
	   , &fExcludeProcessing);
  conf.Add("SaveDataDir",       "where to write data and spectra"
	   , &fOutPath);
  conf.Add("SpecPrefix",        "prefix to names of spectra"
	   , &fHistPfx);
  conf.Add("GeTacGate",         "acceptance window on the gg_tac time (ns)"
	   , &fTacMin, &fTacMax);
  conf.Add("MinFoldDets",       "minimum fold for Ge, Si, NW, LaBr and numTypes to accept event"
	   , minFoldDets).SetRequired(1).SetSize(0, 8);
  conf.Add("TriggerConditions", "Number of trigger conditions. 0 -> Accept all"
	   , &fTriggerCond);
  conf.Add("MinFold",           "DetType (string) an minimum fold on trigger condition N for DetType"
	   , &strDet, minFold).SetRequired(2).SetSize(1, kDetTypes);
  conf.Add("MinDetTypes",       "minimum fold for Ge, Si, NW, LaBr and numTypes to accept event"
	   , &minNumTypes);
  conf.Add("EnergyCal",         "calibration coefficients for Energy: detector channel a0 a1 [a2 a3 a4 a5]"
	   , &xDom, &xSub, vVal).SetRequired(4).SetSize(2, 6);
  conf.Add("EnergyGal",         "energy calibration with exp correction: detector channel offs gain xexp [re-gain]"
	   , &xDom, &xSub, vVal).SetRequired(5).SetSize(2, 4);
  conf.Add("EnergyGain",        "scaling factor for energy spectra"
	   , &fEnergyGain);
  conf.Add("TacGain",           "scaling factor for the ...TT spectra"
	   , &fTacGain);
  conf.Add("GeEnergyGate",      "common acceptance window on germanium energy (keV)"
	   , &fGeEnerMin, &fGeEnerMax);
  conf.Add("TimeGeAC",          "time gate of the Ge-AC correlation"
	   , tacGateGeAC).SetSize(0, 2);
  conf.Add("GeEsumGate",        "acceptance window on Ge sum energy and number of domains for Matr_GGii"
	   , &fGeEsumMin, &fGeEsumMax, &fDomsMatrGGii);
  conf.Add("MatrixGG",          "gamma-gamma matrix for all detectors"
	   , &fSizeMatrGG, &fGainMatrGG);
  conf.Add("H16",               "tag and command-string of generic u16 histogram"
	   , &htag, &hcmd).SetRequired(2);
  conf.Add("H32",               "tag and command-string of generic u32 histogram"
	   , &htag, &hcmd).SetRequired(2);
  conf.Add("HH16",              "tag and command-string of generic u16 det-det histogram"
	   , &htag, &hcmd).SetRequired(2);
  conf.Add("HH32",              "tag and command-string of generic u32 det-det histogram"
	   , &htag, &hcmd).SetRequired(2);
  conf.Add("Verbose",           "verbosity of printouts"
	   , &fVerbose);
  
  if(doList) {conf.Show(gMotherClass); return 0;}
  
  if (partial) {
    // disable "non-calibration" keywords
    conf.Disable("Detectors");
    conf.Disable("ArrayType");
    conf.Disable("Spectra");
    conf.Disable("PackBlocks");
    conf.Disable("Threads");
    conf.Disable("ServerPort");
    conf.Disable("KeyEvent");
    conf.Disable("InpParams");
    conf.Disable("OutParams");
    conf.Disable("OutHasVME");
    conf.Disable("SaveDataDir");
    conf.Disable("SpecPrefix");
    conf.Disable("MatrixGG");
  }
  
  int rv = conf.Read(gMotherClass, fConfigFile);
  
  if(rv != 0)
    return rv;  // error
  
  // decoding hex and oct values should be in Conf
  if(conf.Seen("KeyEventInp")) {
    istringstream iss(keyEventInp);
    iss >> hex >> evtMagicKeyInp;
  }
  if (conf.Seen("KeyEventOut")) {
    istringstream iss(keyEventOut);
    iss >> hex >> evtMagicKeyOut;
  }
  
  which = conf.Find("ArrayType");
  times = conf.Times(which);
  if (times > 0) {
    std::fill_n(arrayMap, 256, -1);
    //memset(arrayMap, -1, sizeof(arrayMap));
    realMaxType = 0;
    for (int nn = 0; nn < times; nn++) {
      conf.Restore(which, nn);
      int aType = DetTypeIndex(strDet);
      if (aType < 0 || aType >= 16) {
	std::cerr << "==> ERROR decoding ArrayType: Invalid array type "
		  << strDet << std::endl;
        return 200;
      }
      if ( dFrom < 0 || dFrom >= 256
	   || dTo   < 0 || dTo   >= 256
	   || dTo < dFrom) {
	std::cerr << "==> ERROR decoding ArrayType: Invalid domain range "
		  << dFrom << " " << dTo << std::endl;
        return 200;
      }
      std::fill_n(arrayMap + dFrom, dTo - dFrom + 1, aType);
      //memset(arrayMap + dFrom, aType, dTo - dFrom + 1);
      realMaxType = max(realMaxType, aType);
    }
    realMaxType = max(realMaxType, 1);    // VME is seen as germanium detector #63  ???
  }

  //if (conf.Seen("ArrayTypes")) {
  //  realMaxType = 0;
  //  for (int ii = 0; ii < 16; ii++) {
  //    if (arrayMap[ii] >= 0) {
  //      std::cout  << "Found an Array with id =  " << arrayMap[ii] << std::endl;
  //      realMaxType = max(realMaxType, arrayMap[ii]);
  //    }
  //  }
  //  realMaxType = max(realMaxType, 1);    // VME is seen as germanium detector #63
  //  std::cout  << "setting realMaxType to " << realMaxType << std::endl;
  //}
  
  //("TimeGeAC",          "time gate of the Ge-AC correlation", tacGateGeAC)
  if (conf.Times("TimeGeAC")) {
    fGeSuppression = (tacGateGeAC[1] >= tacGateGeAC[0]);
    if (tacGateGeAC[1] < tacGateGeAC[0])      // order the window limits so that by giving them in reverse
      swap(tacGateGeAC[0], tacGateGeAC[1]);   // it is possible to have a window without suppression
  }
  
  // ("EnergyCal",         "calibration coefficients for Energy: detector channel a0 a1 [a2 a3 a4 a5]", &xDom, &xSub, xVal)
  which = conf.Find("EnergyCal");
  times = conf.Times(which);
  for (int nn = 0; nn < times; nn++) {
    conf.Restore(which, nn);
    if (xDom < 0 || xDom >= fNumDoms) {
      std::cerr << "==> ERROR decoding EnergyCal: Invalid domain " << xDom << std::endl;
      return 200;
    }
    if (xSub < 0 || xSub >= fNumSubs) {
      std::cerr << "==> ERROR decoding EnergyCal: Invalid channel "
		<< xSub << " for domain " << xDom << std::endl;
      return 200;
    }
    memset(theDoms[xDom].calibs[xSub].ecalv, 0,
	   sizeof(theDoms[xDom].calibs[xSub].ecalv));
    int count = conf.Count(which, 2);
    for (int mm = 0; mm < count; mm++) {
      theDoms[xDom].calibs[xSub].ecalv[mm] = vVal[mm];
    }
    theDoms[xDom].calibs[xSub].cmode = count - 1;
    fGeCalibration = true;
  }
  
  // ("EnergyGal",         "energy calibration with exp correction: detector channel offs gain xexp [re-gain]", &xDom, &xSub, xVal)
  which = conf.Find("EnergyGal");
  times = conf.Times(which);
  for (int nn = 0; nn < times; nn++) {
    conf.Restore(which, nn);
    if (xDom < 0 || xDom >= fNumDoms) {
      cout << "==> ERROR decoding EnergyCal: Invalid domain " << xDom << endl;
      return 200;
    }
    if (xSub < 0 || xSub >= fNumSubs) {
      cout << "==> ERROR decoding EnergyCal: Invalid channel " << xSub << " for domain " << xDom << endl;
      return 200;
    }
    memset(theDoms[xDom].calibs[xSub].ecalv, 0,
	   sizeof(theDoms[xDom].calibs[xSub].ecalv));
    theDoms[xDom].calibs[xSub].ecalv[3] = 1.f;  // in case it is not given
    int count = conf.Count(which, 2);
    for (int mm = 0; mm < count; mm++) {
      theDoms[xDom].calibs[xSub].ecalv[mm] = vVal[mm];
    }
    theDoms[xDom].calibs[xSub].cmode = 0;
    fGeCalibration = true;
  }
  
  if (fTriggerCond == 0) {
    std::cout << "No Trigger condition set: the GlobalFilter will write all events ... " << std::endl;
    theTriggers.clear();
    //theTriggers[0] = new theTrigger_t();
    //theTriggers[0]->init();
  }
  else if (!DecodeTrigger(conf, strDet, minFold)) {
    return 200;
  }

  if (!DecodePartialIO(conf, strDet, strPars))
    return 200;

  if (!DecodeGenHistos(conf, htag, hcmd, partial))
    return 200;

  fGeEnergyGate = (fGeEnerMin > -FLT_MAX || fGeEnerMax < FLT_MAX);
  
  fPromptPeriod = max(1, fPromptPeriod);
  
  return rv;
}

// Factored out of GetParameters
// in view of moving here the actions done in ProcessConfig
bool GlobalWorker::DecodePartialIO(rubuilder::utils::ConfReader &conf,
				   std::string &strDet,
				   std::string &strPars)
{
  int which, times;
  which = conf.Find("InpParams");
  times = conf.Times(which);
  for (int nn = 0; nn < times; nn++) {
    conf.Restore(which, nn);
    int det = DetTypeIndex(strDet);
    if (det == kDetNone) {
      cout << "InpParams: Invalid detector" << strDet << endl;
      return false;
    }
    datPartial[det].strInpPars = strPars;
  }

  which = conf.Find("OutParams");
  times = conf.Times(which);
  for (int nn = 0; nn < times; nn++) {
    conf.Restore(which, nn);
    int det = DetTypeIndex(strDet);
    if (det == kDetNone) {
      cout << "OutParams: Invalid detector " << strDet << endl;
      return false;
    }
    datPartial[det].strOutPars = strPars;
  }
  return true;
}

// Factored out of GetParameters
bool GlobalWorker::DecodeTrigger(rubuilder::utils::ConfReader &conf, 
				 std::string &strDet, 
				 int* minFold)
{
  std::cout << "Configuring " << fTriggerCond << " trigger conditions ... " << std::endl;

  // Starting to put all the multiplicity to 0
  theTriggers.clear();
  for (uint16_t cond = 0; cond < fTriggerCond; cond++) {
    theTriggers.emplace_back();
    //theTriggers[cond] = new theTrigger_t();
    //theTriggers[cond]->init();
  }

  int which = conf.Find("MinFold");
  int times = conf.Times(which);
  for (int nn = 0; nn < times; nn++) {
    conf.Restore(which, nn);
    int det = DetTypeIndex(strDet);
    if (det == kDetNone) {
      cout << "Minfold: Invalid detector name " << strDet << endl;
      return false;
    }
    int numCond = conf.Count(which, 1);
    if (numCond < fTriggerCond) {
      cout << "MinFold: Too few conditions for detector " << strDet
           << ";  expected " << fTriggerCond << " got " << numCond << endl;
      return false;
    }
    for(int ii = 0; ii < fTriggerCond; ii++) {
      theTriggers[ii].setTrigger(det, minFold[ii]);
    }
  }
  return true;
}

// Factored out of GetParameters
bool GlobalWorker::DecodeGenHistos(rubuilder::utils::ConfReader &conf, 
				   std::string &htag, 
				   std::string &hcmd, 
				   bool partial)
{
  // block incrementing GenHistos, hoping they are no more used when deleted
  incrementGH = false;

  int which, times;
  vectGH<uint16_t> tH16, tHH16;
  vectGH<uint32_t> tH32, tHH32;

  // H16 "D(f,t) S P [d,s,p]{l,g,o} [d,s,i]{l,g,o}"
  which = conf.Find("H16");
  times = conf.Times(which);
  for (int nn = 0; nn < times; nn++) {
    conf.Restore(which, nn);
    GenericHisto<uint16_t> hh;
    hh.tag = htag;
    hh.command = hcmd;
    if (hh.decode(fNumDoms, fNumSubs))
      tH16.push_back(hh);
    else {
      cout << "==> ERROR decoding H16 " << htag << " \"" << hcmd << "\"" << endl;
      return false;
    }
  }

  // H32 "D(f,t) S P [d,s,p]{l,g,o} [d,s,p]{l,g,o}"
  which = conf.Find("H32");
  times = conf.Times(which);
  for (int nn = 0; nn < times; nn++) {
    conf.Restore(which, nn);
    GenericHisto<uint32_t> hh;
    hh.tag = htag;
    hh.command = hcmd;
    if (hh.decode(fNumDoms, fNumSubs))
      tH32.push_back(hh);
    else {
      cout << "==> ERROR decoding H32 " << htag << " \"" << hcmd << "\"" << endl;
      return false;
    }
  }

  // HH16 "D(f,l)  D(f,l) [d|(f,l),s,p]{l,g,o} [d|(f,l),s,i]{l,g,o}"
  which = conf.Find("HH16");
  times = conf.Times(which);
  for (int nn = 0; nn < times; nn++) {
    conf.Restore(which, nn);
    GenericHisto<uint16_t> hh;
    hh.tag = htag;
    hh.command = hcmd;
    if (hh.decode(fNumDoms, fNumSubs))
      tHH16.push_back(hh);
    else {
      cout << "==> ERROR decoding HH16 " << htag << " \"" << hcmd << "\"" << endl;
      return false;
    }
  }

  // HH32 "D(f,l)  D(f,l) [d|(f,l),s,p]{l,g,o} [d|(f,l),s,i]{l,g,o}"
  which = conf.Find("HH32");
  times = conf.Times(which);
  for (int nn = 0; nn < times; nn++) {
    conf.Restore(which, nn);
    GenericHisto<uint32_t> hh;
    hh.tag = htag;
    hh.command = hcmd;
    if (hh.decode(fNumDoms, fNumSubs))
      tHH32.push_back(hh);
    else {
      cout << "==> ERROR decoding HH32 " << htag << " \"" << hcmd << "\"" << endl;
      return false;
    }
  }


  // enable incrementing GenHistos
  incrementGH = true;

  return true;
}

void GlobalWorker::UpdateGenHistos(const theDets_t *pD, unsigned char hQueue)
{
  ghdat_t ghdat;
  ghdat.specID = pD->domain - specZero;
  ghdat.tstamp = pD->tstamp;
  ghdat.ptpars = (float *)pD->theSubs;

  if (incrementGH) {
    for (auto &hh : H16) {
      if (HCHECK(hh.histo))
        hh.IncrQ(ghdat, hQueue);
    }
  }
  if (incrementGH) {
    for (auto &hh : H32) {
      if (HCHECK(hh.histo))
        hh.IncrQ(ghdat, hQueue);
    }
  }
}

void GlobalWorker::UpdateGenHistos(const theDets_t *pD1, const theDets_t *pD2, unsigned char hQueue)
{
  ghdat_t ghdat1;
  //ghdat1.tstamp = pD1->tstamp - tstFirst;
  ghdat1.ptpars = (float *)pD1->theSubs;
  ghdat1.specID = pD1->domain - specZero;
  ghdat_t ghdat2;
  //ghdat2.tstamp = pD2->tstamp - tstFirst;
  ghdat2.ptpars = (float *)pD2->theSubs;
  ghdat2.specID = pD2->domain - specZero;

  ghdat2.tstamp = pD2->tstamp - pD1->tstamp;
  ghdat1.tstamp = 0;

  if (incrementGH) {
    for (auto &hh : HH16) {
      if (HCHECK(hh.histo))
        hh.IncrQ(ghdat1, ghdat2, hQueue);
    }
  }

  if (incrementGH) {
    for (auto &hh : HH32) {
      if (HCHECK(hh.histo))
        hh.IncrQ(ghdat1, ghdat2, hQueue);
    }
  }
}
