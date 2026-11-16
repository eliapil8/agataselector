// void FitElossUser(){
//     std::ifstream file("good/eloss_user.dat");
//     std::vector<double> Ec, Enm, IC_E_A, IC_Eres;
//     std::vector<unsigned int> A, Z;

//     std::vector<TGraph*> EnmvsICEA_beta;
//     std::unordered_map<int,TGraph*> EnmvsICEA_z;

//     TGraph *grFit_fast = new TGraph();
//     TGraph *grFit_b15 = new TGraph();
//     TGraph *grFit_b14 = new TGraph();
//     TGraph *grFit_b13 = new TGraph();
//     TGraph *grFit_b12 = new TGraph();
//     TGraph *grFit_b11 = new TGraph();
//     TGraph *grFit_b10 = new TGraph();
//     TGraph *grFit_b9 = new TGraph();
//     TGraph *grFit_b8 = new TGraph();
//     TGraph *grFit_b7 = new TGraph();
//     TGraph *grFit_b6 = new TGraph();
//     TGraph *grFit_b5 = new TGraph();

//     double beta, ec, e1, e2, e3, e4, ic_e_a, ic_e_b, ic_e_c, ic_e_d;
//     unsigned int a, z;
//     std::string line;
//     int i = 0;
//     std::getline(file,line);
//     while(std::getline(file, line)){
//         a=0; z=0; beta=0; ec=0; e1=0; e2=0; e3=0; e4=0; ic_e_a=0; ic_e_b=0;
//         ic_e_c=0; ic_e_d=0; if(A.size()%529==0&&A.size()!=0){
//             EnmvsICEA_beta.push_back(new TGraph(Enm.size(), &IC_E_A[0],
//             &Enm[0])); A.clear(); Z.clear(); Ec.clear(); Enm.clear();
//             IC_E_A.clear();
//             IC_Eres.clear();
//             i++;
//             continue;
//         }

//         std::istringstream iss(line);
//         iss >> a >> z >> beta >> ec >> e1 >> e2 >> e3 >> e4  >> ic_e_a >>
//         ic_e_b >> ic_e_c >> ic_e_d;
//         //if(!(iss >> a >> z >> ec >> e1 >> e2 >> e3 >> e4 >> ic_e_a >>
//         ic_e_res)) break; A.push_back(a); Z.push_back(z); Ec.push_back(ec);
//         Enm.push_back(e1+e2+e3+e4);
//         IC_E_A.push_back(ic_e_a);
//         IC_Eres.push_back(ic_e_b+ic_e_c+ic_e_d);
//         line.clear();

