within REFPROP2Modelica.Testers.Water_MixtureTwoPhase_pT;
function extends saturationPressure
algorithm
  p:=Modelica.Media.Water.WaterIF97_base.saturationPressure(T);
end saturationPressure;

