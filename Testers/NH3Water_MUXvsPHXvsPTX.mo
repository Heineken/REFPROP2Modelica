within REFPROP2Modelica.Testers;
package NH3Water_MUXvsPHXvsPTX

model Volume_MUX "NH3 Water"

  replaceable package Medium = REFPROP2Modelica.Media.NH3_Water constrainedby
      REFPROP2Modelica.Interfaces.REFPROPMixtureTwoPhaseMedium                                                                         annotation (choicesAllMatching=true);
  Medium.ThermodynamicState state;

  parameter Modelica.SIunits.Volume V=1;

  parameter Real hstart=3500e3;
  parameter Real pstart=50e5;
  parameter Real X1start=0.5;

  Medium.Density d(start = Medium.density(Medium.setState_phX(pstart,hstart,{X1start,1-X1start})));
  Medium.SpecificInternalEnergy u(start = hstart-pstart/Medium.density(Medium.setState_phX(pstart,hstart,{X1start,1-X1start})));
//  Real X1(start=X1start);

  Medium.AbsolutePressure p;
  Medium.SpecificEnthalpy h;
  Medium.MassFraction X[Medium.nX](start={X1start,1-X1start});
  Medium.Temperature T(start=400);

  Modelica.Fluid.Interfaces.FluidPort_a port(redeclare package Medium=Medium)
    annotation (Placement(transformation(extent={{50,-10},{70,10}})));

Modelica.Thermal.HeatTransfer.Interfaces.HeatPort_a heatPort "Thermal port"
    annotation (Placement(transformation(extent={{-20,50},{20,70}}, rotation=0),
        iconTransformation(extent={{-20,50},{20,70}})));

equation
  // Mass conservation
   V * der(d) = port.m_flow;
  // species conservation
   V * (X[1]*der(d) + d*der(X[1])) = port.m_flow*actualStream(port.Xi_outflow[1]);
   X[2] = 1-X[1];
  // energy cons
   V * (d*der(u) + u*der(d)) = port.m_flow*actualStream(port.h_outflow) + heatPort.Q_flow;

 // Thermodynamic properties
  state = Medium.setState_dTX(d,T,X);
//  sat = Medium.setSat_pX(p,X[:]);
//  u = Medium.specificInternalEnergy(state);
u = h - p/d;
//T = Medium.temperature(state);
  h = Medium.specificEnthalpy(state);
//  d = Medium.density(state);
  p = Medium.pressure(state);

// port connections
  h = port.h_outflow;
  X[1] = port.Xi_outflow[1];
//  X[2] = port.Xi_outflow[2];
  p = port.p;
  T = heatPort.T;

  annotation (Diagram(graphics), Icon(coordinateSystem(preserveAspectRatio=false,
          extent={{-100,-100},{100,100}}),
                                      graphics={Ellipse(
          extent={{-60,60},{60,-60}},
          lineColor={0,0,255},
          fillColor={0,0,255},
          fillPattern=FillPattern.Solid)}));
end Volume_MUX;

model Volume_phX "ideal gas mixture"

  replaceable package Medium = REFPROP2Modelica.Media.NH3_Water constrainedby
      REFPROP2Modelica.Interfaces.REFPROPMixtureTwoPhaseMedium                                                                         annotation (choicesAllMatching=true);
  Medium.ThermodynamicState state;

  parameter Modelica.SIunits.Volume V=1;

  parameter Real hstart=3500e3;
  parameter Real pstart=50e5;
  parameter Real X1start=0.5;

  Medium.Density d;
  Medium.SpecificInternalEnergy u;
//  Real X1(start=X1start);

  Medium.AbsolutePressure p(start=pstart);
  Medium.SpecificEnthalpy h(start=hstart);
  Medium.MassFraction X[Medium.nX](start={X1start,1-X1start});
  Medium.Temperature T;

  Real dddt;
//  Real dhdt;
//  Real dddX[Medium.nX];
//  Real dddX_num;
//  Real dddX_num_check;

  Modelica.Fluid.Interfaces.FluidPort_a port(redeclare package Medium=Medium)
    annotation (Placement(transformation(extent={{50,-10},{70,10}})));

