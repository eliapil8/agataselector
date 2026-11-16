void FitElossUserAthick(){
    std::string filename = "eloss_user";
    std::pair<double,double> rangeAthickness = {225,265};
    
    for(double t = rangeAthickness.first; t<=rangeAthickness.second; t+=5){
        std::cout << "Here 1" << std::endl;

        std::ifstream file(Form("%s_Athick%3.0f.dat",filename.c_str(),t));
        
        std::vector<double> Ec, Enm, IC_E_A, IC_Eres;
        std::vector<unsigned int> A, Z;

        std::vector<TGraph*> EnmvsICEA_beta;
        std::unordered_map<int,TGraph*> EnmvsICEA_z;

        TGraph *grFit_b15 = new TGraph();
        TGraph *grFit_b14 = new TGraph();
        TGraph *grFit_b13 = new TGraph();
        TGraph *grFit_b12 = new TGraph();
        TGraph *grFit_b11 = new TGraph();
        TGraph *grFit_b10 = new TGraph();
        TGraph *grFit_b9 = new TGraph();
        TGraph *grFit_b8 = new TGraph();
        TGraph *grFit_b7 = new TGraph();
        TGraph *grFit_b6 = new TGraph();
        TGraph *grFit_b5 = new TGraph();

        double beta, ec, e1, e2, e3, e4, ic_e_a, ic_e_res;
        unsigned int a, z;
        std::string line;
        int i = 0;
        std::cout << "Here 2" << std::endl;
        
        //std::getline(file,line);

        while(std::getline(file, line)){
            a=0; z=0; beta=0; ec=0; e1=0; e2=0; e3=0; e4=0; ic_e_a=0; ic_e_res=0; 
            if(A.size()%529==0&&A.size()!=0){
                EnmvsICEA_beta.push_back(new TGraph(Enm.size(), &IC_E_A[0], &Enm[0]));
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
            iss >> a >> z >> beta >> ec >> e1 >> e2 >> e3 >> e4  >> ic_e_a >> ic_e_res;
            //if(!(iss >> a >> z >> ec >> e1 >> e2 >> e3 >> e4 >> ic_e_a >> ic_e_res)) break;
            if(ic_e_res == 0) {
                ic_e_a = 0;
                e1 = 0;
                e2 = 0;
                e3 = 0;
                e4 = 0;
            }

            A.push_back(a);
            Z.push_back(z);
            Ec.push_back(ec);
            Enm.push_back(e1+e2+e3+e4);
            IC_E_A.push_back(ic_e_a);
            IC_Eres.push_back(ic_e_res);
            line.clear();


            if(i==0){
                if(IC_E_A.back()==0 && Enm.back()==0) std::cout << A.size() << std::endl;
                grFit_b5->SetPoint(grFit_b5->GetN(), IC_E_A.back(), Enm.back());
            }
            if(i==1){
                if(IC_E_A.back()==0 && Enm.back()==0) std::cout << A.size() << std::endl;
                grFit_b6->SetPoint(grFit_b6->GetN(), IC_E_A.back(), Enm.back());
            }
            if(i==2){
                if(IC_E_A.back()==0 && Enm.back()==0) std::cout << A.size() << std::endl;
                grFit_b7->SetPoint(grFit_b7->GetN(), IC_E_A.back(), Enm.back());
            }
            if(i==3){
                if(IC_E_A.back()==0 && Enm.back()==0) std::cout << A.size() << std::endl;
                grFit_b8->SetPoint(grFit_b8->GetN(), IC_E_A.back(), Enm.back());
            }
            if(i==4){
                if(IC_E_A.back()==0 && Enm.back()==0) std::cout << A.size() << std::endl;
                grFit_b9->SetPoint(grFit_b9->GetN(), IC_E_A.back(), Enm.back());
            }
            if(i==5){
                if(IC_E_A.back()==0 && Enm.back()==0) std::cout << A.size() << std::endl;
                grFit_b10->SetPoint(grFit_b10->GetN(), IC_E_A.back(), Enm.back());
            }
            if(i==6){
                if(IC_E_A.back()==0 && Enm.back()==0) std::cout << A.size() << std::endl;
                grFit_b11->SetPoint(grFit_b11->GetN(), IC_E_A.back(), Enm.back());
            }
            if(i==7){
                if(IC_E_A.back()==0 && Enm.back()==0) std::cout << A.size() << std::endl;
                grFit_b12->SetPoint(grFit_b12->GetN(), IC_E_A.back(), Enm.back());
            }
            if(i==8){
                if(IC_E_A.back()==0 && Enm.back()==0) std::cout << A.size() << std::endl;
                grFit_b13->SetPoint(grFit_b13->GetN(), IC_E_A.back(), Enm.back());
            }
            if(i==9){
                if(IC_E_A.back()==0 && Enm.back()==0) std::cout << A.size() << std::endl;
                grFit_b14->SetPoint(grFit_b14->GetN(), IC_E_A.back(), Enm.back());
            }
            if(i==10){
                if(IC_E_A.back()==0 && Enm.back()==0) std::cout << A.size() << std::endl;
                grFit_b15->SetPoint(grFit_b15->GetN(), IC_E_A.back(), Enm.back());
            }
        }

        file.clear(); // Clear any error flags
        file.seekg(0, std::ios::beg); // Move the file pointer back to the beginning
        i=0;
        while(std::getline(file, line)){
            a=0; z=0; ec=0; e1=0; e2=0; e3=0; e4=0; ic_e_a=0; ic_e_res=0;

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

            if(IC_E_A.back()==0 && Enm.back()==0) std::cout << A.size() << std::endl;
            if(EnmvsICEA_z.find(z)==EnmvsICEA_z.end()) EnmvsICEA_z.emplace(z,new TGraph());
            //std::cout << z << " npoints " << EnmvsICEA_z[z]->GetN() << std::endl;
            EnmvsICEA_z[z]->SetPoint(EnmvsICEA_z[z]->GetN(),IC_E_A.back(), Enm.back());
        }


        //TCanvas *c1 = new TCanvas(Form("c1_%3.0f",t), Form("c1_%3.0f",t), 1000,1000);
        EnmvsICEA_beta[0]->SetTitle("E_{nm} vs IC_E_A; IC_E_A (MeV); E_{nm} (MeV)");
        EnmvsICEA_beta[0]->SetMarkerStyle(20);
        EnmvsICEA_beta[0]->SetMarkerColor(kRed);
        EnmvsICEA_beta[1]->SetMarkerStyle(21);    
        EnmvsICEA_beta[1]->SetMarkerColor(kBlue);
        EnmvsICEA_beta[2]->SetMarkerStyle(22);
        EnmvsICEA_beta[2]->SetMarkerColor(kGreen);
        EnmvsICEA_beta[3]->SetMarkerStyle(23);
        EnmvsICEA_beta[3]->SetMarkerColor(kOrange);
        EnmvsICEA_beta[4]->SetMarkerStyle(24);
        EnmvsICEA_beta[4]->SetMarkerColor(kBlack);
        EnmvsICEA_beta[5]->SetMarkerStyle(20);
        EnmvsICEA_beta[5]->SetMarkerColor(kRed+3);
        EnmvsICEA_beta[6]->SetMarkerStyle(21);    
        EnmvsICEA_beta[6]->SetMarkerColor(kBlue+3);
        EnmvsICEA_beta[7]->SetMarkerStyle(22);
        EnmvsICEA_beta[7]->SetMarkerColor(kGreen+3);
        EnmvsICEA_beta[8]->SetMarkerStyle(23);
        EnmvsICEA_beta[8]->SetMarkerColor(kOrange+3);
        EnmvsICEA_beta[9]->SetMarkerStyle(24);
        EnmvsICEA_beta[9]->SetMarkerColor(kBlack+3);
        EnmvsICEA_beta[10]->SetMarkerStyle(20);
        EnmvsICEA_beta[10]->SetMarkerColor(kRed-2);

        TGraph * axisframe = new TGraph();
        axisframe->SetPoint(axisframe->GetN(),50,10);
        axisframe->SetPoint(axisframe->GetN(),550,60);
        axisframe->SetTitle("E_{nm} vs IC_E_A;E_A (MeV); E_{nm} (MeV)");
        EnmvsICEA_beta[0]->GetXaxis()->SetRangeUser(51,550);
        EnmvsICEA_beta[0]->GetYaxis()->SetRangeUser(10,60);

        axisframe->SetMarkerColor(kWhite);
        axisframe->Draw("AP");
        EnmvsICEA_beta[0]->Draw("Psame");
        EnmvsICEA_beta[1]->Draw("Psame");
        EnmvsICEA_beta[2]->Draw("Psame");
        EnmvsICEA_beta[3]->Draw("Psame");
        EnmvsICEA_beta[4]->Draw("Psame");
        EnmvsICEA_beta[5]->Draw("Psame");
        EnmvsICEA_beta[6]->Draw("Psame");
        EnmvsICEA_beta[7]->Draw("Psame");
        EnmvsICEA_beta[8]->Draw("Psame");
        EnmvsICEA_beta[9]->Draw("Psame");
        EnmvsICEA_beta[10]->Draw("Psame");


        TLegend *leg = new TLegend(0.6,0.1,0.9,0.4);
        leg->AddEntry(EnmvsICEA_beta[0], "#beta = 0.05", "p");
        leg->AddEntry(EnmvsICEA_beta[1], "#beta = 0.06", "p");
        leg->AddEntry(EnmvsICEA_beta[2], "#beta = 0.07", "p");
        leg->AddEntry(EnmvsICEA_beta[3], "#beta = 0.08", "p");
        leg->AddEntry(EnmvsICEA_beta[4], "#beta = 0.09", "p");
        leg->AddEntry(EnmvsICEA_beta[5], "#beta = 0.1", "p");
        leg->AddEntry(EnmvsICEA_beta[6], "#beta = 0.11", "p");
        leg->AddEntry(EnmvsICEA_beta[7], "#beta = 0.12", "p");
        leg->AddEntry(EnmvsICEA_beta[8], "#beta = 0.13", "p");
        leg->AddEntry(EnmvsICEA_beta[9], "#beta = 0.14", "p");
        leg->AddEntry(EnmvsICEA_beta[10], "#beta = 0.15", "p");
        leg->Draw();

        TCanvas *c2 = new TCanvas(Form("c2_%3.0f",t),Form("c2_%3.0f",t), 1000,1000);
        grFit_b5->GetXaxis()->SetRangeUser(50,550);
        grFit_b6->GetXaxis()->SetRangeUser(50,550);
        grFit_b7->GetXaxis()->SetRangeUser(50,550);
        grFit_b8->GetXaxis()->SetRangeUser(50,550);
        grFit_b9->GetXaxis()->SetRangeUser(50,550);
        grFit_b10->GetXaxis()->SetRangeUser(50,550);
        grFit_b11->GetXaxis()->SetRangeUser(50,550);
        grFit_b12->GetXaxis()->SetRangeUser(50,550);
        grFit_b13->GetXaxis()->SetRangeUser(50,550);
        grFit_b14->GetXaxis()->SetRangeUser(50,550);
        grFit_b15->GetXaxis()->SetRangeUser(50,550);
        grFit_b5->GetYaxis()->SetRangeUser(10,60);
        grFit_b6->GetYaxis()->SetRangeUser(10,60);
        grFit_b7->GetYaxis()->SetRangeUser(10,60);
        grFit_b8->GetYaxis()->SetRangeUser(10,60);
        grFit_b9->GetYaxis()->SetRangeUser(10,60);
        grFit_b10->GetYaxis()->SetRangeUser(10,60);
        grFit_b11->GetYaxis()->SetRangeUser(10,60);
        grFit_b12->GetYaxis()->SetRangeUser(10,60);
        grFit_b13->GetYaxis()->SetRangeUser(10,60);
        grFit_b14->GetYaxis()->SetRangeUser(10,60);
        grFit_b15->GetYaxis()->SetRangeUser(10,60);
        grFit_b5->SetTitle("E_{nm} vs IC_E_A;E_A (MeV); E_{nm} (MeV)");
        grFit_b6->SetTitle("E_{nm} vs IC_E_A;E_A (MeV); E_{nm} (MeV)");
        grFit_b7->SetTitle("E_{nm} vs IC_E_A;E_A (MeV); E_{nm} (MeV)");
        grFit_b8->SetTitle("E_{nm} vs IC_E_A;E_A (MeV); E_{nm} (MeV)");
        grFit_b9->SetTitle("E_{nm} vs IC_E_A;E_A (MeV); E_{nm} (MeV)");
        grFit_b10->SetTitle("E_{nm} vs IC_E_A;E_A (MeV); E_{nm} (MeV)");
        grFit_b11->SetTitle("E_{nm} vs IC_E_A;E_A (MeV); E_{nm} (MeV)");
        grFit_b12->SetTitle("E_{nm} vs IC_E_A;E_A (MeV); E_{nm} (MeV)");
        grFit_b13->SetTitle("E_{nm} vs IC_E_A;E_A (MeV); E_{nm} (MeV)");
        grFit_b14->SetTitle("E_{nm} vs IC_E_A;E_A (MeV); E_{nm} (MeV)");
        grFit_b15->SetTitle("E_{nm} vs IC_E_A;E_A (MeV); E_{nm} (MeV)");
        grFit_b5->SetMarkerStyle(36);
        grFit_b6->SetMarkerStyle(35);
        grFit_b7->SetMarkerStyle(34);
        grFit_b8->SetMarkerStyle(33);
        grFit_b9->SetMarkerStyle(32);
        grFit_b10->SetMarkerStyle(31);
        grFit_b11->SetMarkerStyle(30);
        grFit_b12->SetMarkerStyle(29);
        grFit_b13->SetMarkerStyle(28);
        grFit_b14->SetMarkerStyle(27);
        grFit_b15->SetMarkerStyle(26);
        grFit_b5->SetMarkerSize(1);
        grFit_b6->SetMarkerSize(1);
        grFit_b7->SetMarkerSize(1);
        grFit_b8->SetMarkerSize(1);
        grFit_b9->SetMarkerSize(1);
        grFit_b10->SetMarkerSize(1);
        grFit_b11->SetMarkerSize(1);
        grFit_b12->SetMarkerSize(1);
        grFit_b13->SetMarkerSize(1);
        grFit_b14->SetMarkerSize(1);
        grFit_b15->SetMarkerSize(1);
        grFit_b5->SetMarkerColor(kRed+2);
        grFit_b6->SetMarkerColor(kBlue+2);
        grFit_b7->SetMarkerColor(kGreen+2);
        grFit_b8->SetMarkerColor(kMagenta+2);
        grFit_b9->SetMarkerColor(kOrange+2);
        grFit_b10->SetMarkerColor(kGray);
        grFit_b11->SetMarkerColor(kCyan+2);
        grFit_b12->SetMarkerColor(kRed-2);
        grFit_b13->SetMarkerColor(kBlue-2);
        grFit_b14->SetMarkerColor(kGreen-2);
        grFit_b15->SetMarkerColor(kMagenta-2);
        axisframe->Draw("AP");
        grFit_b5->Draw("P");
        grFit_b6->Draw("P");
        grFit_b7->Draw("P");
        grFit_b8->Draw("P");
        grFit_b9->Draw("P");
        grFit_b10->Draw("P");
        grFit_b11->Draw("P");
        grFit_b12->Draw("P");
        grFit_b13->Draw("P");
        grFit_b14->Draw("P");
        grFit_b15->Draw("P");

        TLegend * legFit = new TLegend(0.6,0.1,0.9,0.4);
        TF1 *fit_b5 = new TF1("fit_b5", "pol1", 50, 590);
        TF1 *fit_b6 = new TF1("fit_b6", "pol1", 50, 590);
        TF1 *fit_b7 = new TF1("fit_b7", "pol1", 50, 590);
        TF1 *fit_b8 = new TF1("fit_b8", "pol1", 50, 590);
        TF1 *fit_b9 = new TF1("fit_b9", "pol1", 50, 590);
        TF1 *fit_b10 = new TF1("fit_b10", "pol1", 50, 590);
        TF1 *fit_b11 = new TF1("fit_b11", "pol1", 50, 590);
        TF1 *fit_b12 = new TF1("fit_b12", "pol1", 50, 590);
        TF1 *fit_b13 = new TF1("fit_b13", "pol1", 50, 590);
        TF1 *fit_b14 = new TF1("fit_b14", "pol1", 50, 590);
        TF1 *fit_b15 = new TF1("fit_b15", "pol1", 50, 590);
        grFit_b5->Fit(fit_b5, "R");
        grFit_b6->Fit(fit_b6, "R");
        grFit_b7->Fit(fit_b7, "R");
        grFit_b8->Fit(fit_b8, "R");
        grFit_b9->Fit(fit_b9, "R");
        grFit_b10->Fit(fit_b10, "R");
        grFit_b11->Fit(fit_b11, "R");
        grFit_b12->Fit(fit_b12, "R");
        grFit_b13->Fit(fit_b13, "R");
        grFit_b14->Fit(fit_b14, "R");
        grFit_b15->Fit(fit_b15, "R");
        fit_b5->SetLineColor(kRed);
        fit_b6->SetLineColor(kBlue);
        fit_b7->SetLineColor(kGreen);
        fit_b8->SetLineColor(kMagenta);
        fit_b9->SetLineColor(kOrange);
        fit_b10->SetLineColor(kBlack);
        fit_b11->SetLineColor(kCyan);
        fit_b12->SetLineColor(kRed-2);
        fit_b13->SetLineColor(kBlue-2);
        fit_b14->SetLineColor(kGreen-2);
        fit_b15->SetLineColor(kMagenta-2);
        fit_b5->Draw("same");
        fit_b6->Draw("same");
        fit_b7->Draw("same");
        fit_b8->Draw("same");
        fit_b9->Draw("same");
        fit_b10->Draw("same");
        fit_b11->Draw("same");
        fit_b12->Draw("same");
        fit_b13->Draw("same");
        fit_b14->Draw("same");
        fit_b15->Draw("same");
        legFit->AddEntry(fit_b5, "#beta = 0.05", "l");
        legFit->AddEntry(fit_b6, "#beta = 0.06", "l");
        legFit->AddEntry(fit_b7, "#beta = 0.07", "l");
        legFit->AddEntry(fit_b8, "#beta = 0.08", "l");
        legFit->AddEntry(fit_b9, "#beta = 0.09", "l");
        legFit->AddEntry(fit_b10, "#beta = 0.10", "l");
        legFit->AddEntry(fit_b11, "#beta = 0.11", "l");
        legFit->AddEntry(fit_b12, "#beta = 0.12", "l");
        legFit->AddEntry(fit_b13, "#beta = 0.13", "l");
        legFit->AddEntry(fit_b14, "#beta = 0.14", "l");
        legFit->AddEntry(fit_b15, "#beta = 0.15", "l");
        legFit->Draw("same");

        std::cout << "Beta = 0.05; E_nm = "<<fit_b5->GetParameter(0)<<" + "<<fit_b5->GetParameter(1)<<"*IC_E_A"<< std::endl;
        std::cout << "Beta = 0.06; E_nm = "<<fit_b6->GetParameter(0)<<" + "<<fit_b6->GetParameter(1)<<"*IC_E_A"<< std::endl;
        std::cout << "Beta = 0.07; E_nm = "<<fit_b7->GetParameter(0)<<" + "<<fit_b7->GetParameter(1)<<"*IC_E_A"<< std::endl;
        std::cout << "Beta = 0.09; E_nm = "<<fit_b8->GetParameter(0)<<" + "<<fit_b8->GetParameter(1)<<"*IC_E_A"<< std::endl;
        std::cout << "Beta = 0.09; E_nm = "<<fit_b9->GetParameter(0)<<" + "<<fit_b9->GetParameter(1)<<"*IC_E_A"<< std::endl;
        std::cout << "Beta = 0.10; E_nm = "<<fit_b10->GetParameter(0)<<" + "<<fit_b10->GetParameter(1)<<"*IC_E_A"<< std::endl;
        std::cout << "Beta = 0.11; E_nm = "<<fit_b11->GetParameter(0)<<" + "<<fit_b11->GetParameter(1)<<"*IC_E_A"<< std::endl;
        std::cout << "Beta = 0.12; E_nm = "<<fit_b12->GetParameter(0)<<" + "<<fit_b12->GetParameter(1)<<"*IC_E_A"<< std::endl;
        std::cout << "Beta = 0.13; E_nm = "<<fit_b13->GetParameter(0)<<" + "<<fit_b13->GetParameter(1)<<"*IC_E_A"<< std::endl;
        std::cout << "Beta = 0.14; E_nm = "<<fit_b14->GetParameter(0)<<" + "<<fit_b14->GetParameter(1)<<"*IC_E_A"<< std::endl;
        std::cout << "Beta = 0.15; E_nm = "<<fit_b15->GetParameter(0)<<" + "<<fit_b15->GetParameter(1)<<"*IC_E_A"<< std::endl;

        //TCanvas *c3 = new TCanvas("c3", "c3", 1000,1000);
        //std::cout << EnmvsICEA_z.size() << std::endl;
//
        //std::vector<Color_t> colors = {kRed, kBlue, kGreen, kOrange, kBlack, kRed+3, kBlue+3, kGreen+3, kOrange+3, kBlack+3, kRed-2, kBlue-2, kGreen-2, kOrange-2};
//
        //TLegend* leg2 = new TLegend(0.1,0.7,0.4,0.9);
//
        //int j = 0;
        //for(auto it = EnmvsICEA_z.begin(); it!=EnmvsICEA_z.end(); it++){
        //    it->second->SetTitle("E_{nm} vs IC_E_A; IC_E_A (MeV); E_{nm} (MeV)");
        //    it->second->SetMarkerStyle(20);
        //    //it->second->SetMarkerColor(kRed);
        //    it->second->SetMarkerColor(colors[it->first%14]);
        //    it->second->SetMarkerStyle(20);
        //    it->second->GetXaxis()->SetRangeUser(50,550);
        //    it->second->GetYaxis()->SetRangeUser(10,60);
        //    //std::cout << it->first << " " << it->second->GetN() << std::endl;
        //    if(it!=EnmvsICEA_z.begin()) it->second->Draw("Psame");
        //    else it->second->Draw("AP");
        //    leg2->AddEntry(it->second, Form("Z = %d", it->first), "p");
        //    std::cout << it->first << " " << it->second->GetN() << std::endl;
        //    ++j;
        //    if(j > 5) break;
        //}
        //leg2->Draw();

        TCanvas *c4 = new TCanvas(Form("c4_%3.0f",t),Form("c4_%3.0f",t), 1000,1000);

        std::vector<double> betas;
        std::vector<double> slope;
        std::vector<double> beta_err;
        std::vector<double> slope_err;

        for(int l = 0; l < 11; ++l){
            
            TF1 * fitFunction;
            switch (l)
            {
            case 0:
                fitFunction = fit_b5; break;
            case 1:
                fitFunction = fit_b6; break;
            case 2:
                fitFunction = fit_b7; break;
            case 3:
                fitFunction = fit_b8; break;
            case 4:
                fitFunction = fit_b9; break;    
            case 5:
                fitFunction = fit_b10; break;
            case 6:
                fitFunction = fit_b11; break;
            case 7:
                fitFunction = fit_b12; break;
            case 8:
                fitFunction = fit_b13; break;
            case 9:
                fitFunction = fit_b14; break;
            case 10:   
                fitFunction = fit_b15; break;    
            default:
                break;
            }
            if(fitFunction->GetParameter(1)<0.001) continue;
            betas.push_back(0.05+l*0.01);
            beta_err.push_back(0);
            slope.push_back(fitFunction->GetParameter(1));
            slope_err.push_back(fitFunction->GetParError(1));
        }

        //TGraphErrors *grSlope = new TGraphErrors(betas.size(), &betas[0], &slope[0], &beta_err[0], &slope_err[0]);
        TGraph *grSlope = new TGraph(betas.size(), &betas[0], &slope[0]);
        grSlope->SetTitle("Slope vs #beta; #beta; Slope");
        grSlope->SetMarkerStyle(20);
        grSlope->SetMarkerColor(kRed);
        grSlope->Draw("AP");


        grSlope->SetName(Form("icEnmBeta%3.0f__0__0",t));
        grSlope->SaveAs(Form("icEnmBeta%3.0f__0__0.root",t));

        //TF1 * fitSlope = new TF1("fitSlope", "[0]+[1]*TMath::Exp((x-[2])/[3])", 0.065, 0.16);
        //fitSlope->SetParameter(0, 0.12);
        //fitSlope->SetParameter(1, 0.1);
        //fitSlope->SetParameter(2, -0.1);
        //fitSlope->SetParameter(3, -0.01);
        TF1 * fitSlope = new TF1("fitSlope", "pol2", 0.065, 0.16);
        grSlope->Fit(fitSlope, "R");
        fitSlope->SetLineColor(kBlue);
        fitSlope->Draw("same");





        
    }

   
    
}