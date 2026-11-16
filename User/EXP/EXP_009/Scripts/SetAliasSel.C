void SetAliasSel(){

	AnalyzedTree->SetAlias("TOF2"," (tof*1e9+0)*1e-9");
	//AnalyzedTree->SetAlias("TOF2"," (tof*1e9-10*2.1)*1e-9");
	//AnalyzedTree->SetAlias("TOF2"," (tof*1e9+0)*1e-9");
	//AnalyzedTree->SetAlias("TOF3"," ( ((col_nbr==0)*(TOF2*1.e9 + 0.24)) + ((col_nbr==1)*(TOF2*1.e9 + 0.2))+ ((col_nbr==2)*(TOF2*1.e9 + 0.16 ))+ ((col_nbr==3)*(TOF2*1.e9 + 0.114))+ ((col_nbr==4)*(TOF2*1.e9 + 0.08 ))+ ((col_nbr==5)*(TOF2*1.e9 + 0.04 ))+ ((col_nbr==6)*(TOF2*1.e9 + 0.))+ ((col_nbr==7)*(TOF2*1.e9 - 0.06))+ ((col_nbr==8)*(TOF2*1.e9 - 1.95))+ ((col_nbr==9)*(TOF2*1.e9 - 2.05)) )*1.e-9");
	//AnalyzedTree->SetAlias("TOF3"," ( ((col_nbr==0)*(TOF2*1.e9 + 0.04)) + ((col_nbr==1)*(TOF2*1.e9 + 0.085))+ ((col_nbr==2)*(TOF2*1.e9 + 0.08 ))+ ((col_nbr==3)*(TOF2*1.e9 + 0.05))+ ((col_nbr==4)*(TOF2*1.e9 + 0.04 ))+ ((col_nbr==5)*(TOF2*1.e9 + 0.02 ))+ ((col_nbr==6)*(TOF2*1.e9 - 0.02))+ ((col_nbr==7)*(TOF2*1.e9 - 0.02))+ ((col_nbr==8)*(TOF2*1.e9 - 0.05))+ ((col_nbr==9)*(TOF2*1.e9 - 0.07)) )*1.e-9");
	//AnalyzedTree->SetAlias("TOF3"," ( ((col_nbr==0)*(TOF2*1.e9 - 6)) + ((col_nbr==1)*(TOF2*1.e9 - 5.81))+ ((col_nbr==2)*(TOF2*1.e9 - 2.75 ))+ ((col_nbr==3)*(TOF2*1.e9 + 2.3))+ ((col_nbr==4)*(TOF2*1.e9 + 3.72))+ ((col_nbr==5)*(TOF2*1.e9 + 1.15))+ ((col_nbr==6)*(TOF2*1.e9 + 1.8))+ ((col_nbr==7)*(TOF2*1.e9 + 1.31))+ ((col_nbr==8)*(TOF2*1.e9 + 0.95))+ ((col_nbr==9)*(TOF2*1.e9 + 1.54)) )*1.e-9");
	//AnalyzedTree->SetAlias("TOF3"," ( ((col_nbr==0)*(TOF2*1.e9 + 0.14)) + ((col_nbr==1)*(TOF2*1.e9 + 0.12))+ ((col_nbr==2)*(TOF2*1.e9 + 0.08 ))+ ((col_nbr==3)*(TOF2*1.e9 + 0.08))+ ((col_nbr==4)*(TOF2*1.e9 + 0.06))+ ((col_nbr==5)*(TOF2*1.e9 + 0.03))+ ((col_nbr==6)*(TOF2*1.e9 + 0.04))+ ((col_nbr==7)*(TOF2*1.e9 + 0.0))+ ((col_nbr==8)*(TOF2*1.e9 - 0.01))+ ((col_nbr==9)*(TOF2*1.e9 - 0.05)) )*1.e-9");
	//AnalyzedTree->SetAlias("TOF3"," ( ((col_nbr==0)*(TOF2*1.e9 + 0.09)) + ((col_nbr==1)*(TOF2*1.e9 + 0.07))+ ((col_nbr==2)*(TOF2*1.e9 + 0.05 ))+ ((col_nbr==3)*(TOF2*1.e9 + 0.02))+ ((col_nbr==4)*(TOF2*1.e9 + 0.01))+ ((col_nbr==5)*(TOF2*1.e9 + 0.0))+ ((col_nbr==6)*(TOF2*1.e9 - 0.02))+ ((col_nbr==7)*(TOF2*1.e9 - 0.04))+ ((col_nbr==8)*(TOF2*1.e9 - 0.025))+ ((col_nbr==9)*(TOF2*1.e9 - 0.1)) )*1.e-9");
	AnalyzedTree->SetAlias("TOF3"," ( ((col_nbr==0)*(TOF2*1.e9 - 0.0)) + ((col_nbr==1)*(TOF2*1.e9 - 0.0))+ ((col_nbr==2)*(TOF2*1.e9 - 0.0 ))+ ((col_nbr==3)*(TOF2*1.e9 + 0.0))+ ((col_nbr==4)*(TOF2*1.e9 - 0.0))+ ((col_nbr==5)*(TOF2*1.e9 + 0.0))+ ((col_nbr==6)*(TOF2*1.e9 + 0.0))+ ((col_nbr==7)*(TOF2*1.e9 + 0.0))+ ((col_nbr==8)*(TOF2*1.e9 + 0.0))+ ((col_nbr==9)*(TOF2*1.e9 + 0.0)) )*1.e-9");

	AnalyzedTree->SetAlias("Beta2","beta*tof/TOF3");
	AnalyzedTree->SetAlias("Gamma2","1/sqrt(1-Beta2*Beta2)");
	AnalyzedTree->SetAlias("BetaGamma2","Beta2*Gamma2");

	AnalyzedTree->SetAlias("BetaGamma","beta*gamma");

	AnalyzedTree->SetAlias("Aoq2","a_over_q*BetaGamma/BetaGamma2");


	AnalyzedTree->SetAlias("Charge1","(charge+0.0235115069099162*ic_e_nm)");
	AnalyzedTree->SetAlias("Charge2","Charge1*a_over_q/Aoq2");
	AnalyzedTree->SetAlias("Charge2corr","Charge2-0.0250112894863224*ic_e_nm");
	AnalyzedTree->SetAlias("Charge2cal","-1.36817531307435+0.172480953932045*Charge2corr");

	AnalyzedTree->SetAlias("q_float2","q_float");
	AnalyzedTree->SetAlias("q_nbr2","q_nbr+1");

	AnalyzedTree->SetAlias("mass2","Aoq2*q_nbr2");
	AnalyzedTree->SetAlias("mass1","a_over_q*(q_nbr)");


	AnalyzedTree->SetAlias("ic_e_a","ic_pads[0]+ic_pads[4]+ic_pads[8]+ic_pads[12]+ic_pads[16]+ic_pads[20]+ic_pads[24]+ic_pads[28]+ic_pads[32]+ic_pads[36]");
	AnalyzedTree->SetAlias("ic_e_b","ic_pads[1]+ic_pads[5]+ic_pads[9]+ic_pads[13]+ic_pads[17]+ic_pads[21]+ic_pads[25]+ic_pads[29]+ic_pads[33]+ic_pads[37]");
	AnalyzedTree->SetAlias("ic_e_c","ic_pads[2]+ic_pads[6]+ic_pads[10]+ic_pads[14]+ic_pads[18]+ic_pads[22]+ic_pads[26]+ic_pads[30]+ic_pads[34]+ic_pads[38]");
	AnalyzedTree->SetAlias("ic_e_d","ic_pads[3]+ic_pads[7]+ic_pads[11]+ic_pads[15]+ic_pads[19]+ic_pads[23]+ic_pads[27]+ic_pads[31]+ic_pads[35]+ic_pads[39]");

}

