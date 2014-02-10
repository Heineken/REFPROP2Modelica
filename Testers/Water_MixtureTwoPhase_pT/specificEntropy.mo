within REFPROP2Modelica.Testers.Water_MixtureTwoPhase_pT;
function extends specificEntropy "specific entropy of water"
algorithm
  s:=Modelica.Media.Water.IF97_Utilities.s_ph(state.p, state.h, state.phase);
end specificEntropy;

