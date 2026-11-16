/* SAURON_calib.C 24/11/23

Macro fro Silicon detector calibration. 

*/

#include <iostream>
#include <ctime>
#include "TH1.h"
#include "TH2.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TSpectrum.h"
#include "TLegend.h"
#include "TCollection.h"
#include "TObject.h"
#include "TTree.h"
#include "TBranch.h"
#include "TList.h"
#include "TRandom.h"
#include "TROOT.h"
#include "TSystem.h"
#include "TGInputDialog.h"
#include "TTree.h"
#include "TFile.h"
#include "TGraph.h"
#include <random>
#include <vector>
#include <fstream>
#include <string>
#include <dirent.h>
#include <errno.h>
#include <algorithm>
#include <iomanip>
#include "hmaker.cxx"

using namespace std;

char path[] = "./SAURON/results/";  // <------------------- 

const int num_peaks = 3;
const int nboards = 2; // number of boards
const int nchannels = 64; // ch in boards

int xmin = 0;
int xmax = 4000;
int chnmin = 0;
int chnmax = 4000;
bool writetofile = true;
bool fitpol2 = false;    // to calibrate using a pol2 (default is pol1)
string alpha = "3alpha";
char dummyText[100];
vector<vector<double>> PeakPositions;
vector<double> peaks;
FILE *fPeakSearch, *fGainResults, *fCalibFile, *fRes;

// Colors for error messages
string default_console = "\033[0m";
string color = "\033[1;31m";
string colorG = "\033[1;32m";
string colorC = "\033[1;36m";
TCanvas *calC = nullptr;

std::time_t t = std::time(0);   // get time now
std::tm* now = std::localtime(&t);
int yrr = now->tm_year + 1900;
int mntt = now->tm_mon + 1;
int ddayy = now->tm_mday;


void create_histo(char *dataFile); 
void orderpeaks(double *xp, int n);
void peak_finder(char *rootSpec, int detnum, int stripnum, int rebin = 1, bool fitSatellite = false,bool fitpol2 = false);
void calibrate(vector<vector<double>> channel, vector<vector<int>> strip_id,int n);
void calibFromFile(int n);
void fitSimple(char *rootSpec, int detnum, int stripnum, int rebin = 1, bool fitSatellite = false,bool fitpol2 = false);
void fitAll(char *rootSpec, int detnum, int rebin = 1, bool fitSatellite = false,bool fitpol2 = false);
void showEne(char *rootSpectra);
  

void create_histo(char *dataFile){

  hmaker(dataFile,path);

}


void orderpeaks(double *xp, int n){

  vector<double> a;
  for(int k = 0; k<n; k++) a.push_back(xp[k]);
  sort(a.begin(),a.end());
  for(int k=0; k<n;k++){
    xp[k] = a.at(k);
  }
  // for(auto b:a) cout<<b<<endl;
  // for(int k=0; k<n;k++) cout<<xp[k]<<endl;
  // return xp;
}

