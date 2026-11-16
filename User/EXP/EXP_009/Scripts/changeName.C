void changeName_(){
    std::pair<int,int> range_z{28,38};
    std::pair<int,int> range_q{20,38};

    for(int i = range_z.first; i <= range_z.second; i++){
        for(int j = range_q.first; j <= range_q.second; j++){
            std::cout << "z " << i << " q " << j << std::endl;
            std::string cutn_z = "z" + std::to_string(i);
            std::string filen_z = "z" + std::to_string(i) + ".root";
            std::string cutn_q = "z" + std::to_string(i) + "q" + std::to_string(j);
            std::string filen_q = "z" + std::to_string(i) + "q" + std::to_string(j) + ".root";
            if(!gSystem->AccessPathName(filen_z.c_str())){
                TFile * file_z = new TFile(filen_z.c_str(), "read");
                TCutG * cut_z = (TCutG*)file_z->Get(cutn_z.c_str());
                std::string newnamez = "dee__1__" + std::to_string(i);
                std::string newnamefz = "dee__1__" + std::to_string(i) + ".root";
                cut_z->SetName(newnamez.c_str());
                file_z->Close();
                TFile * new_file_z = new TFile(newnamefz.c_str(), "recreate");
                cut_z->Write();
                new_file_z->Close();
            }
            if(!gSystem->AccessPathName(filen_q.c_str())){
                TFile * file_q = new TFile(filen_q.c_str(), "read");
                TCutG * cut_q = (TCutG*)file_q->Get(cutn_q.c_str());
                std::string newnameq = "icerbeta__" + std::to_string(i) + "__" + std::to_string(j);
                std::string newnamefq = "icerbeta__" + std::to_string(i) + "__" + std::to_string(j) + ".root";
                cut_q->SetName(newnameq.c_str());
                file_q->Close();
                TFile * new_file_q = new TFile(newnamefq.c_str(), "recreate");
                cut_q->Write();
                new_file_q->Close();
            }


        }
    }
}
void changeNameCharges(){
    std::pair<int,int> range_q{25,43};
        for(int j = range_q.first; j <= range_q.second; j++){
            std::cout << " q " << j << std::endl;
            std::string cutn_q = "q" + std::to_string(j);
            std::string filen_q =  "q" + std::to_string(j) + ".root";
            if(!gSystem->AccessPathName(filen_q.c_str())){
                TFile * file_q = new TFile(filen_q.c_str(), "read");
                TCutG * cut_q = (TCutG*)file_q->Get(cutn_q.c_str());
                std::string newnameq = "icerbeta__0__" + std::to_string(j);
                std::string newnamefq = "icerbeta__0__" + std::to_string(j) + ".root";
                cut_q->SetName(newnameq.c_str());
                file_q->Close();
                TFile * new_file_q = new TFile(newnamefq.c_str(), "recreate");
                cut_q->Write();
                new_file_q->Close();
            }


        }
}

void changeNameZeds(){
    std::pair<int,int> range_z{25,56};
        for(int j = range_z.first; j <= range_z.second; j++){
            std::cout << " z " << j << std::endl;
            std::string cutn_z = "z" + std::to_string(j);
            std::string filen_z =  "z" + std::to_string(j) + ".root";
            if(!gSystem->AccessPathName(filen_z.c_str())){
                TFile * file_z = new TFile(filen_z.c_str(), "read");
                TCutG * cut_z = (TCutG*)file_z->Get(cutn_z.c_str());
                std::string newnamez = "dee__3__" + std::to_string(j);
                std::string newnamefz = "dee__3__" + std::to_string(j) + ".root";
                cut_z->SetName(newnamez.c_str());
                file_z->Close();
                TFile * new_file_z = new TFile(newnamefz.c_str(), "recreate");
                cut_z->Write();
                new_file_z->Close();
            }


        }
}