//         if(i==0){
//             if(IC_E_A.back()==0 && Enm.back()==0) std::cout << A.size() <<
//             std::endl; grFit_b5->SetPoint(grFit_b5->GetN(), IC_E_A.back(),
//             Enm.back());
//         }
//         if(i==1){
//             if(IC_E_A.back()==0 && Enm.back()==0) std::cout << A.size() <<
//             std::endl; grFit_b6->SetPoint(grFit_b6->GetN(), IC_E_A.back(),
//             Enm.back());
//         }
//         if(i==2){
//             if(IC_E_A.back()==0 && Enm.back()==0) std::cout << A.size() <<
//             std::endl; grFit_b7->SetPoint(grFit_b7->GetN(), IC_E_A.back(),
//             Enm.back());
//         }
//         if(i==3){
//             if(IC_E_A.back()==0 && Enm.back()==0) std::cout << A.size() <<
//             std::endl; grFit_b8->SetPoint(grFit_b8->GetN(), IC_E_A.back(),
//             Enm.back());
//         }
//         if(i==4){
//             if(IC_E_A.back()==0 && Enm.back()==0) std::cout << A.size() <<
//             std::endl; grFit_b9->SetPoint(grFit_b9->GetN(), IC_E_A.back(),
//             Enm.back());
//         }
//         if(i==5){
//             if(IC_E_A.back()==0 && Enm.back()==0) std::cout << A.size() <<
//             std::endl; grFit_b10->SetPoint(grFit_b10->GetN(), IC_E_A.back(),
//             Enm.back());
//         }
//         if(i==6){
//             if(IC_E_A.back()==0 && Enm.back()==0) std::cout << A.size() <<
//             std::endl; grFit_b11->SetPoint(grFit_b11->GetN(), IC_E_A.back(),
//             Enm.back());
//         }
//         if(i==7){
//             if(IC_E_A.back()==0 && Enm.back()==0) std::cout << A.size() <<
//             std::endl; grFit_b12->SetPoint(grFit_b12->GetN(), IC_E_A.back(),
//             Enm.back()); grFit_fast->SetPoint(grFit_fast->GetN(),
//             IC_E_A.back(), Enm.back());
//         }
//         if(i==8){
//             if(IC_E_A.back()==0 && Enm.back()==0) std::cout << A.size() <<
//             std::endl; grFit_b13->SetPoint(grFit_b13->GetN(), IC_E_A.back(),
//             Enm.back()); grFit_fast->SetPoint(grFit_fast->GetN(),
//             IC_E_A.back(), Enm.back());
//         }
//         if(i==9){
//             if(IC_E_A.back()==0 && Enm.back()==0) std::cout << A.size() <<
//             std::endl; grFit_b14->SetPoint(grFit_b14->GetN(), IC_E_A.back(),
//             Enm.back()); grFit_fast->SetPoint(grFit_fast->GetN(),
//             IC_E_A.back(), Enm.back());
//         }
//         if(i==10){
//             if(IC_E_A.back()==0 && Enm.back()==0) std::cout << A.size() <<
//             std::endl; grFit_b15->SetPoint(grFit_b15->GetN(), IC_E_A.back(),
//             Enm.back()); grFit_fast->SetPoint(grFit_fast->GetN(),
//             IC_E_A.back(), Enm.back());
//         }
//     }

//     file.clear(); // Clear any error flags
//     file.seekg(0, std::ios::beg); // Move the file pointer back to the
//     beginning i=0; while(std::getline(file, line)){
//         a=0; z=0; ec=0; e1=0; e2=0; e3=0; e4=0; ic_e_a=0; ic_e_b=0; ic_e_c=0;
//         ic_e_d=0;

//         std::istringstream iss(line);
//         iss >> a >> z >> ec >> e1 >> e2 >> e3 >> e4 >> ic_e_a >> ic_e_b >>
//         ic_e_c >> ic_e_d;
//         //if(!(iss >> a >> z >> ec >> e1 >> e2 >> e3 >> e4 >> ic_e_a >>
//         ic_e_res)) break; A.push_back(a); Z.push_back(z); Ec.push_back(ec);
//         Enm.push_back(e1+e2+e3+e4);
//         IC_E_A.push_back(ic_e_a);
//         IC_Eres.push_back(ic_e_b+ic_e_c+ic_e_d);
//         line.clear();

//         if(IC_E_A.back()==0 && Enm.back()==0) std::cout << A.size() <<
//         std::endl; if(EnmvsICEA_z.find(z)==EnmvsICEA_z.end())
//         EnmvsICEA_z.emplace(z,new TGraph());
//         //std::cout << z << " npoints " << EnmvsICEA_z[z]->GetN() <<
//         std::endl;
//         EnmvsICEA_z[z]->SetPoint(EnmvsICEA_z[z]->GetN(),IC_E_A.back(),
//         Enm.back());
//     }

