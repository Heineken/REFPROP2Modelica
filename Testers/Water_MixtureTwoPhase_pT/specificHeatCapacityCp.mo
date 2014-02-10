within REFPROP2Modelica.Testers.Water_MixtureTwoPhase_pT;
function extends specificHeatCapacityCp
  "specific heat capacity at constant pressure of water"
algorithm
  if Modelica.Media.Water.WaterIF97_base.dT_explicit then
    cp:=Modelica.Media.Water.IF97_Utilities.cp_dT(state.d, state.T, state.phase);
  elseif Modelica.Media.Water.WaterIF97_base.pT_explicit then
    cp:=Modelica.Media.Water.IF97_Utilities.cp_pT(state.p, state.T);
  else
    cp:=Modelica.Media.Water.IF97_Utilities.cp_ph(state.p, state.h, state.phase);
  end if;
  annotation(Documentation(info = "<html>
                                <p>In the two phase region this function returns the interpolated heat capacity between the
                                liquid and vapour state heat capacities.</p>
                                </html>"));
end specificHeatCapacityCp;