/*
TCutG* createEllipseCut(int a, int q, double radiusX, double radiusY, int nPoints) {
    // Create arrays to hold the points
    double x[nPoints];
    double y[nPoints];

	double centerX = 1.0*a/q;
	double centerY = q;
	// Calculate the points on the ellipse
    for (int i = 0; i < nPoints; ++i) {
        double angle = 2 * TMath::Pi() * i / nPoints;
        x[i] = centerX + radiusX * TMath::Cos(angle);
        y[i] = centerY + radiusY * TMath::Sin(angle);
    }
    // Create the TCutG object
    TCutG *ellipseCut = new TCutG(Form("a%i_q%i",a,q),nPoints, x, y);
	return ellipseCut;
}
TCutG * createQuadrilateralCut(int a, int q){
	
	double midPointNE = (1.0*(a+4)/(q+1) + 1.0*(a+3)/(q+1))/2.;
	double midPointNW = (1.0*(a+2)/(q+1) + 1.0*(a+3)/(q+1))/2.;
	double midPointSE = (1.0*(a-2)/(q-1) + 1.0*(a-3)/(q-1))/2.;
	double midPointSW = (1.0*(a-4)/(q-1) + 1.0*(a-3)/(q-1))/2.;

	double upWidth = (midPointNE - midPointNW );
	double downWidth = (midPointSE - midPointSW );
	double height = 0.9;
	double x[5] = {midPointNE-0.1*upWidth,midPointNW+0.1*upWidth,midPointSW+0.1*downWidth,midPointSE-0.1*downWidth,midPointNE-0.1*upWidth};
	double y[5] = {q+height/2.,q+height/2.,q-height/2.,q-height/2.,q+height/2.};
	TCutG *quadrilateralCut = new TCutG(Form("a%i_q%i",a,q),5, x, y);
	return quadrilateralCut;
}
TCutG * createRetangularCut(int a, int q){
	
	double midPointNE = (1.0*(a+4)/(q+1) + 1.0*(a+3)/(q+1))/2.;
	double midPointNW = (1.0*(a+2)/(q+1) + 1.0*(a+3)/(q+1))/2.;
	double midPointSE = (1.0*(a-2)/(q-1) + 1.0*(a-3)/(q-1))/2.;
	double midPointSW = (1.0*(a-4)/(q-1) + 1.0*(a-3)/(q-1))/2.;
	double center = 1.0*a/q;

	double upWidth = (midPointNE - midPointNW );
	double downWidth = (midPointSE - midPointSW );
	double height = 0.9;
	double x[5] = {center+0.5*upWidth,center-0.5*upWidth,center-0.5*upWidth,center+0.5*upWidth,center+0.5*upWidth};
	double y[5] = {q+height/2.,q+height/2.,q-height/2.,q-height/2.,q+height/2.};
	TCutG *rectangularCut = new TCutG(Form("a%i_q%i",a,q),5, x, y);
	return rectangularCut;
}

void ionMap(){
	
	double factor = 1;
	TCanvas *c4 = new TCanvas("Qcorr","Qcorr",1000,1000);
	TH2D * hA_Aoqcorr = new TH2D("hA_Aoqcorr","A_{E} vs A/Q with E_{nm} correction;A/Q;Q",1000,2.5,3.6,1000,20,50);
	AnalyzedTree->Draw(Form("charge_cal:a_over_q>>hA_Aoqcorr"),"z_nbr>0","col",1e7);
	TLine *l3_3 = new TLine(3,4e5,3,7e5); l3_3->SetLineColor(kRed); l3_3->SetLineWidth(2); l3_3->Draw("same");

	std::pair<unsigned int,unsigned int> A_range = std::make_pair(50,150);
	std::pair<unsigned int,unsigned int> Q_range = std::make_pair(25,45);

	TGraph *a_over_q_th = new TGraph();

	TGraph *a_over_q_th_25 = new TGraph();
	TGraph *a_over_q_th_30 = new TGraph();
	TGraph *a_over_q_th_35 = new TGraph();
	TGraph *a_over_q_th_40 = new TGraph();
	TGraph *a_over_q_th_45 = new TGraph();

	//TFile * q35_file =	new TFile("qtry.root");
	//TCutG * q35 = (TCutG*)q35_file->Get("qtry");
	//q35->SetName("qtry");
	//TH2D * hA_Aoqcorr_zoom_35 = new TH2D("hA_Aoqcorr_zoom_35","A_{E} vs A/Q with E_{nm} correction;A/Q;E_{corr}/(#gamma - 1)",1000,2.5,3.6,1000,4e5*factor,7e5*factor);
	//AnalyzedTree->Draw(Form("charge_cal:a_over_q>>hA_Aoqcorr_zoom_35",factor),"qtry","",1e7);

	std::map<std::pair<int,int>,std::pair<double,double>> a_over_q_map;

	for (unsigned int a = A_range.first; a < A_range.second; a++){
		for (unsigned int q = Q_range.first; q < Q_range.second; q++){
			double a_over_q  = 1.0*a/q;
			if(a_over_q < 2.2 || a_over_q > 4.0) continue;
			a_over_q_th->SetPoint(a_over_q_th->GetN(),a_over_q,q);
			if(q == 25) a_over_q_th_25->SetPoint(a_over_q_th_25->GetN(),a_over_q,q);
			if(q == 30) a_over_q_th_30->SetPoint(a_over_q_th_30->GetN(),a_over_q,q);
			if(q == 35) a_over_q_th_35->SetPoint(a_over_q_th_35->GetN(),a_over_q,q);
			if(q == 40) a_over_q_th_40->SetPoint(a_over_q_th_40->GetN(),a_over_q,q);
			if(q == 45) a_over_q_th_45->SetPoint(a_over_q_th_45->GetN(),a_over_q,q);

			if(a==A_range.first || a==A_range.second || q==Q_range.first || q==Q_range.second) continue;
			
			double rY = 0.45;
			double rX = 0.;
			rX = std::min((1.0*a/q)-(1.0*(a-1)/q),(1.0*(a+1)/q-(1.0*a/q)))/2.*0.9;

			//TCutG *ellipseCut = createEllipseCut(a, q, rX, rY, 100);

			TCutG *ellipseCut = createRetangularCut(a, q);
			ellipseCut->SetLineColor(kBlack);
			ellipseCut->SetLineWidth(2);
			ellipseCut->Draw("same");
			ellipseCut->SetVarX("a_over_q");
			ellipseCut->SetVarY("charge_cal");



			TH1D * hA_Aoqcorr_ellipse = new TH1D(Form("hA_Aoqcorr_ellipse_%i_%i",a,q),Form("A_{E} vs A/Q with E_{nm} correction;Q;Counts"),1000,2,5);
			AnalyzedTree->Draw(Form("a_over_q>>hA_Aoqcorr_ellipse_%i_%i",a,q),Form("a%i_q%i",a,q),"goff",1e5);
			if(hA_Aoqcorr_ellipse->Integral()>20) a_over_q_map [std::make_pair(a,q)] = std::make_pair(1.0*a/q,hA_Aoqcorr_ellipse->GetMean());
			std::cout << "A: " << a << " Q: " << q << " A/Q: " << 1.0*a/q << " Mean: " << hA_Aoqcorr_ellipse->GetMean() << std::endl;

		}
	}

	a_over_q_th->SetMarkerStyle(20);
	a_over_q_th->SetMarkerSize(0.6);
	a_over_q_th->SetMarkerColor(kRed);
	a_over_q_th_25->SetMarkerStyle(20);
	a_over_q_th_25->SetMarkerSize(0.8);
	a_over_q_th_25->SetMarkerColor(kGreen+1);
	a_over_q_th_30->SetMarkerStyle(20);
	a_over_q_th_30->SetMarkerSize(0.8);
	a_over_q_th_30->SetMarkerColor(kBlue);
	a_over_q_th_35->SetMarkerStyle(20);
	a_over_q_th_35->SetMarkerSize(0.8);
	a_over_q_th_35->SetMarkerColor(kOrange);
	a_over_q_th_40->SetMarkerStyle(20);
	a_over_q_th_40->SetMarkerSize(0.8);
	a_over_q_th_40->SetMarkerColor(kGreen+2);
	a_over_q_th_45->SetMarkerStyle(20);
	a_over_q_th_45->SetMarkerSize(0.8);
	a_over_q_th_45->SetMarkerColor(kRed);

	a_over_q_th->Draw("Psame");
	a_over_q_th_25->Draw("Psame");
	a_over_q_th_30->Draw("Psame");
	a_over_q_th_35->Draw("Psame");
	a_over_q_th_40->Draw("Psame");
	a_over_q_th_45->Draw("Psame");


	TCanvas * a_over_q_map_canvas = new TCanvas("a_over_q_map","a_over_q_map",1000,1000);
	TGraph * a_over_q_map_graph = new TGraph();
	a_over_q_map_graph->SetTitle("A/Q_{mean} - A/Q vs A/Q ; A/Q;A/Q_{mean}");
	for (auto it = a_over_q_map.begin(); it != a_over_q_map.end(); it++){
		if(it->second.second==0) continue;
		a_over_q_map_graph->SetPoint(a_over_q_map_graph->GetN(),it->second.first,it->second.second-it->second.first);
	}
	a_over_q_map_graph->SetMarkerStyle(20);
	a_over_q_map_graph->SetMarkerSize(0.6);
	a_over_q_map_graph->SetMarkerColor(kRed);
	a_over_q_map_graph->Draw("AP");


}

void plotEnergyICvsCalc(){
	TCanvas *c5 = new TCanvas("EnergyICvsCalc","EnergyICvsCalc",1000,1000);
	TH2D * hEIC_ECalc = new TH2D("hEIC_ECalc","E_{IC} vs E_{Calc};E_{IC} (MeV);E_{Calc} (MeV)",1000,0,1000,1000,0,1000);
	
	AnalyzedTree->Draw("energy_m:ic_e_corr>>hEIC_ECalc","z_nbr>0","col",1e7);
	
	

	TLine *l1 = new TLine(0,0,1000,1000); l1->SetLineColor(kRed); l1->SetLineWidth(2); l1->Draw("same");

}

void plotZed(){

	TCanvas *c6 = new TCanvas("Zed","Zed",1000,1000);

	TH2D * hZed = new TH2D("hZed","#Delta E_{A} vs E_{res}",1000,100,4000,2000,500,3000);
	AnalyzedTree->Draw("ic_de_a_corr:ic_e_res_corr>>hZed","x_fp>0.2","col");

	std::pair<int,int> rangeZ = {20,82};

	std::map<int,TCutG*> zedCuts;
	for (int z = rangeZ.first; z < rangeZ.second; z++){
		TFile * z_file =	new TFile(Form("Conf/CUT/PRISMA/ANA/ZED/dee__3__%i.root",z));
		if(z_file->IsZombie()) continue;
		TCutG * z_cut = (TCutG*)z_file->Get(Form("dee__3__%i",z));
		z_cut->SetLineWidth(2);
		z_cut->SetLineColor(z%2==0?kRed:kGreen);
		z_cut->SetHighlight();
		z_cut->SetMarkerStyle(20);
		z_cut->SetMarkerSize(0.6);

		zedCuts[z] = z_cut;
		c6->cd();
		z_cut->Draw("sameLP");

	}


}



void plotICEcorr(){

	TCanvas *c7 = new TCanvas("ICEcorr","ICEcorr",1000,1000);

	std::pair<double,double> rangeLin = {0.065,0.075};
	std::pair<double,double> rangeQuad = {3e-5,4.5e-5};


	int factor = 10;

	TH2D * hQcorr = new TH2D("hQ_corr vs parameters","Q vs parameters;Lin*50+Quad",factor*factor,-0.5,factor*factor-0.5,1000,5e4,8e4);


	for(int i = 0; i < factor; i++){
		for(int j = 0; j < factor; j++){
			double lin = rangeLin.first + 1.0*i*(rangeLin.second-rangeLin.first)/factor;
			double quad = rangeQuad.first + 1.0*j*(rangeQuad.second-rangeQuad.first)/factor;
			TH1D * hQ = new TH1D(Form("hQ_%i",i),Form("Charge; Q (MeV)"),1000,5e4,8e4);
			AnalyzedTree->Draw(Form("(ic_e+%f*ic_de_a+%f*ic_de_a*ic_de_a)/(3.107*(gamma-1))/a_over_q>>hQ_%i",lin,quad,i),"abs(theta_fp*180/3.14159)<1","col",1e6);

			//c7->WaitPrimitive();
			for(int k = 0; k < 1000; k++){
				hQcorr->SetBinContent(hQcorr->GetBin(factor*i+j+1,k+1),hQ->GetBinContent(hQ->GetBin(k)));
			}
			delete hQ;
		}
	}

	hQcorr->Draw("col");

	TCanvas *c8 = new TCanvas("ICEcorr2","ICEcorr2",1000,1000);
	
	TH2D * hQcorr2 = new TH2D("hQ_corr vs parameters","Q vs parameters;Lin*50+Quad",1000,2,4,1000,5e4,8e4);

	AnalyzedTree->Draw("(ic_e+0.09*ic_de_a+3.8e-5*ic_de_a*ic_de_a)/(3.107*(gamma-1))/a_over_q:a_over_q>>hQ_corr vs parameters","abs(theta_fp*180/3.14159)<1","col",1e6);

}

*/

