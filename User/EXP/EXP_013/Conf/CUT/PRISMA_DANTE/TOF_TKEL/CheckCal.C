#include <iostream>
#include <fstream>
#include <string>

using namespace std;


//---------------------------------------------------------------------------------|
//For the USER
void TCutToBan(string cutFileName, string banFileName);
//example: TCutToBan("cutName.root","banName.ban")

void TCutToSpli(string cutFileName, string spliFileName, double val);
//example: TCutToBan("cutName.root","spliName.spli",250)

void TCutToSpli(std::vector<string> cutFileName, string spliFileName, std::vector<double> val);
//example: TCutToBan({"cutName1.root","cutName2.root","cutName3.root"},"spliName.spli",{250,260,270})

void DrawMCPraw(string inFileName, string banFileName, bool selFlag = true, int nrEvts = -1);
//example: DrawMCPraw("treeName.root","banName.ban",false,1e6)
//example: DrawMCPraw("outName.root","banName.ban")

void DrawMCPana(string inFileName, bool selFlag = true, int nrEvts = -1);
//example: DrawMCPana("treeName.root",false,1e6)
//example: DrawMCPana("outName.root")

void DrawMCPanaNew(string inFileName, bool selFlag = true, int nrEvts = -1);
//example: DrawMCPana("treeName.root",false,1e6)
//example: DrawMCPana("outName.root")

void DrawPPAC_Right(string inFileName, string thrFileName, int nrEvts = -1);
void DrawPPAC_Left(string inFileName, string thrFileName, int nrEvts = -1);
void DrawPPAC_Left_sec(string inFileName, string thrFileName, int nrEvts = -1);
void DrawPPAC_Cath(string inFileName, string thrFileName, int nrEvts = -1);
void DrawPPAC_Cath_sec(string inFileName, string thrFileName, int nrEvts = -1);
//example: DrawPPAC_Right("treeName.root","thrName.thres",false,1e6)

void DrawPPAC_Cuts (string inFileName, string banFileName, bool selFlag = true, int nrEvts = -1);
//example: DrawPPAC_Cuts("treeName.root","banName.ban",false,1e6)
//example: DrawPPAC_Cuts("outName.root","banName.ban")

void DrawPPAC_Cuts_Section (string inFileName, string banFileName, bool selFlag = true, int nrEvts = -1);


void DrawPPAC_XfpCal (string inFileName, int nrEvts = -1);
//example: DrawPPAC_Cuts("treeName.root",1e6)

void DrawPPAC_XrCal (string inFileName, string calFileName, bool selFlag = true, int nrEvts = -1);
void DrawPPAC_XlCal (string inFileName, string calFileName, bool selFlag = true, int nrEvts = -1);
//example: DrawPPAC_XrCal("treeName.root","calName.cal",false,1e6)
//example: DrawPPAC_XrCal("outName.root","calName.cal")

void DrawToFs (string inFileName, string thrFileName, int nrEvts = -1);
//example: DrawToFs("treeName.root","thrName.cal",1e6)

void DrawSidePads (string inFileName, string thrFilePattern, bool selFlag = true, int nrEvts = -1);
void DrawIonChPads (string inFileName, string thrFilePattern, bool selFlag = true, int nrEvts = -1);
//example: DrawSidePads("treeName.root","thrPatt_",false,1e6)
//example: DrawSidePads("outName.root","thrPatt_")

void DrawZedA  (string inFileName, string banFilePattern, int Zmin, int Zmax, bool selFlag = true, int nrEvts = -1);
void DrawZedAB (string inFileName, string banFilePattern, int Zmin, int Zmax, bool selFlag = true, int nrEvts = -1);
//example: DrawZedA("treeName.root","banPatt_",12,16,false,1e6)
//example: DrawZedA("outName.root","banPatt_",12,16)

void DrawCharge (string inFileName, string banFilePattern, int Z, int qmin, int qmax, bool selFlag = true, int nrEvts = -1);
//example: DrawCharge("treeName.root","banPatt_",16,13,15,false,1e6)
//example: DrawCharge("outName.root","banPatt_",16,13,15)

