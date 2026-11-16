root -l << EOF
.L ../changeNameGates.C
for(int z = 24; z < 31; z++){
for(int q = 20; q < 26; q++){
convertBanToTCutG(Form("/home/engy/AGATAAnalysisWorkshop/PrismaConf/ban/charge_ban_%i_%i.ban",z,q),Form("charge__0_%i__%i",z,q));
scaleCutG(Form("charge__0_%i__%i.root",z,q),1./29979.2458/0.699570,1.);
}
} 
EOF
