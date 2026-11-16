#include "CycleServer.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <cstring>
#include <chrono>
#include <random>
#include <climits>

using namespace std;

CycleServer::CycleServer(int cycleMin, int cycleMax) :
  tstThis(0), tstPrev(0), tstStart(0), tstWrite(0), tstStep(0),
  mCount(0), sCount(0), pCount(0), lastWrite(0), lastPrompt(0),
  secMinStep(cycleMin), secMaxStep(cycleMax), secDelay(0), secPrompt(0), myNameWidth(14)   // 8 for GALILEO; 14 for AGATA
{
#ifdef MULTIHIST_SPECTRA
  hGroup = nullptr;
#endif  //MULTIHIST_SPECTRA

  tcpServer = nullptr;
  tcpComInt = ECommand::kNone;
  tcpComExt = ECommand::kNone;
  tcpPortno = 16161;
  tcpExitOK = false;    // kExit command must be enabled explicitely (e.g. for offline analysis)

  // TODO Call it from process_configure, to be able to define portno
  // TcpLaunch(tcpPortno);
  //// start thread with a tcp server listening on port 16161;
  //tcpServer = new std::thread( [=]() { TcpServer(16161); } );
}

CycleServer::~CycleServer() {
  if (tcpServer) {
    delete tcpServer;   // ?? does it really kill the thread ??
    tcpServer = nullptr;
  }
}

void CycleServer::Start(const std::string &name, int pprompt, int cycleMin, int cycleMax)
{
  SetPrompt(name);
  secMinStep = cycleMin;
  secMaxStep = cycleMax;
  secPrompt  = pprompt;
  mRate.reset();
  mCount = sCount = pCount = 0;
  tstStart = tstWrite = tstThis = tstPrev = 0;
  lastPrompt = lastWrite = get_wall_time();
  parsPrompt1 = parsPrompt2 = 0;
}

bool CycleServer::CheckComFile()
{
  //if (sigFlags) {
  //  bool erase = (sigFlags&2) == 1;
  //  ExecLong(false, erase);  // write spectra via SIGBRK or SIGUSR1+SIGUSR2
  //  sigFlags &= ~3;
  //  return true;
  //}

  if( comFile.empty() )
    return false;

  FILE *fp = fopen(comFile.c_str(), "rb");
  if(!fp)
    return false;

  // At the moment just check if it has some data
  // Should be developed to decode command lines
  char cbuf[10];
  size_t nr = fread(cbuf, 1, 2, fp);
  bool erase = false;
  if (nr == 0) {    // erase
    erase = false;
  }
  else if (nr == 1) {
#ifdef MULTIHIST_SPECTRA
    if (hGroup) {
      if ((cbuf[0] == 'e' || cbuf[0] == 'E') && (cbuf[1] == 'n' || cbuf[1] == 'N'))   // enable
        hGroup->enable(true);
      if ((cbuf[0] == 'd' || cbuf[0] == 'D') && (cbuf[1] == 'i' || cbuf[1] == 'I'))   // disable
        hGroup->enable(false);
    }
#endif
  }
  else if (nr == 2) {
    if ((cbuf[0] == 'e' || cbuf[0] == 'E')  && (cbuf[1] == 'r' || cbuf[1] == 'R'))    // erase
       erase = true;
#ifdef MULTIHIST_SPECTRA
    if (hGroup) {
      if ((cbuf[0] == 'e' || cbuf[0] == 'E') && (cbuf[1] == 'n' || cbuf[1] == 'N'))   // enable
        hGroup->enable(true);
      if ((cbuf[0] == 'd' || cbuf[0] == 'D') && (cbuf[1] == 'i' || cbuf[1] == 'I'))   // disable
        hGroup->enable(false);
    }
#endif
  }

  fclose(fp);
  RemoveComFile();

  ExecLong(false, erase);  // write spectra: don't change writing interval; erase the spectra (and reset writing interval)

  return true;
}

bool CycleServer::RemoveComFile()
{
//#if OS_TYPE == OS_WINDOWS
//    int done = _unlink(comFile.c_str());
//#else
//    int done =  unlink(comFile.c_str());
//#endif
  int done = remove(comFile.c_str());
  return (done == 0);
}