// void plotZedLines(){
// 	TCanvas* cice = new TCanvas("ZedLines","ZedLines",1000,1000);
// 	std::vector<TCutG*> zedLines;
// 	for (int z = 0; z < 56; z++){
// 		//TFile * z_file = new TFile(Form("Conf/CUT/PRISMA/ANA/ZED/23deg/dee__3__%i.root",z));
// 		TFile * z_file = new TFile(Form("Conf/CUT/PRISMA/ANA/ZED_LINES/23deg/zed__%i__%i.root",z,z));
// 		if(z_file->IsZombie()) continue;
// 		//TCutG * zedLine = (TCutG*)z_file->Get(Form("dee__3__%i",z));
// 		TCutG * zedLine = (TCutG*)z_file->Get(Form("zed__%i__%i",z,z));
// 		zedLines.push_back(zedLine);
// 		zedLine->SetLineColor(z%2==0?kRed:kCyan);
// 		if(z==0) zedLine->SetLineColor(kOrange);
// 		zedLine->SetLineWidth(2);
// 		zedLine->SetMarkerStyle(20);
// 		zedLine->SetMarkerSize(0.6);
// 	}

// 	//double a = 5.46936414065817;
// 	//double b = 0.167843333360655;

// 	//double a = -23.4318267771145;
// 	//double b = 0.174820326473741;
	