const string defTreeName = 	"Runs/run_3000/Tree_0000.root";
const string defOutName = 	"Out/SumThick.root";
void DrawAll(string inTreeName = defTreeName, string inOutName = defOutName);
//---------------------------------------------------------------------------------|

		
//---------------------------------------------------------------------------------|
//Internal functions (NOT FOR THE USER)
TCutG* GetCut(string cutFileName, string cutName, string var1, string var2);
void GetThr(string thrFileName, vector<double>& lower, vector<double>& upper);
void GetCal(string calFileName, vector<double>& As, vector<double>& Bs);
TFile* GetFile(string inFileName);
TLine* GetRedLine();
void DrawPPAC_Signal(string inFileName, string thrFileName, string variable, int nrEvts = -1);
void DrawPPAC_Signal_sec(string inFileName, string thrFileName, string variable, int nrEvts = -1);
//---------------------------------------------------------------------------------|
void DrawAll(string inTreeName = defTreeName, string inOutName = defOutName){
	DrawMCPraw      (inOutName,"PrismaConf/ban/mcp_banana.ban");
	DrawMCPana      (inOutName);
	DrawToFs		(inTreeName,"PrismaConf/threshold/tof.thres",1e6);
	DrawPPAC_Right  (inTreeName,"PrismaConf/threshold/x_right.thres",1e6);
	DrawPPAC_Left   (inTreeName,"PrismaConf/threshold/x_left.thres",1e6);
	DrawPPAC_Cath   (inTreeName,"PrismaConf/threshold/x_cathode.thres",1e6);
	DrawPPAC_Cuts   (inOutName,"PrismaConf/ban/ppac_banana.ban");
	DrawPPAC_XfpCal (inTreeName,1e6);
	DrawPPAC_XrCal  (inOutName,"PrismaConf/cal/right-cath.cal");
	DrawPPAC_XlCal  (inOutName,"PrismaConf/cal/cath-left.cal");
	DrawSidePads    (inOutName,"PrismaConf/threshold/side_");
	DrawIonChPads   (inOutName,"PrismaConf/threshold/IC_");
	DrawZedAB       (inOutName,"PrismaConf/ban/zed_ban_deab_",12,16);
	//DrawCharge		(inOutName,"PrismaConf/ban/charge_55deg_ban_",16,12,16);
	//DrawCharge		(inOutName,"PrismaConf/ban/charge_55deg_ban_",15,12,15);
	//DrawCharge		(inOutName,"PrismaConf/ban/charge_55deg_ban_",15,12,15);
	//DrawCharge		(inOutName,"PrismaConf/ban/charge_55deg_ban_",14,11,14);
	//DrawCharge		(inOutName,"PrismaConf/ban/charge_55deg_ban_",13,11,13);
	//DrawCharge		(inOutName,"PrismaConf/ban/charge_55deg_ban_",12,10,12);
}

//---------------------------------------------------------------------------------|
void TCutToBan(string cutFileName, string banFileName){

       ofstream outBan(banFileName);

       TFile * file = new TFile(cutFileName.c_str()); 
       TCutG * cut = (TCutG*)file->Get(cutFileName.substr(0,cutFileName.length()-5).c_str());

       int n_points = cut->GetN();
       double * x = cut->GetX();
       double * y = cut->GetY();

       for (int j = 0; j < n_points; ++j)      outBan << x[j] << "\t" << y[j] << endl;

       cout << cutFileName << " completed" << endl;
}

//---------------------------------------------------------------------------------|
void TCutToSpli(string cutFileName, string spliFileName, double val){

        ofstream outBan(spliFileName);
        std::cout << "Opening file " << cutFileName << std::endl;

        TFile * file = new TFile(cutFileName.c_str());

        TCutG * cut = (TCutG*)file->Get(cutFileName.substr(0,cutFileName.length()-5).c_str());

        int n_points = cut->GetN();
        double * x = cut->GetX();
        double * y = cut->GetY();

        std::vector<std::pair<double, double>> points;
        for (int j = 0; j < n_points; ++j)      points.emplace_back(x[j],  y[j]-val);

        std::sort(points.begin(), points.end(), [](const auto& a, const auto& b){return a.first<b.first;});
        for (const auto& it: points)      outBan << it.first << "\t" << it.second << endl;

        cout << spliFileName << " completed" << endl;
}

//---------------------------------------------------------------------------------|
void TCutToSpli(std::vector<string> cutFileName, string spliFileName, std::vector<double> val){

        ofstream outBan(spliFileName);

        std::cout << "Reading files\n";
		for(auto oneCutFileName:cutFileName) cout << oneCutFileName << std::endl;

        TFile * file = new TFile(cutFileName.front().c_str());

        TCutG * cut = (TCutG*)file->Get(cutFileName.front().substr(0,cutFileName.front().length()-5).c_str());

        int n_points = cut->GetN();
        double * x = cut->GetX();
        double * y = cut->GetY();

        std::vector<std::pair<double, double>> points;
        for (int j = 0; j < n_points; ++j)      points.emplace_back(x[j],  y[j]-val.front());

        for(int i=1; i<val.size(); ++i){
			TFile * fileCorr = new TFile(cutFileName[i].c_str());
			if(fileCorr == nullptr) throw std::runtime_error("File not opened");
			TCutG * cutCorr = (TCutG*)fileCorr->Get(cutFileName[i].substr(0,cutFileName[i].length()-5).c_str());
			if(cutCorr == nullptr) throw std::runtime_error("Cut not opened");
			for(auto& p: points){
					p.second +=cutCorr->Eval(p.first)-val[i];
			}
        }

        std::sort(points.begin(), points.end(), [](const auto& a, const auto& b){return a.first<b.first;});
        for (const auto& it: points)      outBan << it.first << "\t" << it.second << endl;

        cout << spliFileName << " completed" << endl;
}

