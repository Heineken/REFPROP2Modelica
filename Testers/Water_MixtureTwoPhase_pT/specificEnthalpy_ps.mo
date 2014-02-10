within REFPROP2Modelica.Testers.Water_MixtureTwoPhase_pT;
function specificEnthalpy_ps
  "Computes specific enthalpy as a function of pressure and temperature"
  extends Modelica.Icons.Function;
  input AbsolutePressure p "Pressure";
  input SpecificEntropy s "Specific entropy";
  input FixedPhase phase = 0 "2 for two-phase, 1 for one-phase, 0 if not known";
  output SpecificEnthalpy h "specific enthalpy";
algorithm
  h:=Modelica.Media.Water.IF97_Utilities.h_ps(p, s, phase);
end specificEnthalpy_ps;

