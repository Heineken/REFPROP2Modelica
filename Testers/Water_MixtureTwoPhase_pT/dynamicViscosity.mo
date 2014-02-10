within REFPROP2Modelica.Testers.Water_MixtureTwoPhase_pT;
function extends dynamicViscosity
algorithm
  eta:=Modelica.Media.Water.WaterIF97_base.dynamicViscosity(state);
end dynamicViscosity;