//---------------------------------------------------------------------------------|
void DrawMCPraw(string inFileName, string banFileName, bool selFlag = true, int nrEvts = -1){
	cout << "DrawMCPraw was called" << endl;

	TFile* fIn = GetFile(inFileName);	
	TCutG *cut = GetCut(banFileName,"mcp_cut","Raw.MCP_raw[0]","Raw.MCP_raw[1]");
	
	TCanvas *c1 = new TCanvas("mcp_raw", "raw MCP_Y vs MCP_X", 1024, 768);
	gPad->SetLogz();
	
	if(selFlag){
		TH2D* matr;
		gDirectory->GetObject("Prisma/Raw/MCP/m_MCPX_MCPY_raw", matr);
		matr->Draw("colz");
	}
	else{
		TTree* tree = (TTree*)fIn->Get("PrismaTree");
		if(nrEvts==-1) tree->Draw("Raw.MCP_raw[1]:Raw.MCP_raw[0]>>hmcpraw(1024,0,4096,1024,0,4096)", "", "colz");	
		else tree->Draw("Raw.MCP_raw[1]:Raw.MCP_raw[0]>>hmcpraw(1024,0,4096,1024,0,4096)", "", "colz", nrEvts);
	}
	cut->Draw("same");
}

//---------------------------------------------------------------------------------|
void DrawMCPana(string inFileName, bool selFlag = true, int nrEvts = -1){
	cout << "DrawMCPana was called" << endl;

	TFile* fIn = GetFile(inFileName);
	
	TCanvas *c1 = new TCanvas("mcp_ana", "ana MCP_Y vs MCP_X", 1024, 768);
	gPad->SetLogz();
	
	if(selFlag){
		TH2D* matr;
		gDirectory->GetObject("Prisma/Ana/m_MCPX_MCPY", matr);
		matr->Draw("colz");
	}
	else{
		TTree* tree = (TTree*)fIn->Get("PrismaTree");
		if(nrEvts==-1) tree->Draw("Analyzed.MCP_Y:Analyzed.MCP_X>>hmcpana(1000,-80,80,1000,-80,80)", "", "colz");	
		else tree->Draw("Analyzed.MCP_Y:Analyzed.MCP_X>>hmcpana(1000,-80,80,1000,-80,80)", "", "colz", nrEvts);
	}

	TGraph* tg = new TGraph();
	tg->SetPoint(0,    0,   0);
	tg->SetPoint(1,-21.5, 26.5);
	tg->SetPoint(2, 21.5, 26.5);
	tg->SetPoint(3,-21.5,-26.5);
	tg->SetPoint(4, 21.5,-26.5);
	tg->SetMarkerStyle(88);
	tg->SetMarkerSize(4);
	tg->SetMarkerColor(kRed);
	tg->Draw("Psame");
}

//---------------------------------------------------------------------------------|
void DrawMCPanaNew(string inFileName, bool selFlag = true, int nrEvts = -1){
	cout << "DrawMCPanaNew was called" << endl;

	TFile* fIn = GetFile(inFileName);
	
	TCanvas *c1 = new TCanvas("mcp_anaNew", "ana MCP_Y vs MCP_X", 1024, 768);
	gPad->SetLogz();
	
	if(selFlag){
		TH2D* matr;
		gDirectory->GetObject("Prisma/Ana/m_MCPX_MCPY", matr);
		matr->Draw("colz");
	}
	else{
		TTree* tree = (TTree*)fIn->Get("PrismaTree");
		if(nrEvts==-1) tree->Draw("Analyzed.MCP_Y:Analyzed.MCP_X>>hmcpana(1000,-80,80,1000,-80,80)", "", "colz");	
		else tree->Draw("Analyzed.MCP_Y:Analyzed.MCP_X>>hmcpana(1000,-80,80,1000,-80,80)", "", "colz", nrEvts);
	}
}

//---------------------------------------------------------------------------------|
void DrawPPAC_Right(string inFileName, string thrFileName, int nrEvts = -1){
	cout << "DrawPPAC_Right was called" << endl;
	DrawPPAC_Signal(inFileName,thrFileName,"PPAC_Xright_raw",nrEvts);
}
void DrawPPAC_Left(string inFileName, string thrFileName, int nrEvts = -1){
	cout << "DrawPPAC_Left was called" << endl;
	DrawPPAC_Signal(inFileName,thrFileName,"PPAC_Xleft_raw",nrEvts);
}
void DrawPPAC_Cath(string inFileName, string thrFileName, int nrEvts = -1){
	cout << "DrawPPAC_Cath was called" << endl;
	DrawPPAC_Signal(inFileName,thrFileName,"PPAC_Cathode_raw",nrEvts);
}

void DrawPPAC_Cath_sec(string inFileName, string thrFileName, int nrEvts = -1){
	cout << "DrawPPAC_Cath was called" << endl;
	DrawPPAC_Signal_sec(inFileName,thrFileName,"PPAC_Cathode_raw",nrEvts);
}

void DrawPPAC_Left_sec(string inFileName, string thrFileName, int nrEvts = -1){
	cout << "DrawPPAC_Left was called" << endl;
	DrawPPAC_Signal_sec(inFileName,thrFileName,"PPAC_Xleft_raw",nrEvts);
}


