#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <cmath>
 
#include "Math/IFunction.h"
#include "Minuit2/Minuit2Minimizer.h"
#include "Math/Functor.h"

using namespace std;

//---------------------------------------------------------------------------------------------
//This code uses a ROOT minimizer to find the calibration coefficients for the PRISMA MCP.
//To run it, just write in bash:
//$ root -l
//$ .L MCP_cal.C
//$ MCP_cal(filename)
//---------------------------------------------------------------------------------------------
//Input file has the form
//pos     x_mm  y_mm  x_chan  y_chan
//center  0     0     1500    2500
//tl_cal  -21.5 26.5  2000    3000
//tr_cal  21.5  26.5  1000    3000
//bl_cal  -21.5 -26.5 2000    2000
//br_cal  -21.5 -26.5 1000    2000
//---------------------------------------------------------------------------------------------

class calibration{
  private:
    string inputfile;
    vector <double> par;
    vector<double> x_pos, y_pos, x_raw, y_raw;
  public:

    calibration(string filename);
    void read(string filename);
    void process();

    //print calibrated positions inside file; if filename = default then print on shell
    void print_new(string filename = "default");

    //print calibration parameter
    void print_par();

  private:
    double eval(const double* p);
    void points(const double* p, vector<double>& newx, vector<double>& newy);
    double PI = 3.14159265358979323846;
    
};

//---------------------------------------------------------------------------------------------

void MCP_cal(string filename){
  calibration cal (filename);
  cal.print_new();
  cal.print_par();
}

//---------------------------------------------------------------------------------------------

calibration::calibration(string filename){
  inputfile = filename;
  read(inputfile);
  process();
}

void calibration::read(string filename){
  ifstream fin (filename.c_str());

  //skip first line
  string tmp, tmp2;
  getline(fin, tmp);

  double t;
  while(getline(fin,tmp)){
    
    stringstream iss(tmp);
    cout << tmp << endl;
    iss >> tmp2;
    if(tmp2.find("#") == 0) continue;
    iss >> t;
    x_pos.push_back(t);
    iss >> t;
    y_pos.push_back(t);
    iss >> t;
    x_raw.push_back(t);
    iss >> t;
    y_raw.push_back(t);
  }
}

void calibration::process(){

  ROOT::Math::Minimizer* min =  new ROOT::Minuit2::Minuit2Minimizer("");

  min->SetMaxFunctionCalls(1000000);
  min->SetTolerance(0.001);
  min->SetPrintLevel(1);
  
  ROOT::Math::Functor func(this, &calibration::eval, 10);
  min->SetFunction(func);

  min->SetVariable(0,"x0",1.00, 1e-6);
  min->SetVariable(1,"x1",0.01, 1e-6);
  min->SetVariable(2,"y0",0.01, 1e-6);
  min->SetVariable(3,"y1",1.00, 1e-6);
  min->SetVariable(4,"calxa",50, 1e-6);
  min->SetVariable(5,"calxb",1, 1e-6);
  min->SetVariable(6,"calxc",0.0, 1e-6);
  min->SetVariable(7,"calya",50, 1e-6);
  min->SetVariable(8,"calyb",1, 1e-3);
  min->SetVariable(9,"angle",0.00, 1e-6);//in degrees
  min->SetVariableLimits(9,-10, 10);//in degrees

  min->FixVariable(0);
  //min->FixVariable(1);
  //min->FixVariable(2);
  min->FixVariable(3);
  //min->FixVariable(4);
  //min->FixVariable(5);
  min->FixVariable(6);
  //min->FixVariable(7);
  //min->FixVariable(8);
  min->FixVariable(9);
  min->Minimize();

  for(int i=0; i<10; i++) par.push_back(min->X()[i]);
}

double calibration::eval(const double* p){
  vector<double> newx, newy;
  newx.reserve(x_pos.size());
  newy.reserve(y_pos.size());
  
  //calculate calibrated positions with current parameters
  points(p, newx, newy);



  //calculate FOM
  double ret = 0;
  for(unsigned int i=0; i<x_pos.size(); i++){
    ret += (newx[i]-x_pos[i])*(newx[i]-x_pos[i]) + (newy[i]-y_pos[i])*(newy[i]-y_pos[i]);
  }

  

  return ret;
}

void calibration::points(const double* p, vector<double>& newx, vector<double>& newy){  
  double x0 = p[0], x1 = p[1];
  double y0 = p[2], y1 = p[3];
  double calxa = p[4], calxb = p[5],  calxc = p[6];
  double calya = p[7], calyb = p[8];
  double angle = p[9]*PI/180;//convert to rad
  double xd, yd, xc, yc, xf, yf;
  for(unsigned int i=0; i<x_pos.size(); i++){
    xd = x_raw[i]*x0 + y_raw[i]*x1;
    yd = x_raw[i]*y0 + y_raw[i]*y1;
    xc = calxa + calxb*xd + calxc*xd*xd;
    yc = calya + calyb*yd;
    xf = cos(angle)*xc - sin(angle)*yc;
    yf = sin(angle)*xc + cos(angle)*yc;
    newx.push_back(xf);
    newy.push_back(yf);
  }
}

void calibration::print_new(string filename){
  vector<double> newx, newy;
  points(&par[0], newx, newy);

  if(filename=="default"){
    cout << "\nCalibrated reference points:\n";
    for(unsigned int i=0; i<newx.size(); i++){
      cout << newx[i] << "\t" << newy[i] << "\t\t\t" << sqrt((newx[i]-x_pos[i])*(newx[i]-x_pos[i]) + (newy[i]-y_pos[i])*(newy[i]-y_pos[i])) << endl;
    }
  }
  else{
    ofstream fout (filename.c_str());
    for(unsigned int i=0; i<newx.size(); i++){
      fout << newx[i] << "\t" << newy[i] << endl;
    }
  }
}


void calibration::print_par(){

  cout << "\nCalibration parameters:\n";
  cout << "sum dist sq: " << eval(&par[0]) << "\n"; 
  cout << "|x0 x1|\t|" << par[0]  << "\t" << par[1] << "|\n";
  cout << "|y0 y1|\t|" << par[2]  << "\t" << par[3] << "|\n";
  cout << "cal x: 99 0 3\t" << par[4] << "\t" << par[5] << "\t" << par[6] << "\n";
  cout << "cal y: 99 0 2\t" << par[7] << "\t" << par[8] << "\n";
  cout << "angle: " << par[9] << endl; 

  ofstream fout (("out_"+inputfile).c_str());
  fout << "id 0 cal " << par[4] << "\t" << par[5] << endl;
  fout << "id 1 cal " << par[7] << "\t" << par[8] << endl;
  fout << "id 0 cal " << par[0] << "\t" << par[1] << "\t" << par[2] << "\t" << par[3] << endl;

}


