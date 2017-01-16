#include "cutop.cc"

double PunziFunction(double eff_sig, double bkg_tot, double bkg_fake);
void printcurrunttime();
void fillarray(vector<double>& array, double start, double end, double d);
void GetCutVar(int mass, TString var, double& cutvar_min, double& cutvar_max);

void run_cutop(int sig_mass, bool inclusive=false){

  TH1::SetDefaultSumw2(true);

  int SignalClass;
  if(sig_mass <= 50) SignalClass = 1;
  else if(sig_mass <= 80) SignalClass = 2;
  else if(sig_mass <= 1000) SignalClass = 3;
  else SignalClass = 4;

  TString WORKING_DIR = getenv("PLOTTER_WORKING_DIR");
  TString catversion = getenv("CATVERSION");
  TString dataset = getenv("CATANVERSION");
  
  TString filepath = WORKING_DIR+"/rootfiles/"+dataset+"/UpDownSyst/";
  vector<TString> bkg_prompt_list = {
    //"VV",
    "WZTo3LNu_powheg",
    "ZZTo4L_powheg",
    "Vgamma",
    "top",
    "VVV"
  };
  
  vector<double> cuts_first_pt, cuts_second_pt, cuts_third_pt, cuts_W_pri_mass, cuts_PFMET;
  
  if(SignalClass==1||SignalClass==2){
    fillarray( cuts_first_pt, 25, 100, 5 );
    fillarray( cuts_second_pt, 15, 100, 5 );
    fillarray( cuts_third_pt, 15, 100, 5 );
    fillarray( cuts_W_pri_mass, 85, 300, 5);
    cuts_PFMET.push_back(0.);
  }
  else if(SignalClass==3){

    double min_tmp, max_tmp, dx_tmp;

    GetCutVar(sig_mass, "first_pt", min_tmp, max_tmp);
    if(sig_mass<300) dx_tmp = 5;
    else dx_tmp = 10;
    fillarray( cuts_first_pt, min_tmp, max_tmp, dx_tmp );

    GetCutVar(sig_mass, "second_pt", min_tmp, max_tmp);
    fillarray( cuts_second_pt, min_tmp, max_tmp, dx_tmp );

    fillarray( cuts_third_pt, 15, 80, 5 );
    
    double a, b;
    GetCutVar(sig_mass, "W_pri", a, b);
    fillarray( cuts_W_pri_mass, a, b, 10);

    if(sig_mass<500){
      max_tmp = 100;
      dx_tmp = 5;
    }
    else{
      max_tmp = 150;
      dx_tmp = 10;
    }
    fillarray( cuts_PFMET, 20, max_tmp, dx_tmp);
    
  }
  else if(SignalClass==4){

  }
  else{
    cout << "Signal Class Wrong" << endl;
    return;
  }

  if(inclusive){

    cuts_first_pt.clear(); 
    cuts_second_pt.clear();
    cuts_third_pt.clear();
    cuts_W_pri_mass.clear();
    cuts_PFMET.clear();

    //==== preselection
    if(sig_mass < 0){
      sig_mass = 5;
      cout << "[Preselection]" << endl;
      cuts_first_pt = {99999999.};
      cuts_second_pt = {99999999.};
      cuts_third_pt = {99999999.};
      cuts_W_pri_mass = {99999999.};
      cuts_PFMET = {99999999.};
    }
    //==== low mass
    else if(sig_mass < 80){
      cout << "[Low Mass]" << endl;
      cuts_first_pt = {99999999.}; 
      cuts_second_pt = {99999999.};
      cuts_third_pt = {99999999.};
      cuts_W_pri_mass = {150};
      cuts_PFMET = {0.};
    }
    //==== high mass
    else{
      cout << "[High Mass]" << endl;
      cuts_first_pt = {20.};
      cuts_second_pt = {10.};
      cuts_third_pt = {10.};
      cuts_W_pri_mass = {0.};
      cuts_PFMET = {200.};
    }

  }

  //===============
  //==== FOR TEST
  //===============
  //cuts_first_pt.clear();
  //cuts_second_pt.clear();
  //cuts_third_pt.clear();
  //cuts_W_pri_mass.clear();
  //cuts_PFMET.clear();
  //cuts_first_pt = {40.};
  //cuts_second_pt = {35.};
  //cuts_third_pt = {15.};
  //cuts_W_pri_mass = {80};
  //fillarray( cuts_PFMET, 0, 100, 5);

  Long64_t TOTAL_it = cuts_first_pt.size()*cuts_second_pt.size()*cuts_third_pt.size()*cuts_W_pri_mass.size()*cuts_PFMET.size();
  cout << "#### Cut Variables ####" << endl;

  cout << "first_pt : ";
  for(unsigned int i=0; i<cuts_first_pt.size(); i++) cout << cuts_first_pt.at(i) << ' ';
  cout << endl << "=> # of variables = " << cuts_first_pt.size() << endl;

  cout << "second_pt : ";
  for(unsigned int i=0; i<cuts_second_pt.size(); i++) cout << cuts_second_pt.at(i) << ' ';
  cout << endl << "=> # of variables = " << cuts_second_pt.size() << endl;

  cout << "third_pt : ";
  for(unsigned int i=0; i<cuts_third_pt.size(); i++) cout << cuts_third_pt.at(i) << ' ';
  cout << endl << "=> # of variables = " << cuts_third_pt.size() << endl;

  cout << "W_pri_mass : ";
  for(unsigned int i=0; i<cuts_W_pri_mass.size(); i++) cout << cuts_W_pri_mass.at(i) << ' ';
  cout << endl << "=> # of variables = " << cuts_W_pri_mass.size() << endl;

  cout << "PFMET : ";
  for(unsigned int i=0; i<cuts_PFMET.size(); i++) cout << cuts_PFMET.at(i) << ' ';
  cout << endl << "=> # of variables = " << cuts_PFMET.size() << endl;

  Long64_t LogEvery = 1000;
  
  cout
  << "##################################################" << endl
  << "TOTAL # of Loop = " << TOTAL_it << endl
  << "##################################################" << endl;

  double cut_first_pt_SEL=0., cut_second_pt_SEL=0., cut_third_pt_SEL=0., cut_W_pri_mass_SEL=0., cut_PFMET_SEL=0.;
  double n_bkg_prompt_SEL=0, n_bkg_fake_SEL=0, n_signal_SEL=0, n_data_SEL=0;
  double err_bkg_prompt_SEL=0, err_sig_SEL=0, err_data_SEL=0;
  double eff_sig_SEL=0;
  double max_punzi=0;
  
  Long64_t this_it = 0;
  
  for(unsigned int i_first_pt=0; i_first_pt<cuts_first_pt.size(); i_first_pt++){
    for(unsigned int i_second_pt=0; i_second_pt<cuts_second_pt.size(); i_second_pt++){
      for(unsigned int i_third_pt=0; i_third_pt<cuts_third_pt.size(); i_third_pt++){
        for(unsigned int i_W_pri_mass=0; i_W_pri_mass<cuts_W_pri_mass.size(); i_W_pri_mass++){
          for(unsigned int i_PFMET=0; i_PFMET<cuts_PFMET.size(); i_PFMET++){

            this_it++;
            if(this_it%LogEvery==0){
              cout << "["; printcurrunttime(); cout <<"] ";
              cout
              << this_it<<"/"<<TOTAL_it << " ( "<<100.*this_it/TOTAL_it<<" % ) : Current Max Punzi = " << max_punzi << endl;
              if(SignalClass==1||SignalClass==2){
                cout
                << endl
                << "====================================================================" << endl
                << "(first pt) < " << cut_first_pt_SEL << " GeV" << endl
                << "(second pt) < " << cut_second_pt_SEL << " GeV" << endl
                << "(third pt) < " << cut_third_pt_SEL << " GeV" << endl
                << "W_pri_mass < " << cut_W_pri_mass_SEL << " GeV" << endl;
              }
              else{
                cout
                << endl
                << "====================================================================" << endl
                << "(first pt) > " << cut_first_pt_SEL << " GeV" << endl
                << "(second pt) > " << cut_second_pt_SEL << " GeV" << endl
                << "(third pt) > " << cut_third_pt_SEL << " GeV" << endl
                << "W_pri_mass > " << cut_W_pri_mass_SEL << " GeV" << endl
                << "PFMET > " << cut_PFMET_SEL << " GeV" << endl;
              }
              cout
              //<< "==> Data = " << n_data_SEL << endl
              << "==> Prompt bkg = " << n_bkg_prompt_SEL << endl
              << "==> Fake bkg = " << n_bkg_fake_SEL << endl
              << "==> Total bkg = " << n_bkg_prompt_SEL+n_bkg_fake_SEL << endl
              << "==> n_signal = " << n_signal_SEL << ", eff_sig = " << eff_sig_SEL << endl
              << "==> Max Punzi = " << max_punzi << endl;
            }
            
            double n_bkg_prompt(0.), n_bkg_fake(0.), n_signal(0.), n_data(0.);

            TH1D *hist_bkg_for_err = new TH1D("hist_bkg_for_err", "", 1, 0., 1.); 
            for(unsigned int k=0; k<bkg_prompt_list.size(); k++){
              TString this_samplename = bkg_prompt_list.at(k);
              cutop m_bkg_prompt(filepath+"trilepton_mumumu_ntp_SK"+this_samplename+"_dilep_cat_"+catversion+".root", "Ntp_Central");
              m_bkg_prompt.cut_first_pt = cuts_first_pt.at(i_first_pt);
              m_bkg_prompt.cut_second_pt = cuts_second_pt.at(i_second_pt);
              m_bkg_prompt.cut_third_pt = cuts_third_pt.at(i_third_pt);
              m_bkg_prompt.cut_W_pri_mass = cuts_W_pri_mass.at(i_W_pri_mass);
              m_bkg_prompt.cut_PFMET = cuts_PFMET.at(i_PFMET);
              m_bkg_prompt.signalclass = SignalClass;
              m_bkg_prompt.Loop();
              n_bkg_prompt += m_bkg_prompt.n_weighted;

              if(TOTAL_it==1){
                cout << this_samplename << " : " << m_bkg_prompt.n_weighted << ", error = " << m_bkg_prompt.hist_for_error->GetBinError(1) << endl;
              }

              hist_bkg_for_err->Add(m_bkg_prompt.hist_for_error);
            }

            cutop m_sig(filepath+"trilepton_mumumu_ntp_SKHN_MuMuMu_"+TString::Itoa(sig_mass, 10)+"_cat_"+catversion+".root", "Ntp_Central");
            m_sig.cut_first_pt = cuts_first_pt.at(i_first_pt);
            m_sig.cut_second_pt = cuts_second_pt.at(i_second_pt);
            m_sig.cut_third_pt = cuts_third_pt.at(i_third_pt);
            m_sig.cut_W_pri_mass = cuts_W_pri_mass.at(i_W_pri_mass);
            m_sig.cut_PFMET = cuts_PFMET.at(i_PFMET);
            m_sig.signalclass = SignalClass;
            m_sig.Loop();
            double n_generated = 100000.;
            n_signal = m_sig.n_unweighted;

            cutop m_bkg_fake(filepath+"trilepton_mumumu_ntp_SKfake_sfed_HighdXY_dilep_cat_"+catversion+".root", "Ntp_Central");
            m_bkg_fake.cut_first_pt = cuts_first_pt.at(i_first_pt);
            m_bkg_fake.cut_second_pt = cuts_second_pt.at(i_second_pt);
            m_bkg_fake.cut_third_pt = cuts_third_pt.at(i_third_pt);
            m_bkg_fake.cut_W_pri_mass = cuts_W_pri_mass.at(i_W_pri_mass);
            m_bkg_fake.cut_PFMET = cuts_PFMET.at(i_PFMET);
            m_bkg_fake.signalclass = SignalClass;
            m_bkg_fake.Loop();
            n_bkg_fake = m_bkg_fake.n_weighted;

            cutop m_data(filepath+"trilepton_mumumu_ntp_data_DoubleMuon_cat_"+catversion+".root", "Ntp_Central");
            m_data.cut_first_pt = cuts_first_pt.at(i_first_pt);
            m_data.cut_second_pt = cuts_second_pt.at(i_second_pt);
            m_data.cut_third_pt = cuts_third_pt.at(i_third_pt);
            m_data.cut_W_pri_mass = cuts_W_pri_mass.at(i_W_pri_mass);
            m_data.cut_PFMET = cuts_PFMET.at(i_PFMET);
            m_data.signalclass = SignalClass;
            m_data.Loop();
            n_data = m_data.n_weighted;
            
            
            double this_punzi = PunziFunction(n_signal/n_generated, n_bkg_prompt+n_bkg_fake, n_bkg_fake);
            
            if( this_punzi > max_punzi ){
              cut_first_pt_SEL = cuts_first_pt.at(i_first_pt);
              cut_second_pt_SEL = cuts_second_pt.at(i_second_pt);
              cut_third_pt_SEL = cuts_third_pt.at(i_third_pt);
              cut_W_pri_mass_SEL = cuts_W_pri_mass.at(i_W_pri_mass);
              cut_PFMET_SEL = cuts_PFMET.at(i_PFMET);
              
              n_bkg_prompt_SEL = n_bkg_prompt;
              n_bkg_fake_SEL = n_bkg_fake;
              n_signal_SEL = n_signal;
              eff_sig_SEL = n_signal/n_generated;
              n_data_SEL = n_data;

              err_bkg_prompt_SEL = hist_bkg_for_err->GetBinError(1);
              err_sig_SEL = m_bkg_fake.hist_for_error->GetBinError(1);
              err_data_SEL = m_data.hist_for_error->GetBinError(1);

              max_punzi = this_punzi;
            }
            
          }
        }
      }
    }
  }

  cout << "##################" << endl;
  cout << "#### Finished ####" << endl;
  cout << "##################" << endl << endl;

  if(SignalClass==1||SignalClass==2){
    cout
    << endl
    << "====================================================================" << endl
    << "(first pt) < " << cut_first_pt_SEL << " GeV" << endl
    << "(second pt) < " << cut_second_pt_SEL << " GeV" << endl
    << "(third pt) < " << cut_third_pt_SEL << " GeV" << endl
    << "W_pri_mass < " << cut_W_pri_mass_SEL << " GeV" << endl;
  }
  else{
    cout
    << endl
    << "====================================================================" << endl
    << "(first pt) > " << cut_first_pt_SEL << " GeV" << endl
    << "(second pt) > " << cut_second_pt_SEL << " GeV" << endl
    << "(third pt) > " << cut_third_pt_SEL << " GeV" << endl
    << "W_pri_mass > " << cut_W_pri_mass_SEL << " GeV" << endl
    << "PFMET > " << cut_PFMET_SEL << " GeV" << endl;
  }
  cout
  //<< "==> Data = " << n_data_SEL << endl
  << "==> Prompt bkg = " << n_bkg_prompt_SEL << ", error = " << err_bkg_prompt_SEL << endl
  << "==> Fake bkg = " << n_bkg_fake_SEL << ", error : look at FR_syst plot" << endl
  << "==> Total bkg = " << n_bkg_prompt_SEL+n_bkg_fake_SEL << endl
  << "==> n_signal = " << n_signal_SEL << ", eff_sig = " << eff_sig_SEL << endl
  << "==> Max Punzi = " << max_punzi << endl;
  
}


