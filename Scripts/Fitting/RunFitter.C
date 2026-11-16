#include "Fitter.cxx"

void RunFitter(){
  TFile* file = new TFile("test.root", "read");
  TH1D* h = (TH1D*) file->Get("AgataPrisma/Ion_Cuts_Z16_A32/h_DCBP_ion_32_16");

  Fitter f({{1470, 10}}, true) ;
  f.EnableCanvas();
  f.SetSpec(*h);
  
  //f.ReadParsFromFile("pars.txt", 0);

  auto res = f.Fit();

  std::cout << "integrals : " << res.front().integral.first << " +/-" << res.front().integral.second << std::endl;


  //f.WriteParsOnFile("pars.txt", 0);
}
