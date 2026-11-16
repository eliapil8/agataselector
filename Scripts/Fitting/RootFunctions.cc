#include <vector>
#include <iostream>
#include <string>
#include <cstdio>
#include <cstdarg>
//
using namespace std;

Double_t arr[3];

Double_t Gauss(Double_t *x, Double_t *par)
{
	Double_t arg = 0;
	if (par[2] != 0) arg = (x[0] - par[1])/par[2];
	return par[0]*TMath::Exp(-0.5*arg*arg);
}
//
Double_t GaussWithTail(Double_t *x, Double_t *par) 
{
	if (par[2] != 0)
		return par[0]*(TMath::Exp(-(((x[0]-par[1])*(x[0]-par[1]))/(2.0*par[2]*par[2])))+((par[3]/2.0)*TMath::Exp(((x[0]-par[1])*par[3])+(par[2]*par[2]*par[4]*par[4]/2.0))*TMath::Erfc((0.70710678118655)*(((x[0]-par[1])/par[2])+par[2]*par[4]))));
	else return par[0];
}
//
Double_t Background(Double_t x, Double_t cent, Double_t constant, Double_t slope, Double_t quad)
{
	x = x-cent;
	return constant+slope*x+quad*x*x;
}
//
Double_t BCG(Double_t *v, Double_t *p)
{
	return Background(v[0], p[3], p[0], p[1], p[2]);
}
//
Double_t GaussWithBcg(Double_t *v, Double_t *p)
{
	return Background(v[0], p[6], p[3], p[4], p[5])+Gauss(v,p);
}
//
Double_t GaussTailedWithBcg(Double_t *v, Double_t *p)
{
	return Background(v[0], p[8], p[5], p[6], p[7])+GaussWithTail(v,p);
}
//
Double_t SeveralGaussians(Double_t *x, Double_t *par) 
{
	if(par[5]==0) return -1111;
	Int_t n = (Int_t)par[0];
	Double_t fcn = Background(x[0],par[4],par[1], par[2], par[3]);
	for (Int_t i = 0; i < n; i++)
		fcn += par[7+2*i]*TMath::Exp(-((x[0]-par[6+2*i])*(x[0]-par[6+2*i]))/(2*par[5]*par[5]));
	return fcn;
}
//
Double_t SeveralGaussiansTailed(Double_t *x, Double_t *par) 
{
	if(par[5]==0) return -1111;
	Int_t n = (Int_t)par[0];
	Double_t fcn = Background(x[0],par[4],par[1], par[2], par[3]);
	for (Int_t i = 0; i < n; i++)
		fcn += par[9+2*i]*(TMath::Exp(-(((x[0]-par[8+2*i])*(x[0]-par[8+2*i]))/(2*par[7]*par[7])))+((par[5]/2)*TMath::Exp(((x[0]-par[8+2*i])*par[5])+(par[7]*par[7]*par[6]*par[6]/2))*TMath::Erfc((0.70710678118655)*(((x[0]-par[8+2*i])/par[7])+par[7]*par[6]))));
	return fcn;
}

Int_t DoFit(TH1D *&orig, Double_t low, Double_t high, Double_t FWHM, Double_t dFWHM, Bool_t tail, Int_t n, va_list Energies);

Double_t* FitNgaussians(string histoName, Double_t low, Double_t high, Double_t FWHM, Double_t dFWHM, Bool_t tail, Int_t n, ...)
{
	TH1D *spectrum = (TH1D*)gDirectory->Get(histoName.c_str());
	va_list Energies;
	va_start(Energies, n);
	Int_t stat = DoFit(spectrum, low, high, FWHM, dFWHM, tail, n, Energies);
	va_end(Energies);
	return stat == 0 ?  arr :  nullptr;
}

Double_t* FitNgaussians(TH1D *&spectrum, Double_t low, Double_t high, Double_t FWHM, Double_t dFWHM, Bool_t tail, Int_t n, ...)
{
	va_list Energies;
	va_start(Energies, n);
	Int_t stat = DoFit(spectrum, low, high, FWHM, dFWHM, tail, n, Energies);
	va_end(Energies);
	return stat == 0 ?  arr : nullptr;
}

