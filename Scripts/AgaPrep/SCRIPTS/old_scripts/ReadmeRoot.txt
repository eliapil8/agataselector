TChain *ch = new TChain("TreeMaster")
ch->Add("Tree_*.root")
ch->Draw("coreE0:coreTS/1e8/3600-25>>h(1000,0,10,4000,0,2000)","coreId==23","colz");
ch->Draw("hitE:TSHit/1e8/3600-25>>h(1000,0,10,4000,0,2000)","hitId==25&&hitSg==","colz");