void changeNameAoverq(){
    std::pair<int,int> range_z{28,38};
    std::pair<int,int> range_q{25,43};
    for(int i = range_z.first; i <= range_z.second; i++){
        for(int j = range_q.first; j <= range_q.second; j++){
            std::cout << "z " << i << " q " << j << std::endl;
            TFile * file_q = new TFile("spl_xfp.root", "read");
            TGraph * cut_q = (TGraph*)file_q->Get("spl_xfp");
            std::string newnameq = "aoverqxfp__" + std::to_string(i) + "_" + std::to_string(j) + "__" + std::to_string(3*j);
            std::string newnamefq = "aoverqxfp__" + std::to_string(i) + "_" + std::to_string(j) + "__" + std::to_string(3*j)+ ".root";
            cut_q->SetName(newnameq.c_str());
            file_q->Close();
            TFile * new_file_q = new TFile(newnamefq.c_str(), "recreate");
            cut_q->Write();
            new_file_q->Close();
        }
    }
}

void changeNameICsplineXfp(){
            TFile * file_q = new TFile("icdeaxfp__0.root", "read");
            TGraph * cut_q = (TGraph*)file_q->Get("icdeaxfp__0");
            std::string newnameq = "icdeaxfp__0__0";
            std::string newnamefq = "icdeaxfp__0__0.root";
            cut_q->SetName(newnameq.c_str());
            file_q->Close();
            TFile * new_file_q = new TFile(newnamefq.c_str(), "recreate");
            cut_q->Write();
            new_file_q->Close();
}


void changeNameICsplineYfp(){
            TFile * file_q = new TFile("icdeayfp__0.root", "read");
            TGraph * cut_q = (TGraph*)file_q->Get("icdeayfp__0");
            std::string newnameq = "icdeayfp__0__0";
            std::string newnamefq = "icdeayfp__0__0.root";
            cut_q->SetName(newnameq.c_str());
            file_q->Close();
            TFile * new_file_q = new TFile(newnamefq.c_str(), "recreate");
            cut_q->Write();
            new_file_q->Close();
}


void mergeGraphs(){
    TGraph * tot = new TGraph();
    for(int i = 0; i < 10; ++i){
        TFile * fi = new TFile(Form("Splines/TofvsXfp/spl_s%i.root",i), "read");
        TGraph * gr = (TGraph*)fi->Get(Form("spl_s%i",i));
        for(int j = 0; j < gr->GetN(); ++j){
            double x,y;
            gr->GetPoint(j,x,y);
            if(x < 0) continue;
            tot->SetPoint(tot->GetN(),x/1000.,y);
        }
    }
    tot->SetMarkerStyle(20);
    tot->SetMarkerSize(0.5);
    tot->SetMarkerColor(kBlack);
    tot->Draw("AP");
    tot->SetName("spl_xfp");
    tot->SetTitle("spl_xfp");
    TFile * of = new TFile(Form("Splines/TofvsXfp/spl_xfp.root"), "recreate");
    tot->Write();
}

//void changeNameAoverqXfpModify(){
//    std::pair<int,int> range_z{25,56};
//    std::pair<int,int> range_q{25,43};
//    for(int i = range_z.first; i <= range_z.second; i++){
//        for(int j = range_q.first; j <= range_q.second; j++){
//            std::cout << "z " << i << " q " << j << std::endl;
//            TFile * file_q = new TFile("spl_xfp.root", "read");
//            TGraph * cut_q = (TGraph*)file_q->Get("spl_xfp");
//            for(int k = 0; k < cut_q->GetN(); ++k){
//                double x,y;
//                cut_q->GetPoint(k,x,y);
//                //if(x < 0) continue;
//                cut_q->SetPoint(k,x*1000,y/100.);
//            }
//            std::string newnameq = "aoverqxfp__" + std::to_string(i) + "_" + std::to_string(j) + "__" + std::to_string(3*j);
//            std::string newnamefq = "aoverqxfp__" + std::to_string(i) + "_" + std::to_string(j) + "__" + std::to_string(3*j)+ ".root";
//            cut_q->SetName(newnameq.c_str());
//            file_q->Close();
//            TFile * new_file_q = new TFile(newnamefq.c_str(), "recreate");
//            cut_q->Write();
//            new_file_q->Close();
//        }
//    }
//}

