void FitElossAthickNew(std::string folder = ""){
    std::string filename = "eloss_user";
    std::pair<double,double> rangeAthickness = {225,295};
    
    for(double t = rangeAthickness.first; t<=rangeAthickness.second; t+=5){
        std::cout << "Here 1" << std::endl;

        std::ifstream file(Form("%s/%s_Athick%3.0f.dat",folder.c_str(),filename.c_str(),t));
        
        std::vector<double> Ec, Enm, IC_E_A, IC_Eres;
        std::vector<unsigned int> A, Z;

        std::vector<TGraph*> EnmvsICEA_beta;
        std::unordered_map<int,std::vector<TGraph*>> EnmvsICEA_z;

        std::vector<TGraph*> grFit_betas;
        std::vector<TF1*> fFit_betas;


        double beta, ec, e1, e2, e3, e4, ic_e_a, ic_e_res;
        unsigned int a, z;
        std::string line;
        int i = 0;
        std::cout << "Here 2" << std::endl;

        std::vector<Color_t> colors = {kRed, kBlue, kGreen, kMagenta, kOrange, kBlack, kCyan, kRed-2, kBlue-2, kGreen-2, kMagenta-2};
        

        double firstBeta = -1;
        double lastBeta = -1;

        while(std::getline(file, line)){
            
            a=0; z=0; beta=0; ec=0; e1=0; e2=0; e3=0; e4=0; ic_e_a=0; ic_e_res=0; 
            if(A.size()%529==0&&A.size()!=0){
                grFit_betas.push_back(new TGraph(Enm.size(), &IC_E_A[0], &Enm[0]));
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
            //if(ic_e_res == 0) {
            //    ic_e_a = 0;
            //    e1 = 0;
            //    e2 = 0;
            //    e3 = 0;
            //    e4 = 0;
            //}
            if(firstBeta == -1 && i == 0) firstBeta = beta;
            if(beta != 0)lastBeta = beta;

            A.push_back(a);
            Z.push_back(z);
            Ec.push_back(ec);
            Enm.push_back(e1+e2+e3+e4);
            IC_E_A.push_back(ic_e_a);
            IC_Eres.push_back(ic_e_res);
            line.clear();

            if(EnmvsICEA_z.find(z) == EnmvsICEA_z.end()){
                EnmvsICEA_z[z] = std::vector<TGraph*>();
            }
            if(EnmvsICEA_z[z].size()<i+1) EnmvsICEA_z[z].push_back(new TGraph());

            EnmvsICEA_z[z][i]->SetPoint(EnmvsICEA_z[z][i]->GetN(), ic_e_a, e1+e2+e3+e4);

        }



        TGraph * axisframe = new TGraph();
        axisframe->SetPoint(axisframe->GetN(),50,10);
        axisframe->SetPoint(axisframe->GetN(),550,100);
        axisframe->SetTitle("E_{nm} vs IC_E_A;E_A (MeV); E_{nm} (MeV)");
        axisframe->SetMarkerColor(kWhite);
        axisframe->Draw("AP");

        TLegend *leg = new TLegend(0.6,0.1,0.9,0.4);


        double betaIncrement = (lastBeta - firstBeta)/grFit_betas.size();

        std::cout << "BETA START: "<<firstBeta<<"; BETA END: "<<lastBeta<<"; BETA INCREMENT: "<<betaIncrement<<std::endl;

        for(int i = 0; i < grFit_betas.size(); ++i){
            grFit_betas[i]->SetTitle("E_{nm} vs IC_E_A; IC_E_A (MeV); E_{nm} (MeV)");
            grFit_betas[i]->SetMarkerStyle(20+i);
            grFit_betas[i]->SetMarkerColor(colors[i%colors.size()]);
            grFit_betas[i]->GetXaxis()->SetRangeUser(50,550);
            grFit_betas[i]->GetYaxis()->SetRangeUser(10,100);
            grFit_betas[i]->Draw("Psame");
            leg->AddEntry(grFit_betas[i], Form("#beta = %3.2f", firstBeta+i*betaIncrement), "p");
            fFit_betas.push_back(new TF1(Form("fit_b%3.0f",firstBeta+i*betaIncrement), "pol1", 50, 590));
            //fFit_betas.back()->FixParameter(0,0);

            int validPoints = 0;
            for(int j = 0; j < grFit_betas[i]->GetN(); ++j){
                double x, y;
                grFit_betas[i]->GetPoint(j, x, y);
                if(y>1&&x>1){
                    validPoints++;
                }
            }
            if(validPoints>100) grFit_betas[i]->Fit(fFit_betas[i], "R");
            else fFit_betas[i]->SetParameters(0,0);
            fFit_betas[i]->SetLineColor(colors[i%colors.size()]);
            fFit_betas[i]->Draw("same");
            std::cout << "Beta = "<<firstBeta+i*betaIncrement<<"; E_nm = "<<fFit_betas[i]->GetParameter(0)<<" + "<<fFit_betas[i]->GetParameter(1)<<"*IC_E_A"<< std::endl;
        }
        
        leg->Draw();
    

        TCanvas *c4 = new TCanvas(Form("c4_%3.0f",t),Form("c4_%3.0f",t), 1000,1000);

        std::vector<double> betas;
        std::vector<double> slope;
        std::vector<double> intercept;
        std::vector<double> beta_err;
        std::vector<double> slope_err;
        std::vector<double> intercept_err;

        for(int l = 0; l < grFit_betas.size(); ++l){
            
            TF1 * fitFunction = fFit_betas[l];
            
            if(fitFunction->GetParameter(1)<0.001) continue;
            betas.push_back(firstBeta+l*betaIncrement);
            beta_err.push_back(0);
            intercept.push_back(fitFunction->GetParameter(0));
            intercept_err.push_back(fitFunction->GetParError(0));
            slope.push_back(fitFunction->GetParameter(1));
            slope_err.push_back(fitFunction->GetParError(1));
        }

        TGraphErrors *grSlope = new TGraphErrors(betas.size(), &betas[0], &slope[0], &beta_err[0], &slope_err[0]);
        grSlope->SetTitle("Slope vs #beta; #beta; Slope");
        grSlope->SetMarkerStyle(20);
        grSlope->SetMarkerColor(kRed);
        //grSlope->Draw("AP");

        TGraph * grToSave = new TGraph(betas.size(), &betas[0], &slope[0]);

        grToSave->SetName(Form("icEnmBeta%3.0f__0__0",t));
        grToSave->SaveAs(Form("%s/icEnmBeta%3.0f__0__0.root",folder.c_str(),t));

        //TF1 * fitSlope = new TF1("fitSlope", "[0]+[1]*TMath::Exp((x-[2])/[3])", 0.065, 0.16);
        //fitSlope->SetParameter(0, 0.12);
        //fitSlope->SetParameter(1, 0.1);
        //fitSlope->SetParameter(2, -0.1);
        //fitSlope->SetParameter(3, -0.01);
        TF1 * fitSlope = new TF1("fitSlope", "pol2", 0.065, 0.16);
        grSlope->Fit(fitSlope, "R");
        fitSlope->SetLineColor(kBlue);
        //fitSlope->Draw("same");

        TCanvas *c5 = new TCanvas(Form("c5_%3.0f",t),Form("c5_%3.0f",t), 1000,1000);
        TGraphErrors *grIntercept = new TGraphErrors(betas.size(), &betas[0], &intercept[0], &beta_err[0], &intercept_err[0]);
        grIntercept->SetTitle("Intercept vs #beta; #beta; Intercept");
        grIntercept->SetMarkerStyle(20);
        grIntercept->SetMarkerColor(kRed);
        //grIntercept->Draw("AP");

        TGraph * grToSave2 = new TGraph(betas.size(), &betas[0], &intercept[0]);

        grToSave2->SetName(Form("icEnmBeta%3.0f__1__0",t));
        grToSave2->SaveAs(Form("%s/icEnmBeta%3.0f__1__0.root",folder.c_str(),t));
        
        
        
        TCanvas * cz = new TCanvas("cz","cz", 1000,1000);
    
        TGraph* axisframe_beta = new TGraph();
        axisframe_beta->SetPoint(axisframe_beta->GetN(),firstBeta,0);
        axisframe_beta->SetPoint(axisframe_beta->GetN(),lastBeta,100);
        axisframe_beta->SetMarkerSize(0.00001);
        //axisframe_beta->Draw("AP");


        for(auto it: EnmvsICEA_z){


            //axisframe_beta->Draw("AP");

            int z = it.first;


            TCanvas * cz = new TCanvas(Form("cz_%3.0f_%d",t,z),Form("cz_%3.0f_%d",t,z), 1000,1000);
            cz->Divide(7,7);

            int j = 0;
        
            TGraph * z_vs_beta = new TGraph();
            for(int k = 0; k < it.second.size(); ++k){
                double x = firstBeta + k*betaIncrement;
                double y = it.second[k]->GetMean(2);
                z_vs_beta->SetPoint(z_vs_beta->GetN(), x, y);

                j++;
                cz->cd(j);
                axisframe->Draw("AP");
                it.second[k]->SetTitle(Form("E_{nm} vs IC_E_A for Z = %d; IC_E_A (MeV); E_{nm} (MeV)", z));
                it.second[k]->SetMarkerStyle(20);
                it.second[k]->SetMarkerSize(0.3);
                it.second[k]->SetMarkerColor(colors[z%colors.size()]);
                it.second[k]->Draw("Psame");
            }
            
            z_vs_beta->SetName(Form("icenmzbeta__%3.0f_%d__0",t,z));
            z_vs_beta->SetTitle(Form("Z = %d; Beta; E_{nm} (MeV)", z));
            z_vs_beta->SetMarkerStyle(20);
            z_vs_beta->SetMarkerColor(colors[z%colors.size()]);
            //z_vs_beta->Draw("PLsame");
            
            TFile * f = new TFile(Form("%s/EnmvsBetaZ/icenmzbeta__%3.0f_%d__0.root",folder.c_str(),t,z), "RECREATE");
            z_vs_beta->Write();
            f->Close();


            TGraph * z_graph = it.second.back();
            z_graph->SetTitle(Form("E_{nm} vs IC_E_A for Z = %d; IC_E_A (MeV); E_{nm} (MeV)", z));
            z_graph->SetMarkerStyle(20);
            z_graph->SetMarkerColor(colors[z%colors.size()]);
            //z_graph->Draw("Psame");
            delete cz;
        }


    }

   
    

    
}