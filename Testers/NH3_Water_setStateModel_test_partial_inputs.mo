within REFPROP2Modelica.Testers;
model NH3_Water_setStateModel_test_partial_inputs

package Medium = REFPROP2Modelica.Media.NH3_Water;
//package Medium2 = REFPROP2Modelica.Media.NH3_Water;

//  Medium.PartialDersInputChoice partialDersInputChoice = Medium.PartialDersInputChoice.none;
//  Medium2.PartialDersInputChoice partialDersInputChoice2 = Medium.PartialDersInputChoice.pTX_numeric;

  Medium.ThermodynamicState state;
  Medium.ThermodynamicState state2;

//  input PartialDersInputChoice partialDersInputChoice=PartialDersInputChoice.none;

  Medium.AbsolutePressure p;
  Medium.SpecificEnthalpy h;
  Medium.MassFraction X[2];

  parameter Medium.PartialDersInputChoice partialDersInputChoice0 =   Medium.PartialDersInputChoice.none;
  parameter Medium.PartialDersInputChoice partialDersInputChoice1 =   Medium.PartialDersInputChoice.phX_numeric;
  parameter Medium.PartialDersInputChoice partialDersInputChoice2 =   Medium.PartialDersInputChoice.phX_pseudoanalytic;
  parameter Medium.PartialDersInputChoice partialDersInputChoice3 =   Medium.PartialDersInputChoice.pTX_numeric;

equation
  p=50e5;
  h=3e5+time*100e3;
  X={0.5,0.5};

  state = Medium.setState_phX(p,h,X);
  state2 = Medium.setState_phX(p,h,X,partialDersInputChoice=3);

end NH3_Water_setStateModel_test_partial_inputs;