// 	//double a = -32.2502991511494;	
// 	//double b = 0.180588055718551;

// 	//double a = -20.1403988330936;	
// 	//double b = 0.172186428682321;

// 	double a = -16.6522;
// 	double b = 0.173269;

// 	TH2D * hZedLines = new TH2D("hZedLines","ZedLines",1000,-500,3000,1000,-500,3000);
// 	AnalyzedTree->Draw(Form("(ic_de_a_corr+%f)/%f:(ic_e_res_corr+%f)/%f>>hZedLines",-1*a,b,-1*a,b),"x_fp>0.2","col");

// 	for (auto zedLine : zedLines){
// 		zedLine->Draw("LPsame");
// 	}
// }

// void plotQLines(){
// 	TCanvas* cice = new TCanvas("ZedLines","ZedLines",1000,1000);
// 	std::vector<TCutG*> qLines;
// 	for (int q = 0; q < 56; q++){
// 		//TFile * z_file = new TFile(Form("Conf/CUT/PRISMA/ANA/ZED/23deg/dee__3__%i.root",z));
// 		TFile * q_file = new TFile(Form("Conf/CUT/PRISMA/ANA/Q_LINES/23deg/q__%i__%i.root",q,q));
// 		if(q_file->IsZombie()) continue;
// 		//TCutG * zedLine = (TCutG*)z_file->Get(Form("dee__3__%i",z));
// 		TCutG * qLine = (TCutG*)q_file->Get(Form("q__%i__%i",q,q));
// 		qLines.push_back(qLine);
// 		qLine->SetLineColor(q%2==0?kRed:kCyan);
// 		if(q==0) qLine->SetLineColor(kOrange);
// 		qLine->SetLineWidth(2);
// 		qLine->SetMarkerStyle(20);
// 		qLine->SetMarkerSize(0.6);
// 	}

// 	TH2D * hQlines = new TH2D("hQlines","hQlines",1000,0.06,0.17,1000,18,50);
// 	AnalyzedTree->Draw(Form("charge_cal:beta>>hQlines"),"zed>1&&x_fp>0.2","col");

// 	for (auto qLine : qLines){
// 		qLine->Draw("LPsame");
// 	}
// }

// void plotMCPPPACGates(std::string filename = "Out/sum-27_7.root"){
// 	TCanvas* cice = new TCanvas("PPACGates","PPACGates",1000,1000);
// 	cice->Divide(5,2);
// 	std::vector<TCutG*> ppacGates;

// 	TFile * file = new TFile(filename.c_str());
// 	for (int i = 0; i < 10; i++){
// 		TFile * cut_file = new TFile(Form("Conf/CUT/PRISMA/RAW/PPAC/cath__%i__1.root",i));
// 		if(cut_file->IsZombie()) continue;
// 		TCutG * zedLine = (TCutG*)cut_file->Get(Form("cath__%i__1",i));
// 		ppacGates.push_back(zedLine);
// 		ppacGates.back()->SetLineColor(kOrange);
// 		ppacGates.back()->SetLineWidth(2);
// 		ppacGates.back()->SetMarkerStyle(20);
// 		ppacGates.back()->SetMarkerSize(0.6);
	
// 		if(file->IsZombie()) continue;
// 		TH2D * ppac = (TH2D*)file->Get(Form("Prisma/Raw/PPAC/m_XRpXL_CATHODE_%i",i));
	
// 		cice->cd(i+1);
// 		ppac->Draw("col");
// 		ppacGates[i]->Draw("LPsame");
// 	}

// 	TCanvas* cmcp = new TCanvas("MCPGates","MCPGates",1000,1000);
// 	TH2D * mcp = (TH2D*)file->Get("Prisma/Raw/MCP/m_MCPX_MCPY_raw_0");

// 	TFile * mcp_file = new TFile("Conf/CUT/PRISMA/RAW/MCP/mcp__0__1.root");
// 	TCutG* mcpGate = (TCutG*)mcp_file->Get("mcp__0__1");
// 	mcpGate->SetLineColor(kOrange);
// 	mcpGate->SetLineWidth(2);
// 	mcpGate->SetMarkerStyle(20);

// 	cmcp->cd();
// 	mcp->Draw("col");
// 	mcpGate->Draw("LPsame");	

// }


// void plotCalibrationEnergy(){
// 	std::unordered_map<int,std::vector<TH1D*>> hCal_Zed;

// 	std::pair<double,double> rangeLin = {32,44};

// 	double gamma_m_1 = 0.0055;

// 	std::vector<int> zedRange;
// 	std::vector<std::vector<double>> peaks_ecorr;
// 	std::vector<std::vector<double>> peaks_etot;
// 	std::vector<double> mean_e_nm;


// 	for (int z = rangeLin.second; z > rangeLin.first; z--){
// 		zedRange.push_back(z);
// 		TH1D * h_e_corr = new TH1D(Form("h_E_corr_%i",z),Form("Calibration Energy Zed %i",z),1000,0,150);
// 		TH1D * h_e_tot = new TH1D(Form("h_E_tot_%i",z),Form("Calibration Energy Zed %i",z),1000,0,150);
// 		TH1D * h_e_nm = new TH1D(Form("h_Enm_%i",z),Form("Calibration Energy Zed %i",z),1000,0,50);

