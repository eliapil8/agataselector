using namespace std;


/////////////////////////////////////// CAL 20 deg ///////////////////////////////////////
//double CalX (double x, double y){
//  double x0 = 1;
//  double x1 = -0.0709864;
//  double p0 =  43.5595;
//  double p1 = -0.0324243 ;
//  
//  double X = x*x0 + y*x1;
//  return p0 + p1*X;
//}
//
//double CalY (double x, double y){
//  double y0 = 0.0484821;
//  double y1 = 1.;
//  double p0 =  -81.9025 ;
//  double p1 = 0.0332038;
//  
//  double Y = x*y0 + y*y1;
//  return p0 + p1*Y;
//}

///////////////////////////////////////// CAL 23 deg ///////////////////////////////////////
//
//double CalX (double x, double y){
//  double x0 = 1;
//  double x1 = -0.0511768;
//  double p0 =  45.518;
//  double p1 = -0.033497 ;
//  
//  double X = x*x0 + y*x1;
//  return p0 + p1*X;
//}
////
//double CalY (double x, double y){
//  double y0 = 0.0477834;
//  double y1 = 1.;
//  double p0 =  -80.5752 ;
//  double p1 = 0.0331091;
//  
//  double Y = x*y0 + y*y1;
//  return p0 + p1*Y;
//}

/////////////////////////////////////////// CAL 20 deg 2 ///////////////////////////////////////
//double CalX (double x, double y){
//  double x0 = 1;
//  double x1 = -0.0594989;
//  double p0 =  45.011;
//  double p1 = -0.032959 ;
//  
//  double X = x*x0 + y*x1;
//  return p0 + p1*X;
//}
//
//double CalY (double x, double y){
//  double y0 = 0.0502648;
//  double y1 = 1.;
//  double p0 =  -80.8111 ;
//  double p1 = 0.0327584;
//  
//  double Y = x*y0 + y*y1;
//  return p0 + p1*Y;
//}

///////////////////////////////////////// CAL 20 deg 3 ///////////////////////////////////////
double CalX (double x, double y){
  double x0 = 1;
  double x1 = -0.0583079;
  double p0 =  44.6934;
  double p1 = -0.0330652 ;
  
  double X = x*x0 + y*x1;
  return p0 + p1*X;
}

double CalY (double x, double y){
  double y0 = 0.0591291;
  double y1 = 1.;
  double p0 =  -80.2859 ;
  double p1 = 0.0325172;
  
  double Y = x*y0 + y*y1;
  return p0 + p1*Y;
}

void DrawCalMCP( string data_file_name, string cal_file_name ){
   TFile *fileIn = new TFile(data_file_name.c_str(), "READ");
  if(fileIn->IsOpen()==0){
   cout << "Could not find input file " << data_file_name << endl;
   return;
  }
  else{
    cout << "Got the file " << data_file_name << endl;
  }
  TTree* tree = (TTree*)fileIn->Get("PrismaTree");
  //TChain * tree = new TChain("PrismaTree");
  //tree->Add(Form("%s/cal40_run_0040/Tree_00*",data_file_name.c_str()));
  //tree->Add(Form("%s/cal50_run_0050/Tree*",data_file_name.c_str()));
  TCanvas *c1 = new TCanvas("c1", "MCP_Y vs MCP_X", 1024, 768);
  gPad->SetLogz();

  TH2D * h_mcp = new TH2D("h_mcp","MCP Y vs MCP X; X (mm); Y (mm)",500,-90,45,500,-70,70);
  tree->Draw("Analyzed.MCP_Y:Analyzed.MCP_X>>h_mcp","mcp_ok","colz",1e8);
  //tree->Draw("mcp_y*1000:mcp_x*1000>>h_mcp","mcp_ok","colz",1e8);
  gStyle->SetOptStat(0);
 
    ifstream fIn(cal_file_name);
    
    string temp, temp2;
    double x_real, y_real, x_raw, y_raw, x_cal, y_cal;
    
    vector<double> X_real, Y_real, X_cal, Y_cal, X_raw, Y_raw;

    getline(fIn,temp);

    while(getline(fIn,temp)){
        stringstream iss(temp);
        
        iss >> temp2;
        if(temp2.find("#") == 0) continue;
        iss >> x_real >> y_real >> x_raw >> y_raw;
        X_real.push_back(x_real);
        Y_real.push_back(y_real);
        X_cal.push_back(CalX(x_raw,y_raw));
        Y_cal.push_back(CalY(x_raw,y_raw));
        X_raw.push_back(x_raw);
        Y_raw.push_back(y_raw);
    }
    
//    TCanvas *c1 = new TCanvas("c1", "MCP_Y vs MCP_X", 1024, 768);
    TGraph * ref_points = new TGraph(X_cal.size(),&X_real[0],&Y_real[0]);
    TGraph * cal_points = new TGraph(X_cal.size(),&X_cal[0],&Y_cal[0]);
    TGraph * raw_points = new TGraph(X_raw.size(),&X_raw[0],&Y_raw[0]);


    ref_points->SetTitle("MCP calibration; X (mm); Y (mm)");

    ref_points->SetLineWidth(3);
    cal_points->SetLineWidth(3);
    ref_points->SetMarkerSize(3);
    cal_points->SetMarkerSize(3);
    ref_points->SetMarkerStyle(51);
    cal_points->SetMarkerStyle(52);
    ref_points->SetMarkerColor(kRed);
    cal_points->SetMarkerColor(kGreen);

    TLegend * leg = new TLegend();
    leg->AddEntry(ref_points,"Reference Points","P");
    leg->AddEntry(cal_points,"Calibrated Points","P");

    ref_points->Draw("Psame");
    cal_points->Draw("Psame");
    leg->Draw("same");

    TCanvas * c2 = new TCanvas("c2", "MCP_Y vs MCP_X raw", 1024, 768);
    TH2D * h_mcp_raw = new TH2D("h_mcp_raw","MCP Y vs MCP X raw; X; Y",1000,0,4096,1000,0,4096);
    tree->Draw("MCP_raw[1]:MCP_raw[0]>>h_mcp_raw","","colz",1e8);

    raw_points->SetLineWidth(3);
    raw_points->SetMarkerSize(3);
    raw_points->SetMarkerStyle(51);
    raw_points->SetMarkerColor(kRed);

    raw_points->Draw("Psame");



}