void DrawPPAC_Signal(string inFileName, string thrFileName, string variable, int nrEvts = -1){

	TFile* fIn = GetFile(inFileName);

	vector<double> lower, upper;
	GetThr(thrFileName,lower,upper);

	TCanvas *c1 = new TCanvas(variable.c_str(), variable.c_str(), 1600, 600);
	c1->Divide(5,2);

	TLine *Lower[10];
	TLine *Upper[10];

	TTree* tree = (TTree*)fIn->Get("PrismaTree");
	for(int i=0; i<10; ++i){
		cout << "Plotting PPAC section " << i << "\t thr:\t[" << lower[i] << ";" << upper[i] << "]" << endl;
		
		c1->cd(i+1);	
		gPad->SetLogy();			

		if(nrEvts == -1) tree->Draw(Form("Raw.%s[%i]>>hppac%s%i(1024,0,4096)",variable.c_str(),i,variable.c_str(),i),"","");		
		else tree->Draw(Form("Raw.%s[%i]>>hppac%s%i(1024,0,4096)",variable.c_str(),i,variable.c_str(),i),"","",nrEvts);	
		
		Lower[i] = GetRedLine();
		Upper[i] = GetRedLine();
		Lower[i]->DrawLine(lower[i],-10,lower[i],1e6);
		Upper[i]->DrawLine(upper[i],-10,upper[i],1e6);	
	}	
}
//----------------------------------------------------------------------------------------------------------------------------------|

void DrawPPAC_Signal_sec(string inFileName, string thrFileName, string variable, int nrEvts = -1){

     TFile* fIn = GetFile(inFileName);
 
     vector<double> lower, upper;
     GetThr(thrFileName,lower,upper);
 
     TCanvas *c1 = new TCanvas(variable.c_str(), variable.c_str(), 1600, 600);
     //c1->Divide(5,2);
 
     TLine *Lower[10];
     TLine *Upper[10];
 
     TTree* tree = (TTree*)fIn->Get("PrismaTree");
     for(int i=4; i<=4; ++i){
         cout << "Plotting PPAC section " << i << "\t thr:\t[" << lower[i] << ";" << upper[i] << "]" << endl;
 
         c1->cd(i+1);
         gPad->SetLogy();
 
         if(nrEvts == -1) tree->Draw(Form("Raw.%s[%i]>>hppac%s%i(1024,0,4096)",variable.c_str(),i,variable.c_str(),i),"","");
         else tree->Draw(Form("Raw.%s[%i]>>hppac%s%i(1024,0,4096)",variable.c_str(),i,variable.c_str(),i),"","",nrEvts);
 
         Lower[i] = GetRedLine();
         Upper[i] = GetRedLine();
         Lower[i]->DrawLine(lower[i],-10,lower[i],1e6);
         Upper[i]->DrawLine(upper[i],-10,upper[i],1e6);
     }


}


//---------------------------------------------------------------------------------|
void DrawPPAC_Cuts (string inFileName, string banFileName, bool selFlag = true, int nrEvts = -1){
	cout << "DrawPPAC_Cuts was called" << endl;

	TFile* fIn = GetFile(inFileName);	

	TCanvas *c1 = new TCanvas("PPAC_Cuts", "PPAC_Xright_raw + PPAC_Xleft_raw vs PPAC_Cathode_raw", 1600, 900);
	c1->Divide(2,5);

	for(int i=0; i<10; i++){
		c1->cd(i+1);
		gPad->SetLogz();
		cout << "Plotting PPAC section " << i << endl;

		if(selFlag){
			TH2D* matr;
			gDirectory->GetObject(Form("Prisma/Raw/PPAC/m_XRpXL_CATHODE_%i",i), matr);
			matr->Draw("colz");
		}
		else{
			TTree* tree = (TTree*)fIn->Get("PrismaTree");
			if(nrEvts==-1) tree->Draw(Form("Raw.PPAC_Xright_raw[%i]+Raw.PPAC_Xleft_raw[%i]:Raw.PPAC_Cathode_raw[%i]>>hppacCuts%i(1024,0,4096,1024,0,8192)",i,i,i,i), "", "colz");	
			else tree->Draw(Form("Raw.PPAC_Xright_raw[%i]+Raw.PPAC_Xleft_raw[%i]:Raw.PPAC_Cathode_raw[%i]>>hppacCuts%i(1024,0,4096,1024,0,8192)",i,i,i,i), "", "colz", nrEvts);
		}
	}

	ifstream banFile(banFileName);
	if(banFile.is_open()){

		TCutG* cuts[10];
		string line;
		getline(banFile,line);

		for(int i=0; i<10; i++){
			
			c1->cd(i+1);			

			cuts[i] = new TCutG(Form("ppac_cut%i",i));
			cuts[i]->SetVarX(Form("Raw.PPAC_Cathode_raw[%i]",i));
			cuts[i]->SetVarY(Form("Raw.PPAC_Xright_raw[%i]+Raw.PPAC_Xleft_raw[%i]",i,i));
			
			string a, b;
			vector <double> x_v, y_v;

			while(getline(banFile,line)){

				stringstream ss (line);
				ss >> a >> b;
		
				if(a == "ADC") break;				
				x_v.push_back(stod(a));
				y_v.push_back(stod(b));
			}
			
			for(unsigned int j=0; j<x_v.size(); ++j) cuts[i]->SetPoint(j, x_v[j], y_v[j]);
			
			cout << "\nLoaded cut:" << endl;	
			cuts[i]->Print();
			cuts[i]->SetLineColor(kRed);
			cuts[i]->SetLineWidth(2);
			cuts[i]->Draw("same");
		}
	}
	else cout << "Could not open file " << banFileName << endl;
}