// 		AnalyzedTree->Draw(Form("ic_e_corr/(931.49432*(gamma-1))>>h_E_corr_%i",z),Form("abs(a_over_q-3)<0.02&&z_nbr>0&&abs(gamma-1-%f)<0.00008&&z_nbr==%i",gamma_m_1,z),"col",1e7);
// 		AnalyzedTree->Draw(Form("ic_e_tot/(931.49432*(gamma-1))>>h_E_tot_%i",z),Form("abs(a_over_q-3)<0.02&&z_nbr>0&&abs(gamma-1-%f)<0.00008&&z_nbr==%i",gamma_m_1,z),"col",1e7);
// 		AnalyzedTree->Draw(Form("ic_e_nm/(931.49432*(gamma-1))>>h_Enm_%i",z),Form("abs(a_over_q-3)<0.02&&z_nbr>0&&abs(gamma-1-%f)<0.00008&&z_nbr==%i",gamma_m_1,z),"col",1e7);
// 		hCal_Zed[z] = {h_e_corr,h_e_tot,h_e_nm};
// 	}

// 	int n_max_peaks = 5;

// 	std::ofstream file("calibrationEnergy.txt");
// 	TFile * hfile = new TFile("calibrationEnergy.root","RECREATE");
// 	for (auto z : zedRange){

// 		auto iter = hCal_Zed.find(z);

// 		if (iter == hCal_Zed.end()) continue;

// 		auto it = *iter;

// 		for (auto h : it.second){
// 			h->Write();
// 		}
// 		TSpectrum *s = new TSpectrum(n_max_peaks);
// 		s->Search(hCal_Zed[it.first][0],2,"",0.1);
// 		std::vector<double> xpeaks(n_max_peaks,0);
// 		for (int i = 0; i < s->GetNPeaks(); ++i) {
// 			xpeaks[i] = s->GetPositionX()[i];
// 		}
// 		std::sort(xpeaks.begin(),xpeaks.end(),[](double a, double b){return a > b;});
// 		peaks_ecorr.push_back(xpeaks);

// 		TSpectrum *s2 = new TSpectrum(n_max_peaks);
// 		s2->Search(hCal_Zed[it.first][1],2,"",0.1);
// 		std::vector<double> xpeaks2(n_max_peaks,0);
// 		for (int i = 0; i < s2->GetNPeaks(); ++i) {
// 			xpeaks2[i] = s2->GetPositionX()[i];
// 		}
// 		std::sort(xpeaks2.begin(),xpeaks2.end(),[](double a, double b){return a > b;});
// 		peaks_etot.push_back(xpeaks2);

		
// 		double mean_e_nm_value = hCal_Zed[it.first][2]->GetMean();
// 		mean_e_nm.push_back(mean_e_nm_value);
// 	}
// 		//file << mean_e_nm << std::endl;
// 		//file << std::endl;
// 		//for (auto x : xpeaks){
// 		//	file << x << std::endl;
// 		//}
// 		//file << std::endl;
// 		//for (auto x : xpeaks2){
// 		//	file << x << std::endl;
// 		//}
// 		//file << std::endl;
// 		//file << std::endl;

// 	for (int i = 0; i < zedRange.size(); i++){
// 		int z = zedRange[i];
// 		file << "\t"<< z << "\t" ;
// 	}
// 	file << std::endl;
// 	for (int i = 0; i < zedRange.size(); i++){
// 		file << "\t" << mean_e_nm[i] << "\t";
// 	}
// 	file << std::endl;
// 	file << std::endl;
// 	for (int i = 0; i < n_max_peaks; i++){
// 		for (int j = 0; j < zedRange.size(); j++){
// 			if (i < peaks_ecorr[j].size()){
// 				file << "\t" << peaks_ecorr[j][i] << "\t";
// 			}
// 			else{
// 				file << "\t" << 0 << "\t";
// 			}
// 		}
// 		file << std::endl;
// 	}
// 	file << std::endl;
// 	file << std::endl;
// 	for (int i = 0; i < n_max_peaks; i++){
// 		for (int j = 0; j < zedRange.size(); j++){
// 			if (i < peaks_etot[j].size()){
// 				file << "\t" << peaks_etot[j][i] << "\t";
// 			}
// 			else{
// 				file << "\t" << 0 << "\t";
// 			}
// 		}
// 		file << std::endl;
// 	}

// }


// int extractNumber(const std::string& input) {
//     // Find the positions of the delimiters
//     size_t first_delim = input.find("__");
//     size_t second_delim = input.find("_", first_delim + 2);
//     size_t third_delim = input.find("__", second_delim + 1);

//     if (first_delim == std::string::npos || second_delim == std::string::npos || third_delim == std::string::npos) {
//         std::cerr << "Invalid format" << std::endl;
//         return -1;
//     }

//     // Extract the substring between the second and third delimiters
//     std::string number_str = input.substr(second_delim + 1, third_delim - second_delim - 1);

//     // Convert the substring to an integer
//     int number = std::stoi(number_str);

//     return number;
// }

// void plotzedEnmLines(std::string folder, int thickness){
// 	TCanvas* cice = new TCanvas("zedEnmLines","zedEnmLines",1000,1000);
// 	std::vector<TGraph*> zedEnmLines;
// 	for (int z = 26; z < 60; z++){
// 		TFile * z_file = new TFile(Form("%s/icenmzbeta__%3d_%d__0.root",folder.c_str(),thickness,z));
// 		if(z_file->IsZombie()) continue;
// 		//TCutG * zedLine = (TCutG*)z_file->Get(Form("dee__3__%i",z));
// 		TGraph * zedLine = (TCutG*)z_file->Get(Form("icenmzbeta__%3d_%d__0",thickness,z));
// 		zedEnmLines.push_back(zedLine);
// 	}

// 	TGraph * axisframe = new TGraph();
// 	axisframe->SetTitle("E_{nm} vs Beta by Z;Beta ;E_{nm} (MeV)");
// 	axisframe->SetPoint(axisframe->GetN(),0.06,10);
// 	axisframe->SetPoint(axisframe->GetN(),0.16,90);
// 	axisframe->SetMarkerSize(0.00001);
// 	axisframe->SetMarkerStyle(20);
// 	axisframe->SetMarkerColor(kWhite);
// 	axisframe->Draw("AP");

// 	TLegend * leg = new TLegend(0.1,0.1,0.4,0.3);
// 	leg->SetNColumns(3);

// 	for (auto zedLine : zedEnmLines){
// 		zedLine->Draw("LPsame");
// 		int z = extractNumber((std::string) zedLine->GetName());
// 		leg->AddEntry(zedLine,Form("%d",z),"LP");
// 	}
// 	leg->Draw("same");

	

// }


// std::pair<double,double> findHistEdge(TH1D * hist ){
//    double max = hist-> GetMaximum();
//    double leftEdge = 0;
//    double rightEdge = 0;
//    bool leftFound = false;

//    double heightfactor = 0.01;

//    for (int i = 1; i < hist->GetNbinsX(); ++i) {
//        double binContent = hist->GetBinContent(i);
//        if (binContent > max*heightfactor+1) {
//                leftEdge = hist->GetBinCenter(i);
//                break;
//        }
//    }
//    for (int i = (hist->GetNbinsX()-1); i > 0; --i) {
//        double binContent = hist->GetBinContent(i);
//        if (binContent > max*heightfactor+1) {
//                rightEdge = hist->GetBinCenter(i);
//                break;
//        }
//    }
//    return std::make_pair(leftEdge,rightEdge);
// }


