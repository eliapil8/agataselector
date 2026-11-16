#include <iostream>
#include <fstream>
#include <vector>
#include <TGraph.h>
#include <TCanvas.h>

int plotPrismaTraj() {
    std::ifstream inputFile("traj.log");
    if (!inputFile) {
        std::cout << "Failed to open the input file." << std::endl;
        return 1;
    }

    bool drewFirst = false;
    std::vector<TGraph*> graphs_xz;
    std::vector<TGraph*> graphs_sy;
    std::vector<double> xValues;
    std::vector<double> yValues;
    std::vector<double> zValues;
    std::vector<double> sValues;
    double x,y,z;
    std::string line;
    TCanvas* canvas_1 = new TCanvas("canvas_1", "Trajectories XZ", 800, 800);
    TCanvas* canvas_2 = new TCanvas("canvas_2", "Trajectories SY", 800, 800);
    TH2 * h1 = new TH2F("h1",";X;Z",100, -7, 0.5,100,-0.50,7);
    TH2 * h2 = new TH2F("h2",";S;Y",100, -0.5, 7, 100, -0.3, 0.3);


    TGraph* ref;
    TLine *lDipIn;
    TLine *lDipOut;
    TLine *lPPAC;

    TLegend *leg1 = new TLegend(0.1,0.7,0.48,0.9);
    TLegend *leg2 = new TLegend(0.1,0.7,0.48,0.9);

    std::vector<std::string> keywords{
        "GEOMETRY",
        "NEW",
    };
    std::string parname;

    std::vector<int> colors{
        kRed+2,
        kBlue+2,
        kGreen+2,
        kYellow+3,
        kMagenta+2,
        kCyan+2,
        kOrange+1,
        kViolet+2,
        kTeal+1,
        kAzure+2,
    };

    while (std::getline(inputFile, line)) {

        if (std::find(keywords.begin(), keywords.end(), line) != keywords.end()){
            if (!xValues.empty() && !yValues.empty() && !zValues.empty()) {
                if(!drewFirst){
                        lDipIn = new TLine(xValues[zValues.size()-6],zValues[zValues.size()-6],xValues[xValues.size()-5],zValues[zValues.size()-5]);lDipIn->SetLineColor(kRed);
                        lDipOut = new TLine(xValues[zValues.size()-4],zValues[zValues.size()-4],xValues[xValues.size()-3],zValues[zValues.size()-3]);lDipOut->SetLineColor(kRed);
                        lPPAC = new TLine(xValues[zValues.size()-2],zValues[zValues.size()-2],xValues[xValues.size()-1],zValues[zValues.size()-1]); lPPAC->SetLineColor(kBlue);
                        for(int i = 0; i < 6; i++){
                            xValues.pop_back();
                            yValues.pop_back();
                            zValues.pop_back(); 
                        } 
                        ref = new TGraph(xValues.size(), &xValues[0], &zValues[0]);
                        ref->SetMarkerStyle(20);
                        ref->SetMarkerSize(1);
                        ref->SetMarkerColor(kBlack);
                        drewFirst = true;
                        xValues.clear();
                        yValues.clear();
                        zValues.clear();
                        sValues.clear();
                }
                else{
                    TGraph* graph_xz = new TGraph(xValues.size(), &xValues[0], &zValues[0]);
                    graphs_xz.push_back(graph_xz);
                    graphs_xz.back()->SetMarkerStyle(20);
                    graphs_xz.back()->SetMarkerSize(0.5); 
                    int color = rand()%100;
                    graphs_xz.back()->SetMarkerColor(color);
                    graphs_xz.back()->SetLineColor(color);

                    for(auto i=0; i < xValues.size(); i++){
                        if(i==0) sValues.push_back(0);
                        else sValues.push_back(sValues[i-1]+sqrt(pow(xValues[i]-xValues[i-1],2)+pow(zValues[i]-zValues[i-1],2)));

                    }

                    TGraph* graph_sy = new TGraph(yValues.size(), &sValues[0], &yValues[0]);
                    graphs_sy.push_back(graph_sy);
                    graphs_sy.back()->SetMarkerStyle(20);
                    graphs_sy.back()->SetMarkerSize(0.5); 
                    graphs_sy.back()->SetMarkerColor(color);
                    graphs_sy.back()->SetLineColor(color);

                    xValues.clear();
                    yValues.clear();
                    zValues.clear();
                    sValues.clear();
                }
                
            }
        } else {
            std::istringstream iss(line);
            if (iss >> parname >>  x >> y >> z) {
                xValues.push_back(x);
                yValues.push_back(y);
                zValues.push_back(z);
            }
        }
    }

    std::cout << "Read " << graphs_xz.size() << " graphs." << std::endl;
    inputFile.close();

    canvas_1->cd();
    h1->Draw();

    ref->Draw("LPsame");

    lDipIn->Draw("same");
    lDipOut->Draw("same");
    lPPAC->Draw("same");
    // Plotting code using TGraphs goes here
    int maxIt = 15;
    int it = 0;
    for (auto graph : graphs_xz) {
        graph->SetMarkerColor(colors[it]);
        graph->SetLineColor(colors[it]);
        graph->Draw("LPsame");
        leg1->AddEntry(graph,Form("g%d",it),"lp");


        ++it;
        if(it == maxIt) break;
    }
    leg1->Draw("same");

    canvas_2->cd();
    h2->Draw();
    // Plotting code using TGraphs goes here
    it = 0;
    for (auto graph : graphs_sy) {
        graph->Draw("LPsame");
        leg2->AddEntry(graph,Form("gg%d",it),"lp");

        ++it;
        if(it == maxIt) break;
    }
    leg2->Draw("same");
    return 0;
}