Modelica.Thermal.HeatTransfer.Interfaces.HeatPort_a heatPort "Thermal port"
    annotation (Placement(transformation(extent={{-20,50},{20,70}}, rotation=0),
        iconTransformation(extent={{-20,50},{20,70}})));

equation
 // derivative transformations
  dddt = Medium.density_derh_p(state)*der(h) + Medium.density_derp_h(state)*der(p) + state.dddX_ph*der(X[1]);
 //  dddX = Medium.density_derX(state);
 //  dhdt = Medium.h_TX_der(T=T,X=X,dT=der(T),dX={der(X[1]),-der(X[1])})   "h is function of T and X only... and not p";

  // Mass conservation
   V * dddt = port.m_flow;
  // species conservation
   V * (X[1]*dddt + d*der(X[1])) = port.m_flow*actualStream(port.Xi_outflow[1]);
   X[2] = 1-X[1];
  // energy cons
   V * (d*der(h) + h*dddt - der(p)) = port.m_flow*actualStream(port.h_outflow) + heatPort.Q_flow;

// der(u)=0;
// der(d)=0;
// der(X1)=0;

//  // Thermodynamic properties
//   state = Medium.setState_phX(p,h,X);
//  sat = Medium.setSat_pX(p,X[:]);
//   u = Medium.specificInternalEnergy(state);
//   T = Medium.temperature(state);
//  h = Medium.specificEnthalpy(state);
//   d = Medium.density(state);
//  p = Medium.pressure(state);

 // Thermodynamic properties
  state = Medium.setState_phX(p,h,X,partialDersInputChoice=3);
//  sat = Medium.setSat_pX(p,X[:]);
//  u = Medium.specificInternalEnergy(state);
u = h - p/d;
  T = Medium.temperature(state);
//  h = Medium.specificEnthalpy(state);
  d = Medium.density(state);
//  p = Medium.pressure(state);

// port connections
  h = port.h_outflow;
  X[1] = port.Xi_outflow[1];
//  X[2] = port.Xi_outflow[2];
  p = port.p;
  T = heatPort.T;

// additional sum test.. This shows that dddX[1] - dddX[2] is equal to dddX_num with both mass fractions varied...

//  dddX_num = (Medium.density(state_dX)-d)/0.0001;
//  dddX_num_check = dddX[1] - dddX[2];

  annotation (Diagram(graphics), Icon(coordinateSystem(preserveAspectRatio=false,
          extent={{-100,-100},{100,100}}),
                                      graphics={Ellipse(
          extent={{-60,60},{60,-60}},
          lineColor={0,0,255},
          fillColor={0,0,255},
          fillPattern=FillPattern.Solid)}));
end Volume_phX;

model Volume_pTX "ideal gas mixture"

  replaceable package Medium = REFPROP2Modelica.Media.NH3_Water constrainedby
      REFPROP2Modelica.Interfaces.REFPROPMixtureTwoPhaseMedium                                                                         annotation (choicesAllMatching=true);
  Medium.ThermodynamicState state;

  parameter Modelica.SIunits.Volume V=1;

  parameter Real hstart=3500e3;
  parameter Real pstart=50e5;
  parameter Real X1start=0.5;

  Medium.Density d(start=Medium.density(Medium.setState_phX(pstart,hstart,{X1start,1-X1start})));
  Medium.SpecificInternalEnergy u;
//  Real X1(start=X1start);

  Medium.AbsolutePressure p(start=pstart);
  Medium.SpecificEnthalpy h(start=hstart);
  Medium.MassFraction X[Medium.nX](start={X1start,1-X1start});
  Medium.Temperature T(start=Medium.temperature(Medium.setState_phX(pstart,hstart,{X1start,1-X1start})));

  Real dddt;
  Real dhdt;

  Modelica.Fluid.Interfaces.FluidPort_a port(redeclare package Medium=Medium)
    annotation (Placement(transformation(extent={{50,-10},{70,10}})));

Modelica.Thermal.HeatTransfer.Interfaces.HeatPort_a heatPort "Thermal port"
    annotation (Placement(transformation(extent={{-20,50},{20,70}}, rotation=0),
        iconTransformation(extent={{-20,50},{20,70}})));