void CycleServer::Prompt(int id, int nevs, int bsize, bool isAll)
{
  parsPrompt1 += 1;
  parsPrompt2 += bsize;

  double tnow   = get_wall_time();
  double telaps = tnow - lastPrompt;

  if( (int)telaps >= secPrompt || tcpComInt != ECommand::kNone) { // always if secPrompt == 0
    double drate = (double)(mCount - pCount);
#ifdef GAL_OFFLINE
    drate /= telaps;                  // analysis rate
#else
    drate *= (double)tst1second / (tstThis + 1 - tstPrev);   // acquisistion rate
#endif
    if (tcpComInt != ECommand::kNone) {
      ExecTcpCommand();
    }
    else if (mCount > sCount) {   // if new events have been produced
      CheckComFile();             // presence of comFile forces a write cycle
    }
    if (id >= 0)
      cout << setw(3) << id;
    else
      cout << "   ";
    cout << left << setw(myNameWidth) << myName << right;
    int avsize = int(double(parsPrompt2) / parsPrompt1);   // average size
    cout << " " << setw(6) << nevs << " ev (" << setw(7) << avsize << ")";
    if (parsPrompt1 > 1)
      cout << "*" << left << setw(4) << parsPrompt1 << right;
    else
      cout << "     ";
    cout << " Tot=" << setw(8) << mCount << " " << setw(6) << (int)(drate + 0.5) << "/s";
    if (isAll)
      cout << endl;
    lastPrompt = tnow;
    pCount = mCount;
    tstPrev = tstThis;
    parsPrompt1 = parsPrompt2 = 0;
}
}

void CycleServer::Exec(uint64_t tstNew, int nevts) {
  mCount += nevts;
  tstThis = tstNew;

  if(!tstWrite) {
    // first time only: set up the spectrum-save intervals
    tstStart  = tstThis;
    SetTstStep(tst1second*secMinStep);
    tstWrite  = tstThis + tstStep;
    // to avoid clogging the machine with all actors writing the spectra at the same time,
    // add a (truly) random delay of up to secMinStep to the various actors
#if 1
    std::random_device rdv;
    auto irn = rdv();
    double rrrr = double(rdv()) / double(UINT_MAX); // use the second value of rdv
#else
    double rrrr = get_wall_time();
    rrrr += get_cpu_time() * 12345.;  // increase randomness ??
    rrrr -= floor(rrrr);              // keep fractional part
#endif
    rrrr *= secMinStep;               // scale to secMinStep
    tstWrite += uint64_t(tst1second*rrrr);
    lastWrite = get_wall_time();
  }

  if(tstThis >= tstWrite) {       // time to write
#if NRV_TYPE != NRV_ONLINE
    // As offline analysis can run much faster than the "timestamp seconds" we require a minimum elapsed time
    int elapsedSeconds = int(get_wall_time() - lastWrite);
    if(elapsedSeconds >= secMinStep)
#endif
      ExecLong(true, false);  // write spectra: extend writing interval; do not erase the spectra
  }
}

void CycleServer::ExecLong(bool newStep, bool erase)
{
#ifdef MULTIHIST_SPECTRA
  if(!hGroup)
    return;
#endif  //MULTIHIST_SPECTRA

  if(mCount <= sCount)
    return;             // to avoid re-writing if no new data

#ifdef MULTIHIST_SPECTRA
  bool ok = hGroup->write(erase, true);
  //ok = hGroup->write(erase, true); // just to stress the system
  //ok = hGroup->write(erase, true);
  //ok = hGroup->write(erase, true);
  if( ok ) {
    if(erase)
      SetTstStep(tst1second*secMinStep);  // reset writing interval
    else if(newStep)
      SetTstStep(2*tstStep);              // increase writing interval
    lastWrite = get_wall_time();
  }
#endif  //MULTIHIST_SPECTRA

  sCount   = mCount;
  tstWrite = tstThis + tstStep;
}

void CycleServer::Finish() {
  Flush();  // flush all increment queues
  ExecLong(false, false);
  Remove();
}

void CycleServer::Flush() {
#ifdef MULTIHIST_SPECTRA
  if(hGroup)
    hGroup->flush();
#endif  //MULTIHIST_SPECTRA
}

void CycleServer::Remove() {
#ifdef MULTIHIST_SPECTRA
  if (hGroup)
    hGroup->remove();
#endif  //MULTIHIST_SPECTRA
}

void CycleServer::Save(bool erase) {
   ExecLong(false, erase);
}