//     TCanvas *c1 = new TCanvas("c1", "c1", 1000,1000);
//     EnmvsICEA_beta[0]->SetTitle("E_{nm} vs IC_E_A; IC_E_A (MeV); E_{nm}
//     (MeV)"); EnmvsICEA_beta[0]->SetMarkerStyle(20);
//     EnmvsICEA_beta[0]->SetMarkerColor(kRed);
//     EnmvsICEA_beta[1]->SetMarkerStyle(21);
//     EnmvsICEA_beta[1]->SetMarkerColor(kBlue);
//     EnmvsICEA_beta[2]->SetMarkerStyle(22);
//     EnmvsICEA_beta[2]->SetMarkerColor(kGreen);
//     EnmvsICEA_beta[3]->SetMarkerStyle(23);
//     EnmvsICEA_beta[3]->SetMarkerColor(kOrange);
//     EnmvsICEA_beta[4]->SetMarkerStyle(24);
//     EnmvsICEA_beta[4]->SetMarkerColor(kBlack);
//     EnmvsICEA_beta[5]->SetMarkerStyle(20);
//     EnmvsICEA_beta[5]->SetMarkerColor(kRed+3);
//     EnmvsICEA_beta[6]->SetMarkerStyle(21);
//     EnmvsICEA_beta[6]->SetMarkerColor(kBlue+3);
//     EnmvsICEA_beta[7]->SetMarkerStyle(22);
//     EnmvsICEA_beta[7]->SetMarkerColor(kGreen+3);
//     EnmvsICEA_beta[8]->SetMarkerStyle(23);
//     EnmvsICEA_beta[8]->SetMarkerColor(kOrange+3);
//     EnmvsICEA_beta[9]->SetMarkerStyle(24);
//     EnmvsICEA_beta[9]->SetMarkerColor(kBlack+3);
//     EnmvsICEA_beta[10]->SetMarkerStyle(20);
//     EnmvsICEA_beta[10]->SetMarkerColor(kRed-2);

//     TGraph * axisframe = new TGraph();
//     axisframe->SetPoint(axisframe->GetN(),50,10);
//     axisframe->SetPoint(axisframe->GetN(),550,60);
//     axisframe->SetTitle("E_{nm} vs IC_E_A;E_A (MeV); E_{nm} (MeV)");
//     EnmvsICEA_beta[0]->GetXaxis()->SetRangeUser(51,550);
//     EnmvsICEA_beta[0]->GetYaxis()->SetRangeUser(10,60);

//     axisframe->SetMarkerColor(kWhite);
//     axisframe->Draw("AP");
//     EnmvsICEA_beta[0]->Draw("Psame");
//     EnmvsICEA_beta[1]->Draw("Psame");
//     EnmvsICEA_beta[2]->Draw("Psame");
//     EnmvsICEA_beta[3]->Draw("Psame");
//     EnmvsICEA_beta[4]->Draw("Psame");
//     EnmvsICEA_beta[5]->Draw("Psame");
//     EnmvsICEA_beta[6]->Draw("Psame");
//     EnmvsICEA_beta[7]->Draw("Psame");
//     EnmvsICEA_beta[8]->Draw("Psame");
//     EnmvsICEA_beta[9]->Draw("Psame");
//     EnmvsICEA_beta[10]->Draw("Psame");

//     TLegend *leg = new TLegend(0.6,0.1,0.9,0.4);
//     leg->AddEntry(EnmvsICEA_beta[0], "#beta = 0.05", "p");
//     leg->AddEntry(EnmvsICEA_beta[1], "#beta = 0.06", "p");
//     leg->AddEntry(EnmvsICEA_beta[2], "#beta = 0.07", "p");
//     leg->AddEntry(EnmvsICEA_beta[3], "#beta = 0.08", "p");
//     leg->AddEntry(EnmvsICEA_beta[4], "#beta = 0.09", "p");
//     leg->AddEntry(EnmvsICEA_beta[5], "#beta = 0.1", "p");
//     leg->AddEntry(EnmvsICEA_beta[6], "#beta = 0.11", "p");
//     leg->AddEntry(EnmvsICEA_beta[7], "#beta = 0.12", "p");
//     leg->AddEntry(EnmvsICEA_beta[8], "#beta = 0.13", "p");
//     leg->AddEntry(EnmvsICEA_beta[9], "#beta = 0.14", "p");
//     leg->AddEntry(EnmvsICEA_beta[10], "#beta = 0.15", "p");
//     leg->Draw();

