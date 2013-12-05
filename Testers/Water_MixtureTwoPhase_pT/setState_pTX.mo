within REFPROP2Modelica.Testers.Water_MixtureTwoPhase_pT;
function extends setState_pTX
  "Return thermodynamic state of water as function of p and T"
algorithm
  state:=ThermodynamicState(d=  density_pT(p, T), T=  T, phase=  0, h=  specificEnthalpy_pTX(p, s), p=  p, X=  X, s=  specificEntropy_pT(p.T), q=  -1, GVF=  -1, d_l=  -1, d_g=  -1);
end setState_pTX;

