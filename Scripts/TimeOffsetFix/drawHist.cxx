int drawHist() {
    TFile* _file0 = TFile::Open("out.root");
    TH1D * h, *horg;
    _file0->GetObject("h", horg);
    h           = (TH1D*)horg->Clone("hmod");

    Double_t th = 0.4;
    for(int i = 3; i < h->GetNbinsX() - 2; ++i) {
        uint     mult(0);
        Double_t binI = h->GetBinContent(i);
        for(int j = i - 2; j < i + 3; ++j)
            if(h->GetBinContent(j) > th * binI) ++mult;

        if(mult < 3) h->SetBinContent(i, 0);
        //    h->SetBinContent(i,h->GetBinContent(i-1));
    }
    TAxis*   ax     = h->GetXaxis();
    Double_t binMax = h->GetMaximumBin();
    Double_t valMax = ax->GetBinCenter(binMax);
    ax->SetRangeUser(valMax - 800, valMax + 800);
    horg->GetXaxis()->SetRangeUser(valMax - 800, valMax + 800);
    h->Draw();
    new TCanvas;
    horg->Draw();
    cout << Form("%.lf", valMax);
    return 0;
}