void changeNameAoverqXmcpModify(){
    std::pair<int,int> range_z{25,56};
    std::pair<int,int> range_q{25,43};
    for(int i = range_z.first; i <= range_z.second; i++){
        for(int j = range_q.first; j <= range_q.second; j++){
            std::cout << "z " << i << " q " << j << std::endl;
            TFile * file_q = new TFile("mcpx_3.root", "read");
            TGraph * cut_q = (TGraph*)file_q->Get("mcpx_3");
            for(int k = 0; k < cut_q->GetN(); ++k){
                double x,y;
                cut_q->GetPoint(k,x,y);
                //if(x < 0) continue;
                cut_q->SetPoint(k,x*1000,y);
            }
            std::string newnameq = "aoverqxmcp__" + std::to_string(i) + "_" + std::to_string(j) + "__" + std::to_string(3*j);
            std::string newnamefq = "aoverqxmcp__" + std::to_string(i) + "_" + std::to_string(j) + "__" + std::to_string(3*j)+ ".root";
            cut_q->SetName(newnameq.c_str());
            file_q->Close();
            TFile * new_file_q = new TFile(newnamefq.c_str(), "recreate");
            cut_q->Write();
            new_file_q->Close();
        }
    }
}

void changeNameAoverqYmcpModify(){
    std::pair<int,int> range_z{25,56};
    std::pair<int,int> range_q{25,43};
    for(int i = range_z.first; i <= range_z.second; i++){
        for(int j = range_q.first; j <= range_q.second; j++){
            std::cout << "z " << i << " q " << j << std::endl;
            TFile * file_q = new TFile("mcpy_3.root", "read");
            TGraph * cut_q = (TGraph*)file_q->Get("mcpy_3");
            for(int k = 0; k < cut_q->GetN(); ++k){
                double x,y;
                cut_q->GetPoint(k,x,y);
                //if(x < 0) continue;
                cut_q->SetPoint(k,x*1000,y);
            }
            std::string newnameq = "aoverqymcp__" + std::to_string(i) + "_" + std::to_string(j) + "__" + std::to_string(3*j);
            std::string newnamefq = "aoverqymcp__" + std::to_string(i) + "_" + std::to_string(j) + "__" + std::to_string(3*j)+ ".root";
            cut_q->SetName(newnameq.c_str());
            file_q->Close();
            TFile * new_file_q = new TFile(newnamefq.c_str(), "recreate");
            cut_q->Write();
            new_file_q->Close();
        }
    }
}


void changeNameAoverqXfpModify(){
    TFile * file_q = new TFile("icdeaxfp__0__0.root", "read");
    TGraph * cut_q = (TGraph*)file_q->Get("icdeaxfp__0__0");
    for(int k = 0; k < cut_q->GetN(); ++k){
        double x,y;
        cut_q->GetPoint(k,x,y);
        //if(x < 0) continue;
        cut_q->SetPoint(k,x*1000.,y);
    }
    std::string newnameq = "icdeaxfp__0__0";
    std::string newnamefq = "icdeaxfp__0__0.root";
    cut_q->SetName(newnameq.c_str());
    file_q->Close();
    TFile * new_file_q = new TFile(newnamefq.c_str(), "recreate");
    cut_q->Write();
    new_file_q->Close();

}


void changeNameZeds2(){
    std::pair<int,int> range_z{28,30};
        for(int j = range_z.first; j <= range_z.second; j++){
            std::cout << " z " << j << std::endl;
            std::string cutn_z = "z__3__" + std::to_string(j);
            std::string filen_z =  "23deg/old/z__3__" + std::to_string(j) + ".root";
            if(!gSystem->AccessPathName(filen_z.c_str())){
                
                int l = j + 3;

                TFile * file_z = new TFile(filen_z.c_str(), "read");
                TCutG * cut_z = (TCutG*)file_z->Get(cutn_z.c_str());
                for(int k = 0; k < cut_z->GetN(); ++k){
                    double x,y;
                    cut_z->GetPoint(k,x,y);
                    cut_z->SetPoint(k,x+32.2502991511494*2/0.180588055718551,y+32.2502991511494*2/0.180588055718551);
                }
                std::string newnamez = "dee__3__" + std::to_string(l);
                std::string newnamefz = "23deg/dee__3__" + std::to_string(l) + ".root";
                cut_z->SetName(newnamez.c_str());
                file_z->Close();
                TFile * new_file_z = new TFile(newnamefz.c_str(), "recreate");
                cut_z->Write();
                new_file_z->Close();
            }


        }
}