// double multiGaussian(double *x, double *par){
// 	double value = 0;
// 	for (int i = 0; i < par[0]; i++){
// 		value += par[i*3+1]*TMath::Gaus(x[0],par[i*3+2],par[i*3+3]);
// 	}
// 	return value;
// }



// void fitZedAplot(std::pair<int,int> z_range, std::pair<int,int> a_range,std::string printGraphs = ""){
	

// 	TFile * cutFile = new TFile("goodEvts.root");
// 	TCutG * goodEvts = (TCutG*)cutFile->Get("goodEvts");
// 	goodEvts->SetVarX("mass");
// 	goodEvts->SetVarY("zed");

// 	TH2D* m_Zed_mass = new TH2D("m_Zed_mass","m_Zed_mass",1000,50,150,1000,20,60);


// 	TCanvas *c_z_a = new TCanvas("c_z_a","c_z_a",1000,1000);
// 	AnalyzedTree->Draw("zed:mass>>m_Zed_mass","goodEvts&&tof_ok&&mcp_ok","col");

// 	std::vector<TCanvas*> canvases;

// 	std::map<int,TGraph*> zedVsA;

// 	std::map<int,TF1*> zedVsA_gaussian;
// 	std::map<int,TF1*> zedVsA_fit;
// 	std::map<int,TF1*> zedVsA_fit_smooth;

// 	std::map<int,TSpectrum*> zedSpectra;


// 	int nCanvas = 0;
// 	int nPad = 0;

// 	int nDiv = 4;

// 	for(int z = z_range.first; z <= z_range.second; z++){
// 		zedVsA[z] = new TGraph();
// 	}
// 	for(int a = a_range.first; a < a_range.second; a++){

// 		TH1D * hZed = m_Zed_mass->ProjectionY(Form("hZed_%i",a),m_Zed_mass->GetXaxis()->FindBin(a-0.49),m_Zed_mass->GetXaxis()->FindBin(a+0.49));
// 		hZed->Rebin(2);
		
// 		if(nPad%(nDiv*nDiv)==0){
// 			canvases.push_back(new TCanvas(Form("c%i",nCanvas),Form("c%i",nCanvas),1000,1000));
// 			canvases.back()->Divide(nDiv,5);
// 			nCanvas++;
// 		}
		
// 		canvases.back()->cd(nPad%(nDiv*nDiv)+1);
// 		nPad++;

// 		hZed->Draw();

// 		auto edges = findHistEdge(hZed);
		
// 		int nr_z = round(edges.second) - round(edges.first) + 1;

// 		zedVsA_gaussian[a] = new TF1(Form("f%i",a),multiGaussian,edges.first-3,edges.second+3,nr_z*3+1);
// 		zedVsA_gaussian[a]->FixParameter(0,nr_z);

// 		double max = hZed->GetMaximum();

// 		for(int i = 0; i < nr_z; i++){
// 			zedVsA_gaussian[a]->SetParameter(i*3+1, max/2.); 						zedVsA_gaussian[a]->SetParLimits(i*3+1,0, max*2); 
// 			zedVsA_gaussian[a]->SetParameter(i*3+2,round(edges.first)+i);		zedVsA_gaussian[a]->SetParLimits(i*3+2,round(edges.first)+i-0.4, round(edges.first)+i+0.4);
// 			zedVsA_gaussian[a]->SetParameter(i*3+3,0.2);						zedVsA_gaussian[a]->SetParLimits(i*3+3,0.1,0.4);
// 		}
// 		hZed->Fit(Form("f%i",a),"R");

// 		for(int i = 0; i < nr_z; i++){
// 			if(zedVsA_gaussian[a]->GetParameter(i*3+2) < z_range.first || zedVsA_gaussian[a]->GetParameter(i*3+2) > z_range.second) continue;
// 			int z_int = round( zedVsA_gaussian[a]->GetParameter(i*3+2));
// 			if( abs(zedVsA_gaussian[a]->GetParameter(i*3+2)-(round(edges.first)+i-0.4))<0.0001 || abs(zedVsA_gaussian[a]->GetParameter(i*3+2)-(round(edges.first)+i+0.4))<0.0001) continue;
// 			if(z_int < z_range.first || z_int > z_range.second) continue;
// 			zedVsA[z_int]->SetPoint(zedVsA[z_int]->GetN(),a,zedVsA_gaussian[a]->GetParameter(i*3+2));
// 		}

// 		std::vector<TF1*> gauss_draw;
// 		for(int i = 0; i < nr_z; i++){
// 			if(zedVsA_gaussian[a]->GetParameter(i*3+2) < z_range.first || zedVsA_gaussian[a]->GetParameter(i*3+2) > z_range.second) continue;
// 			gauss_draw.push_back(new TF1(Form("f%i_%i",a,i),multiGaussian,z_range.first,z_range.second,4));
// 			gauss_draw.back()->FixParameter(0,1);
// 			gauss_draw.back()->SetParameter(1,zedVsA_gaussian[a]->GetParameter(i*3+1));
// 			gauss_draw.back()->SetParameter(2,zedVsA_gaussian[a]->GetParameter(i*3+2));
// 			gauss_draw.back()->SetParameter(3,zedVsA_gaussian[a]->GetParameter(i*3+3));
// 			gauss_draw.back()->SetLineColor(kBlue+i);
// 			gauss_draw.back()->SetLineWidth(2);
// 			gauss_draw.back()->SetLineStyle(3);
// 			gauss_draw.back()->SetNpx(10000);
// 			gauss_draw.back()->Draw("same");
// 		}



// 		//zedSpectra[a] = new TSpectrum(5);
// 		//zedSpectra[a]->Search(hZed,0.3,"",0.05);
// 		//std::vector<double> xpeaks(zedSpectra[a]->GetNPeaks());
// 		//for (int i = 0; i < zedSpectra[a]->GetNPeaks(); ++i) {
// 		//	xpeaks[i] = zedSpectra[a]->GetPositionX()[i];
// 		//	int z_int  = (int) xpeaks[i];
// 		//	zedVsA[z_int]->SetPoint(zedVsA[z_int]->GetN(),a,xpeaks[i]);
// 		//}		
		
// 	}
	
// 	TCanvas * c = new TCanvas("fit_zed_a","fit_zed_a",1000,1000);
// 	c->Divide(6,5);
// 	int count = 0;
// 	for(int z = z_range.first; z < z_range.second; z++){
// 		c->cd(count+1);
// 		if(zedVsA[z]->GetN() == 0) continue;
// 		zedVsA[z]->SetMarkerStyle(20);
// 		zedVsA[z]->SetMarkerSize(0.6);
// 		zedVsA[z]->SetMarkerColor(kViolet);
// 		zedVsA[z]->SetTitle(Form("Zed %i",z));
// 		zedVsA[z]->Draw("AP");
// 		double midrange = (zedVsA[z]->GetX()[zedVsA[z]->GetN()-1]-zedVsA[z]->GetX()[0])/2;
// 		zedVsA_fit[z] = new TF1(Form("f%i",z),"pol1",zedVsA[z]->GetMean(1)-midrange*2,zedVsA[z]->GetMean(1)+midrange*2);
// 		zedVsA[z]->Fit(Form("f%i",z),"R","",zedVsA[z]->GetMean(1)-midrange*0.6,zedVsA[z]->GetMean(1)+midrange*0.6);
// 		count++;
// 	}
	