double PunziFunction(double eff_sig, double bkg_tot, double bkg_fake){
  
  double den = 1 + sqrt( bkg_tot + (0.30 * bkg_fake)*(0.30 * bkg_fake) );
  //double den = sqrt( bkg_tot );
  
  return eff_sig/den;
  
}


void printcurrunttime(){
  
  TDatime datime;
  cout << datime.GetYear()<<"/"<<datime.GetMonth()<<"/"<<datime.GetDay()<<" "<<datime.GetHour()<<":"<<datime.GetMinute()<<":"<<datime.GetSecond();
  
}

void fillarray(vector<double>& array, double start, double end, double d){
  
  int n_total = (end-start)/d+1;
  
  for(int i=0; i<n_total; i++){
    array.push_back( start + d * i );
  }
  
}

void GetCutVar(int mass, TString var, double& cutvar_min, double& cutvar_max){
  
  if(mass == 90){
    if(var == "W_pri"){
      cutvar_min = 80;
      cutvar_max = 150;
    }
    else if(var == "first_pt"){
      cutvar_min = 25;
      cutvar_max = 50;
    }
    else if(var == "second_pt"){
      cutvar_min = 15;
      cutvar_max = 60;
    }
  }
  else if(mass == 100){
    if(var == "W_pri"){
      cutvar_min = 80;
      cutvar_max = 200;
    }
    else if(var == "first_pt"){
      cutvar_min = 25;
      cutvar_max = 60;
    }
    else if(var == "second_pt"){
      cutvar_min = 15;
      cutvar_max = 60;
    }
  }
  else if(mass == 150){
    if(var == "W_pri"){
      cutvar_min = 100;
      cutvar_max = 300;
    }
    else if(var == "first_pt"){
      cutvar_min = 25;
      cutvar_max = 70;
    }
    else if(var == "second_pt"){
      cutvar_min = 15;
      cutvar_max = 60;
    }
  }
  else if(mass == 200){
    if(var == "W_pri"){
      cutvar_min = 200;
      cutvar_max = 350;
    }
    else if(var == "first_pt"){
      cutvar_min = 25;
      cutvar_max = 100;
    }
    else if(var == "second_pt"){
      cutvar_min = 15;
      cutvar_max = 70;
    }
  }
  else if(mass == 300){
    if(var == "W_pri"){
      cutvar_min = 250;
      cutvar_max = 400;
    }
    else if(var == "first_pt"){
      cutvar_min = 50;
      cutvar_max = 150;
    }
    else if(var == "second_pt"){
      cutvar_min = 15;
      cutvar_max = 100;
    }
  }
  else if(mass == 400){
    if(var == "W_pri"){
      cutvar_min = 400;
      cutvar_max = 600;
    }
    else if(var == "first_pt"){
      cutvar_min = 100;
      cutvar_max = 200;
    }
    else if(var == "second_pt"){
      cutvar_min = 15;
      cutvar_max = 140;
    }
  }
  else if(mass == 500){
    if(var == "W_pri"){
      cutvar_min = 500;
      cutvar_max = 700;
    }
    else if(var == "first_pt"){
      cutvar_min = 100;
      cutvar_max = 250;
    }
    else if(var == "second_pt"){
      cutvar_min = 100;
      cutvar_max = 180;
    }
  }
  else if(mass == 700){
    if(var == "W_pri"){
      cutvar_min = 700;
      cutvar_max = 800;
    }
    else if(var == "first_pt"){
      cutvar_min = 150;
      cutvar_max = 250;
    }
    else if(var == "second_pt"){
      cutvar_min = 100;
      cutvar_max = 250;
    }
  }
  else if(mass == 1000){
    if(var == "W_pri"){
      cutvar_min = 800;
      cutvar_max = 1500;
    }
    else if(var == "first_pt"){
      cutvar_min = 150;
      cutvar_max = 300;
    }
    else if(var == "second_pt"){
      cutvar_min = 100;
      cutvar_max = 250;
    }
  }
  else{

  }
  
  
}