void changeNameZedLines(){
    std::pair<int,int> range_z{25,50};
        for(int j = range_z.first; j <= range_z.second; j++){
            std::cout << " z " << j << std::endl;
            std::string cutn_z = "zed__0__" + std::to_string(j);
            std::string filen_z =  "23deg/zed__0__" + std::to_string(j) + ".root";
            if(!gSystem->AccessPathName(filen_z.c_str())){
                
                int l = 0;
                if(j<39) l = j;
                else  l = j+1;

                double a = -32.2502991511494;
                double b = 0.180588055718551;

                TFile * file_z = new TFile(filen_z.c_str(), "read");
                TCutG * cut_z = (TCutG*)file_z->Get(cutn_z.c_str());
                for(int k = 0; k < cut_z->GetN(); ++k){
                    double x,y;
                    cut_z->GetPoint(k,x,y);
                    cut_z->SetPoint(k,(x-a)/b,(y-a)/b);
                }
                std::string newnamez = "zed__"+std::to_string(l)+"__" + std::to_string(l);
                std::string newnamefz = "23deg/zed__"+std::to_string(l)+"__" + std::to_string(l) + ".root";
                cut_z->SetName(newnamez.c_str());
                file_z->Close();
                TFile * new_file_z = new TFile(newnamefz.c_str(), "recreate");
                cut_z->Write();
                new_file_z->Close();
            }


        }
}

void changeNameAoqTime(){
    std::vector<std::string> folders {"RUNS_17-20","RUNS_22-24","RUNS_25-34","RUNS_35-36","RUNS_37-38","RUNS_39-40","RUNS_41-56","RUNS_57-59","RUNS_60-78","RUNS_80"};
        for(auto folder : folders){
            std::cout << " folder " << folder << std::endl;
            std::string cutn_z = "aoqvstime__0__0";
            std::string filen_z =  folder + "/aoqvstime__0__0.root";
            if(!gSystem->AccessPathName(filen_z.c_str())){
                TFile * file_z = new TFile(filen_z.c_str(), "read");
                TCutG * cut_z = (TCutG*)file_z->Get(cutn_z.c_str());
                std::string newnamez = "aoqvstime__1__1";
                std::string newnamefz = folder + "/aoqvstime__1__1.root";
                cut_z->SetName(newnamez.c_str());
                file_z->Close();
                TFile * new_file_z = new TFile(newnamefz.c_str(), "recreate");
                cut_z->Write();
                new_file_z->Close();
            }


        }
}


void changeNameZedLines29(){
    std::pair<int,int> range_z{25,83};
        for(int j = range_z.first; j <= range_z.second; j++){
            std::cout << " z " << j << std::endl;
            std::string cutn_z = "zed29__3__" + std::to_string(j);
            std::string filen_z =  "Conf/CUT/PRISMA/ANA/ZED/run29/zed29__3__" + std::to_string(j) + ".root";
            
                
            //int l = 0;
            //if(j<39) l = j;
            //else  l = j+1;
            double a = -16;
            double b = 0.17745652492429;
            TFile * file_z = new TFile(filen_z.c_str(), "read");
            if(file_z->IsZombie()) continue;
            TCutG * cut_z = (TCutG*)file_z->Get(cutn_z.c_str());
            for(int k = 0; k < cut_z->GetN(); ++k){
                double x,y;
                cut_z->GetPoint(k,x,y);
                cut_z->SetPoint(k,(x-a)/b,(y-a)/b);
            }
            std::string newnamez = "zed29__3__" + std::to_string(j);
            std::string newnamefz = "Conf/CUT/PRISMA/ANA/ZED/run29/zed29__3__" + std::to_string(j) + ".root";
            cut_z->SetName(newnamez.c_str());
            file_z->Close();
            TFile * new_file_z = new TFile(newnamefz.c_str(), "recreate");
            cut_z->Write();
            new_file_z->Close();


        }
}


void changeNameQ29(){
    std::pair<int,int> range_q{30,46};
        for(int j = range_q.second; j >= range_q.first; j--){
            std::cout << " q " << j << std::endl;
            std::string cutn_q = "q__2_82__" + std::to_string(j);
            std::string filen_q =  "Conf/CUT/PRISMA/ANA/CHARGE/run29/q__2_82__" + std::to_string(j) + ".root";
            
                
            int l = j+9;

            TFile * file_q = new TFile(filen_q.c_str(), "read");
            if(file_q->IsZombie()) continue;
            TCutG * cut_q = (TCutG*)file_q->Get(cutn_q.c_str());
            std::string newnameq = "q__2_82__" + std::to_string(l);
            std::string newnamefq = "Conf/CUT/PRISMA/ANA/CHARGE/run29/charge__2_82__" + std::to_string(l) + ".root";
            cut_q->SetName(newnameq.c_str());
            file_q->Close();
            TFile * new_file_q = new TFile(newnamefq.c_str(), "recreate");
            cut_q->Write();
            new_file_q->Close();


        }
}