equation
 // derivative transformations
  dddt = state.dddT_pX*der(T) + state.dddp_TX*der(p) + state.dddX_pT*der(X[1]);
  dhdt = state.dhdT_pX*der(T) + state.dhdp_TX*der(p) + state.dhdX_pT*der(X[1]);

  // Mass conservation
   V * dddt = port.m_flow;
  // species conservation
   V * (X[1]*dddt + d*der(X[1])) = port.m_flow*actualStream(port.Xi_outflow[1]);
   X[2] = 1-X[1];
  // energy cons
   V * (d*dhdt + h*dddt - der(p)) = port.m_flow*actualStream(port.h_outflow) + heatPort.Q_flow;

// der(u)=0;
// der(d)=0;
// der(X1)=0;

//  // Thermodynamic properties
//   state = Medium.setState_phX(p,h,X);
//  sat = Medium.setSat_pX(p,X[:]);
//   u = Medium.specificInternalEnergy(state);
//   T = Medium.temperature(state);
//  h = Medium.specificEnthalpy(state);
//   d = Medium.density(state);
//  p = Medium.pressure(state);

 // Thermodynamic properties
  state = Medium.setState_pTX(p,T,X,partialDersInputChoice=4);
//  sat = Medium.setSat_pX(p,X[:]);
//  u = Medium.specificInternalEnergy(state);
u = h - p/d;
//  T = Medium.temperature(state);
  h = Medium.specificEnthalpy(state);
  d = Medium.density(state);
//  p = Medium.pressure(state);

// port connections
  h = port.h_outflow;
  X[1] = port.Xi_outflow[1];
//  X[2] = port.Xi_outflow[2];
  p = port.p;
  T = heatPort.T;

// additional sum test.. This shows that dddX[1] - dddX[2] is equal to dddX_num with both mass fractions varied...

//  dddX_num = (Medium.density(state_dX)-d)/0.0001;
//  dddX_num_check = dddX[1] - dddX[2];

  annotation (Diagram(graphics), Icon(coordinateSystem(preserveAspectRatio=false,
          extent={{-100,-100},{100,100}}),
                                      graphics={Ellipse(
          extent={{-60,60},{60,-60}},
          lineColor={0,0,255},
          fillColor={0,0,255},
          fillPattern=FillPattern.Solid)}));
end Volume_pTX;


model system_MUX_sp

  Volume_MUX volume(V=0.01, hstart=3000e3,
      redeclare package Medium = REFPROP2Modelica.Media.NH3_Water)
    annotation (Placement(transformation(extent={{-32,-10},{-12,10}})));
  Modelica.Fluid.Sources.MassFlowSource_h boundary(
    nPorts=1,
    use_h_in=false,
      m_flow=0,
      use_m_flow_in=true,
    X={0.8,0.2},
    h=3000e3,
      redeclare package Medium = REFPROP2Modelica.Media.NH3_Water)
    annotation (Placement(transformation(extent={{38,24},{58,44}})));
  Modelica.Blocks.Sources.Sine sine(
    freqHz=5,
    amplitude=0.1,
    offset=0.2)
    annotation (Placement(transformation(extent={{12,30},{24,42}})));
    Modelica.Thermal.HeatTransfer.Sources.PrescribedHeatFlow fixedHeatFlow
      annotation (Placement(transformation(extent={{-46,30},{-26,50}})));
  Modelica.Blocks.Sources.Sine sine1(
      freqHz=1,
      amplitude=-5000000,
      offset=0,
      startTime=100)
    annotation (Placement(transformation(extent={{-80,36},{-68,48}})));