void peak_finder(char *rootSpec, int detnum, int stripnum, int rebin = 1, bool fitSatellite = false, bool fitpol2 = false){


  Double_t *xpeaks;
  int min =0, max=0;
  TH2F **hh;
  TH1D *hp;// = new TH1D*[nboards];
  TFile *rootIn = new TFile(rootSpec,"READ");
  TCanvas *fCan=new TCanvas("Can",Form("SAURON Board %i channel %i Calibration",detnum,stripnum),1000,800);
  TSpectrum *s;  
  
  hh = nullptr;
  hp = nullptr;
  

  
  /*    O p e n  T h e  S p e c t r a    */
  
  // From previous saved spectra

    if(rootIn==nullptr) {
      cout<<color<< "Error: Cannot open SAURON spectra "<<rootSpec << std::endl;
      cout<<default_console<<endl;
      return;
    }
    else
      printf("File %s opened\n",rootSpec);
    
    
    printf("\n///////////////////////////////\n");
    printf("Board %d Strip %d\n\n",detnum,stripnum);
    
    
    // S o m e  N a m e s
    // -----------------------------------------------------
    
    char dummyRes[100];
    char dummyText[100];

    sprintf(dummyText,"%s/CentroidSAURON_Board_%d_%s_%d%d%d.txt",path,detnum,alpha.c_str(),ddayy,mntt,yrr);
    sprintf(dummyRes,"%s/ResolutionSAURON_Board%d_%s_%d%d%d.txt",path,detnum,alpha.c_str(),ddayy,mntt,yrr);
    
    //-----------------------------------------------------
    
    
    
    /*  S e a r c h i n g   P e a k s  */
    
    
    hh = new TH2F*[nboards];
    for(int g = 0; g<nboards+1; g++)
      hh[g] = (TH2F*)rootIn->Get(Form("h%i",g));
    
    fCan->cd();
    hp = (TH1D*)hh[detnum]->ProjectionX(Form("hp%i[%i]",detnum,stripnum),stripnum+1,stripnum+1,"d");
    
    if(hp->Integral() < 10){
      cout<<color<<"Board "<<detnum<<" channel "<<stripnum<<" seems empty. I'll skip it"<<endl;
      cout<<default_console<<endl;
      return;  
    }

    hp->Rebin(rebin);
    
    vector<double> Data, height;
    
    hp->GetXaxis()->SetRangeUser(xmin,xmax);
    
    s = new TSpectrum(2*num_peaks);
    Int_t nfound = s->Search(hp,2,"",0.3);
    xpeaks = s->GetPositionX();
    orderpeaks(xpeaks,nfound);
    
    gPad->Modified(); gPad->Update();
    printf("Found %d candidate peaks to fit\n",nfound);
    printf("Peak\tPosition\n");
    
    for(int j =0; j<nfound; j++) 
      printf("%d\t%f\n",j+1,xpeaks[j]);
    
    gPad->WaitPrimitive();
    /*cout<<"\nHappy?\nType Y/y or N/n"<<endl;
    string answ;
    getline(cin,answ);
    */
    int ncounts = 0;
    while(nfound != num_peaks){
      
      if(ncounts==5) break;

      cout<<"Enter sigma and threshold"<<endl;
      int sig;
      double thr;
      //   std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      // std::cin.clear();
      cin>>sig>>thr;
      cout<<endl;
      
      nfound = s->Search(hp,sig,"",thr);
      xpeaks = s->GetPositionX();
      orderpeaks(xpeaks,nfound);
      
      gPad->Modified(); gPad->Update();
      printf("Found %d candidate peaks to fit\n",nfound);
      printf("Peak\tPosition\n");
      
      ncounts++;
      for(int j =0; j<nfound; j++) 
	  printf("%d\t%f\n",j+1,xpeaks[j]);
    
    }
    
    
    /*__________ F i t   P e a k s _________*/
    
    writetofile = false;
    
    for(int i=0; i<nfound; i++)
      {
	Data.push_back( *(xpeaks+i) );
      }

    TF1 *f;
    double rangeL[] = {0.03,0.03,0.03};
    double rangeH[] = {0.03,0.03,0.04};
    if(fitSatellite){
      for(int dd=0; dd<num_peaks; dd++){
	rangeL[dd] = 0.022;
	rangeH[dd] = 0.02;
      }
    }
    vector<double> sigma;
    vector<double> peaks;
    vector<double> integral;
    
    cout<<"\nRanges for fitting are:";
    cout<<"\nLeft\tRight"<<endl;
    for(int w = 0; w<num_peaks; w++)
      cout<<rangeL[w]<<"\t"<<rangeH[w]<<endl;

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.clear();
    cout<<"Type Y/y to use default ranges or N/n to insert yours"<<endl;
    char answ = cin.get();
    if(answ != 'Y' && answ != 'y'){
      cout<<"Enter rangeL/rangeR"<<endl;
      double rngL, rngR;
      int count = 0;
      for(int w = 0; w<num_peaks; w++){
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	std::cin.clear();
	cin>>rngL>>rngR;
	rangeL[w] = rngL;
	rangeH[w] = rngR;
	count++;
      }
      cout<<endl;
    }

    cout<<"\nNew ranges for fitting are:";
    cout<<"\nLeft\tRight"<<endl;
    for(int w = 0; w<num_peaks; w++)
      cout<<rangeL[w]<<"\t"<<rangeH[w]<<endl;
    
    if(!Data.empty()){
      sort( Data.begin(), Data.end() ); // ?? useless
      
      for(int i=0; i<Data.size(); i++)
	{
	  int counts = hp->GetBinContent(Data[i]);
	  height.push_back(counts);
	}
      
      
      printf("*********************\n");
      printf("Gaussian Fit\n");
      
      if(!fitSatellite)
	for(int i = 0; i<Data.size(); i++){
	  
	  f = new TF1("f", "gaus(0)+pol0(3)", Data[i]-rangeL[i]*Data[0], Data[i]+rangeH[i]*Data[0]);
	  f->SetParameter(0,height[i]);  
	  f->SetParameter(1,Data[i]);  
	  f->SetParameter(2,8);
	  f->SetParameter(3,0);
	  f->SetParLimits(1,0.85*Data[i],1.15*Data[i]);
	  f->SetParLimits(2,1,20);
	  hp->Fit("f","SLRQ+");
	  
	  peaks.push_back(f->GetParameter(1));
	  sigma.push_back(f->GetParameter(2));
	  integral.push_back(f->GetParameter(0));
	  printf("%f\n",f->GetParameter(1));
	  delete f;
	}
    
      else{
	
	for(int i = 0; i<Data.size(); i++){
	  
	  f = new TF1("f", "gaus(0)+gaus(3)+pol0(6)", Data[i]-rangeL[i]*Data[0], Data[i]+rangeH[i]*Data[0]);
	  f->SetParameter(0,0.9*height[i]);  
	  f->SetParameter(1,Data[i]);  
	  f->SetParameter(2,6); 
	  f->SetParameter(3,0.5*height[i]);  
	  f->SetParameter(4,0.99*Data[i]);  
	  f->SetParameter(5,8);  
	  if(i==2){
	    f->SetParameter(2,6);
	    f->SetParameter(5,9);
	  }
	  if(i==1){
	    f->SetParameter(2,5);
	    f->SetParameter(5,8);
	  }
	  f->SetParameter(6,0);
	  f->SetParLimits(1,0.999*Data[i],1.15*Data[i]);
	  f->SetParLimits(4,0.95*Data[i],0.998*Data[i]);
	  f->SetParLimits(3,0.1*height[i],10*height[i]);
	  f->SetParLimits(2,2,13);
	  f->SetParLimits(5,2,10);
	  if(i==2) f->SetParLimits(5,2,12);
	  hp->Fit("f","SLRQ+");

	  TF1 *fg1 = new TF1("fg1", "gaus(0)", Data[i]-rangeL[i]*Data[0], Data[i]+rangeH[i]*Data[0]);
	  TF1 *fg2 = new TF1("fg2", "gaus(0)", Data[i]-rangeL[i]*Data[0], Data[i]+rangeH[i]*Data[0]);
	  fg1->SetParameter(0,f->GetParameter(0));  
	  fg1->SetParameter(1,f->GetParameter(1));  
	  fg1->SetParameter(2,f->GetParameter(2)); 
	  fg2->SetParameter(0,f->GetParameter(3));  
	  fg2->SetParameter(1,f->GetParameter(4));  
	  fg2->SetParameter(2,f->GetParameter(5)); 

	  fg1->SetLineColor(kBlue);
	  fg2->SetLineColor(kGreen+3);

	  fg1->Draw("same");
	  fg2->Draw("same");
	  
	  peaks.push_back(f->GetParameter(1));
	  sigma.push_back(f->GetParameter(2));
	  integral.push_back(f->GetParameter(0));
	  printf("%f\n",f->GetParameter(1));
	  delete f;
	}
	
      }
      
    gPad->WaitPrimitive();
    
    }// !Data.empty()
    
    Data.clear();
    
    cout<<"\nWrite the resuts?\nType Y/y or N/n"<<endl;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.clear();
    answ = cin.get();
    if( answ == 'Y' || answ == 'y') writetofile = true;
    else cout<<"\nBye!"<<endl;
    
    if(writetofile){
      
      
      if(!gSystem->IsFileInIncludePath(dummyText))
	fPeakSearch=fopen(dummyText,"w");
      else fPeakSearch=fopen(dummyText,"a");
      if(!gSystem->IsFileInIncludePath(dummyRes))
	fRes=fopen(dummyRes,"w");
      else fRes=fopen(dummyRes,"a");
      
      fprintf(fPeakSearch,"%d ",stripnum);
      for (auto z : peaks) fprintf(fPeakSearch,"%f ",z);
      fprintf(fPeakSearch,"\n");
      
      
      fprintf(fRes,"%d ",stripnum);
      int cnt =0;
      for (auto z : sigma){
	fprintf(fRes,"%f %f\t",z,2.355*z*100./peaks.at(cnt));
	cnt++;
      }
      fprintf(fRes,"\n");
      
      //  fCan->Print(canName);
      fclose(fPeakSearch);
      fclose(fRes);
      
      
      /*     C a l i b r a t i o n     */
      /* ----------------------------- */
      
      TF1 *fcal;
      fcal = nullptr;
      double p0, p1;
      char Dummy[100];
      sprintf(Dummy,"%s/CalibSAURON_Board_%d_%s_%d%d%d.txt",path,detnum,alpha.c_str(),ddayy,mntt,yrr);
      if(!gSystem->IsFileInIncludePath(Dummy))
	      fCalibFile=fopen(Dummy,"w");
      else fCalibFile=fopen(Dummy,"a");
      
      
      const int dim = peaks.size();
      double chn[dim];
      for(int k =0; k< dim; k++)
	chn[k] = peaks.at(k);
      
      double energy[] = {5.15659,5.48556,5.80477};
      
      TGraph *g = new TGraph(dim,chn,energy);
      g->SetTitle(Form("Det %i channel % i",detnum,stripnum));
      if(fitpol2) fcal = new TF1("fcal","pol2",chnmin,chnmax);
      else fcal = new TF1("fcal","pol1",chnmin,chnmax);
      calC = new TCanvas("calC","Calibration");
      calC->cd();
      g->SetMarkerStyle(31);
      g->SetMarkerColor(kBlue);
      g->Draw("AP");
      g->Fit("fcal","QPSR");
      gPad->Modified();
      calC->Update();
      
      double chisq;
      chisq = fcal->GetChisquare();
      printf("ChiSq\n");
      printf("%f\n",chisq);
      
      
      if(fitpol2)
	fprintf(fCalibFile,"%d %f %f %f\n",stripnum,fcal->GetParameter(0),fcal->GetParameter(1),fcal->GetParameter(2));
      
      else
	fprintf(fCalibFile,"%d %f %f\n",stripnum,fcal->GetParameter(0),fcal->GetParameter(1));
      
      
      
      fclose(fCalibFile);
      
      cout<<colorC<<"Writing to file "<<Dummy<<endl;
      cout<<default_console<<endl;
      
      // delete fcal;
      // delete calC;
      
      delete s;      
      delete hp;
      delete fCan;
      // delete calC;
    } // if(writetofile)
    
    integral.clear();
    peaks.clear();
    sigma.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.clear();
    
}
  