//     TCanvas *c2 = new TCanvas("c2", "c2", 1000,1000);
//     grFit_b5->GetXaxis()->SetRangeUser(50,550);
//     grFit_b6->GetXaxis()->SetRangeUser(50,550);
//     grFit_b7->GetXaxis()->SetRangeUser(50,550);
//     grFit_b8->GetXaxis()->SetRangeUser(50,550);
//     grFit_b9->GetXaxis()->SetRangeUser(50,550);
//     grFit_b10->GetXaxis()->SetRangeUser(50,550);
//     grFit_b11->GetXaxis()->SetRangeUser(50,550);
//     grFit_fast->GetXaxis()->SetRangeUser(50,550);
//     grFit_b5->GetYaxis()->SetRangeUser(10,60);
//     grFit_b6->GetYaxis()->SetRangeUser(10,60);
//     grFit_b7->GetYaxis()->SetRangeUser(10,60);
//     grFit_b8->GetYaxis()->SetRangeUser(10,60);
//     grFit_b9->GetYaxis()->SetRangeUser(10,60);
//     grFit_b10->GetYaxis()->SetRangeUser(10,60);
//     grFit_b11->GetYaxis()->SetRangeUser(10,60);
//     grFit_fast->GetYaxis()->SetRangeUser(10,60);
//     grFit_b5->SetTitle("E_{nm} vs IC_E_A;E_A (MeV); E_{nm} (MeV)");
//     grFit_b6->SetTitle("E_{nm} vs IC_E_A;E_A (MeV); E_{nm} (MeV)");
//     grFit_b7->SetTitle("E_{nm} vs IC_E_A;E_A (MeV); E_{nm} (MeV)");
//     grFit_b8->SetTitle("E_{nm} vs IC_E_A;E_A (MeV); E_{nm} (MeV)");
//     grFit_b9->SetTitle("E_{nm} vs IC_E_A;E_A (MeV); E_{nm} (MeV)");
//     grFit_b10->SetTitle("E_{nm} vs IC_E_A;E_A (MeV); E_{nm} (MeV)");
//     grFit_b11->SetTitle("E_{nm} vs IC_E_A;E_A (MeV); E_{nm} (MeV)");
//     grFit_fast->SetTitle("E_{nm} vs IC_E_A;E_A (MeV); E_{nm} (MeV)");
//     grFit_b5->SetMarkerStyle(36);
//     grFit_b6->SetMarkerStyle(35);
//     grFit_b7->SetMarkerStyle(34);
//     grFit_b8->SetMarkerStyle(33);
//     grFit_b9->SetMarkerStyle(32);
//     grFit_b10->SetMarkerStyle(31);
//     grFit_b11->SetMarkerStyle(30);
//     grFit_fast->SetMarkerStyle(29);
//     grFit_b5->SetMarkerSize(1);
//     grFit_b6->SetMarkerSize(1);
//     grFit_b7->SetMarkerSize(1);
//     grFit_b8->SetMarkerSize(1);
//     grFit_b9->SetMarkerSize(1);
//     grFit_b10->SetMarkerSize(1);
//     grFit_b11->SetMarkerSize(1);
//     grFit_fast->SetMarkerSize(1);
//     grFit_b5->SetMarkerColor(kRed+2);
//     grFit_b6->SetMarkerColor(kBlue+2);
//     grFit_b7->SetMarkerColor(kGreen+2);
//     grFit_b8->SetMarkerColor(kMagenta+2);
//     grFit_b9->SetMarkerColor(kOrange+2);
//     grFit_b10->SetMarkerColor(kGray);
//     grFit_b11->SetMarkerColor(kCyan+2);
//     grFit_fast->SetMarkerColor(kViolet+2);
//     axisframe->Draw("AP");
//     grFit_b5->Draw("P");
//     grFit_b6->Draw("P");
//     grFit_b7->Draw("P");
//     grFit_b8->Draw("P");
//     grFit_b9->Draw("P");
//     grFit_b10->Draw("P");
//     grFit_b11->Draw("P");
//     grFit_fast->Draw("P");

