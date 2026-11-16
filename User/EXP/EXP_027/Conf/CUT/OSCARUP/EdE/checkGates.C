void checkGates(){
	TFile* fileSel = TFile::Open("test.root");
	
	std::vector<std::pair<int,int>>ions{{16,8},{14,6}};
	//std::vector<std::pair<int,int>>ions{{14,6}};

	for(int p=0; p<16; p++){									
	  for(int s=4*(p%4); s<4*(p%4)+4; s++){
            std::string histName = "m_dE_E_" + to_string(p) + "_" + to_string(s);
	    std::cout << "Reading histo " << histName << std::endl;
	    fileSel->cd();
	    TH2D* hist;
	    hist = fileSel->Get<TH2D>(Form("OscarUp/EdE/%s",histName.c_str()));
	    std::vector<TCutG*> cutList;
	    for(auto ion:ions){
              std::string cutName = "A" + to_string(ion.first) + "_Z" + to_string(ion.second) + "_p" + to_string(p) + "_s" + to_string(s);
	      try{
                //TFile* file = TFile::Open(("wideCarbonGates/"+cutName+".root").c_str(),"read");
                TFile* file = TFile::Open((cutName+".root").c_str(),"read");
                cutList.push_back((TCutG*)file->Get(cutName.c_str()));
	        file->Close();
	      }
	      catch(...){
	        std::cout << "Cut " << cutName << " not found" << std::endl;
	      }
	    }
	    TCanvas * tc = new TCanvas(Form("p%i_s%i",p,s),Form("p%i_s%i",p,s),100,100,1200,800);
	    hist->Draw("colz");
	    for(auto cut:cutList) cut->SetLineColor(kRed);
	    for(auto cut:cutList) cut->SetLineWidth(3);
	    for(auto cut:cutList) cut->Draw("same");
	    //std::cin.get();
	    //delete tc;
	  }
	}
}