void changeNameZedLines73(){
    std::pair<int,int> range_z{25,83};
        for(int j = range_z.first; j <= range_z.second; j++){
            std::cout << " z " << j << std::endl;
            std::string cutn_z = "z__3__" + std::to_string(j);
            std::string filen_z =  "Conf/CUT/PRISMA/ANA/ZED/run73/z__3__" + std::to_string(j) + ".root";
            
                
            //int l = 0;
            //if(j<39) l = j;
            //else  l = j+1;
            double a = -16.6522;
            double b =  0.173269;
            TFile * file_z = new TFile(filen_z.c_str(), "read");
            if(file_z->IsZombie()) continue;
            TCutG * cut_z = (TCutG*)file_z->Get(cutn_z.c_str());
            for(int k = 0; k < cut_z->GetN(); ++k){
                double x,y;
                cut_z->GetPoint(k,x,y);
                cut_z->SetPoint(k,(x-a)/b,(y-a)/b);
            }
            std::string newnamez = "z__3__" + std::to_string(j);
            std::string newnamefz = "Conf/CUT/PRISMA/ANA/ZED/run73/z__3__" + std::to_string(j) + ".root";
            cut_z->SetName(newnamez.c_str());
            file_z->Close();
            TFile * new_file_z = new TFile(newnamefz.c_str(), "recreate");
            cut_z->Write();
            new_file_z->Close();


        }
}


void changeNameQ73(){
    std::pair<int,int> range_q{30,46};
        for(int j = range_q.second; j >= range_q.first; j--){
            std::cout << " q " << j << std::endl;
            std::string cutn_q = "q__2_82__" + std::to_string(j);
            std::string filen_q =  "Conf/CUT/PRISMA/ANA/CHARGE/run73/q__2_82__" + std::to_string(j) + ".root";
            
                
            int l = j+9;

            TFile * file_q = new TFile(filen_q.c_str(), "read");
            if(file_q->IsZombie()) continue;
            TCutG * cut_q = (TCutG*)file_q->Get(cutn_q.c_str());
            std::string newnameq = "q__2_82__" + std::to_string(l);
            std::string newnamefq = "Conf/CUT/PRISMA/ANA/CHARGE/run29/charge__2_82__" + std::to_string(l) + ".root";
            cut_q->SetName(newnameq.c_str());
            file_q->Close();
            TFile * new_file_q = new TFile(newnamefq.c_str(), "recreate");
            cut_q->Write();
            new_file_q->Close();


        }
}


void rescaleCut(std::string path, double factorX = 1, double factorY = 1){
    std::string filename = path.substr(path.find_last_of("/")+1);
    std::string cutname = filename.substr(0,filename.find_last_of("."));

    TFile * file = new TFile(path.c_str(), "update");
    TCutG * cut = (TCutG*)file->Get(cutname.c_str());
    if(cut == nullptr){
        std::cerr << "Cut not found" << std::endl;
        return;
    }
    for(int k = 0; k < cut->GetN(); ++k){
        double x,y;
        cut->GetPoint(k,x,y);
        cut->SetPoint(k,x*factorX,y*factorY);
    }
    cut->Write();
    file->Close();
    
}

void shiftCut(std::string path, double shiftX = 0, double shiftY = 0){
    std::string filename = path.substr(path.find_last_of("/")+1);
    std::string cutname = filename.substr(0,filename.find_last_of("."));

    TFile * file = new TFile(path.c_str(), "update");
    TCutG * cut = (TCutG*)file->Get(cutname.c_str());
    if(cut == nullptr){
        std::cerr << "Cut not found" << std::endl;
        return;
    }
    for(int k = 0; k < cut->GetN(); ++k){
        double x,y;
        cut->GetPoint(k,x,y);
        cut->SetPoint(k,x+shiftX,y+shiftY);
    }
    cut->Write();
    file->Close();
    
}