void calibrate(vector<vector<double>> channel, vector<vector<int>> strip_id, int n){
  
  fitpol2 = false;
  TF1 *fcal;
  double p0, p1;
  char Dummy[100];
  sprintf(Dummy,"%s/CalibSAURON_Board_%d_%s_%d%d%d.txt",path,n,alpha.c_str(),ddayy,mntt,yrr);

  if(!gSystem->IsFileInIncludePath(dummyText))
    fCalibFile=fopen(Dummy,"w");
  else fCalibFile=fopen(Dummy,"a");
  
  if(channel.empty()){
    cerr<<color<<"Could not find centroids for calibration... Please check input vector\n"<<endl;
    cout<<default_console<<endl;
    fclose(fCalibFile);
    return;
  }
  
  for(int r = 0; r < channel.size(); r++){

    int numstrip;
    if(!channel.at(r).empty()) numstrip = strip_id.at(r).at(0);
    else numstrip = -999;
    
    if(channel.at(r).size()!=3){
      
      cout<<color<<"---------->\n";
      cout<<"Board "<<n<<" channel "<<numstrip<<" shows a strange number of peaks (try to change the rangeL, maybe the satellite was fitted too).\n  Set the calibration parameters to 0 1"<<endl;
      cout<<default_console<<endl;
      fprintf(fCalibFile,"%d %f %f\n",numstrip,0.,1.);
      continue;
      
    }
    const int dim = channel.at(r).size();
    double chn[dim];
    for(int k =0; k< dim; k++)
      chn[k] = channel.at(r).at(k);

    double energy[] = {5.15659,5.48556,5.80477};
    TGraph *g = new TGraph(3,chn,energy);
    g->SetTitle(Form("channel #%i, Det %i",numstrip,n));
    if(fitpol2) fcal = new TF1("fcal","pol2",chnmin,chnmax);
    else fcal = new TF1("fcal","pol1",chnmin,chnmax);
    if(!calC) calC = new TCanvas("calC","Calibration");
    calC->cd();
    g->SetMarkerStyle(31);
    g->SetMarkerColor(kBlue);
    g->Draw("AP");
    g->Fit("fcal","QPSR");
    gPad->Modified();
    calC->Update();
    
    double chisq;
    
    chisq = fcal->GetChisquare();
    printf("ChiSq\n");
    
    if(fitpol2){
      fprintf(fCalibFile,"%d %f %f %f\n",numstrip,fcal->GetParameter(0),fcal->GetParameter(1),fcal->GetParameter(2));
    }
    else{
      
      fprintf(fCalibFile,"%d %f %f\n",numstrip,fcal->GetParameter(0),fcal->GetParameter(1));
      
    }
    
    
    delete fcal;
  }
  
  cout<<colorC<<"Writing to file "<<Dummy<<endl;
  cout<<default_console<<endl;
  fclose(fCalibFile);
}



