void changeNames(){
	int a, z;
	a = 6, z = 14;

	for(int p=0; p<16; p++){
	  for(int s=4*(p%4); s<4*(p%4)+4; s++){
            std::string oldName = "A" + to_string(a) + "_Z" + to_string(z) + "_p" + to_string(p) + "_s" + to_string(s);
            std::string newName = "A" + to_string(z) + "_Z" + to_string(a) + "_p" + to_string(p) + "_s" + to_string(s);
	    std::cout << "Changing " << oldName << " to " << newName << std::endl;
            TFile* file = TFile::Open((oldName+".root").c_str());
            TCutG* cut = (TCutG*)file->Get(oldName.c_str());
            cut->SetName(newName.c_str());
            TFile* newfile = new TFile((newName+".root").c_str(),"recreate");
            cut->Write();
            newfile->Write();
            newfile->Close();
	  }
	}
	
	//a = 8, z = 16;
	/*
	for(int p=0; p<16; p++){
	  for(int s=4*(p%4); s<4*(p%4)+4; s++){
            std::string oldName = "A" + to_string(a) + "_Z" + to_string(z) + "_p" + to_string(p) + "_s" + to_string(s);
            std::string newName = "A" + to_string(z) + "_Z" + to_string(a) + "_p" + to_string(p) + "_s" + to_string(s);
	    std::cout << "Changing " << oldName << " to " << newName << std::endl;
            TFile* file = TFile::Open((oldName+".root").c_str());
            TCutG* cut = (TCutG*)file->Get(oldName.c_str());
            cut->SetName(newName.c_str());
            TFile* newfile = new TFile((newName+".root").c_str(),"recreate");
            cut->Write();
            newfile->Write();
            newfile->Close();
	  }
	}
	*/
	
}
