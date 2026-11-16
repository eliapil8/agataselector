#include <TH1D.h>
#include <TFile.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>


void fix(){

std::ifstream agafile("./agata.txt");
std::ifstream ancfile("./anc.txt");
std::string line;


std::vector<signed long long> aga;
std::vector<signed long long> anc;


unsigned long long nraga{80000};
unsigned long long nrspi{300000};

unsigned long long cntraga{0};
unsigned long long cntrspi{0};


while(std::getline(agafile, line)){
	if(++cntraga> nraga) break;
	aga.emplace_back(std::stoull(line));	
}
while(std::getline(ancfile, line)){
	if(++cntrspi> nrspi) break;
	anc.emplace_back(std::stoull(line));	
}
std::cout << "Finishedasdf \n";

auto diff = aga.front()-anc.front();
std::cout << "initial stamp difference: " << Form("%llu",diff) << std::endl;

//diff = 0;
diff -= 5e8;
auto rangeL = diff-5e7;
auto rangeR = diff+5e7;
int nBins = 1e8;
std::cout << rangeL/1e8 << " s\t" << rangeR/1e8 << " s" << std::endl; 

TH1D* h = new TH1D("h", "h", nBins, rangeL, rangeR);

int cnt{0};
for(const auto& ag: aga){
	if((++cnt)%1000 == 0) std::cout<<"\r"<<cnt << "/" << aga.size() << std::flush;
        for(const auto& an: anc){
            if((ag-an)<rangeR && (ag-an)>rangeL) h->Fill(ag-an);
	}
}

TFile f("out.root", "recreate");
h->Write();
f.Write();
}

int main(){
fix();
}
