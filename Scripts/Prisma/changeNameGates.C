void changeNameGates(){
    std::pair<int,int> range_z{25,56};
    std::pair<int,int> range_q{25,43};
    for(int i = range_z.first; i <= range_z.second; i++){
        for(int j = range_q.first; j <= range_q.second; j   ++){
            std::cout << "z " << i << " q " << j << std::endl;
            std::string oldnameq = "icerbeta__0_" + std::to_string(i)  + "__" + std::to_string(j);
            std::string oldnamefq = "icerbeta__0_" + std::to_string(i)  + "__" + std::to_string(j)+ ".root";
            TFile * file_q = new TFile(oldnamefq.c_str(), "read");
            if(file_q->IsZombie()) continue;
            TGraph * cut_q = (TGraph*)file_q->Get(oldnameq.c_str());
            if(i == 0){
                for(int k = 0; k < cut_q->GetN(); ++k){
                    double x,y;
                    cut_q->GetPoint(k,x,y);
                    //if(x < 0) continue;
                    cut_q->SetPoint(k,1000*x,y);
                }
            }
            std::string newnameq = "icerbeta__0_" + std::to_string(i)  + "__" + std::to_string(j);
            std::string newnamefq = "icerbeta__0_" + std::to_string(i)  + "__" + std::to_string(j)+ ".root";
            cut_q->SetName(newnameq.c_str());
            file_q->Close();
            TFile * new_file_q = new TFile(newnamefq.c_str(), "recreate");
            cut_q->Write();
            new_file_q->Close();
        }
    }
}



void convertBanToTCutG(std::string banFileName, std::string rootFilePattern, bool multiBan=false){


    std::ifstream banFile(banFileName);
    if (!banFile.is_open()){
        std::cout << "Error opening file" << std::endl;
        return;
    }

    if(!multiBan){
        std::string line;
        std::vector<double> x_v,y_v;
        double x,y;
        while (std::getline(banFile, line)){
            std::istringstream iss(line);
            iss >> x >> y;
            std::cout << x << " " << y << std::endl;
            x_v.push_back(x);
            y_v.push_back(y);
            line.clear();
            iss.clear();
        }
        TFile * file = new TFile((rootFilePattern+".root").c_str(), "recreate");
        TCutG * cut = new TCutG(rootFilePattern.c_str(), x_v.size(), &x_v[0], &y_v[0]);
        file->cd();
        cut->Write();
        file->Close();
    }
    else{
        int id = -1;
        std::string line;
        std::vector<double> x_v,y_v;
        double x,y;
        std::string temp; 
        while (std::getline(banFile, line)){
            std::istringstream iss(line);
            if(line.find("ADC") != std::string::npos){
                if(id>=0){
                    TFile * file = new TFile((rootFilePattern+"__"+std::to_string(id)+"__1.root").c_str(), "recreate");
                    TCutG * cut = new TCutG((rootFilePattern+"__"+std::to_string(id)+"__1").c_str(), x_v.size(), &x_v[0], &y_v[0]);
                    file->cd();
                    cut->Write();
                    file->Close();
                    x_v.clear();
                    y_v.clear();
                }
                iss >> temp >> id;
                continue;
            }
            iss >> x >> y;
            std::cout << x << " " << y << std::endl;
            x_v.push_back(x);
            y_v.push_back(y);
            line.clear();
            iss.clear();
        }
        if(id==9){
            TFile * file = new TFile((rootFilePattern+"__"+std::to_string(id)+"__1.root").c_str(), "recreate");
            TCutG * cut = new TCutG((rootFilePattern+"__"+std::to_string(id)+"__1").c_str(), x_v.size(), &x_v[0], &y_v[0]);
            file->cd();
            cut->Write();
            file->Close();
            x_v.clear();
            y_v.clear();
        }
    }
    
}

