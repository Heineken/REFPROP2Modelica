within REFPROP2Modelica.Testers.Water_MixtureTwoPhase_pT;
function specificEnthalpy_pTX
  input Modelica.SIunits.Pressure p;
  input Modelica.SIunits.Temp_K T;
  input MassFraction X[:] "mass fraction m_NaCl/m_Sol";
  input FixedPhase phase = 0 "2 for two-phase, 1 for one-phase, 0 if not known";
  /*  input MassFraction q "(min=0,max=1)";
  input Real y "molar fraction of gas in gas phase";*/
  //  input Real[3] TP;
  output Modelica.SIunits.SpecificEnthalpy h = Modelica.Media.Water.WaterIF97_base.specificEnthalpy_pT(p, T);
algorithm
  //  Modelica.Utilities.Streams.print("specificEnthalpy_pTXqy("+String(p)+","+String(T)+",X,"+String(q)+","+String(y)+")");
  annotation(LateInline = true, inverse(T = temperature_phX(p=  p, h=  h, X=  X, phase=  phase)));
end specificEnthalpy_pTX;

