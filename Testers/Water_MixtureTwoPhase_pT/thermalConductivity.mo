within REFPROP2Modelica.Testers.Water_MixtureTwoPhase_pT;
function extends thermalConductivity "Thermal conductivity of water"
algorithm
  lambda:=Modelica.Media.Water.IF97_Utilities.thermalConductivity(state.d, state.T, state.p, state.phase);
end thermalConductivity;

