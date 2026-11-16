{
//========= Macro generated from object: CUTG_above155/Graph
//========= by ROOT version6.26/06
   
   cutg = new TCutG("above155",7);
   cutg->SetVarX("");
   cutg->SetVarY("");
   cutg->SetTitle("Graph");
   cutg->SetFillStyle(1000);
   cutg->SetPoint(0,154.313,104.462);
   cutg->SetPoint(1,162.047,105.518);
   cutg->SetPoint(2,162.351,1.31312);
   cutg->SetPoint(3,162.047,-0.447098);
   cutg->SetPoint(4,153.828,0.433013);
   cutg->SetPoint(5,154.04,104.462);
   cutg->SetPoint(6,154.313,104.462);
   cutg->Draw("");

	TFile f("theta_above155.root","RECREATE");
	f.cd();
	cutg->Write();

}