void calibFromFile(int n){

  ifstream fIn;
  int id = -999;
  double c1=0, c2=0, c3=0;
  char filein[100];
  
  sprintf(filein,"%s/CentroidSAURON_Board_%d_%s_%d%d%d.txt",path,n,alpha.c_str(),ddayy,mntt,yrr);

  fIn.open(filein);
    
  
  vector<vector<double>> calibCentr;
  vector<double> calib;
  vector<vector<int>> strip_id;
  vector<int> strips;
  while(fIn>>id>>c1>>c2>>c3){
    calib.push_back(c1);
    calib.push_back(c2);
    calib.push_back(c3);
    strips.push_back(id);
    
    if(!calib.empty() && !strips.empty()){
      sort(calib.begin(),calib.end());
      calibCentr.push_back(calib);
      strip_id.push_back(strips);
      calib.clear();
      strips.clear();
    }
  }
  if(!calibCentr.empty() && !strip_id.empty()){
    printf("---------------------\n");
    printf("Calibration Board %d\n",n);
    calibrate(calibCentr,strip_id,n);
  }
  else{
    printf("---------------------\n");
    printf("Skipping calibration of empty Board %d\n",n);
  }

  
  fIn.close();

}

void fitSimple(char *rootSpec, int detnum, int stripnum, int rebin = 1, bool fitSatellite = false, bool fitpol2 = false){


  Double_t *xpeaks;
  int min =0, max=0;
  TH2F **hh;
  TH1D *hp;// = new TH1D*[nboards];
  TFile *rootIn = new TFile(rootSpec,"READ");
  TCanvas *fCan=new TCanvas("Can",Form("SAURON Board %i strip %i Calibration",detnum,stripnum),1000,800);
  TSpectrum *s;  
  
  hh = nullptr;
  hp = nullptr;
  

  
  /*    O p e n  T h e  S p e c t r a    */
  
  // From previous saved spectra

    if(rootIn==nullptr) {
      cout<<color<< "Error: Cannot open SAURON spectra "<<rootSpec << std::endl;
      cout<<default_console<<endl;
      return;
    }
    else
      printf("File %s opened\n",rootSpec);
    
    
    printf("\n///////////////////////////////\n");
    printf("Board %d channel %d\n\n",detnum,stripnum);
    
    
    // S o m e  N a m e s
    // -----------------------------------------------------
    
    char dummyRes[100];
    char dummyText[100];

    sprintf(dummyText,"%s/CentroidSAURON_Board_%d_%s_%d%d%d.txt",path,detnum,alpha.c_str(),ddayy,mntt,yrr);
    sprintf(dummyRes,"%s/ResolutionSAURON_Board%d_%s_%d%d%d.txt",path,detnum,alpha.c_str(),ddayy,mntt,yrr);
    
    //-----------------------------------------------------
    
    
    
    /*  S e a r c h i n g   P e a k s  */
    
    
    hh = new TH2F*[nboards];
    for(int g = 0; g<nboards+1; g++)
      hh[g] = (TH2F*)rootIn->Get(Form("h%i",g));
    
    fCan->cd();
    hp = (TH1D*)hh[detnum]->ProjectionX(Form("hp%i[%i]",detnum,stripnum),stripnum+1,stripnum+1,"d");
    hp->SetName(Form("h%i",stripnum));

    if(hp->Integral() < 10){
      cout<<color<<"Board "<<detnum<<" channel "<<stripnum<<" seems empty. I'll skip it"<<endl;
      cout<<default_console<<endl;
      return;  
    }

    hp->Rebin(rebin);
    
    vector<double> Data, height;
    
    hp->GetXaxis()->SetRangeUser(xmin,xmax);
    
    s = new TSpectrum(2*num_peaks);
    Int_t nfound = s->Search(hp,2,"",0.3);
    xpeaks = s->GetPositionX();
    orderpeaks(xpeaks,nfound);
    
    gPad->Modified(); gPad->Update();
    printf("Found %d candidate peaks to fit\n",nfound);
    printf("Peak\tPosition\n");
    
    for(int j =0; j<nfound; j++) 
      printf("%d\t%f\n",j+1,xpeaks[j]);
    
    if(nfound!=num_peaks){
      cout<<color<<"Attention!!\nFound "<<nfound<<" peaks instead of "<<num_peaks<<endl;
      cout<<default_console<<endl;
    }

    
    /*__________ F i t   P e a k s _________*/
    
    writetofile = false;
    
    for(int i=0; i<nfound; i++)
      {
    	Data.push_back( *(xpeaks+i) );
      }

    TF1 *f;
    double rangeL[] = {0.03,0.03,0.03};
    double rangeH[] = {0.03,0.03,0.04};
    if(fitSatellite){
      for(int dd=0; dd<num_peaks; dd++){
	  rangeL[dd] = 0.022;
	  rangeH[dd] = 0.02;
      }
    }
    vector<double> sigma;
    vector<double> peaks;
    vector<double> integral;
    
    cout<<"\nRanges for fitting are:";
    cout<<"\nLeft\tRight"<<endl;
    for(int w = 0; w<num_peaks; w++)
      cout<<rangeL[w]<<"\t"<<rangeH[w]<<endl;


    if(!Data.empty()){
      sort( Data.begin(), Data.end() ); // ?? useless
      
      for(int i=0; i<Data.size(); i++)
	{
	  int counts = hp->GetBinContent(Data[i]);
	  height.push_back(counts);
	}
      
      
      printf("*********************\n");
      printf("Gaussian Fit\n");
      
      if(!fitSatellite)
	for(int i = 0; i<Data.size(); i++){
	  
	  f = new TF1("f", "gaus(0)+pol0(3)", Data[i]-rangeL[i]*Data[0], Data[i]+rangeH[i]*Data[0]);
	  f->SetParameter(0,height[i]);  
	  f->SetParameter(1,Data[i]);  
	  f->SetParameter(2,8);
	  f->SetParameter(3,0);
	  f->SetParLimits(1,0.85*Data[i],1.15*Data[i]);
	  f->SetParLimits(2,1,20);
	  hp->Fit("f","SLRQ+");
	  
	  peaks.push_back(f->GetParameter(1));
	  sigma.push_back(f->GetParameter(2));
	  integral.push_back(f->GetParameter(0));
	  printf("%f\n",f->GetParameter(1));
	  delete f;
	}
    
      else{
	
	for(int i = 0; i<Data.size(); i++){
	  
	  f = new TF1("f", "gaus(0)+gaus(3)+pol0(6)", Data[i]-rangeL[i]*Data[0], Data[i]+rangeH[i]*Data[0]);
	  f->SetParameter(0,0.9*height[i]);  
	  f->SetParameter(1,Data[i]);  
	  f->SetParameter(2,6); 
	  f->SetParameter(3,0.5*height[i]);  
	  f->SetParameter(4,0.99*Data[i]);  
	  f->SetParameter(5,8);  
	  if(i==2){
	    f->SetParameter(2,6);
	    f->SetParameter(5,9);
	  }
	  if(i==1){
	    f->SetParameter(2,5);
	    f->SetParameter(5,8);
	  }
	  f->SetParameter(6,0);
	  f->SetParLimits(1,0.999*Data[i],1.15*Data[i]);
	  f->SetParLimits(4,0.95*Data[i],0.998*Data[i]);
	  f->SetParLimits(3,0.1*height[i],10*height[i]);
	  f->SetParLimits(2,2,13);
	  f->SetParLimits(5,2,10);
	  if(i==2) f->SetParLimits(5,2,12);
	  hp->Fit("f","SLRQ+");

	  TF1 *fg1 = new TF1("fg1", "gaus(0)", Data[i]-rangeL[i]*Data[0], Data[i]+rangeH[i]*Data[0]);
	  TF1 *fg2 = new TF1("fg2", "gaus(0)", Data[i]-rangeL[i]*Data[0], Data[i]+rangeH[i]*Data[0]);
	  fg1->SetParameter(0,f->GetParameter(0));  
	  fg1->SetParameter(1,f->GetParameter(1));  
	  fg1->SetParameter(2,f->GetParameter(2)); 
	  fg2->SetParameter(0,f->GetParameter(3));  
	  fg2->SetParameter(1,f->GetParameter(4));  
	  fg2->SetParameter(2,f->GetParameter(5)); 

	  fg1->SetLineColor(kBlue);
	  fg2->SetLineColor(kGreen+3);

	  fg1->Draw("same");
	  fg2->Draw("same");
	  
	  peaks.push_back(f->GetParameter(1));
	  sigma.push_back(f->GetParameter(2));
	  integral.push_back(f->GetParameter(0));
	  printf("%f\n",f->GetParameter(1));
	  delete f;
	}
	
      }
      
    }// !Data.empty()
    
    Data.clear();
      
      
      if(!gSystem->IsFileInIncludePath(dummyText))
	      fPeakSearch=fopen(dummyText,"w");
      else fPeakSearch=fopen(dummyText,"a");
      if(!gSystem->IsFileInIncludePath(dummyRes))
	      fRes=fopen(dummyRes,"w");
      else fRes=fopen(dummyRes,"a");
      
      fprintf(fPeakSearch,"%d ",stripnum);
      for (auto z : peaks) fprintf(fPeakSearch,"%f ",z);
      fprintf(fPeakSearch,"\n");
      
      
      fprintf(fRes,"%d ",stripnum);
      int cnt =0;
      for (auto z : sigma){
	    fprintf(fRes,"%f %f\t",z,2.355*z*100./peaks.at(cnt));
	    cnt++;
      }
      fprintf(fRes,"\n");
      
      //  fCan->Print(canName);
      fclose(fPeakSearch);
      fclose(fRes);
      
      
      /*     C a l i b r a t i o n     */
      /* ----------------------------- */
      
      TF1 *fcal;
      fcal = nullptr;
      double p0, p1;
      char Dummy[100];
      sprintf(Dummy,"%s/CalibSAURON_Board_%d_%s_%d%d%d.txt",path,detnum,alpha.c_str(),ddayy,mntt,yrr);
      if(!gSystem->IsFileInIncludePath(Dummy))
	      fCalibFile=fopen(Dummy,"w");
      else fCalibFile=fopen(Dummy,"a");
      
      
      const int dim = peaks.size();
      double chn[dim];
      for(int k =0; k< dim; k++)
	      chn[k] = peaks.at(k);
      
      double energy[] = {5.15659,5.48556,5.80477};
      
      TGraph *g = new TGraph(dim,chn,energy);
      g->SetTitle(Form("Det %i channel % i",detnum,stripnum));
      if(fitpol2) fcal = new TF1("fcal","pol2",chnmin,chnmax);
      else fcal = new TF1("fcal","pol1",chnmin,chnmax);
      calC = new TCanvas("calC","Calibration");
      calC->cd();
      g->SetMarkerStyle(31);
      g->SetMarkerColor(kBlue);
      g->Draw("AP");
      g->Fit("fcal","QPSR");
      gPad->Modified();
      calC->Update();
      
      double chisq;
      chisq = fcal->GetChisquare();
      printf("ChiSq\n");
      printf("%f\n",chisq);
      
      
      if(fitpol2)
  	fprintf(fCalibFile,"%d %f %f %f\n",stripnum,fcal->GetParameter(0),fcal->GetParameter(1),fcal->GetParameter(2));
      
      else
  	fprintf(fCalibFile,"%d %f %f\n",stripnum,fcal->GetParameter(0),fcal->GetParameter(1));
      
      
      
      fclose(fCalibFile);
      
      cout<<colorC<<"Writing to file "<<Dummy<<endl;
      cout<<default_console<<endl;
      
  /* Output Spectra */
      TFile *rootFileOut = new TFile(Form("%s/CalibratedSpectra_Board_%d_%s_%d%d%d.root",path,detnum,alpha.c_str(),ddayy,mntt,yrr),"UPDATE"); 
      rootFileOut->cd();
      hp->Write();
      calC->Write();
      rootFileOut->Close();


      delete s;      
      delete hp;
      delete fCan;
      // delete calC;
    
    integral.clear();
    peaks.clear();
    sigma.clear();
    
}
  


void fitAll(char *rootSpec, int detnum, int rebin = 1, bool fitSatellite = false,bool fitpol2 = false){
  
for(int k=0; k<nchannels; k++){

fitSimple(rootSpec, detnum,k,rebin,fitSatellite,fitpol2);

}


}