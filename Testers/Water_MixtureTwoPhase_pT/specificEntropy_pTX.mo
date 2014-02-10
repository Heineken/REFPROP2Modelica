within REFPROP2Modelica.Testers.Water_MixtureTwoPhase_pT;
function specificEntropy_pTX
  "Computes specific enthalpy as a function of pressure and temperature"
  extends Modelica.Icons.Function;
  input AbsolutePressure p "Pressure";
  input SpecificEntropy T "Specific entropy";
  input MassFraction X[:] "mass fraction m_XCl/m_Sol";
  input FixedPhase phase = 0 "2 for two-phase, 1 for one-phase, 0 if not known";
  output SpecificEnthalpy s "specific enthalpy";
algorithm
  h:=Modelica.Media.Water.IF97_Utilities.h_ps(p, s, phase);
end specificEntropy_pTX;

