within REFPROP2Modelica.Testers.Water_MixtureTwoPhase_pT;
function extends saturationTemperature
algorithm
  //T := Modelica.Media.Water.IF97_Utilities.BaseIF97.Basic.tsat(p);
  T:=Modelica.Media.Water.WaterIF97_base.saturationTemperature(p);
end saturationTemperature;

