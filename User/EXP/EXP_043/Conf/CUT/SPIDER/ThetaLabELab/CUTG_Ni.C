{
//========= Macro generated from object: CUTG_Ni/Graph
//========= by ROOT version6.26/06
   
   cutg = new TCutG("Ni",7);
   cutg->SetVarX("");
   cutg->SetVarY("");
   cutg->SetTitle("Graph");
   cutg->SetFillStyle(1000);
   cutg->SetPoint(0,124.922,116.432);
   cutg->SetPoint(1,132.657,117.488);
   cutg->SetPoint(2,167.416,96.5412);
   cutg->SetPoint(3,164.686,58.6964);
   cutg->SetPoint(4,124.862,59.2244);
   cutg->SetPoint(5,124.649,116.432);
   cutg->SetPoint(6,124.922,116.432);
   cutg->Draw("");
	TFile f("theta_Ni.root","RECREATE");
	f.cd();
	cutg->Write();

}
