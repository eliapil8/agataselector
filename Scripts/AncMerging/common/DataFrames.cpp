#include "DataFrames.h"
#include "rubuilder/utils/Misc.h"

// case insensitive comparisons
uint32_t OutDataKey(const std::string& keyDataOut) {
  if (stringEq(keyDataOut, "PrismaRaw") 
      || stringHas(keyDataOut, "VME")) {
    return keyDataVME;
  }
  else if(stringEq(keyDataOut, "NEDA")){
    return keyDataNEDA;
  }
  else if(stringEq(keyDataOut, "EAGLE")){
    return keyDataEAGLE;
  }
  else if(stringEq(keyDataOut, "TC")){
    return keyDataTC;
  }
  else if (stringEq(keyDataOut, "PrismaAna")){
    return keyDataPrisma;
  } 
  else if (stringEq(keyDataOut, "Spider")) {
    return keyDataSpider;
  }
  else if (stringEq(keyDataOut, "Dante")) {
    return keyDataDante;
  }
  if (stringEq(keyDataOut, "Euclides")){
    return keyDataEuclides;
  }
  else if(stringHas(keyDataOut, "LaBr")){
    return keyDataLaBr;
  }
  else if (stringWhere(keyDataOut, "NWDigi") == 0
	   || stringWhere(keyDataOut, "NWall") == 0) {
    return keyDataNWDigital;
  }
  else if(stringHas(keyDataOut,"Sauron")){
    return keyDataSauron;
  }
  else if (stringEq(keyDataOut, "Oscar")) {
    return keyDataOscar;
  }
  else if (stringEq(keyDataOut, "RFD")) {
    return keyDataRFD;
  }
  //else if (stringEq(keyDataOut, "Anc8")) {
  //  return keyDataAnc8;
  //}
  //else if (stringEq(keyDataOut, "Anc9")) {
  //  return keyDataAnc9;
  //}
  else {
    std::cout << "Unknown Detector type: " << keyDataOut << std::endl;
    return 0xFA0201AF; // ??
  }
}

int DetTypeIndex(const std::string& strDetType)
{

  if (stringEq(strDetType, "VME")) {
    return kDetVME;
  }
  else if (stringEq(strDetType, "Euclides")
	   || stringEq(strDetType, "Si")) {
    return kDetSi;
  }
  else if (stringWhere(strDetType, "NWDigi") == 0
	   || stringWhere(strDetType, "NWall") == 0) {
    return kDetNWDigital;
  }
  else if (stringEq(strDetType, "Dante")) {
    return kDetDante;
  }
  else if (stringEq(strDetType, "Spider")) {
    return kDetSpider;
  }
  else if (stringEq(strDetType, "V1730")) {
    return kDetV1730;
  }
  else if (stringEq(strDetType, "RFD")) {
    return kDetRFD;
  }
  else {
    return kDetNone;
  }
}

std::string OutDataType(const uint32_t &frameKey)
{
  std::string outDataType;
  switch(frameKey){
  case keyEventFormat:
    outDataType ="header";
    break;
  case keyEventGlobal:
    outDataType = "event";
      break;
  case keyEventGE:
    outDataType = "event:hpge";
    break;
  case keyEventSI:
    outDataType = "event:si";
      break;
  case keyEventNW:
    outDataType = "event:labr";
    break;
  case keyDataVME:
    outDataType = "data:anc:vme";
    break;
  case keyDataEuclides:
    outDataType = "data:anc:euclides";
    break;
  case keyDataNWDigital:
    outDataType = "data:anc:NWdigital";
    break;
  case keyDataLaBr:
    outDataType = "data:anc:labr";
    break;
  case keyDataSpider:
    outDataType = "data:anc:spider";
    break;
  case keyDataOscar:
    outDataType = "data:anc:oscar";
    break;
  case keyDataRFD:
    outDataType = "data:anc:rfd";
    break;
  case keyDataNEDA:
    outDataType = "data:neda";
    break;
  case keyDataEAGLE:
    outDataType = "data:eagle";
    break;
  case keyDataSauron:
    outDataType = "data:anc:sauron";
    break;
  default:
    outDataType = "data:unknown";
      break;
  }
  return outDataType;
}