void convertPolyToTGraph(std::string polyFileName, std::string rootFilePattern, bool debug = false){
    std::ifstream polyFile(polyFileName);
    if (!polyFile.is_open()){
        std::cout << "Error opening file" << std::endl;
        return;
    }

    std::map<std::pair<int,int>,TF1*> polynomials;
    std::string line;
    std::vector<double> coeff;
    int z, q, order, temp;
    double c;
    std::vector<double> x_v,y_v;

    while (std::getline(polyFile, line)){
        if(line[0] == '#') continue;
        if(line.empty()) continue;
        std::istringstream iss(line);
        iss >> temp >> z  >> order;
        std::cout << z << " " << order << " ";
        for(int i = 0; i < order; i++){
            iss >> c;
            coeff.push_back(c);
            std::cout << c << " ";
        }
        TF1 * poly = new TF1((rootFilePattern+"__"+std::to_string(z)+"__"+std::to_string(q)).c_str(), Form("pol%d",order-1), -100,100);
        for(int i = 0; i < order; i++){
            if(i == 0) poly->SetParameter(i, 300);
            else poly->SetParameter(i, coeff[i]);
        }
        std::cout << std::endl;
        polynomials[std::make_pair(z,q)] = poly;
        TCanvas * c = new TCanvas();
        poly->Draw();
        if(debug) c->WaitPrimitive();

        double x_i = -50;
        double x_f =  50;
        int n_points = 50;
        for(int i = 0; i < n_points+1; i++){
            double x_c = x_i+i*(x_f-x_i)/n_points;
            x_v.push_back(x_c/1000.);
            y_v.push_back(poly->Eval(i)/100.);

        }

        TFile * file = new TFile((rootFilePattern+"__"+std::to_string(z)+"_0__3.root").c_str(), "recreate");
        TGraph * graph = new TGraph(x_v.size(), &x_v[0], &y_v[0]);
        graph->SetName((rootFilePattern+"__"+std::to_string(z)+"_0__3").c_str());
        file->cd();
        graph->Write();
        file->Close();
        
        x_v.clear();
        y_v.clear();
        coeff.clear();

    }   
}

void convertSpliToTGraph(std::string spliFileName, std::string rootFilePattern){

    std::ifstream spliFile(spliFileName);
    if (!spliFile.is_open()){
        std::cout << "Error opening file" << std::endl;
        return;
    }

    
    std::string line;
    std::vector<double> x_v,y_v;
    double x,y;
    while (std::getline(spliFile, line)){
        std::istringstream iss(line);
        iss >> x >> y;
        std::cout << x << " " << y << std::endl;
        x_v.push_back(x/1000.);
        y_v.push_back(y/100.+3);
        line.clear();
        iss.clear();
    }
    TFile * file = new TFile((rootFilePattern+".root").c_str(), "recreate");
    TGraph * graph = new TGraph(x_v.size(), &x_v[0], &y_v[0]);
    graph->SetName(rootFilePattern.c_str());
    file->cd();
    graph->Write();
    file->Close();
}

void scaleCutG(std::string cutFileName, double xScale=1., double yScale=1.){
    TFile * file = new TFile(cutFileName.c_str(), "read");
    if(file->IsZombie()){
        std::cout << "Error opening file" << std::endl;
        return;
    }
    std::string cutName = cutFileName.substr(0,cutFileName.find("."));
    std::cout << cutName << std::endl;
    TCutG * cut = (TCutG*)file->Get(cutName.c_str());
    if(cut == nullptr){
        std::cout << "Error opening cut" << std::endl;
        return;
    }
    int n_points = cut->GetN();
    std::vector<double> x_v(n_points), y_v(n_points);
    TFile * outFile = new TFile(cutFileName.c_str(), "recreate");
    //TCutG * outCut = new TCutG(cutName.c_str(), n_points);
    for(int i = 0; i < n_points; i++){
        double x,y;
        cut->GetPoint(i,x,y);
        x_v[i] = x*xScale;
        y_v[i] = y*yScale;
        cut->SetPoint(i,x_v[i],y_v[i]);
    }
    outFile->cd();
    cut->Write();
    outFile->Close();
}


void copyGates(){
    int z_in = 28;
    std::pair<int,int> range_z = {26,30};
    std::pair<int,int> range_q = {22,26};
    for(int i = range_z.first; i <= range_z.second; i++){
        for (int j = range_q.first; j <= range_q.second; j++){
            
            if(i == z_in) continue;

            TFile * file = new TFile(("charge__0_"+std::to_string(z_in)+"__"+std::to_string(j)+".root").c_str(), "read");
            if(file->IsZombie()) continue;

            TCutG * cut = (TCutG*)file->Get(("charge__0_"+std::to_string(z_in)+"__"+std::to_string(j)).c_str());
            if(cut == nullptr) continue;
            

            TCutG * new_cut = new TCutG(("charge__0_"+std::to_string(i)+"__"+std::to_string(j)).c_str(), cut->GetN());

            for(int k = 0; k < cut->GetN(); k++){
                double x,y;
                cut->GetPoint(k,x,y);
                new_cut->SetPoint(k,x,y);
            }
            
            TFile * new_file = new TFile(("charge__0_"+std::to_string(i)+"__"+std::to_string(j)+".root").c_str(), "recreate");
            
            new_cut->Write();
            new_file->Close();
       }
    }
}