Int_t DoFit(TH1D *&orig, Double_t low, Double_t high, Double_t FWHM, Double_t dFWHM, Bool_t tail, Int_t n, va_list Energies)
{
	Char_t fcnTitle[200];
	Float_t r = 0.5, epsilon = 0.02;
	Char_t name[64];
//
	gStyle->SetOptStat(0);
	gStyle->SetPadTopMargin(0.1);
	gStyle->SetPadBottomMargin(0.12);
	gStyle->SetPadLeftMargin(0.07);
	gStyle->SetPadRightMargin(0.04);
	gStyle->SetLabelOffset(0.02,"X");
	gStyle->SetTitleOffset(1.5,"X");
	gStyle->SetTitleOffset(0.9,"Y");
	gStyle->SetTickLength(0.01,"X");
	gStyle->SetTickLength(0.01,"Y");
	gStyle->SetTitleAlign(22);
	gStyle->SetTitleY(.85);
	TGaxis::SetMaxDigits(4);
	gROOT->ForceStyle();
// 
	Int_t count=0;
	sprintf(name,"HistoClone_%d",count);
	while(gDirectory->Get(name))
	{
		count++;
		sprintf(name,"HistoClone_%d",count);
	}
	TH1D *fitHistogram = (TH1D*) orig->Clone(name);
	fitHistogram->GetXaxis()->SetLabelSize(0);
	Int_t noPar = 2*n+6;
	TF1 *fitFcn;
	if(tail)	fitFcn = new TF1("fitFcn",SeveralGaussiansTailed,low,high,noPar+2);
	else	fitFcn = new TF1("fitFcn",SeveralGaussians,low,high,noPar);
//
	sprintf(name,"FitCanvas_%d",count);
	TCanvas *fitCanvas = new TCanvas(name,name);
	fitCanvas->ToggleEventStatus();
	sprintf(name,"Pad1_%d",count);
	TPad *pad1 = new TPad(name, name, 0, r-epsilon, 1, 1);
	pad1->SetBottomMargin(epsilon);
	fitCanvas->cd();
	pad1->SetNumber(1);
	pad1->Draw();
	pad1->cd();
	fitHistogram->Draw();
//
	Double_t ksig = 2*sqrt(2*log(2));
	Double_t sigma = FWHM/ksig;
	Double_t dsigma = dFWHM/ksig;
	Double_t E;
//
	fitHistogram->GetXaxis()->SetRange(fitHistogram->GetXaxis()->FindBin(low)+1,fitHistogram->GetXaxis()->FindBin(high)-1);
	fitFcn->FixParameter(0,n);
	fitFcn->SetParName(0,"#Peaks");
	fitFcn->SetParameter(1,fitHistogram->GetBinContent(fitHistogram->GetXaxis()->FindBin(low)));
	fitFcn->SetParName(1,"BcgConstant");
	fitFcn->SetParName(2,"BcgSlope");
	fitFcn->SetParameter(3,0.0);
	fitFcn->SetParName(3,"BcgQuad");
//
	if(tail)
	{
		fitFcn->SetParameter(5,0.15);
		fitFcn->SetParLimits(5,1e-15,1.5);
		fitFcn->SetParName(5,"Tail1");
		fitFcn->SetParameter(6,0.1);
		fitFcn->SetParLimits(6,1e-15,1.5);
		fitFcn->SetParName(6,"Tail2");
		fitFcn->SetParameter(7,sigma);
		fitFcn->SetParLimits(7,sigma-dsigma,sigma+dsigma);
		fitFcn->SetParName(7,"Sigma");
//
		for (Int_t i=0; i<n; i++) 
		{
			E = va_arg(Energies, Double_t);
			if(E<low || E>high)
			{
				E<=0 ? cout << "************ E" << i << " is missing!!! ************" << endl : cout << "************ E" << i << " is out of range!!! ************" << endl;
				return 1;
			}
			fitFcn->SetParameter(8+2*i,E);
			fitFcn->SetParLimits(8+2*i,low+(E-low)*0.8,high-(high-E)*0.8);
			sprintf(name,"Energy_%d",i);
			fitFcn->SetParName(8+2*i,name);
//
			fitFcn->SetParameter(9+2*i, fitHistogram->GetBinContent(fitHistogram->GetXaxis()->FindBin(E)));
			fitFcn->SetParLimits(9+2*i, 0.0, 1E20);
			fitFcn->SetParError(9+2*i,fitFcn->GetParameter(9+2*i)/2);
			fitFcn->SetParLimits(9+2*i, 0.0, fitFcn->GetParameter(9+2*i)*10);
			sprintf(name,"Aplitude_%d",i);
			fitFcn->SetParName(9+2*i,name);
		}
		fitFcn->SetParameter(4,fitFcn->GetParameter(8));
		fitFcn->SetParName(4,"BcgCenter");
	}
	else
	{
		fitFcn->SetParameter(5,sigma);
		fitFcn->SetParLimits(5,sigma-dsigma,sigma+dsigma);
		fitFcn->SetParName(5,"Sigma");
//
		for (Int_t i=0; i<n; i++) 
		{
			E = va_arg(Energies, Double_t);
			cout<< E << endl;
			if(E<low || E>high)
			{
				E<=0.1 ? cout << "************ E" << i << " is missing!!! ************" << endl : cout << "************ E" << i << " is out of range!!! ************" << endl;
				return 1;
			}
			fitFcn->SetParameter(6+2*i,E);
			fitFcn->SetParLimits(6+2*i,low+(E-low)*0.8,high-(high-E)*0.8);
			sprintf(name,"Energy_%d",i);
			fitFcn->SetParName(6+2*i,name);
//
			fitFcn->SetParameter(7+2*i, fitHistogram->GetBinContent(fitHistogram->GetXaxis()->FindBin(E)));
			fitFcn->SetParError(7+2*i,fitFcn->GetParameter(7+2*i)/2);
			fitFcn->SetParLimits(7+2*i, 0.0, fitFcn->GetParameter(7+2*i)*10);
			sprintf(name,"Aplitude_%d",i);
			fitFcn->SetParName(7+2*i,name);
		}
		fitFcn->SetParameter(4,fitFcn->GetParameter(6));
		fitFcn->SetParName(4,"BcgCenter");
	}
//
	TFitResultPtr res;
//	for (Int_t i=0; i<4; i++)
//		fitHistogram->Fit("fitFcn","RQ");
	res = fitHistogram->Fit("fitFcn","RMES");
//
	TMatrixDSym covMat = res->GetCovarianceMatrix();
	TF1** component = new TF1*[n];
	TF1** g = new TF1*[n];
	TF1** back = new TF1*[n];
	cout << "Fit results:" << endl;
	cout << "Background: Center "<< fitFcn->GetParameter(4) <<"; Const "<< fitFcn->GetParameter(1) <<"; Slope "<< fitFcn->GetParameter(2) <<"; Quadratic "<< fitFcn->GetParameter(3) << endl << endl;
//
	if(tail)
		for (Int_t i = 0; i < n; i++)
		{
			sprintf(fcnTitle,"comp_%d",i);
			component[i] = new TF1(fcnTitle,GaussTailedWithBcg,low,high,9);
			component[i]->SetParameter(0,fitFcn->GetParameter(9+2*i));
			component[i]->SetParameter(1,fitFcn->GetParameter(8+2*i));
			component[i]->SetParameter(2,fitFcn->GetParameter(7));
			component[i]->SetParameter(3,fitFcn->GetParameter(5));
			component[i]->SetParameter(4,fitFcn->GetParameter(6));
			component[i]->SetParameter(5,fitFcn->GetParameter(1));
			component[i]->SetParameter(6,fitFcn->GetParameter(2));
			component[i]->SetParameter(7,fitFcn->GetParameter(3));
			component[i]->SetParameter(8,fitFcn->GetParameter(4));
			component[i]->SetLineColor(8);
			component[i]->SetLineStyle(7);
			component[i]->Draw("SAME");
//
			sprintf(fcnTitle,"gauss_%d",i);
			g[i] = new TF1(fcnTitle,GaussWithTail,low,high,5);
			g[i]->SetParameters(fitFcn->GetParameter(9+2*i),fitFcn->GetParameter(8+2*i),fitFcn->GetParameter(7),fitFcn->GetParameter(5),fitFcn->GetParameter(6));
			g[i]->SetParError(0,fitFcn->GetParError(9+2*i));
			g[i]->SetParError(1,fitFcn->GetParError(8+2*i));
			g[i]->SetParError(2,fitFcn->GetParError(7));
			g[i]->SetParError(3,fitFcn->GetParError(5));
			g[i]->SetParError(4,fitFcn->GetParError(6));
//
			Double_t componentCov[25];
			componentCov[0] = covMat[9+2*i][9+2*i]; componentCov[1] = covMat[9+2*i][8+2*i]; componentCov[2] = covMat[9+2*i][7]; componentCov[3] = covMat[9+2*i][5]; componentCov[4] = covMat[9+2*i][6];
			componentCov[5] = covMat[8+2*i][9+2*i]; componentCov[6] = covMat[8+2*i][8+2*i]; componentCov[7] = covMat[8+2*i][7]; componentCov[8] = covMat[8+2*i][5]; componentCov[9] = covMat[8+2*i][6];
			componentCov[10] = covMat[7][9+2*i]; componentCov[11] = covMat[7][8+2*i]; componentCov[12] = covMat[7][7]; componentCov[13] = covMat[7][5]; componentCov[14] = covMat[7][6];
			componentCov[15] = covMat[5][9+2*i]; componentCov[16] = covMat[5][8+2*i]; componentCov[17] = covMat[5][7]; componentCov[18] = covMat[5][5]; componentCov[19] = covMat[5][6];
			componentCov[20] = covMat[6][9+2*i]; componentCov[21] = covMat[6][8+2*i]; componentCov[22] = covMat[6][7]; componentCov[23] = covMat[6][5]; componentCov[24] = covMat[6][6];
			TMatrixDSym compCovMat(5);
			compCovMat.SetMatrixArray(componentCov,"");
//		
			cout << "Peak " << i << ": " << endl;
			cout << "\t Energy = " << Form("%.2f",fitFcn->GetParameter(8+2*i)) << " +/- " << fitFcn->GetParError(8+2*i) << endl;
			if(i<3) arr[i] = fitFcn->GetParameter(8+2*i);
			cout << "\t FWHM = " << ksig*fitFcn->GetParameter(7) << " +/- " << ksig*fitFcn->GetParError(7) << endl;
			cout << "\t Volume= " << Form("%.2f",g[i]->Integral(low,high) / fitHistogram->GetBinWidth(1)) <<" +/- "<< g[i]->IntegralError(low,high,g[i]->GetParameters(),compCovMat.GetMatrixArray()) / fitHistogram->GetBinWidth(1) << endl;
			cout << "\t Tail 1 = " << fitFcn->GetParameter(5) << " +/- " << fitFcn->GetParError(5) << endl;
			cout << "\t Tail 2 = " << fitFcn->GetParameter(6) << " +/- " << fitFcn->GetParError(6) << endl<< endl;
		}
	else
		for (Int_t i = 0; i < n; i++) 
		{
			sprintf(fcnTitle,"comp_%d",i);
			component[i] = new TF1(fcnTitle,GaussWithBcg,low,high,7);
			component[i]->SetParameter(0,fitFcn->GetParameter(7+2*i));
			component[i]->SetParameter(1,fitFcn->GetParameter(6+2*i));
			component[i]->SetParameter(2,fitFcn->GetParameter(5));
			component[i]->SetParameter(3,fitFcn->GetParameter(1));
			component[i]->SetParameter(4,fitFcn->GetParameter(2));
			component[i]->SetParameter(5,fitFcn->GetParameter(3));
			component[i]->SetParameter(6,fitFcn->GetParameter(4));
			component[i]->SetLineColor(8);
			component[i]->SetLineStyle(7);
			component[i]->Draw("SAME");
//
			sprintf(fcnTitle,"gauss_%d",i);
			g[i] = new TF1(fcnTitle,Gauss,low,high,3);
			g[i]->SetParameters(fitFcn->GetParameter(7+2*i),fitFcn->GetParameter(6+2*i),fitFcn->GetParameter(5));
			g[i]->SetParError(0,fitFcn->GetParError(7+2*i));
			g[i]->SetParError(1,fitFcn->GetParError(6+2*i));
			g[i]->SetParError(2,fitFcn->GetParError(5));
//
			Double_t componentCov[9];
			componentCov[0] = covMat[7+2*i][7+2*i];
			componentCov[1] = covMat[7+2*i][6+2*i];
			componentCov[2] = covMat[7+2*i][5];
			componentCov[3] = covMat[6+2*i][7+2*i];
			componentCov[4] = covMat[6+2*i][6+2*i];
			componentCov[5] = covMat[6+2*i][5];
			componentCov[6] = covMat[5][7+2*i];
			componentCov[7] = covMat[5][6+2*i];
			componentCov[8] = covMat[5][5];
			TMatrixDSym compCovMat(3);
			compCovMat.SetMatrixArray(componentCov,"");
//		
			cout << "Peak " << i << ": " << endl;
			cout << "\t Energy = " << Form("%.2f",fitFcn->GetParameter(6+2*i)) << " +/- " << fitFcn->GetParError(6+2*i) << endl;
			if(i<3) arr[i] = fitFcn->GetParameter(6+2*i);
			cout << "\t FWHM = " << ksig*fitFcn->GetParameter(5) << " +/- " << ksig*fitFcn->GetParError(5) << endl;
			cout << "\t Volume= " << Form("%.2f",g[i]->Integral(low,high) / fitHistogram->GetBinWidth(1)) <<" +/- "<< g[i]->IntegralError(low,high,g[i]->GetParameters(),compCovMat.GetMatrixArray()) / fitHistogram->GetBinWidth(1) << endl << endl;
		}
	cout << "Chi square:		" << Form("%.2f",fitFcn->GetChisquare()) << endl;
	cout << "Reduced Chi square:	" << Form("%.2f",fitFcn->GetChisquare()/((Double_t)(fitFcn->GetNDF()))) << endl;
//
	sprintf(name,"dif_%d",count);
	TH1D* diff = new TH1D(name,"Difference",fitHistogram->GetXaxis()->GetNbins(), fitHistogram->GetXaxis()->GetXmin(), fitHistogram->GetXaxis()->GetXmax());
	sprintf(name,"errp_%d",count);
	TH1D* errp = new TH1D(name,"error 2 sigma+",fitHistogram->GetXaxis()->GetNbins(), fitHistogram->GetXaxis()->GetXmin(), fitHistogram->GetXaxis()->GetXmax());
	sprintf(name,"errm_%d",count);
	TH1D* errm = new TH1D(name,"error 2 sigma-",fitHistogram->GetXaxis()->GetNbins(), fitHistogram->GetXaxis()->GetXmin(), fitHistogram->GetXaxis()->GetXmax());
//
	for (Int_t i = fitHistogram->GetXaxis()->FindBin(low); i < fitHistogram->GetXaxis()->FindBin(high); i++)
	{
		diff->SetBinContent(i,fitHistogram->GetBinContent(i));
		errp->SetBinContent(i,2*sqrt(TMath::Abs(fitHistogram->GetBinContent(i))));
	}
//
	diff->Add(fitFcn,-1);
	diff->GetXaxis()->SetRange(fitHistogram->GetXaxis()->FindBin(low)+1,fitHistogram->GetXaxis()->FindBin(high)-1);;
	errm->Add(errp,-1);
	errm->SetLineColor(2);
	errp->SetLineColor(2);
//
	sprintf(name,"Pad2_%d",count);
	fitCanvas->cd();
	TPad *pad2 = new TPad(name, name, 0, 0, 1, r*(1-epsilon));
	pad2->SetTopMargin(0);
	pad2->SetFillColor(0);
	pad2->SetFillStyle(0);
	pad2->SetNumber(2);
	pad2->Draw();
	pad2->cd();
//
	diff->Draw();
	errp->Draw("same");
	errm->Draw("same");
	if(!tail)	
		for (Int_t i = 0; i < n; i++) 
			cout << Form("%.2f",fitFcn->GetParameter(6+2*i)) << endl;
	return 0;
}