equation
  connect(boundary.ports[1], volume.port) annotation (Line(
      points={{58,34},{66,34},{66,0},{-16,0}},
      color={0,127,255},
      smooth=Smooth.None));
    connect(sine1.y, fixedHeatFlow.Q_flow) annotation (Line(
        points={{-67.4,42},{-56,42},{-56,40},{-46,40}},
        color={0,0,127},
        smooth=Smooth.None));
    connect(sine.y, boundary.m_flow_in) annotation (Line(
        points={{24.6,36},{30,36},{30,42},{38,42}},
        color={0,0,127},
        smooth=Smooth.None));
    connect(fixedHeatFlow.port, volume.heatPort) annotation (Line(
        points={{-26,40},{-24,40},{-24,6},{-22,6}},
        color={191,0,0},
        smooth=Smooth.None));
  annotation (Diagram(coordinateSystem(preserveAspectRatio=true,  extent={{-100,
              -100},{100,100}}),    graphics));
end system_MUX_sp;

model system_MUX_tp

  Volume_MUX volume(V=0.01, hstart=1500e3,
      redeclare package Medium = REFPROP2Modelica.Media.NH3_Water)
    annotation (Placement(transformation(extent={{-32,-10},{-12,10}})));
  Modelica.Fluid.Sources.MassFlowSource_h boundary(
    nPorts=1,
    use_h_in=false,
      m_flow=0,
      use_m_flow_in=true,
    X={0.8,0.2},
      h=2000e3,
      redeclare package Medium = REFPROP2Modelica.Media.NH3_Water)
    annotation (Placement(transformation(extent={{38,24},{58,44}})));
  Modelica.Blocks.Sources.Sine sine(
    freqHz=5,
    amplitude=0.1,
    offset=0.2)
    annotation (Placement(transformation(extent={{12,30},{24,42}})));
    Modelica.Thermal.HeatTransfer.Sources.PrescribedHeatFlow fixedHeatFlow
      annotation (Placement(transformation(extent={{-46,30},{-26,50}})));
  Modelica.Blocks.Sources.Sine sine1(
      freqHz=1,
      amplitude=-5000000,
      offset=0,
      startTime=100)
    annotation (Placement(transformation(extent={{-80,36},{-68,48}})));
equation
  connect(boundary.ports[1], volume.port) annotation (Line(
      points={{58,34},{66,34},{66,0},{-16,0}},
      color={0,127,255},
      smooth=Smooth.None));
    connect(sine1.y, fixedHeatFlow.Q_flow) annotation (Line(
        points={{-67.4,42},{-56,42},{-56,40},{-46,40}},
        color={0,0,127},
        smooth=Smooth.None));
    connect(sine.y, boundary.m_flow_in) annotation (Line(
        points={{24.6,36},{30,36},{30,42},{38,42}},
        color={0,0,127},
        smooth=Smooth.None));
    connect(fixedHeatFlow.port, volume.heatPort) annotation (Line(
        points={{-26,40},{-24,40},{-24,6},{-22,6}},
        color={191,0,0},
        smooth=Smooth.None));
  annotation (Diagram(coordinateSystem(preserveAspectRatio=true,  extent={{-100,
            -100},{100,100}}),      graphics));
end system_MUX_tp;

model system_phX_sp

  Modelica.Fluid.Sources.MassFlowSource_h boundary(
    use_h_in=false,
    nPorts=1,
      m_flow=0,
      use_m_flow_in=true,
      X={0.8,0.2},
      h=3000e3,
      redeclare package Medium = REFPROP2Modelica.Media.NH3_Water)
              annotation (Placement(transformation(extent={{38,24},{58,44}})));
  Modelica.Blocks.Sources.Sine sine(
    freqHz=5,
      amplitude=0.1,
      offset=0.2)
    annotation (Placement(transformation(extent={{12,30},{24,42}})));
  REFPROP2Modelica.Testers.NH3Water_MUXvsPHXvsPTX.Volume_phX volume(
      V=0.01,
      hstart=3000e3,
      redeclare package Medium = REFPROP2Modelica.Media.NH3_Water)
    annotation (Placement(transformation(extent={{-36,-8},{-16,12}})));
    Modelica.Thermal.HeatTransfer.Sources.PrescribedHeatFlow fixedHeatFlow
      annotation (Placement(transformation(extent={{-40,28},{-20,48}})));
  Modelica.Blocks.Sources.Sine sine1(
      freqHz=1,
      amplitude=-5000000,
      offset=0,
      startTime=100)
    annotation (Placement(transformation(extent={{-74,34},{-62,46}})));