//     TLegend * legFit = new TLegend(0.6,0.1,0.9,0.4);
//     TF1 *fit_b5 = new TF1("fit_b5", "pol1", 10, 590);
//     TF1 *fit_b6 = new TF1("fit_b6", "pol1", 10, 590);
//     TF1 *fit_b7 = new TF1("fit_b7", "pol1", 10, 590);
//     TF1 *fit_b8 = new TF1("fit_b8", "pol1", 10, 590);
//     TF1 *fit_b9 = new TF1("fit_b9", "pol2", 10, 590);
//     TF1 *fit_b10 = new TF1("fit_b10", "pol2", 50, 590);
//     TF1 *fit_b11 = new TF1("fit_b11", "pol2", 50, 590);
//     TF1 *fit_fast = new TF1("fit_fast", "pol2", 50, 590);
//     grFit_b5->Fit(fit_b5, "R");
//     grFit_b6->Fit(fit_b6, "R");
//     grFit_b7->Fit(fit_b7, "R");
//     grFit_b8->Fit(fit_b8, "R");
//     grFit_b9->Fit(fit_b9, "R");
//     grFit_b10->Fit(fit_b10, "R");
//     grFit_b11->Fit(fit_b11, "R");
//     grFit_fast->Fit(fit_fast, "R");
//     fit_b5->SetLineColor(kRed);
//     fit_b6->SetLineColor(kBlue);
//     fit_b7->SetLineColor(kGreen);
//     fit_b8->SetLineColor(kMagenta);
//     fit_b9->SetLineColor(kOrange);
//     fit_b10->SetLineColor(kBlack);
//     fit_b11->SetLineColor(kCyan);
//     fit_fast->SetLineColor(kViolet);
//     fit_b5->Draw("same");
//     fit_b6->Draw("same");
//     fit_b7->Draw("same");
//     fit_b8->Draw("same");
//     fit_b9->Draw("same");
//     fit_b10->Draw("same");
//     fit_b11->Draw("same");
//     fit_fast->Draw("same");
//     legFit->AddEntry(fit_b5, "#beta = 0.05", "l");
//     legFit->AddEntry(fit_b6, "#beta = 0.06", "l");
//     legFit->AddEntry(fit_b7, "#beta = 0.07", "l");
//     legFit->AddEntry(fit_b8, "#beta = 0.08", "l");
//     legFit->AddEntry(fit_b9, "#beta = 0.09", "l");
//     legFit->AddEntry(fit_b10, "#beta = 0.10", "l");
//     legFit->AddEntry(fit_b11, "#beta = 0.11", "l");
//     legFit->AddEntry(fit_fast, "#beta > 0.11", "l");
//     legFit->Draw("same");

//     std::cout << "Beta = 0.05; E_nm = "<<fit_b5->GetParameter(0)<<" +
//     "<<fit_b5->GetParameter(1)<<"*IC_E_A"<< std::endl; std::cout << "Beta =
//     0.06; E_nm = "<<fit_b6->GetParameter(0)<<" +
//     "<<fit_b6->GetParameter(1)<<"*IC_E_A"<< std::endl; std::cout << "Beta =
//     0.07; E_nm = "<<fit_b7->GetParameter(0)<<" +
//     "<<fit_b7->GetParameter(1)<<"*IC_E_A"<< std::endl; std::cout << "Beta =
//     0.09; E_nm = "<<fit_b8->GetParameter(0)<<" +
//     "<<fit_b8->GetParameter(1)<<"*IC_E_A"<< std::endl; std::cout << "Beta =
//     0.09; E_nm = "<<fit_b9->GetParameter(0)<<" +
//     "<<fit_b9->GetParameter(1)<<"*IC_E_A +
//     "<<fit_b9->GetParameter(2)<<"*IC_E_A^2"<< std::endl; std::cout << "Beta =
//     0.10; E_nm = "<<fit_b10->GetParameter(0)<<" +
//     "<<fit_b10->GetParameter(1)<<"*IC_E_A +
//     "<<fit_b10->GetParameter(2)<<"*IC_E_A^2"<< std::endl; std::cout << "Beta
//     = 0.11; E_nm = "<<fit_b11->GetParameter(0)<<" +
//     "<<fit_b11->GetParameter(1)<<"*IC_E_A +
//     "<<fit_b11->GetParameter(2)<<"*IC_E_A^2"<< std::endl; std::cout << "Beta
//     > 0.11; E_nm = "<<fit_fast->GetParameter(0)<<" +
//     "<<fit_fast->GetParameter(1)<<"*IC_E_A +
//     "<<fit_fast->GetParameter(2)<<"*IC_E_A^2"<< std::endl;

