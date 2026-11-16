void plotEn1vsXY(){
	long double nentries = 5e7;

	TCanvas * cz = new TCanvas("ICDEAvsICEres","ICDEAvsICEres",1000,1000);
	TH2D * h_ICDEAvsICEres = new TH2D("h_ICDEAvsICEres","h_ICDEAvsICEres;IC_DE_A; IC_E_RES",1000,500,4000,1000,500,3000);
	AnalyzedTree->Draw("ic_de_a:(ic_e-ic_de_a)>>h_ICDEAvsICEres","x_fp>0.18&&abs(theta_fp)<5*TMath::DegToRad()","colz",nentries);

	TCanvas * czrot = new TCanvas("en1vsen2","en1vsen2",1000,1000);
	TH2D * h_ICDEAvsICEresRot = new TH2D("h_en1vsen2","h_en1vsen2;en2;en1",1000,500,4000,1000,500,3000);
	AnalyzedTree->Draw("ic_en1:ic_en2>>h_en1vsen2","x_fp>0.18&&abs(theta_fp)<5*TMath::DegToRad()","colz",nentries);

	TFile * spline_file_x = new TFile("Conf/CUT/PRISMA/ANA/ICDEA_XFP/icdeaxfp__0__0.root");
	TFile * spline_file_y = new TFile("Conf/CUT/PRISMA/ANA/ICDEA_YFP/icdeayfp__0__0.root");
	TGraph * spline_graph_x = (TGraph *) spline_file_x->Get("icdeaxfp__0__0");
	TGraph * spline_graph_y = (TGraph *) spline_file_y->Get("icdeayfp__0__0");
	TSpline3 * spline_x = new TSpline3("spline_x", spline_graph_x);
	TSpline3 * spline_y = new TSpline3("spline_y", spline_graph_y);

	TCanvas * c1 = new TCanvas("en1vsX","en1vsX",1000,1000);
	TH2D * h_en1vsX = new TH2D("h_en1vsX","h_en1vsX;X_FP (m); IC_DE_A",1000,0,1,1000,1000,3000);
	AnalyzedTree->Draw("ic_en1:x_fp>>h_en1vsX","x_fp>0.180&&abs(y_fp)<0.01&&ic_en2>1000&&ic_en2<1500&&abs(theta_fp)<5*TMath::DegToRad()","colz",nentries);
	spline_x->Draw("same");
	spline_x->SetLineColor(kRed);
	spline_graph_x->SetMarkerStyle(20);
	spline_graph_x->Draw("Psame");

	TCanvas * c2 = new TCanvas("en1vsY","en1vsY",1000,1000);
	TH2D * h_en1vsY = new TH2D("h_en1vsY","h_en1vsY;Y_FP (m); IC_DE_A",1000,-0.065,0.065,1000,1000,3000);
	AnalyzedTree->Draw("ic_en1:y_fp>>h_en1vsY","x_fp>0.180&&abs(x_fp-0.5)<0.05&&ic_en2>1000&&ic_en2<1500&&abs(theta_fp)<5*TMath::DegToRad()","colz",nentries);
	spline_y->Draw("same");
	spline_y->SetLineColor(kRed);
	spline_graph_y->SetMarkerStyle(20);
	spline_graph_y->Draw("Psame");

}