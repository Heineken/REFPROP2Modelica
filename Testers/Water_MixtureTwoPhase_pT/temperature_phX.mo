within REFPROP2Modelica.Testers.Water_MixtureTwoPhase_pT;
function temperature_phX "numerically inverts specificEnthalpy_liquid_pTX"
  input Modelica.SIunits.Pressure p;
  input Modelica.SIunits.SpecificEnthalpy h;
  input MassFraction X[:] "mass fraction m_XCl/m_Sol";
  input FixedPhase phase = 0 "2 for two-phase, 1 for one-phase, 0 if not known";
  output Modelica.SIunits.Temp_K T = Modelica.Media.Water.WaterIF97_base.temperature_ph(p, h);
algorithm
  //  Modelica.Utilities.Streams.print("temperature_phX");
  annotation(LateInline = true, inverse(h = specificEnthalpy_pTX(p=  p, T=  T, phase=  phase, X=  X)));
end temperature_phX;

