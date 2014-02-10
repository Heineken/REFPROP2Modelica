within REFPROP2Modelica.Testers.Water_MixtureTwoPhase_pT;
function extends setState_psX
  "Return thermodynamic state of water as function of p and s"
algorithm
  state:=ThermodynamicState(d=  density_ps(p, s), T=  temperature_ps(p, s), phase=  0, h=  specificEnthalpy_ps(p, s), p=  p, X=  X, s=  s, q=  -1, GVF=  -1, d_l=  -1, d_g=  -1);
end setState_psX;

