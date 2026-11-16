TCutG* anaCV(TCanvas* cv, int b, int c, std::pair<int, int> p, TCutG* oldcut){

	if(oldcut) oldcut->Draw("pl");
	cv->WaitPrimitive();
	auto cut = dynamic_cast<TCutG*>(cv->GetPrimitive("CUTG"));
	std::stringstream name;
	name << "A";
	name << p.first;
	name << "_Z";
	name << p.second;
	name << "_p";
	name << b;
	name << "_s";
	name << c;
	cut->SetName(name.str().c_str());
	name << ".root";
	cut->SaveAs(name.str().c_str());
	cut->SetLineColor(kRed+1);
	cut->SetMarkerStyle(20);
	return static_cast<TCutG*>(cut->Clone("CUTG"));
}

void makeGates(std::string inFile="test.root", 
                std::string dir="OscarUp/EdE",
                std::string matrixPattern="m_dE_E_",
                std::vector<std::pair<int,int>> particles={{14,6},}){

        TFile f(inFile.c_str(), "read");
        TDirectory* tdir = dynamic_cast<TDirectory*>(f.Get(dir.c_str()));
        TIter contents(tdir->GetListOfKeys());
        TKey*    key{nullptr};
        TObject* obj{nullptr};
        for(const auto& it: particles){
                std::stringstream partName; 
                partName << "Setting gates for ";
                partName << "A = " << it.first; 
                partName << " Z = " << it.second; 
                TText* text= new TText(20, 70, partName.str().c_str());
                TCutG* cut{nullptr};
                while((key = dynamic_cast<TKey*>(contents()))) {
                        obj = tdir->Get(key->GetName());
                        if(obj->InheritsFrom("TH2D") ) {
                                std::string name(obj->GetName());
                                if (name.find(matrixPattern)!=0) continue;
                                std::string idxName = name.substr(matrixPattern.size());
                                auto idx1 = std::stoi(std::regex_replace(idxName.substr(0, idxName.find("_")), std::regex(R"([\D])"), ""));
                                auto idx2 = std::stoi(std::regex_replace(idxName.substr(idxName.find("_")+1), std::regex(R"([\D])"), ""));
                                std::cout << idx1 << " " << idx2 << std::endl;
                                TCanvas* cv = new TCanvas(name.c_str(),"",1500,1200);

                                auto h = dynamic_cast<TH2D*>(obj);
                                h->Draw("colz");
                                text->Draw();
                                cut = anaCV(cv, idx1, idx2, it, cut);
                                cv->Delete();
                                delete cv;
                        }
                }
        }
}