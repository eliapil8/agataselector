void FitEloss(){
    //std::ifstream file("../eloss.dat");
    std::ifstream file("good/eloss_user.dat");
    std::vector<double> Ec, Enm, IC_E_A, IC_Eres;
    std::vector<unsigned int> A, Z;

    std::vector<TGraph*> EnmvsICEA;
    TGraph *grFit = new TGraph();

    double ec, e1, e2, e3, e4, ic_e_a, ic_e_res;
    unsigned int a, z;
    std::string line;
    int i = 0;
    while(std::getline(file, line)){
        a=0; z=0; ec=0; e1=0; e2=0; e3=0; e4=0; ic_e_a=0; ic_e_res=0;
        if(A.size()%529==0&&A.size()!=0){
            EnmvsICEA.push_back(new TGraph(Enm.size(), &IC_E_A[0], &Enm[0]));
            A.clear();
            Z.clear();
            Ec.clear();
            Enm.clear();
            IC_E_A.clear();
            IC_Eres.clear();
            i++;
            continue;
        } 

        std::istringstream iss(line);
        iss >> a >> z >> ec >> e1 >> e2 >> e3 >> e4 >> ic_e_a >> ic_e_res;
        //if(!(iss >> a >> z >> ec >> e1 >> e2 >> e3 >> e4 >> ic_e_a >> ic_e_res)) break;
        A.push_back(a);
        Z.push_back(z);
        Ec.push_back(ec);
        Enm.push_back(e1+e2+e3+e4);
        IC_E_A.push_back(ic_e_a);
        IC_Eres.push_back(ic_e_res);
        line.clear();

        if(i>1){
            if(IC_E_A.back()==0 && Enm.back()==0) std::cout << A.size() << std::endl;
            grFit->SetPoint(i*529+A.size(), IC_E_A.back(), Enm.back());
        }

    }

    TCanvas *c1 = new TCanvas("c1", "c1", 1000,1000);
    EnmvsICEA[0]->SetTitle("E_{nm} vs IC_E_A; IC_E_A (MeV); E_{nm} (MeV)");
    EnmvsICEA[0]->SetMarkerStyle(20);
    EnmvsICEA[0]->SetMarkerColor(kRed);
    EnmvsICEA[1]->SetMarkerStyle(21);    
    EnmvsICEA[1]->SetMarkerColor(kBlue);
    EnmvsICEA[2]->SetMarkerStyle(22);
    EnmvsICEA[2]->SetMarkerColor(kGreen);
    EnmvsICEA[3]->SetMarkerStyle(23);
    EnmvsICEA[3]->SetMarkerColor(kOrange);
    EnmvsICEA[4]->SetMarkerStyle(24);
    EnmvsICEA[4]->SetMarkerColor(kBlack);

    EnmvsICEA[0]->Draw("AP");
    EnmvsICEA[1]->Draw("P");
    EnmvsICEA[2]->Draw("P");
    EnmvsICEA[3]->Draw("P");
    EnmvsICEA[4]->Draw("P");

    TLegend *leg = new TLegend(0.1,0.7,0.4,0.9);
    leg->AddEntry(EnmvsICEA[0], "#beta = 0.09", "p");
    leg->AddEntry(EnmvsICEA[1], "#beta = 0.10", "p");
    leg->AddEntry(EnmvsICEA[2], "#beta = 0.11", "p");
    leg->AddEntry(EnmvsICEA[3], "#beta = 0.12", "p");
    leg->AddEntry(EnmvsICEA[4], "#beta = 0.13", "p");
    leg->Draw();

    TCanvas *c2 = new TCanvas("c2", "c2", 1000,1000);
    grFit->SetTitle("E_{nm} vs IC_E_A; IC_E_A (MeV); E_{nm} (MeV)");
    grFit->SetMarkerStyle(36);
    grFit->SetMarkerColor(kViolet+3);
    grFit->Draw("AP");
    
    TF1 *fit = new TF1("fit", "pol2", 150, 500);
    grFit->Fit(fit, "R");
    fit->Draw("same");
    
}