//---------------------------------------------------------------------------------|
void DrawPPAC_Cuts_Section (string inFileName, string banFileName, bool selFlag = true, int nrEvts = -1){
	cout << "DrawPPAC_Cuts was called" << endl;

	TFile* fIn = GetFile(inFileName);	

	TCanvas *c1 = new TCanvas("PPAC_Cuts_Section", "PPAC_Xright_raw + PPAC_Xleft_raw vs PPAC_Cathode_raw", 1600, 900);
	//c1->Divide(2,5);

	for(int i=7; i<8; i++){
		c1->cd(i+1);
		gPad->SetLogz();
		cout << "Plotting PPAC section " << i << endl;

		if(selFlag){
			TH2D* matr;
			gDirectory->GetObject(Form("Prisma/Raw/PPAC/m_XRpXL_CATHODE_%i",i), matr);
			matr->Draw("colz");
		}
		else{
			TTree* tree = (TTree*)fIn->Get("PrismaTree");
			if(nrEvts==-1) tree->Draw(Form("Raw.PPAC_Xright_raw[%i]+Raw.PPAC_Xleft_raw[%i]:Raw.PPAC_Cathode_raw[%i]>>hppacCuts%i(1024,0,4096,1024,0,8192)",i,i,i,i), "", "colz");	
			else tree->Draw(Form("Raw.PPAC_Xright_raw[%i]+Raw.PPAC_Xleft_raw[%i]:Raw.PPAC_Cathode_raw[%i]>>hppacCuts%i(1024,0,4096,1024,0,8192)",i,i,i,i), "", "colz", nrEvts);
		}
	}

	ifstream banFile(banFileName);
	if(banFile.is_open()){

		TCutG* cuts[10];
		string line;
		getline(banFile,line);

		for(int i=0; i<10; i++){
			
			c1->cd(i+1);			

			cuts[i] = new TCutG(Form("ppac_cut%i",i));
			cuts[i]->SetVarX(Form("Raw.PPAC_Cathode_raw[%i]",i));
			cuts[i]->SetVarY(Form("Raw.PPAC_Xright_raw[%i]+Raw.PPAC_Xleft_raw[%i]",i,i));
			
			string a, b;
			vector <double> x_v, y_v;

			while(getline(banFile,line)){

				stringstream ss (line);
				ss >> a >> b;
		
				if(a == "ADC") break;				
				x_v.push_back(stod(a));
				y_v.push_back(stod(b));
			}
			
			for(unsigned int j=0; j<x_v.size(); ++j) cuts[i]->SetPoint(j, x_v[j], y_v[j]);
			
			cout << "\nLoaded cut:" << endl;	
			cuts[i]->Print();
			cuts[i]->SetLineColor(kRed);
			cuts[i]->SetLineWidth(2);
			cuts[i]->Draw("same");
		}
	}
	else cout << "Could not open file " << banFileName << endl;
}





//---------------------------------------------------------------------------------|
void DrawPPAC_XfpCal (string inFileName, int nrEvts = -1){
	cout << "DrawPPAC_XfpCal was called" << endl;

	TFile* fIn = GetFile(inFileName);
	TTree* tree = (TTree*)fIn->Get("PrismaTree");	

	TCanvas *c1 = new TCanvas("PPAC_XfpCal", "Xfp Cal", 1600, 900);
	c1->Divide(5,2);

	TLine *Left[10];
	TLine *Right[10];

	for(int i=0; i<10; i++){
		c1->cd(i+1);
		gPad->SetLogz();
		cout << "Plotting PPAC section " << i << endl;

		if(nrEvts==-1) tree->Draw(Form("Analyzed.X_FP:Raw.PPAC_Xright_raw[%i]-Raw.PPAC_Xleft_raw[%i]>>hppacXfp%i(1024,-4096,4096,500,%i,%i)",i,i,i,100*i-50,100*(i+1)+50),"","colz");
		else tree->Draw(Form("Analyzed.X_FP:Raw.PPAC_Xright_raw[%i]-Raw.PPAC_Xleft_raw[%i]>>hppacXfp%i(1024,-4096,4096,500,%i,%i)",i,i,i,100*i-50,100*(i+1)+50),"","colz",nrEvts);

		Left[i]  = GetRedLine();
		Right[i] = GetRedLine();
		Left[i] ->DrawLine(-4096,100*i,4096,100*i);
		Right[i]->DrawLine(-4096,100*(i+1),4096,100*(i+1));
	}
}

