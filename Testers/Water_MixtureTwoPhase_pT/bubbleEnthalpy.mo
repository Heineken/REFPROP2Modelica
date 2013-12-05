within REFPROP2Modelica.Testers.Water_MixtureTwoPhase_pT;
function extends bubbleEnthalpy "boiling curve specific enthalpy of water"
algorithm
  hl:=Modelica.Media.Water.IF97_Utilities.BaseIF97.Regions.hl_p(sat.psat);
end bubbleEnthalpy;