// 	std::ofstream file(Form("zed_mass.cal"));			

// 	for(int z = z_range.first; z < z_range.second; z++){
// 		if(zedVsA_fit[z] == nullptr) continue;
// 		double a = zedVsA_fit[z]->GetParameter(0);
// 		double b = zedVsA_fit[z]->GetParameter(1);
// 		file << z << " " << a << " " << b << std::endl;
// 	}

// 	std::ofstream file2(Form("zed_mass_points.cal"));
// 	for(int z = z_range.first; z < z_range.second; z++){
// 		for(int i = 0; i < zedVsA[z]->GetN(); i++){
// 			double x,y;
// 			zedVsA[z]->GetPoint(i,x,y);
// 			file2 << z << " " << x << " " << y << std::endl;
// 		}
// 	}

// 	TCanvas * parameterCanvas = new TCanvas("parameters","parameters",1000,1000);
// 	parameterCanvas->Divide(2,1);

// 	TGraph* slope = new TGraph();
// 	TGraph* intercept = new TGraph();
	
// 	for(int z = z_range.first; z < z_range.second; z++){
// 		if(zedVsA_fit[z] == nullptr) continue;
// 		slope->SetPoint(slope->GetN(),z,zedVsA_fit[z]->GetParameter(1));
// 		intercept->SetPoint(intercept->GetN(),z,zedVsA_fit[z]->GetParameter(0));
// 	}

// 	parameterCanvas->cd(1);
// 	slope->SetTitle("Slope vs Zed;Zed;Slope");
// 	slope->SetMarkerStyle(20);
// 	slope->SetMarkerSize(1.2);
// 	slope->SetMarkerColor(kViolet);
// 	slope->Draw("AP");

// 	TF1 * slope_fit = new TF1("slope_fit","pol2",z_range.first,z_range.second);
// 	slope->Fit("slope_fit","","",z_range.first+2.5,z_range.second);

// 	parameterCanvas->cd(2);
// 	intercept->SetTitle("Intercept vs Zed;Zed;Intercept");
// 	intercept->SetMarkerStyle(23);
// 	intercept->SetMarkerSize(1.2);
// 	intercept->SetMarkerColor(kCyan+2);
// 	intercept->Draw("AP");

// 	TF1 * intercept_fit = new TF1("intercept_fit","pol2",z_range.first,z_range.second);
// 	intercept->Fit("intercept_fit","","",z_range.first+2.5,z_range.second);
	
// 	c_z_a->Divide(2,1);
// 	c_z_a->cd(1);
// 	m_Zed_mass->Draw("col");

// 	gStyle->SetOptStat(0);
// 	for(int z = z_range.first; z <= z_range.second; z++){
// 		if(zedVsA_fit[z] == nullptr) continue;
// 		zedVsA_fit[z]->SetLineColor(kViolet);
// 		zedVsA_fit[z]->SetLineWidth(2);
// 		zedVsA_fit[z]->Draw("sameL");
// 	}
// 	c_z_a->cd(2);
// 	m_Zed_mass->Draw("col");

// 	gStyle->SetOptStat(0);

// 	for(int z = z_range.first; z <= z_range.second; z++){
// 		if(zedVsA_fit[z] == nullptr) continue;
// 		zedVsA_fit_smooth[z] = new TF1(Form("f_smooth%i",z),"pol1",zedVsA[z]->GetX()[0],zedVsA[z]->GetX()[zedVsA[z]->GetN()-1]);
// 		zedVsA_fit_smooth[z]->SetParameter(0,intercept_fit->Eval(z));
// 		zedVsA_fit_smooth[z]->SetParameter(1,slope_fit->Eval(z));
// 		zedVsA_fit_smooth[z]->SetLineColor(kCyan+2);
// 		zedVsA_fit_smooth[z]->SetLineWidth(2);
// 		zedVsA_fit_smooth[z]->Draw("sameL");
// 	}



// 	if(printGraphs != ""){
// 		for(int z = z_range.first; z <= z_range.second; z++){
// 			if(zedVsA_fit[z] == nullptr) continue;
// 			double  xmin,xmax;
// 			zedVsA_fit[z]->GetRange(xmin,xmax);
// 			TGraph * zedVsA_fit_graph = new TGraph();
// 			zedVsA_fit_graph->SetName(Form("zvsa__%i__%i",z,z));
// 			zedVsA_fit_graph->SetTitle(Form("zvsa__%i__%i",z,z));
// 			zedVsA_fit_graph->SetPoint(0,xmin,zedVsA_fit[z]->Eval(xmin));
// 			zedVsA_fit_graph->SetPoint(1,xmax,zedVsA_fit[z]->Eval(xmax));
// 			TFile * zed_file = new TFile(Form("%s/zvsa__%i__%i.root",printGraphs.c_str(),z,z),"RECREATE");
// 			zedVsA_fit_graph->Write();
// 			zed_file->Close();
// 		}
// 	}

// }

// double lineWithHole(double *x, double *par){
// 	if(x[0] > par[2] && x[0] < par[3]) return 0;
// 	return par[0]+x[0]*par[1];
// }

// void fillHolesXfp(){ //Not to be used here
// 	TH1D * xfp = new TH1D("xfp","xfp",2000,0,1);
// 	xfp->SetLineColor(kBlack);
// 	AnalyzedTree->Draw("x_fp>>xfp","","col");

// 	std::vector<TF1*> xfp_fits;
// 	std::vector<TF1*> xfp_lines;
// 	std::vector<TGraph*> xfp_graphs;
	
// 	for(int i = 0; i < 10; i++){
		
// 		if(i>0) {
// 			xfp_fits.push_back(new TF1(Form("f_edge_%i",i),lineWithHole,i*0.1-0.01,i*0.1+0.01,4));
// 			xfp_fits.back()->SetParameter(0,100);
// 			xfp_fits.back()->SetParameter(1,0);
// 			xfp_fits.back()->FixParameter(2,i*0.1-0.005);
// 			xfp_fits.back()->FixParameter(3,i*0.1+0.005);
// 			xfp->Fit(Form("f_edge_%i",i),"RN+");
// 			xfp_fits.back()->SetLineColor(kRed);
// 			xfp_fits.back()->SetLineWidth(2);
// 			xfp_fits.back()->Draw("same");

// 			xfp_graphs.push_back(new TGraph());			
			