//---------------------------------------------------------------------------------|
void DrawPPAC_XrCal (string inFileName, string calFileName, bool selFlag = true, int nrEvts = -1){
	cout << "DrawPPAC_XrCal was called" << endl;

	TFile* fIn = GetFile(inFileName);

	vector<double> A, B; // y = A + B*x
	GetCal(calFileName,A,B);

	TCanvas *c1 = new TCanvas("PPAC_XrCal", "Xr Cal", 1600, 900);
	c1->Divide(5,2);

	TLine *Line[10];

	for(int i=0; i<10; i++){
		c1->cd(i+1);
		gPad->SetLogz();
		cout << "Plotting PPAC section " << i << endl;

		if(selFlag){
			TH2D* matr;
			gDirectory->GetObject(Form("Prisma/Misc/m_XFP_XRmCATHODE_%i",i), matr);
			matr->Draw("colz");
		}
		else{
			TTree* tree = (TTree*)fIn->Get("PrismaTree");
			if(nrEvts==-1) tree->Draw(Form("Analyzed.X_FP:Raw.PPAC_Xright_raw[%i]-Raw.PPAC_Cathode_raw[%i]>>hppacXr%i(1024,-4096,4096,1200,-100,1100)",i,i,i), "", "colz");	
			else tree->Draw(Form("Analyzed.X_FP:Raw.PPAC_Xright_raw[%i]-Raw.PPAC_Cathode_raw[%i]>>hppacXr%i(1024,-4096,4096,1200,-100,1100)",i,i,i), "", "colz", nrEvts);
		}

		double a = A[i], b = B[i];
		Line[i] = GetRedLine();
		Line[i]->DrawLine(-4096,a+b*(-4096),4096,a+b*4096);
	}
}

void DrawPPAC_XlCal (string inFileName, string calFileName, bool selFlag = true, int nrEvts = -1){
	cout << "DrawPPAC_XlCal was called" << endl;

	TFile* fIn = GetFile(inFileName);

	vector<double> A, B; // y = A + B*x
	GetCal(calFileName,A,B);

	TCanvas *c1 = new TCanvas("PPAC_XlCal", "Xl Cal", 1600, 900);
	c1->Divide(5,2);

	TLine *Line[10];

	for(int i=0; i<10; i++){
		c1->cd(i+1);
		gPad->SetLogz();
		cout << "Plotting PPAC section " << i << endl;

		if(selFlag){
			TH2D* matr;
			gDirectory->GetObject(Form("Prisma/Misc/m_XFP_CATHODEmXL_%i",i), matr);
			matr->Draw("colz");
		}
		else{
			TTree* tree = (TTree*)fIn->Get("PrismaTree");
			if(nrEvts==-1) tree->Draw(Form("Analyzed.X_FP:Raw.PPAC_Cathode_raw[%i]-Raw.PPAC_Xleft_raw[%i]>>hppacXl%i(1024,-4096,4096,1200,-100,1100)",i,i,i), "", "colz");	
			else tree->Draw(Form("Analyzed.X_FP:Raw.PPAC_Cathode_raw[%i]-Raw.PPAC_Xleft_raw[%i]>>hppacXl%i(1024,-4096,4096,1200,-100,1100)",i,i,i), "", "colz", nrEvts);
		}

		double a = A[i], b = B[i];
		Line[i] = GetRedLine();
		Line[i]->DrawLine(-4096,a+b*(-4096),4096,a+b*4096);
	}
}

//---------------------------------------------------------------------------------|
void DrawToFs (string inFileName, string thrFileName, int nrEvts = -1){
	cout << "DrawToFs was called" << endl;

	TFile* fIn = GetFile(inFileName);

	vector<double> lower, upper;
	GetThr(thrFileName,lower,upper);

	TCanvas *c1 = new TCanvas("tof", "tof", 1600, 600);
	c1->Divide(5,2);

	TLine *Lower[10];
	TLine *Upper[10];

	TTree* tree = (TTree*)fIn->Get("PrismaTree");
	for(int i=0; i<10; ++i){
		cout << "Plotting ToF section " << i << "\t thr:\t[" << lower[i] << ";" << upper[i] << "]" << endl;
		
		c1->cd(i+1);	
		gPad->SetLogy();			

		if(nrEvts == -1) tree->Draw(Form("Raw.TOF_raw[%i]>>htof%i(1024,0,4096)",i,i),"","");		
		else tree->Draw(Form("Raw.TOF_raw[%i]>>htof%i(1024,0,4096)",i,i),"","",nrEvts);	
		
		Lower[i] = GetRedLine();
		Upper[i] = GetRedLine();
		Lower[i]->DrawLine(lower[i],-10,lower[i],1e6);
		Upper[i]->DrawLine(upper[i],-10,upper[i],1e6);	
	}
}

