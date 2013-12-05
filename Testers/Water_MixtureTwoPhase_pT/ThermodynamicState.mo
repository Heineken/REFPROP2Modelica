within REFPROP2Modelica.Testers.Water_MixtureTwoPhase_pT;
record extends ThermodynamicState
  "a selection of variables that uniquely defines the thermodynamic state"
  /*  AbsolutePressure p "Absolute pressure of medium";
  Temperature T(unit="K") "Temperature of medium";
  MassFraction X[nX] "Mass fraction of NaCl in kg/kg";*/
  SpecificEnthalpy h "Specific enthalpy";
  SpecificEntropy s "Specific entropy";
  Density d(start = 300) "density";
  Real GVF "Gas Void Fraction";
  Density d_l(start = 300) "density liquid phase";
  Density d_g(start = 300) "density gas phase";
  Real q "vapor quality on a mass basis [mass vapor/total mass]";
  annotation(Documentation(info = "<html>

</html>"));
end ThermodynamicState;