//     TCanvas *c3 = new TCanvas("c3", "c3", 1000,1000);
//     std::cout << EnmvsICEA_z.size() << std::endl;

//     std::vector<Color_t> colors = {kRed, kBlue, kGreen, kOrange, kBlack,
//     kRed+3, kBlue+3, kGreen+3, kOrange+3, kBlack+3, kRed-2, kBlue-2,
//     kGreen-2, kOrange-2};

//     TLegend* leg2 = new TLegend(0.1,0.7,0.4,0.9);

//     int j = 0;
//     for(auto it = EnmvsICEA_z.begin(); it!=EnmvsICEA_z.end(); it++){
//         it->second->SetTitle("E_{nm} vs IC_E_A; IC_E_A (MeV); E_{nm} (MeV)");
//         it->second->SetMarkerStyle(20);
//         //it->second->SetMarkerColor(kRed);
//         it->second->SetMarkerColor(colors[it->first%14]);
//         it->second->SetMarkerStyle(20);
//         it->second->GetXaxis()->SetRangeUser(50,550);
//         it->second->GetYaxis()->SetRangeUser(10,60);
//         //std::cout << it->first << " " << it->second->GetN() << std::endl;
//         if(it!=EnmvsICEA_z.begin()) it->second->Draw("Psame");
//         else it->second->Draw("AP");
//         leg2->AddEntry(it->second, Form("Z = %d", it->first), "p");
//         std::cout << it->first << " " << it->second->GetN() << std::endl;
//         ++j;
//         if(j > 5) break;
//     }
//     leg2->Draw();

//    TFile * output = new TFile("ElossGraphLine.root", "RECREATE");
//     grFit_fast->Write("grFit_fast");
//     grFit_b15->Write("grFit_b15");
//     grFit_b14->Write("grFit_b14");
//     grFit_b13->Write("grFit_b13");
//     grFit_b12->Write("grFit_b12");
//     grFit_b11->Write("grFit_b11");
//     grFit_b10->Write("grFit_b10");
//     grFit_b9->Write("grFit_b9");
//     grFit_b8->Write("grFit_b8");
//     grFit_b7->Write("grFit_b7");
//     grFit_b6->Write("grFit_b6");
//     grFit_b5->Write("grFit_b5");
//     for(auto it = EnmvsICEA_z.begin(); it!=EnmvsICEA_z.end(); it++){
//         it->second->Write(Form("EnmvsICEA_z_%d", it->first));
//     }

// }

#include "TCanvas.h"
#include "TF1.h"
#include "TFile.h"
#include "TGraph.h"
#include "TLegend.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>