//---------------------------------------------------------------------------------|
void DrawSidePads (string inFileName, string thrFilePattern, bool selFlag = true, int nrEvts = -1){
	cout << "DrawSidePads was called" << endl;

	TFile* fIn = GetFile(inFileName);

	vector<string>Rows = {"A","B","C","D"};

	TCanvas *c1 = new TCanvas("Side", "Side", 1200, 800);
	c1->Divide(2,2);

	for(unsigned int r=0; r<Rows.size(); r++){
		string thrFileName = thrFilePattern + Rows[r] + ".thres";

		vector<double> lower, upper; // y = A + B*x
		GetThr(thrFileName,lower,upper);

		for(int i=0; i<5; i++){
			c1->cd((2*r+i)+1);
			gPad->SetLogy();
			cout << "Plotting Side pad " << Rows[r] << i << "\t thr:\t[" << lower[i] << ";" << upper[i] << "]" << endl;

			if(selFlag){
				TH1D* hist;
				gDirectory->GetObject(Form("Prisma/Raw/IC/h_Side_%s_%i",Rows[r].c_str(),i), hist);
				hist->Draw();
			}
			else{
				TTree* tree = (TTree*)fIn->Get("PrismaTree");
				if(nrEvts==-1) tree->Draw(Form("Raw.Side_%s_raw[%i]>>hside%s%i(1024,0,4096)",Rows[r].c_str(),i,Rows[r].c_str(),i), "", "");
				else tree->Draw(Form("Raw.Side_%s_raw[%i]>>hside%s%i(1024,0,4096)",Rows[r].c_str(),i,Rows[r].c_str(),i), "", "", nrEvts);
			}
			TLine* Lower = GetRedLine();
			TLine* Upper = GetRedLine();
			Lower->DrawLine(lower[i],0,lower[i],1e6);
			Upper->DrawLine(upper[i],0,upper[i],1e6);
		}
	}
}
void DrawIonChPads (string inFileName, string thrFilePattern, bool selFlag = true, int nrEvts = -1){
	cout << "DrawIonChPads was called" << endl;

	TFile* fIn = GetFile(inFileName);

	vector<string>Rows = {"A","B","C","D"};

	for(unsigned int r=0; r<Rows.size(); r++){
		TCanvas *c1 = new TCanvas(Form("IC_%s",Rows[r].c_str()), Form("IonCh Row %s",Rows[r].c_str()), 1200, 800);
		//c1->Divide(2,5);
		
		string thrFileName = thrFilePattern + Rows[r] + ".thres";

		vector<double> lower, upper; // y = A + B*x
		GetThr(thrFileName,lower,upper);

		for(int i=2; i<3; i++){
			c1->cd(i+1);
			gPad->SetLogy();
			cout << "Plotting IC pad " << Rows[r] << i << "\t thr:\t[" << lower[i] << ";" << upper[i] << "]" << endl;

			if(selFlag){
				TH1D* hist;
				gDirectory->GetObject(Form("Prisma/Raw/IC/h_IC_%s_%i",Rows[r].c_str(),i), hist);
				hist->Draw();
			}
			else{
				TTree* tree = (TTree*)fIn->Get("PrismaTree");
				if(nrEvts==-1) tree->Draw(Form("Raw.IC_%s_raw[%i]>>hICpads%s%i(1024,0,4096)",Rows[r].c_str(),i,Rows[r].c_str(),i), "", "");
				else tree->Draw(Form("Raw.IC_%s_raw[%i]>>hICpads%s%i(1024,0,4096)",Rows[r].c_str(),i,Rows[r].c_str(),i), "", "", nrEvts);
			}
			TLine* Lower = GetRedLine();
			TLine* Upper = GetRedLine();
			Lower->DrawLine(lower[i],0,lower[i],1e6);
			Upper->DrawLine(upper[i],0,upper[i],1e6);
		}
	}
}

//---------------------------------------------------------------------------------|
void DrawZedA (string inFileName, string banFilePattern, int Zmin, int Zmax, bool selFlag = true, int nrEvts = -1){
	cout << "DrawZed was called (zmin = " << Zmin << "; zmax = " << Zmax << ")" << endl;

	TFile* fIn = GetFile(inFileName);

	TCutG* cuts[Zmax-Zmin+1];
	for(int z=Zmin; z<=Zmax; z++) 
		cuts[z-Zmin] = GetCut(banFilePattern+to_string(z)+".ban",Form("z%i",z),"Analyzed.IC_E","Analyzed.IC_DE_A");
	
	TCanvas* c1 = new TCanvas("DEA_E", "IC_dE_A vs IC_E", 1200, 800);
	gPad->SetLogz();
	
	if(selFlag){
		TH2D* matr;
		gDirectory->GetObject("Prisma/Ana/DE_E/DEA_E/m_ICDEA_ICE_tot", matr);
		matr->Draw("colz");
	}
	else{
		TTree* tree = (TTree*)fIn->Get("PrismaTree");
		if(nrEvts==-1) tree->Draw("Analyzed.IC_DE_A:Analyzed.IC_E>>hDEA_E(1000,0,10000,1000,0,10000)","","colz");
		else tree->Draw("Analyzed.IC_DE_A:Analyzed.IC_E>>hDEA_E(1000,0,10000,1000,0,10000)","","colz",nrEvts);
	}

	for(int i=0; i<Zmax-Zmin+1; i++) cuts[i]->Draw("same");
}

void DrawZedAB (string inFileName, string banFilePattern, int Zmin, int Zmax, bool selFlag = true, int nrEvts = -1){
	cout << "DrawZed was called (zmin = " << Zmin << "; zmax = " << Zmax << ")" << endl;

	TFile* fIn = GetFile(inFileName);

	TCutG* cuts[Zmax-Zmin+1];
	for(int z=Zmin; z<=Zmax; z++) 
		cuts[z-Zmin] = GetCut(banFilePattern+to_string(z)+".ban",Form("z%i",z),"Analyzed.IC_E","Analyzed.IC_DE_AB");
	
	TCanvas* c1 = new TCanvas("DEAB_E", "IC_dE_AB vs IC_E", 1200, 800);
	gPad->SetLogz();
	
	if(selFlag){
		TH2D* matr;
		gDirectory->GetObject("Prisma/Ana/DE_E/DEAB_E/m_ICDEAB_ICE_tot", matr);
		matr->Draw("colz");
	}
	else{
		TTree* tree = (TTree*)fIn->Get("PrismaTree");
		if(nrEvts==-1) tree->Draw("Analyzed.IC_DE_AB:Analyzed.IC_E>>hDEAB_E(1000,0,10000,1000,0,10000)","","colz");
		else tree->Draw("Analyzed.IC_DE_AB:Analyzed.IC_E>>hDEAB_E(1000,0,10000,1000,0,10000)","","colz",nrEvts);
	}

	for(int i=0; i<Zmax-Zmin+1; i++) cuts[i]->Draw("same");
}