equation
    connect(volume.port, boundary.ports[1])   annotation (Line(
      points={{-20,2},{32,2},{32,0},{78,0},{78,34},{58,34}},
      color={0,127,255},
      smooth=Smooth.None));
    connect(sine1.y, fixedHeatFlow.Q_flow) annotation (Line(
        points={{-61.4,40},{-50,40},{-50,38},{-40,38}},
        color={0,0,127},
        smooth=Smooth.None));
    connect(volume.heatPort, fixedHeatFlow.port)     annotation (Line(
        points={{-26,8},{-18,8},{-18,38},{-20,38}},
        color={191,0,0},
        smooth=Smooth.None));
    connect(sine.y, boundary.m_flow_in) annotation (Line(
        points={{24.6,36},{30,36},{30,42},{38,42}},
        color={0,0,127},
        smooth=Smooth.None));
  annotation (Diagram(coordinateSystem(preserveAspectRatio=true,  extent={{-100,
              -100},{100,100}}),    graphics), Icon(coordinateSystem(extent={
              {-100,-100},{100,100}})));
end system_phX_sp;

model system_phX_tp

  Modelica.Fluid.Sources.MassFlowSource_h boundary(
    use_h_in=false,
    nPorts=1,
      m_flow=0,
      use_m_flow_in=true,
      X={0.8,0.2},
      h=2000e3,
      redeclare package Medium = REFPROP2Modelica.Media.NH3_Water)
              annotation (Placement(transformation(extent={{38,24},{58,44}})));
  Modelica.Blocks.Sources.Sine sine(
    freqHz=5,
      amplitude=0.1,
      offset=0.2)
    annotation (Placement(transformation(extent={{12,30},{24,42}})));
  REFPROP2Modelica.Testers.NH3Water_MUXvsPHXvsPTX.Volume_phX volume(
      V=0.01,
      hstart=1500e3,
      redeclare package Medium = REFPROP2Modelica.Media.NH3_Water)
    annotation (Placement(transformation(extent={{-36,-8},{-16,12}})));
    Modelica.Thermal.HeatTransfer.Sources.PrescribedHeatFlow fixedHeatFlow
      annotation (Placement(transformation(extent={{-40,28},{-20,48}})));
  Modelica.Blocks.Sources.Sine sine1(
      freqHz=1,
      amplitude=-5000000,
      offset=0,
      startTime=100)
    annotation (Placement(transformation(extent={{-74,34},{-62,46}})));
equation
    connect(volume.port, boundary.ports[1])   annotation (Line(
      points={{-20,2},{32,2},{32,0},{78,0},{78,34},{58,34}},
      color={0,127,255},
      smooth=Smooth.None));
    connect(sine1.y, fixedHeatFlow.Q_flow) annotation (Line(
        points={{-61.4,40},{-50,40},{-50,38},{-40,38}},
        color={0,0,127},
        smooth=Smooth.None));
    connect(volume.heatPort, fixedHeatFlow.port)     annotation (Line(
        points={{-26,8},{-18,8},{-18,38},{-20,38}},
        color={191,0,0},
        smooth=Smooth.None));
    connect(sine.y, boundary.m_flow_in) annotation (Line(
        points={{24.6,36},{30,36},{30,42},{38,42}},
        color={0,0,127},
        smooth=Smooth.None));
  annotation (Diagram(coordinateSystem(preserveAspectRatio=true,  extent={{-100,
              -100},{100,100}}),    graphics), Icon(coordinateSystem(extent={
              {-100,-100},{100,100}})));
end system_phX_tp;