void CycleServer::TcpVerify(const char * message)
{
  string com(message);
  cout << "Received message: " << com;
  if ( stringWhere(com, "write") == 0 )
    tcpComInt = ECommand::kSave;
  else if ( stringWhere(com, "erase") == 0 )
    tcpComInt = ECommand::kErase;
  else if ( stringWhere(com, "enable") == 0 )
    tcpComInt = ECommand::kEnable;
  else if ( stringWhere(com, "disable") == 0 )
    tcpComInt = ECommand::kDisable;
  else if ( stringWhere(com, "configure") == 0 )
    tcpComExt = ECommand::kConfigure;
  else if ( tcpExitOK && stringWhere(com, "exit") == 0 )  // if enebled
    tcpComExt = ECommand::kExit;
  else {
    tcpComInt = ECommand::kNone;
    tcpComExt = ECommand::kNone;
    cout << "   <==== UNKNOWN MESSAGE\n\n";
    cout << "Valid messages are: \n";
    cout << "write        ==>  write spectra to disk\n";
    cout << "erase        ==>  erase spectra content\n";
    cout << "enable       ==>  incrementing spectra\n";
    cout << "disable      ==>  incrementing spectra\n";
    cout << "configure    ==>  reload configuration file\n";
    if(tcpExitOK)
      cout << "exit         ==>  stop the program\n";
  }
  cout << endl;
}

bool CycleServer::ExecTcpCommand()
{
  switch (tcpComInt) {
#ifdef MULTIHIST_SPECTRA
  case ECommand::kSave:
    ExecLong(false, false);  // write spectra
    break;
  case ECommand::kErase:
    ExecLong(false, true);   // write spectra, erase the spectra and reset writing interval
    break;
  case ECommand::kEnable:
    if (hGroup)
      hGroup->enable(true);
    break;
  case ECommand::kDisable:
    if (hGroup)
      hGroup->enable(false);
    break;
#endif
  default:
    break;
  }

  tcpComInt = ECommand::kNone;
  return true;
}

bool CycleServer::TcpLaunch(int portno)
{
  // as we don't have (yet?) a mechanism to shut it down, it can only be started once
  if (tcpServer || portno <= 1000 || portno >= 65536)
    return false;

  tcpServer = new std::thread([=]() { TcpServer(portno); });
  return tcpServer != nullptr;
}

#if OS_TYPE == OS_WINDOWS
// Using netlink library to avoid "learning how to do it in Windows"

#include "netlink/socket.h"

void CycleServer::TcpServer(int portno)
{
  NL::init();

  cout << "Starting command server..." << endl;
  NL::Socket server(portno);

  char buffer[256];
  int turn = 0;
  for (; ; turn++) {
    cout << turn << " -- Listening on port " << portno << endl;
    NL::Socket* clientConnection = server.accept();
    memset(buffer, 0, sizeof(buffer));
    while (clientConnection->read(buffer, 255)) {
      TcpVerify(buffer);
      //clientConnection->send("I got your message", 18);
      memset(buffer, 0, sizeof(buffer));
    }
    delete clientConnection;
    //cout << turn << " -- Client disconnected " << endl;
  }

  cout << "Served " << turn << " turns " << endl;

}

#else
// netlink is multi-platform but it is better to code it directly
// in order to remove that dependency for the "normal" linux case

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void CycleServer::TcpServer(int portno)
{
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    cout << "ERROR opening socket" << endl;
    return;
  }

  sockaddr_in serv_addr;
  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);
  if (::bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    cout << "ERROR on binding" << endl;
    return;
  }

  listen(sockfd, 1);

  struct sockaddr_in cli_addr;
  socklen_t cli_len = sizeof(cli_addr);

  char buffer[256];
  int turn = 0;
  for (; ; turn++) {
    cout << turn << " -- Listening on port " << portno << endl;

    int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &cli_len);
    if (newsockfd < 0) {
      cout << "ERROR on accept" << endl;
      return;
    }
    memset(buffer, 0, sizeof(buffer));
    while (int nr = read(newsockfd, buffer, 255)) {
      if (nr < 0) {
        cout << "ERROR reading from socket" << endl;
        return;
      }
      TcpVerify(buffer);
      //cout << "Message: " << buffer << endl;
      //int nw = write(newsockfd, "I got your message", 18);
      //if (nw < 0) {
      //  cout << "ERROR writing to socket" << endl;
      //  return;
      //}
      memset(buffer, 0, sizeof(buffer));
    }
    close(newsockfd);
    //cout << turn << " -- Client disconnected " << endl;
  }

  close(sockfd);
  return;
}

#endif
