{
//========= Macro generated from object: CUTG_less135/Graph
//========= by ROOT version6.26/06
   
   TCutG *cutg = new TCutG("less135",7);
   cutg->SetVarX("");
   cutg->SetVarY("");
   cutg->SetTitle("Graph");
   cutg->SetFillStyle(1000);
   cutg->SetPoint(0,125.832,104.11);
   cutg->SetPoint(1,133.567,105.166);
   cutg->SetPoint(2,133.87,0.96108);
   cutg->SetPoint(3,133.567,-0.799143);
   cutg->SetPoint(4,125.347,0.0809683);
   cutg->SetPoint(5,125.559,104.11);
   cutg->SetPoint(6,125.832,104.11);
     cutg->Draw("");

   TFile f("theta_less_135.root","RECREATE");
   f.cd();
   cutg->Write();
}
