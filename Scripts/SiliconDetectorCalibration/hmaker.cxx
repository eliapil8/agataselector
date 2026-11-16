#include <ctime>
#include <iostream>

void hmaker(char* data, std::string filepath) {
    TCanvas* c1 = new TCanvas("c1", "c1");
    c1->cd();

    int         detector = 0;
    TFile*      file     = new TFile(data, "READ");
    TTree*      tree     = (TTree*)file->Get("Data_R");
    string      command;
    string      cond;
    string      _draw = "COLZ";

    std::time_t t     = std::time(0); // get time now
    std::tm*    now   = std::localtime(&t);
    int         yr    = now->tm_year + 1900;
    int         mnt   = now->tm_mon + 1;
    int         dday  = now->tm_mday;

    char        ddummy[1000];
    sprintf(ddummy, "%s/histocalib_%d%d%d.root", filepath.c_str(), dday, mnt,
            yr);
    TFile savefile(ddummy, "RECREATE");

    for(int b = 0; b < 3; b++) {
        //  if(b==1) continue;
        {
            // file->cd();
            command = "Channel:Energy>>h" + to_string(b)
                    + "(7500,0,15000,63,0,63)";

            cond = "Board == " + to_string(b);
            tree->Draw(command.c_str(), cond.c_str(), _draw.c_str());
            string obj = "h" + to_string(b);
            TH2F*  h   = (TH2F*)(gDirectory->Get(obj.c_str()));
            cout << command << " " << cond << endl;
            savefile.cd();
            h->Write();

            //	detector++;
            /*	command = "(Channel-8):Energy>>h" + to_string(detector) +
            "(1750,500,4000,16,0,16)"; cond = "Board == " +to_string(b) +" &&
            Channel > 7 ";
            tree->Draw(command.c_str(),cond.c_str(),_draw.c_str());
            obj = "h" + to_string(detector);
            h = (TH2F*)(gDirectory->Get(obj.c_str()));
            cout << command << " " << cond << endl;
            savefile.cd();
            h->Write();

            detector++;*/
        }
    }
    delete c1;
}