void FitElossUser() {
  std::ifstream file("good/eloss_user.dat");
  if (!file.is_open())
    return;
  std::vector<unsigned int> A, Z;
  std::vector<double> Ec, Enm, IC_E_A, IC_Eres;
  std::vector<TGraph *> EnmvsICEA_beta;
  std::unordered_map<int, TGraph *> EnmvsICEA_z;
  const std::vector<double> betas = {0.05, 0.06, 0.07, 0.08, 0.09, 0.10,
                                     0.11, 0.12, 0.13, 0.14, 0.15};
  EnmvsICEA_beta.reserve(betas.size());
  for (size_t i = 0; i < betas.size(); ++i)
    EnmvsICEA_beta.push_back(new TGraph());
  std::vector<TGraph *> grFits;
  grFits.reserve(betas.size());
  for (size_t i = 0; i < betas.size(); ++i)
    grFits.push_back(new TGraph());
  TGraph *grFast = new TGraph();
  const std::vector<int> markerStylesBeta = {20, 20, 20, 20, 20, 20,
                                             23, 23, 23, 23, 23};
  const std::vector<Color_t> colorsBeta = {kMagenta, kPink,  kGray,   kRed,
                                           kBlue,    kGreen, kOrange, kBlack,
                                           kViolet,  kCyan};
  std::string line;
  int slice = 0, count = 0;
  std::getline(file, line);
  while (std::getline(file, line)) {
    if (count && count % 529 == 0) {
      ++slice;
      A.clear();
      Z.clear();
      Ec.clear();
      Enm.clear();
      IC_E_A.clear();
      IC_Eres.clear();
    }
    std::istringstream iss(line);
    unsigned int a, z;
    double beta, ec, e1, e2, e3, e4, ic_a, ic_b, ic_c, ic_d;
    iss >> a >> z >> beta >> ec >> e1 >> e2 >> e3 >> e4 >> ic_a >> ic_b >>
        ic_c >> ic_d;
    A.push_back(a);
    Z.push_back(z);
    Ec.push_back(ec);
    Enm.push_back(e1 + e2 + e3 + e4);
    IC_E_A.push_back(ic_a);
    IC_Eres.push_back(ic_b + ic_c + ic_d);
    if (IC_E_A.back() == 0 && Enm.back() == 0)
      std::cout << "Slice " << slice << " zero at idx " << count
                << ", A.size()=" << A.size() << "\n";
    if (slice < (int)betas.size())
      EnmvsICEA_beta[slice]->SetPoint(EnmvsICEA_beta[slice]->GetN(),
                                      IC_E_A.back(), Enm.back());
    if (slice < (int)grFits.size())
      grFits[slice]->SetPoint(grFits[slice]->GetN(), IC_E_A.back(), Enm.back());
    if (slice >= 7 && slice < (int)grFits.size())
      grFast->SetPoint(grFast->GetN(), IC_E_A.back(), Enm.back());
    ++count;
  }
  file.clear();
  file.seekg(0);
  std::getline(file, line);
  while (std::getline(file, line)) {
    std::istringstream iss(line);
    unsigned int a, z;
    double ec, e1, e2, e3, e4, ic_a, ic_b, ic_c, ic_d;
    iss >> a >> z >> ec >> e1 >> e2 >> e3 >> e4 >> ic_a >> ic_b >> ic_c >> ic_d;
    double valEnm = e1 + e2 + e3 + e4, valIC = ic_a;
    if (!EnmvsICEA_z.count(z))
      EnmvsICEA_z[z] = new TGraph();
    EnmvsICEA_z[z]->SetPoint(EnmvsICEA_z[z]->GetN(), valIC, valEnm);
  }
  TCanvas *c1 = new TCanvas("c1", "Beta slices", 1024, 768);
  gPad->SetGrid();
  gPad->SetLeftMargin(0.15);
  gPad->SetBottomMargin(0.15);
  gPad->SetRightMargin(0.05);
  gPad->SetTopMargin(0.05);
  TGraph *frame = new TGraph();
  frame->SetPoint(0, 100, 13);
  frame->SetPoint(1, 530, 60);
  frame->SetTitle("; #Delta E_{A} [MeV]; E_{nm} [MeV]");
  frame->GetXaxis()->SetRangeUser(100, 530);
  frame->GetYaxis()->SetRangeUser(13, 60);
  frame->GetXaxis()->SetLabelSize(0.05);
  frame->GetYaxis()->SetLabelSize(0.05);
  frame->GetXaxis()->SetTitleSize(0.055);
  frame->GetYaxis()->SetTitleSize(0.055);
  frame->SetMarkerColor(kWhite);
  frame->Draw("AP");
  for (size_t i = 0; i < EnmvsICEA_beta.size(); ++i) {
    if (i < 3)
      continue; // Skip the first three beta slices
    TGraph *g = EnmvsICEA_beta[i];
    g->SetMarkerStyle(markerStylesBeta[i % markerStylesBeta.size()]);
    g->SetMarkerColor(colorsBeta[i % colorsBeta.size()]);
    g->Draw("Psame");
  }
  TLegend *leg1 = new TLegend(0.75, 0.17, 0.93, 0.6);
  for (size_t i = 0; i < betas.size(); ++i) {
    if (i < 3)
      continue;
    leg1->AddEntry(EnmvsICEA_beta[i], Form("#beta=%.2f", betas[i]), "p");
  }
  leg1->SetTextSize(0.04);
  leg1->SetTextAlign(22);
  leg1->Draw();

  TCanvas *c2 = new TCanvas("c2", "Fits", 1000, 1000);
  frame->Draw("AP");
  for (size_t i = 0; i < grFits.size(); ++i) {
    TGraph *g = grFits[i];
    g->GetXaxis()->SetRangeUser(100, 530);
    g->GetYaxis()->SetRangeUser(13, 60);
    g->SetMarkerStyle(markerStylesBeta[i % markerStylesBeta.size()]);
    g->SetMarkerSize(0.8);
    g->SetMarkerColor(colorsBeta[i % colorsBeta.size()]);
    g->Draw("P");
  }
  grFast->SetMarkerStyle(markerStylesBeta.back());
  grFast->SetMarkerSize(1);
  grFast->SetMarkerColor(kViolet + 2);
  // grFast->Draw("P");
  std::vector<TF1 *> fitsTF;
  for (size_t i = 0; i < grFits.size(); ++i) {

    TF1 *f = new TF1(Form("fit_b%.0f", betas[i] * 100), "pol1", 10, 590);
    fitsTF.push_back(f);
    grFits[i]->Fit(f, "RN");
    f->SetLineColor(colorsBeta[i % colorsBeta.size()]);
    f->Draw("same");
    std::cout << "Beta=" << betas[i] << "; E_nm=" << f->GetParameter(0) << " + "
              << f->GetParameter(1) << "*IC_E_A\n";
  }
  TF1 *fitFast = new TF1("fit_fast", "pol2", 50, 590);
  // grFast->Fit(fitFast, "R");
  fitFast->SetLineColor(kViolet + 2);
  fitFast->Draw("same");
  std::cout << "Beta>0.11; E_nm=" << fitFast->GetParameter(0) << " + "
            << fitFast->GetParameter(1) << "*IC_E_A + "
            << fitFast->GetParameter(2) << "*IC_E_A^2\n";
  TLegend *leg2 = new TLegend(0.6, 0.1, 0.9, 0.4);
  for (size_t i = 0; i < betas.size(); ++i)
    leg2->AddEntry(fitsTF[i], Form("#beta=%.2f", betas[i]), "l");
  // leg2->AddEntry(fitFast, "#beta>0.11", "l");
  leg2->Draw();
  TCanvas *c3 = new TCanvas("c3", "Z slices", 1000, 1000);
  std::cout << "Z slices:" << EnmvsICEA_z.size() << "\n";
  size_t j = 0;
  for (auto &it : EnmvsICEA_z) {
    if (j == 0)
      it.second->Draw("AP");
    else
      it.second->Draw("Psame");
    std::cout << it.first << " " << it.second->GetN() << "\n";
    if (++j > 6)
      break;
  }
  TFile *outFile = new TFile("ElossGraphLine.root", "RECREATE");
  for (auto g : grFits) g->Write();
  grFast->Write();
  for (auto &it : EnmvsICEA_z) it.second->Write(Form("EnmvsICEA_z_%d", it.first));
  outFile->Close();
  c1->SaveAs("/home/engy/FusionFission/Processing/PRISMA/Macros/PlotsFinal/enm_dea_srim.png");
  c1->SaveAs("/home/engy/FusionFission/Processing/PRISMA/Macros/PlotsFinal/enm_dea_srim.eps");
}