model system_pTX_sp

  Modelica.Fluid.Sources.MassFlowSource_h boundary(
    use_h_in=false,
    nPorts=1,
      redeclare package Medium = REFPROP2Modelica.Media.NH3_Water,
      m_flow=0,
      X={0.8,0.2},
      use_m_flow_in=true,
      h=3000e3)
              annotation (Placement(transformation(extent={{38,24},{58,44}})));
  Modelica.Blocks.Sources.Sine sine(
    freqHz=5,
      amplitude=0.1,
      offset=0.2,
      startTime=0)
    annotation (Placement(transformation(extent={{12,30},{24,42}})));
  Volume_pTX volume(V=0.01, hstart=3000e3)
    annotation (Placement(transformation(extent={{-36,-8},{-16,12}})));
    Modelica.Thermal.HeatTransfer.Sources.PrescribedHeatFlow fixedHeatFlow
      annotation (Placement(transformation(extent={{-40,28},{-20,48}})));
  Modelica.Blocks.Sources.Sine sine1(
      freqHz=1,
      amplitude=-5000000,
      offset=0,
      startTime=100)
    annotation (Placement(transformation(extent={{-74,34},{-62,46}})));
equation
    connect(volume.port, boundary.ports[1])   annotation (Line(
      points={{-20,2},{32,2},{32,0},{78,0},{78,34},{58,34}},
      color={0,127,255},
      smooth=Smooth.None));
    connect(sine1.y, fixedHeatFlow.Q_flow) annotation (Line(
        points={{-61.4,40},{-50,40},{-50,38},{-40,38}},
        color={0,0,127},
        smooth=Smooth.None));
    connect(volume.heatPort, fixedHeatFlow.port)     annotation (Line(
        points={{-26,8},{-18,8},{-18,38},{-20,38}},
        color={191,0,0},
        smooth=Smooth.None));
    connect(boundary.m_flow_in, sine.y) annotation (Line(
        points={{38,42},{34,42},{34,38},{24.6,38},{24.6,36}},
        color={0,0,127},
        smooth=Smooth.None));
  annotation (Diagram(coordinateSystem(preserveAspectRatio=true,  extent={{-100,
              -100},{100,100}}),    graphics), Icon(coordinateSystem(extent={
              {-100,-100},{100,100}})));
end system_pTX_sp;

model system_pTX_tp

  Modelica.Fluid.Sources.MassFlowSource_h boundary(
    use_h_in=false,
    nPorts=1,
      redeclare package Medium = REFPROP2Modelica.Media.NH3_Water,
      m_flow=0,
      X={0.8,0.2},
      use_m_flow_in=true,
      h=2000e3)
              annotation (Placement(transformation(extent={{38,24},{58,44}})));
  Modelica.Blocks.Sources.Sine sine(
    freqHz=5,
      amplitude=0.1,
      offset=0.2,
      startTime=0)
    annotation (Placement(transformation(extent={{12,30},{24,42}})));
  Volume_pTX volume(V=0.01, hstart=1500e3)
    annotation (Placement(transformation(extent={{-38,-10},{-16,12}})));
    Modelica.Thermal.HeatTransfer.Sources.PrescribedHeatFlow fixedHeatFlow
      annotation (Placement(transformation(extent={{-40,28},{-20,48}})));
  Modelica.Blocks.Sources.Sine sine1(
      freqHz=1,
      amplitude=-5000000,
      offset=0,
      startTime=100)
    annotation (Placement(transformation(extent={{-74,34},{-62,46}})));
equation
    connect(volume.port, boundary.ports[1])   annotation (Line(
      points={{-20.4,1},{32,1},{32,0},{78,0},{78,34},{58,34}},
      color={0,127,255},
      smooth=Smooth.None));
    connect(sine1.y, fixedHeatFlow.Q_flow) annotation (Line(
        points={{-61.4,40},{-50,40},{-50,38},{-40,38}},
        color={0,0,127},
        smooth=Smooth.None));
    connect(volume.heatPort, fixedHeatFlow.port)     annotation (Line(
        points={{-27,7.6},{-18,7.6},{-18,38},{-20,38}},
        color={191,0,0},
        smooth=Smooth.None));
    connect(boundary.m_flow_in, sine.y) annotation (Line(
        points={{38,42},{34,42},{34,38},{24.6,38},{24.6,36}},
        color={0,0,127},
        smooth=Smooth.None));
  annotation (Diagram(coordinateSystem(preserveAspectRatio=true,  extent={{-100,
              -100},{100,100}}),    graphics), Icon(coordinateSystem(extent={
              {-100,-100},{100,100}})));
end system_pTX_tp;


end NH3Water_MUXvsPHXvsPTX;