//---------------------------------------------------------------------------------|
void DrawCharge (string inFileName, string banFilePattern, int Z, int qmin, int qmax, bool selFlag = true, int nrEvts = -1){
	cout << "DrawCharge was called (Z = " << Z << "; qmin = " << qmin << "; qmax = " << qmax << ")" << endl;

	TFile* fIn = GetFile(inFileName);

	TCutG* cuts[qmax-qmin+1];
	for(int q=qmin; q<=qmax; q++) 
		cuts[q-qmin] = GetCut(banFilePattern+to_string(Z)+"_"+to_string(q)+".ban",Form("z%iq%i",Z,q),"Analyzed.IC_E","Analyzed.RBeta");
	
	TCanvas* c1 = new TCanvas(Form("ChargeZ%i",Z), "IC_E vs RBeta", 1200, 800);
	gPad->SetLogz();
	
	if(selFlag){
		TH2D* matr;
		gDirectory->GetObject(Form("Prisma/Ana/ICE_RBeta/m_ICE_RBeta_%i",Z), matr);
		matr->Draw("colz");
	}
	else{
		TTree* tree = (TTree*)fIn->Get("PrismaTree");
		if(nrEvts==-1) tree->Draw(Form("Analyzed.IC_E:Analyzed.RBeta>>hChargeZ%i(1000,0,4000,1000,0,10000)",Z),"","colz");
		else tree->Draw(Form("Analyzed.IC_E:Analyzed.RBeta>>hChargeZ%i(1000,0,4000,1000,0,10000)",Z),"","colz",nrEvts);
	}
	
	for(int i=0; i<qmax-qmin+1; i++) cuts[i]->Draw("same");
}

//---------------------------------------------------------------------------------|
TCutG* GetCut(string cutFileName, string cutName, string varX, string varY){

	TCutG *cut = new TCutG(cutName.c_str());
	cut->SetVarX(varX.c_str());
	cut->SetVarY(varY.c_str());
	
	ifstream banFile(cutFileName);
	if(banFile.is_open()){

		string a, b, temp;
		vector <double> x_v, y_v;

		while(getline(banFile,temp)){
			if(temp[0]=='#') continue;			
			
			stringstream ss;
			ss << temp;
			ss >> a >> b;			
			
			x_v.push_back(stod(a));
			y_v.push_back(stod(b));	
		}			

		for (unsigned int j = 0; j < x_v.size(); ++j){
				cut->SetPoint(j,x_v[j],y_v[j]);
		}	
	} 
	else cout << "Could not open file " << cutFileName << endl;
 	banFile.close();
 	
	cout << "\nLoaded cut: " << endl;	
	cut->Print();
	cut->SetLineColor(kRed);
	cut->SetLineWidth(2);
	return cut;
}

//---------------------------------------------------------------------------------|
void GetThr(string thrFileName, vector<double>& lower, vector<double>& upper){

	ifstream thrFile(thrFileName);
	if(thrFile.is_open()){

		string line;
		while(getline(thrFile, line)){
			if(line[0]=='#') continue;
			stringstream ss (line);
			int a, b, c;
			float low, up;
			ss >> a >> b >> c >> low >> up;

			lower.push_back(low);
			upper.push_back(up);
		}
	} 
	else cout << "Could not open file " << thrFileName << endl;	
	thrFile.close();
}

//---------------------------------------------------------------------------------|
void GetCal(string calFileName, vector<double>& As, vector<double>& Bs){

	ifstream calFile(calFileName);
	if(calFile.is_open()){

		string line;
		while(getline(calFile, line)){
			if(line[0]=='#') continue;
			stringstream ss (line);
			int a, b, c;
			float A, B;
			ss >> a >> b >> c >> A >> B;

			As.push_back(A);
			Bs.push_back(B);
		}
	} 
	else cout << "Could not open file " << calFileName << endl;	
	calFile.close();
}

//---------------------------------------------------------------------------------|
TFile* GetFile(string inFileName){
	TFile* fIn = new TFile(inFileName.c_str(), "READ");
	if(fIn->IsOpen()==0){
		cout << "Could not find input file " << inFileName << endl;
		return nullptr;
	}
	else{
		cout << "Got the file " << inFileName << endl;
	}
	return fIn;
}

//---------------------------------------------------------------------------------|
TLine* GetRedLine(){
	TLine* lin = new TLine();
	lin->SetLineColor(kRed);
	lin->SetLineWidth(2);
	return lin;
}