// 			xfp_lines.push_back(new TF1(Form("f_edge_line_%i",i),"pol1",i*0.1-0.01,i*0.1+0.01));
// 			xfp_lines.back()->SetParameter(0,xfp_fits.back()->GetParameter(0));
// 			xfp_lines.back()->SetParameter(1,xfp_fits.back()->GetParameter(1));
// 			xfp_lines.back()->SetLineColor(kViolet+2);
// 			xfp_lines.back()->SetLineStyle(7);
// 			xfp_lines.back()->Draw("same");
// 			for(int j = 0; j < xfp->GetNbinsX(); j++){
// 				double x = xfp->GetBinCenter(j);
// 				double y = xfp->GetBinContent(j)/xfp_lines.back()->Eval(x);
// 				if(x > i*0.1-0.01 && x < i*0.1+0.01) xfp_graphs.back()->SetPoint(xfp_graphs.back()->GetN(),x,y);
// 			}
// 		}
// 	}
// 	for(int i = 0; i < 10; i++){
// 		xfp_fits.push_back(new TF1(Form("f_mid_%i",i),lineWithHole,i*0.1+0.04,i*0.1+0.06,4));
// 		xfp_fits.back()->SetParameter(0,100);
// 		xfp_fits.back()->SetParameter(1,0);
// 		xfp_fits.back()->FixParameter(2,i*0.1+0.045);
// 		xfp_fits.back()->FixParameter(3,i*0.1+0.055);

// 		xfp->Fit(Form("f_mid_%i",i),"R+");
// 		xfp_fits.back()->SetLineColor(kRed);
// 		xfp_fits.back()->SetLineWidth(2);
// 		xfp_fits.back()->Draw("same");

// 		xfp_graphs.push_back(new TGraph());			
		
// 		xfp_lines.push_back(new TF1(Form("f_mid_line_%i",i),"pol1",i*0.1+0.04,i*0.1+0.06));
// 		xfp_lines.back()->SetParameter(0,xfp_fits.back()->GetParameter(0));
// 		xfp_lines.back()->SetParameter(1,xfp_fits.back()->GetParameter(1));
// 		xfp_lines.back()->SetLineColor(kViolet+2);
// 		xfp_lines.back()->SetLineStyle(7);
// 		xfp_lines.back()->Draw("same");
// 		for(int j = 0; j < xfp->GetNbinsX(); j++){
// 			double x = xfp->GetBinCenter(j);
// 			double y = xfp->GetBinContent(j)/xfp_fits.back()->Eval(x);
// 			if(x > i*0.1+0.04 && x < i*0.1+0.06) xfp_graphs.back()->SetPoint(xfp_graphs.back()->GetN(),x,y);
// 		}
// 	}


// }

// std::vector<TGraph*> readLiseFile(const std::string& filename) {
//     std::ifstream file(filename);
//     if (!file.is_open()) {
//         std::cerr << "Error: Cannot open file " << filename << std::endl;
//         return {};
//     }

//     std::vector<TGraph*> graphs;
//     std::string line;
//     std::vector<double> x, y, x2, y2;

// 	std::getline(file, line);
// 	std::getline(file, line);
// 	std::getline(file, line);
// 	std::getline(file, line);
// 	std::getline(file, line);
//     while (std::getline(file, line)) {
    
// 		std::istringstream iss(line);
	
//         double x_val, y_val, x2_val, y2_val;
//         iss >> x_val >> y_val >> x2_val >> y2_val;
//         x.push_back(x_val);
//         y.push_back(y_val);
// 		x2.push_back(x2_val);
// 		y2.push_back(y2_val);

//     }

//     // Create a TGraph for the last set of points if any
//     if (!x.empty() && !y.empty()) {
//         TGraph* graph = new TGraph(x.size(), x.data(), y.data());
//         graphs.push_back(graph);
//     }

//     file.close();
//     return graphs;
// }

// void plotKinematicsPb(double energyScaleNi = 1,double  energyScaleZn = 1, double thetaShift = 0){
// 	TCanvas * c = new TCanvas("c","c",1000,1000);
// 	gPad->SetLeftMargin(0.15);
// 	TH2D * EvsTheta = new TH2D("EvsTheta","Energy vs Theta of scattered Pb; Theta (deg); Energy (MeV)",1000,12,24,1000,300,1000);
// 	AnalyzedTree->Draw("energy_m:theta*TMath::RadToDeg()>>EvsTheta","x_fp>0.2&&a_nbr==208&&z_nbr==82&&TS<14.3e12","col");

// 	gStyle->SetOptStat(0);
// 	std::vector<TGraph*> graphs_64Ni = readLiseFile("208Pb_64Ni_kin.txt");
// 	std::vector<TGraph*> graphs_70Zn = readLiseFile("208Pb_70Zn_kin.txt");

// 	for (auto graph : graphs_64Ni) {
// 		for(int i = 0; i < graph->GetN(); i++){
// 			double x,y;
// 			graph->GetPoint(i,x,y);
// 			graph->SetPoint(i,x+thetaShift,y*energyScaleNi*208);
// 		}
// 	}
// 	for (auto graph : graphs_70Zn) {
// 		for(int i = 0; i < graph->GetN(); i++){
// 			double x,y;
// 			graph->GetPoint(i,x,y);
// 			graph->SetPoint(i,x+thetaShift,y*energyScaleZn*208);
// 		}
// 	}

// 	graphs_64Ni[0]->SetLineColor(kRed);
// 	graphs_70Zn[0]->SetLineColor(kBlue);
// 	graphs_64Ni[0]->SetLineWidth(2);
// 	graphs_70Zn[0]->SetLineWidth(2);

// 	graphs_64Ni[0]->Draw("Lsame");
// 	graphs_70Zn[0]->Draw("Lsame");

// }


// void compareMCPRuns(std::string filename1, std::string filename2){
// 	TCanvas * c = new TCanvas("c","c",1600,1000);
// 	//TH2D * mcp_run27 = new TH2D("mcp_run27","mcp_run27",1000,-0.05,0.05,1000,-0.05,0.05);
// 	//TH2D * mcp_run40 = new TH2D("mcp_run40","mcp_run40",1000,-0.05,0.05,1000,-0.05,0.05);
	
// 	TFile * file1 = new TFile(filename1.c_str());
// 	TFile * file2 = new TFile(filename2.c_str());
	
// 	TTree * tree1 = (TTree*)file1->Get("AnalyzedTree");
// 	TTree * tree2 = (TTree*)file2->Get("AnalyzedTree");

// 	gStyle->SetOptStat(0);

// 	c->Divide(2,1);
// 	c->cd(1);
// 	gPad->SetGridx();
// 	gPad->SetGridy();
// 	tree1->SetAlias("phi_deg","(phi>0?phi*TMath::RadToDeg():(phi+2*TMath::Pi())*TMath::RadToDeg())");
// 	tree1->Draw("phi_deg:theta*TMath::RadToDeg()-20>>mcp_run27(1000,-10,10,1000,140,220)","tof_ok&&mcp_ok","col",2e7);
// 	c->cd(2);
// 	gPad->SetGridx();
// 	gPad->SetGridy();
// 	tree2->SetAlias("phi_deg","(phi>0?phi*TMath::RadToDeg():(phi+2*TMath::Pi())*TMath::RadToDeg())");
// 	tree2->Draw("phi_deg:theta*TMath::RadToDeg()-23>>mcp_run40(1000,-10,10,1000,140,220)","tof_ok&&mcp_ok","col",2e7);	
// }