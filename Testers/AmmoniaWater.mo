within REFPROP2Modelica.Testers;
package AmmoniaWater

  model NH3_Water_setSatModel

  package Medium = REFPROP2Modelica.Media.NH3_Water;

     Medium.ThermodynamicState dewstate;
     Medium.ThermodynamicState bubstate;

    Medium.SaturationProperties satL;
    Medium.SaturationProperties satL2;
    Medium.SaturationProperties satV;

    Medium.AbsolutePressure p;
    Medium.MassFraction X[2];

  Real A[3];
  Real B[2];

  Real sigma;
  equation
    (A[1],A[2],A[3],B) =  Medium.criticalProperties(X);

    p=100e5;
    X={0.5,0.5};

    satL = Medium.setSat_pX(p,X,kph=1,calcTransport=false);
    satL2 = Medium.setSat_TX(satL.Tsat,X,kph=1,calcTransport=false);
    bubstate = Medium.setBubbleState(satL);

    satV = Medium.setSat_pX(p,X,kph=2,calcTransport=false);
    dewstate = Medium.setDewState(satV);

    sigma = Medium.surfaceTension(satL);

  end NH3_Water_setSatModel;

  model NH3_Water_setStateModel

  package Medium = REFPROP2Modelica.Media.NH3_Water;

    Medium.ThermodynamicState state;
  //  Medium.SaturationProperties sat;

   Medium.AbsolutePressure p;
   Medium.SpecificEnthalpy h;
   Medium.MassFraction X[2];
   Medium.Density d;

  Medium.MassFraction Xdef[Medium.nX]=Medium.X_default;

  Real q;

  Real dddh_pX;
  Real dddp_hX;
  Real dddh_pX_num;
  Real dddp_hX_num;

  //Real dddX_ph_num;

  equation
    p=100e5;
    h=3e5 + time*25e5;
    X={0.5,0.5};

    state = Medium.setState_phX(p,h,X,calcTransport=false,partialDersInputChoice=3);
    q = Medium.vapourQuality(state);
    d = Medium.density(state);

    dddh_pX = Medium.density_derh_p(state);
    dddh_pX_num = Medium.density(Medium.setState_phX(p,h+1,X))-d;

    dddp_hX = Medium.density_derp_h(state);
    dddp_hX_num = Medium.density(Medium.setState_phX(p+1,h,X))-d;

  //  dddX_ph_num = (Medium.density(Medium.setState_phX(p,h,cat(1,{X[1]+0.0001},{X[2]-0.0001})))-d)/0.0001;

  end NH3_Water_setStateModel;

  model NH3_Water_setState_plus_setSat

  package Medium = REFPROP2Modelica.Media.NH3_Water;

    Medium.ThermodynamicState state;
    Medium.SaturationProperties sat;

   Medium.AbsolutePressure p;
   Medium.SpecificEnthalpy h;
   Medium.MassFraction X[2];

  Medium.MassFraction Xdef[Medium.nX]=Medium.X_default;

  //Real dddX_ph_num;

     Medium.ThermodynamicState dewstate;
     Medium.ThermodynamicState bubstate;

  equation
    p=150e5;
    h=3e5 + time*25e5;
    X={0.5,0.5};

    state = Medium.setState_phX(p,h,X,calcTransport=false,partialDersInputChoice=3);

    sat = state.sat;

    bubstate = Medium.setBubbleState(sat);
    dewstate = Medium.setDewState(sat);

  //  sat = if state.q<=0 then Medium.setSat_pX(p,X,kph=1) elseif state.q>=1 then Medium.setSat_pX(p,X,kph=2) else state.sat;

  end NH3_Water_setState_plus_setSat;

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
    Volume_phX                                                 volume(
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
    Volume_phX                                                 volume(
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

package TransportPropsTest
  model MassFractionSweep_visc_vap

      parameter Modelica.SIunits.Pressure p=100e5;
      parameter Modelica.SIunits.Temperature T=325 + 273.15;
      Modelica.SIunits.MassFraction X[2];

    package MediumWilke = REFPROP2Modelica.Media.NH3_Water (
           redeclare function DynamicViscosityVapor =
            REFPROP2Modelica.Media.NH3_Water.dynamicViscosity_VAPWilke);
      MediumWilke.ThermodynamicState state=MediumWilke.setState_pTX(
            p,
            T,
            X,
            calcTransport=false);

    package MediumReich = REFPROP2Modelica.Media.NH3_Water (
       redeclare function DynamicViscosityVapor =
            REFPROP2Modelica.Media.NH3_Water.dynamicViscosity_VAPReichenberg);

    package MediumChung = REFPROP2Modelica.Media.NH3_Water (
           redeclare function DynamicViscosityVapor =
            REFPROP2Modelica.Media.NH3_Water.dynamicViscosity_VAPChung);

    package MediumChungErrorWeight = REFPROP2Modelica.Media.NH3_Water (
               redeclare function DynamicViscosityVapor =
            REFPROP2Modelica.Media.NH3_Water.dynamicViscosity_VAPChungPureErrorWeight);

      //Real eta_reich,eta_chung,eta_chungErrorWeight;
      Real eta_wilke;
      Real eta_reich;
      Real eta_chung;
      Real eta_chungErrorWeight;

  equation
      X = {time,1 - time};

      eta_wilke = MediumWilke.dynamicViscosity(state);
      eta_reich = MediumReich.dynamicViscosity(state);
      eta_chung = MediumChung.dynamicViscosity(state);
      eta_chungErrorWeight = MediumChungErrorWeight.dynamicViscosity(state);

      annotation (experiment(Interval=0.01));
  end MassFractionSweep_visc_vap;

  model EnthalpySweep_visc_vap
    package Medium = REFPROP2Modelica.Media.NH3_Water;
    parameter Medium.AbsolutePressure p=150e5;
  //  parameter Medium.Temperature Tstart=50+273.15;
    parameter Medium.Temperature Tend=500+273.15;
    parameter Medium.MassFraction X[2]={0.5,0.5};
    parameter Real hstart =  Medium.specificEnthalpy(Medium.setState_pqX(p,1,X));
    parameter Real hend = Medium.specificEnthalpy(Medium.setState_pTX(p,Tend,X));
    Medium.ThermodynamicState state = Medium.setState_phX(p,h,X);

    Real h;
    Real eta_chung,eta_chungWeight,eta_wilke,eta_reichenberg;

    package MediumWilke = REFPROP2Modelica.Media.NH3_Water (
           redeclare function DynamicViscosityVapor =
            REFPROP2Modelica.Media.NH3_Water.dynamicViscosity_VAPWilke);

    package MediumReich = REFPROP2Modelica.Media.NH3_Water (
       redeclare function DynamicViscosityVapor =
            REFPROP2Modelica.Media.NH3_Water.dynamicViscosity_VAPReichenberg);

    package MediumChung = REFPROP2Modelica.Media.NH3_Water (
           redeclare function DynamicViscosityVapor =
            REFPROP2Modelica.Media.NH3_Water.dynamicViscosity_VAPChung);

    package MediumChungErrorWeight = REFPROP2Modelica.Media.NH3_Water (
               redeclare function DynamicViscosityVapor =
            REFPROP2Modelica.Media.NH3_Water.dynamicViscosity_VAPChungPureErrorWeight);
  equation
    h=hstart + (hend-hstart)*time;

    eta_chung = Medium.dynamicViscosity_VAPChung(state);
    eta_chungWeight = Medium.dynamicViscosity_VAPChungPureErrorWeight(state);
    eta_wilke = Medium.dynamicViscosity_VAPWilke(state);
    eta_reichenberg = Medium.dynamicViscosity_VAPReichenberg(state);

      annotation (experiment(Interval=0.01));
  end EnthalpySweep_visc_vap;

  model MassFractionSweep_visc_liq

    parameter Modelica.SIunits.Pressure p=50e5;
    parameter Modelica.SIunits.Temperature T=50 + 273.15;
    Modelica.SIunits.MassFraction X[2];

    package Medium = REFPROP2Modelica.Media.NH3_Water;
    Medium.ThermodynamicState state=Medium.setState_pTX(
        p,
        T,
        X,
        calcTransport=false);

    Real eta_conde,eta_elsayed,eta_HDK,eta_TejaRice,eta_TejaRiceStecco,eta_TejaRiceSassensTcrit;

    package MediumConde = REFPROP2Modelica.Media.NH3_Water (
      redeclare function DynamicViscosityLiquid =
            REFPROP2Modelica.Media.NH3_Water.dynamicViscosity_LIQconde);

    package MediumElSayed = REFPROP2Modelica.Media.NH3_Water (
      redeclare function DynamicViscosityLiquid =
            REFPROP2Modelica.Media.NH3_Water.dynamicViscosity_LIQelsayed);

    package MediumHdbKaltetechnik = REFPROP2Modelica.Media.NH3_Water (
      redeclare function DynamicViscosityLiquid =
            REFPROP2Modelica.Media.NH3_Water.dynamicViscosity_LIQHDK);

    package MediumSteccoDesideri = REFPROP2Modelica.Media.NH3_Water (
      redeclare function DynamicViscosityLiquid =
            REFPROP2Modelica.Media.NH3_Water.dynamicViscosity_LIQTejeRiceSteccoWay);

    package MediumTejaRice = REFPROP2Modelica.Media.NH3_Water (
      redeclare function DynamicViscosityLiquid =
            REFPROP2Modelica.Media.NH3_Water.dynamicViscosity_LIQTejeRice);

    package MediumTejaRiceSassen = REFPROP2Modelica.Media.NH3_Water (
      redeclare function DynamicViscosityLiquid =
            REFPROP2Modelica.Media.NH3_Water.dynamicViscosity_LIQTejeRiceSassensTcrit);

  equation
    X = {time,1 - time};

    eta_conde = MediumConde.dynamicViscosity(state);
    eta_elsayed = MediumElSayed.dynamicViscosity(state);
    eta_HDK = MediumHdbKaltetechnik.dynamicViscosity(state);
    eta_TejaRiceStecco = MediumSteccoDesideri.dynamicViscosity(state);
    eta_TejaRice = MediumTejaRice.dynamicViscosity(state);
    eta_TejaRiceSassensTcrit = MediumTejaRiceSassen.dynamicViscosity(state);

    annotation (experiment(Interval=0.01));
  end MassFractionSweep_visc_liq;

  model EnthalpySweep_visc_liq

    package Medium = REFPROP2Modelica.Media.NH3_Water;
    Medium.ThermodynamicState state = Medium.setState_phX(p,h,X);

    parameter Medium.AbsolutePressure p=100e5;
    parameter Medium.Temperature Tstart=50+273.15;
    parameter Medium.MassFraction X[2]={0.5,0.5};
    Real h;
    parameter Real hstart =  Medium.specificEnthalpy(Medium.setState_pTX(p,Tstart,X));
    parameter Real hend = Medium.specificEnthalpy(Medium.setState_pqX(p,0,X));
    Real eta_conde,eta_elsayed,eta_HDK,eta_TejaRiceStecco;

    package MediumConde = REFPROP2Modelica.Media.NH3_Water (
      redeclare function DynamicViscosityLiquid =
            REFPROP2Modelica.Media.NH3_Water.dynamicViscosity_LIQconde);

    package MediumElSayed = REFPROP2Modelica.Media.NH3_Water (
      redeclare function DynamicViscosityLiquid =
            REFPROP2Modelica.Media.NH3_Water.dynamicViscosity_LIQelsayed);

    package MediumHdbKaltetechnik = REFPROP2Modelica.Media.NH3_Water (
      redeclare function DynamicViscosityLiquid =
            REFPROP2Modelica.Media.NH3_Water.dynamicViscosity_LIQHDK);

    package MediumSteccoDesideri = REFPROP2Modelica.Media.NH3_Water (
      redeclare function DynamicViscosityLiquid =
            REFPROP2Modelica.Media.NH3_Water.dynamicViscosity_LIQTejeRiceSteccoWay);

  equation
    h=hstart + (hend-hstart)*time;

    eta_conde = MediumConde.dynamicViscosity(state);
    eta_elsayed = MediumElSayed.dynamicViscosity(state);
    eta_HDK = MediumHdbKaltetechnik.dynamicViscosity(state);
    eta_TejaRiceStecco = MediumSteccoDesideri.dynamicViscosity(state);

    annotation (experiment(Interval=0.01));
  end EnthalpySweep_visc_liq;

  model MassFractionSweep_cond_vap

    parameter Modelica.SIunits.Pressure p=100e5;
    parameter Modelica.SIunits.Temperature T=325 + 273.15;
    Modelica.SIunits.MassFraction X[2];

    package MediumAverage = REFPROP2Modelica.Media.NH3_Water (
      redeclare function ThermalConductivityVapor =
            REFPROP2Modelica.Media.NH3_Water.thermalConductivity_VAPLinearMolePoling);
    MediumAverage.ThermodynamicState state=MediumAverage.setState_pTX(
        p,
        T,
        X,
        calcTransport=false);

    package MediumMasonSaxena = REFPROP2Modelica.Media.NH3_Water (
          redeclare function ThermalConductivityVapor =
            REFPROP2Modelica.Media.NH3_Water.thermalConductivity_VAPWilke);
    package MediumChung = REFPROP2Modelica.Media.NH3_Water (
          redeclare function ThermalConductivityVapor =
            REFPROP2Modelica.Media.NH3_Water.thermalConductivity_VAPChung);
    package MediumChungErrorWeight = REFPROP2Modelica.Media.NH3_Water (
                redeclare function ThermalConductivityVapor =
            REFPROP2Modelica.Media.NH3_Water.thermalConductivity_VAPChungPureErrorWeight);

    Real lambda_wilke,lambda_polingLin,lambda_chung,lambda_chungErrorWeight;

  equation
    X = {time,1 - time};

    lambda_wilke = MediumAverage.thermalConductivity(state);
    lambda_polingLin = MediumMasonSaxena.thermalConductivity(state);
    lambda_chung = MediumChung.thermalConductivity(state);
    lambda_chungErrorWeight = MediumChungErrorWeight.thermalConductivity(state);

    annotation (experiment(Interval=0.01));
  end MassFractionSweep_cond_vap;

  model EnthalpySweep_cond_vap

    package Medium = REFPROP2Modelica.Media.NH3_Water;
      parameter Medium.AbsolutePressure p=150e5;
      //  parameter Medium.Temperature Tstart=50+273.15;
      parameter Medium.Temperature Tend=500 + 273.15;
      parameter Medium.MassFraction X[2]={0.5,0.5};
      parameter Real hstart=Medium.specificEnthalpy(Medium.setState_pqX(
            p,
            1,
            X));
      parameter Real hend=Medium.specificEnthalpy(Medium.setState_pTX(
            p,
            Tend,
            X));
      Medium.ThermodynamicState state=Medium.setState_phX(
            p,
            h,
            X);

      Real h;
      Real lambda_wilke;
      Real lambda_chung;
      Real lambda_chungErrorWeight;
      Real lambda_polingLin;

    package MediumAverage = REFPROP2Modelica.Media.NH3_Water (
      redeclare function ThermalConductivityVapor =
            REFPROP2Modelica.Media.NH3_Water.thermalConductivity_VAPLinearMolePoling);
    package MediumMasonSaxena = REFPROP2Modelica.Media.NH3_Water (
          redeclare function ThermalConductivityVapor =
            REFPROP2Modelica.Media.NH3_Water.thermalConductivity_VAPWilke);
    package MediumChung = REFPROP2Modelica.Media.NH3_Water (
          redeclare function ThermalConductivityVapor =
            REFPROP2Modelica.Media.NH3_Water.thermalConductivity_VAPChung);
    package MediumChungErrorWeight = REFPROP2Modelica.Media.NH3_Water (
                redeclare function ThermalConductivityVapor =
            REFPROP2Modelica.Media.NH3_Water.thermalConductivity_VAPChungPureErrorWeight);

  equation
      h = hstart + (hend - hstart)*time;

      lambda_wilke = MediumAverage.thermalConductivity(state);
      lambda_polingLin = MediumMasonSaxena.thermalConductivity(state);
      lambda_chung = MediumChung.thermalConductivity(state);
      lambda_chungErrorWeight = MediumChungErrorWeight.thermalConductivity(
        state);

      annotation (experiment(Interval=0.01));
  end EnthalpySweep_cond_vap;

  model MassFractionSweep_cond_liq

    parameter Modelica.SIunits.Pressure p=50e5;
    parameter Modelica.SIunits.Temperature T=50 + 273.15;
    Modelica.SIunits.MassFraction X[2];

    package Medium = REFPROP2Modelica.Media.NH3_Water;
    Medium.ThermodynamicState state = Medium.setState_pTX(p,T,X,calcTransport=false);

    package MediumConde = REFPROP2Modelica.Media.NH3_Water (
      redeclare function ThermalConductivityLiquid =
            REFPROP2Modelica.Media.NH3_Water.thermalConductivity_LIQConde);
    package MediumElSayed = REFPROP2Modelica.Media.NH3_Water (
      redeclare function ThermalConductivityLiquid =
            REFPROP2Modelica.Media.NH3_Water.thermalConductivity_LIQLinearMoleElsayed);
    package MediumFilippov = REFPROP2Modelica.Media.NH3_Water (
      redeclare function ThermalConductivityLiquid =
            REFPROP2Modelica.Media.NH3_Water.thermalConductivity_LIQFilipov);
    package MediumJamieson = REFPROP2Modelica.Media.NH3_Water (
      redeclare function ThermalConductivityLiquid =
            REFPROP2Modelica.Media.NH3_Water.thermalConductivity_LIQJamie);

   Real lambda_elsayed,lambda_conde,lambda_filipov,lambda_jamieson;

  equation
    X={time,1-time};

    lambda_conde = MediumConde.thermalConductivity(state);
    lambda_elsayed = MediumElSayed.thermalConductivity(state);
    lambda_filipov = MediumFilippov.thermalConductivity(state);
    lambda_jamieson = MediumJamieson.thermalConductivity(state);

    annotation (experiment(Interval=0.01));
  end MassFractionSweep_cond_liq;

  model EnthalpySweep_cond_liq

    package Medium = REFPROP2Modelica.Media.NH3_Water;
    Medium.ThermodynamicState state = Medium.setState_phX(p,h,X);

    parameter Medium.AbsolutePressure p=100e5;
    parameter Medium.Temperature Tstart=50+273.15;
    parameter Medium.MassFraction X[2]={0.5,0.5};
    Real h;
    parameter Real hstart =  Medium.specificEnthalpy(Medium.setState_pTX(p,Tstart,X));
    parameter Real hend = Medium.specificEnthalpy(Medium.setState_pqX(p,0,X));

    Real lambda_elsayed,lambda_conde,lambda_filipov,lambda_jamieson;

    package MediumConde = REFPROP2Modelica.Media.NH3_Water (
      redeclare function ThermalConductivityLiquid =
            REFPROP2Modelica.Media.NH3_Water.thermalConductivity_LIQConde);
    package MediumElSayed = REFPROP2Modelica.Media.NH3_Water (
      redeclare function ThermalConductivityLiquid =
            REFPROP2Modelica.Media.NH3_Water.thermalConductivity_LIQLinearMoleElsayed);
    package MediumFilippov = REFPROP2Modelica.Media.NH3_Water (
      redeclare function ThermalConductivityLiquid =
            REFPROP2Modelica.Media.NH3_Water.thermalConductivity_LIQFilipov);
    package MediumJamieson = REFPROP2Modelica.Media.NH3_Water (
      redeclare function ThermalConductivityLiquid =
            REFPROP2Modelica.Media.NH3_Water.thermalConductivity_LIQJamie);

  equation
    h=hstart + (hend-hstart)*time;
  //  h=hstart + (2e6-hstart)*time;

    lambda_conde = MediumConde.thermalConductivity(state);
    lambda_elsayed = MediumElSayed.thermalConductivity(state);
    lambda_filipov = MediumFilippov.thermalConductivity(state);
    lambda_jamieson = MediumJamieson.thermalConductivity(state);

    annotation (experiment(Interval=0.01));
  end EnthalpySweep_cond_liq;

  model MassFractionSweepSurfaceTension

    package Medium = REFPROP2Modelica.Media.NH3_Water;
    Medium.SaturationProperties  sat;
    Real T;
    Medium.MassFraction X[2];
    Real sigma;

  equation
    X={time,1-time};
    sat = Medium.setSat_TX(T,X,calcTransport=true);
    T=350+273.15;

    sigma = Medium.surfaceTension(sat);

  end MassFractionSweepSurfaceTension;

package TestCore
  model liquid_viscosity_sweep

  package Medium = REFPROP2Modelica.Media.NH3_Water;
    parameter Integer n=101;
    Medium.ThermodynamicState state50[n] annotation(hideResult=true);
    Medium.ThermodynamicState state90[n] annotation(hideResult=true);
    Medium.ThermodynamicState state130[n] annotation(hideResult=true);
    Medium.AbsolutePressure p;

    Real x1[n] = linspace(0,1,n);

    Real eta_conde50[n];
    Real eta_elsayed50[n];
    Real eta_HDK50[n];
    Real eta_TejaRice50[n];
    Real eta_TejaRiceStecco50[n];

    Real eta_conde90[n];
    Real eta_elsayed90[n];
    Real eta_HDK90[n];
    Real eta_TejaRice90[n];
    Real eta_TejaRiceStecco90[n];

    Real eta_conde130[n];
    Real eta_elsayed130[n];
    Real eta_HDK130[n];
    Real eta_TejaRice130[n];
    Real eta_TejaRiceStecco130[n];

  equation
    p=50e5;

    for i in 1:n loop
      state50[i] = Medium.setState_pTX(p,50+273.15,{x1[i],1-x1[i]});
      eta_conde50[i] = Media.NH3_Water.dynamicViscosity_LIQconde(                                               state50[i]);
      eta_elsayed50[i] = Media.NH3_Water.dynamicViscosity_LIQelsayed(                                               state50[i]);
      eta_HDK50[i] = Media.NH3_Water.dynamicViscosity_LIQHDK(                                               state50[i]);
      eta_TejaRice50[i] = Media.NH3_Water.dynamicViscosity_LIQTejeRice(state50[i]);
      eta_TejaRiceStecco50[i] = Media.NH3_Water.dynamicViscosity_LIQTejeRiceSteccoWay(state50[i]);

      state90[i] = Medium.setState_pTX(p,90+273.15,{x1[i],1-x1[i]});
      eta_conde90[i] = Media.NH3_Water.dynamicViscosity_LIQconde(                                               state90[i]);
      eta_elsayed90[i] = Media.NH3_Water.dynamicViscosity_LIQelsayed(                                               state90[i]);
      eta_HDK90[i] = Media.NH3_Water.dynamicViscosity_LIQHDK(                                               state90[i]);
      eta_TejaRice90[i] = Media.NH3_Water.dynamicViscosity_LIQTejeRice(state90[i]);
      eta_TejaRiceStecco90[i] = Media.NH3_Water.dynamicViscosity_LIQTejeRiceSteccoWay(state90[i]);

      state130[i] = Medium.setState_pTX(p,130+273.15,{x1[i],1-x1[i]});
      eta_conde130[i] = Media.NH3_Water.dynamicViscosity_LIQconde(state130[i]);
      eta_elsayed130[i] = Media.NH3_Water.dynamicViscosity_LIQelsayed(state130[i]);
      eta_HDK130[i] = Media.NH3_Water.dynamicViscosity_LIQHDK(state130[i]);
      eta_TejaRice130[i] = Media.NH3_Water.dynamicViscosity_LIQTejeRice(state130[i]);
      eta_TejaRiceStecco130[i] = Media.NH3_Water.dynamicViscosity_LIQTejeRiceSteccoWay(state130[i]);

    end for;

  end liquid_viscosity_sweep;

  model vapor_viscosity_sweep

  package Medium = REFPROP2Modelica.Media.NH3_Water;
    parameter Integer n=101;
    Medium.ThermodynamicState state_325_050[n] annotation(hideResult=true);
    Medium.ThermodynamicState state_325_100[n] annotation(hideResult=true);
    Medium.ThermodynamicState state_525_100[n] annotation(hideResult=true);
    Medium.ThermodynamicState state_425_100[n] annotation(hideResult=true);
  // Medium.AbsolutePressure p;

    Real x1[n] = linspace(0,1,n);

    Real eta_reich_325_050[n];
    Real eta_reich_325_100[n];
    Real eta_reich_525_100[n];
    Real eta_reich_425_100[n];

    Real eta_wilke_325_050[n];
    Real eta_wilke_325_100[n];
    Real eta_wilke_525_100[n];
    Real eta_wilke_425_100[n];

    Real eta_chung_325_050[n];
    Real eta_chung_325_100[n];
    Real eta_chung_525_100[n];
    Real eta_chung_425_100[n];

    Real eta_chungWeight_325_050[n];
    Real eta_chungWeight_325_100[n];
    Real eta_chungWeight_525_100[n];
    Real eta_chungWeight_425_100[n];
  equation
    for i in 1:n loop

  //     state_315_100[i] = Medium.setState_pTX(100e5,315+273.15,{x1[i],1-x1[i]});
  //     eta_reich_315_100[i] = Media.NH3_Water.dynamicViscosity_VAPReichenberg(                                               state_315_100[i]);
  //     eta_wilke_315_100[i] = Media.NH3_Water.dynamicViscosity_VAPWilke(                                               state_315_100[i]);
  //
  //     state_330_125[i] = Medium.setState_pTX(125e5,330+273.15,{x1[i],1-x1[i]});
  //     eta_reich_330_125[i] = Media.NH3_Water.dynamicViscosity_VAPReichenberg(                                               state_330_125[i]);
  //     eta_wilke_330_125[i] = Media.NH3_Water.dynamicViscosity_VAPWilke(                                               state_330_125[i]);
  //
  //     state_345_150[i] = Medium.setState_pTX(150e5,345+273.15,{x1[i],1-x1[i]});
  //     eta_reich_345_150[i] = Media.NH3_Water.dynamicViscosity_VAPReichenberg(                                               state_345_150[i]);
  //     eta_wilke_345_150[i] = Media.NH3_Water.dynamicViscosity_VAPWilke(                                               state_345_150[i]);

  //     state_325_050[i] = Medium.setState_pTX(050e5,325+273.15,{x1[i],1-x1[i]});
  //     eta_reich_325_050[i] = Media.NH3_Water.dynamicViscosity_VAPReichenberg(                                               state_325_050[i]);
  //     eta_wilke_325_050[i] = Media.NH3_Water.dynamicViscosity_VAPWilke(                                               state_325_050[i]);
  //     eta_chung_325_050[i] = Media.NH3_Water.dynamicViscosity_VAPChung(                                               state_325_050[i]);
  //
  //     state_325_100[i] = Medium.setState_pTX(100e5,325+273.15,{x1[i],1-x1[i]});
  //     eta_reich_325_100[i] = Media.NH3_Water.dynamicViscosity_VAPReichenberg(                                               state_325_100[i]);
  //     eta_wilke_325_100[i] = Media.NH3_Water.dynamicViscosity_VAPWilke(                                               state_325_100[i]);
  //     eta_chung_325_100[i] = Media.NH3_Water.dynamicViscosity_VAPChung(                                               state_325_100[i]);
  //
  //     state_375_100[i] = Medium.setState_pTX(100e5,375+273.15,{x1[i],1-x1[i]});
  //     eta_reich_375_100[i] = Media.NH3_Water.dynamicViscosity_VAPReichenberg(                                               state_375_100[i]);
  //     eta_wilke_375_100[i] = Media.NH3_Water.dynamicViscosity_VAPWilke(                                               state_375_100[i]);
  //     eta_chung_375_100[i] = Media.NH3_Water.dynamicViscosity_VAPChung(                                               state_375_100[i]);
  //
  //     state_425_100[i] = Medium.setState_pTX(100e5,425+273.15,{x1[i],1-x1[i]});
  //     eta_reich_425_100[i] = Media.NH3_Water.dynamicViscosity_VAPReichenberg(                                               state_425_100[i]);
  //     eta_wilke_425_100[i] = Media.NH3_Water.dynamicViscosity_VAPWilke(                                               state_425_100[i]);
  //     eta_chung_425_100[i] = Media.NH3_Water.dynamicViscosity_VAPChung(                                               state_425_100[i]);

      state_325_050[i] = Medium.setState_pTX(050e5,325+273.15,{x1[i],1-x1[i]});
      eta_reich_325_050[i] = Media.NH3_Water.dynamicViscosity_VAPReichenberg(                                               state_325_050[i]);
      eta_wilke_325_050[i] = Media.NH3_Water.dynamicViscosity_VAPWilke(                                               state_325_050[i]);
      eta_chung_325_050[i] = Media.NH3_Water.dynamicViscosity_VAPChung(                                               state_325_050[i]);
      eta_chungWeight_325_050[i] = Media.NH3_Water.dynamicViscosity_VAPChungPureErrorWeight(                                               state_325_050[i]);

      state_325_100[i] = Medium.setState_pTX(100e5,325+273.15,{x1[i],1-x1[i]});
      eta_reich_325_100[i] = Media.NH3_Water.dynamicViscosity_VAPReichenberg(                                               state_325_100[i]);
      eta_wilke_325_100[i] = Media.NH3_Water.dynamicViscosity_VAPWilke(                                               state_325_100[i]);
      eta_chung_325_100[i] = Media.NH3_Water.dynamicViscosity_VAPChung(                                               state_325_100[i]);
      eta_chungWeight_325_100[i] = Media.NH3_Water.dynamicViscosity_VAPChungPureErrorWeight(                                               state_325_100[i]);

      state_425_100[i] = Medium.setState_pTX(100e5,425+273.15,{x1[i],1-x1[i]});
      eta_reich_425_100[i] = Media.NH3_Water.dynamicViscosity_VAPReichenberg(                                               state_425_100[i]);
      eta_wilke_425_100[i] = Media.NH3_Water.dynamicViscosity_VAPWilke(                                               state_425_100[i]);
      eta_chung_425_100[i] = Media.NH3_Water.dynamicViscosity_VAPChung(                                               state_425_100[i]);
      eta_chungWeight_425_100[i] = Media.NH3_Water.dynamicViscosity_VAPChungPureErrorWeight(                                               state_425_100[i]);

      state_525_100[i] = Medium.setState_pTX(100e5,525+273.15,{x1[i],1-x1[i]});
      eta_reich_525_100[i] = Media.NH3_Water.dynamicViscosity_VAPReichenberg(                                               state_525_100[i]);
      eta_wilke_525_100[i] = Media.NH3_Water.dynamicViscosity_VAPWilke(                                               state_525_100[i]);
      eta_chung_525_100[i] = Media.NH3_Water.dynamicViscosity_VAPChung(                                               state_525_100[i]);
      eta_chungWeight_525_100[i] = Media.NH3_Water.dynamicViscosity_VAPChungPureErrorWeight(                                               state_525_100[i]);

    end for;

  end vapor_viscosity_sweep;

  model liquid_conductivity_sweep

  package Medium = REFPROP2Modelica.Media.NH3_Water;
    parameter Integer n=101;

    Medium.ThermodynamicState state50_100[n] annotation(hideResult=true);
    Medium.ThermodynamicState state90_100[n] annotation(hideResult=true);
    Medium.ThermodynamicState state130_100[n] annotation(hideResult=true);

    Real x1[n] = linspace(0,1,n);

     Real lambda_elsayedLin50_100[n];
     Real lambda_filipov50_100[n];
     Real lambda_jamieson50_100[n];
     Real lambda_conde50_100[n];

     Real lambda_elsayedLin90_100[n];
     Real lambda_filipov90_100[n];
     Real lambda_jamieson90_100[n];
     Real lambda_conde90_100[n];

     Real lambda_elsayedLin130_100[n];
     Real lambda_filipov130_100[n];
     Real lambda_jamieson130_100[n];
     Real lambda_conde130_100[n];

  equation
    for i in 1:n loop
       state50_100[i] = Medium.setState_pTX(100e5,50+273.15,{x1[i],1-x1[i]});
       lambda_elsayedLin50_100[i] = Media.NH3_Water.thermalConductivity_LIQLinearMoleElsayed(                                               state50_100[i]);
       lambda_filipov50_100[i] = Media.NH3_Water.thermalConductivity_LIQFilipov(                                               state50_100[i]);
       lambda_jamieson50_100[i] = Media.NH3_Water.thermalConductivity_LIQJamie(                                               state50_100[i]);
       lambda_conde50_100[i] = Media.NH3_Water.thermalConductivity_LIQConde(                                               state50_100[i]);

       state90_100[i] = Medium.setState_pTX(100e5,90+273.15,{x1[i],1-x1[i]});
       lambda_elsayedLin90_100[i] = Media.NH3_Water.thermalConductivity_LIQLinearMoleElsayed(                                               state90_100[i]);
       lambda_filipov90_100[i] = Media.NH3_Water.thermalConductivity_LIQFilipov(                                               state90_100[i]);
       lambda_jamieson90_100[i] = Media.NH3_Water.thermalConductivity_LIQJamie(                                               state90_100[i]);
       lambda_conde90_100[i] = Media.NH3_Water.thermalConductivity_LIQConde(                                               state90_100[i]);

       state130_100[i] = Medium.setState_pTX(100e5,130+273.15,{x1[i],1-x1[i]});
       lambda_elsayedLin130_100[i] = Media.NH3_Water.thermalConductivity_LIQLinearMoleElsayed(                                               state130_100[i]);
       lambda_filipov130_100[i] = Media.NH3_Water.thermalConductivity_LIQFilipov(                                               state130_100[i]);
       lambda_jamieson130_100[i] = Media.NH3_Water.thermalConductivity_LIQJamie(                                               state130_100[i]);
       lambda_conde130_100[i] = Media.NH3_Water.thermalConductivity_LIQConde(                                               state130_100[i]);

    end for;

  end liquid_conductivity_sweep;

  model vapor_conductivity_sweep

  package Medium = REFPROP2Modelica.Media.NH3_Water;
    parameter Integer n=101;
    Medium.ThermodynamicState state_325_050[n] annotation(hideResult=true);
    Medium.ThermodynamicState state_325_100[n] annotation(hideResult=true);
    Medium.ThermodynamicState state_425_100[n] annotation(hideResult=true);
    Medium.ThermodynamicState state_525_100[n] annotation(hideResult=true);
  // Medium.AbsolutePressure p;

    Real x1[n] = linspace(0,1,n);

    Real lambda_polingLin_325_050[n];
    Real lambda_polingLin_325_100[n];
    Real lambda_polingLin_425_100[n];
    Real lambda_polingLin_525_100[n];

    Real lambda_wilke_325_050[n];
    Real lambda_wilke_325_100[n];
    Real lambda_wilke_425_100[n];
    Real lambda_wilke_525_100[n];

    Real lambda_chung_325_050[n];
    Real lambda_chung_325_100[n];
    Real lambda_chung_425_100[n];
    Real lambda_chung_525_100[n];

    Real lambda_chungErrorWeight_325_050[n];
    Real lambda_chungErrorWeight_325_100[n];
    Real lambda_chungErrorWeight_425_100[n];
    Real lambda_chungErrorWeight_525_100[n];

  equation
    for i in 1:n loop

      state_325_050[i] = Medium.setState_pTX(050e5,325+273.15,{x1[i],1-x1[i]});
      lambda_polingLin_325_050[i] = Media.NH3_Water.thermalConductivity_VAPLinearMolePoling(                                               state_325_050[i]);
      lambda_wilke_325_050[i] = Media.NH3_Water.thermalConductivity_VAPWilke(                                               state_325_050[i]);
      lambda_chung_325_050[i] = Media.NH3_Water.thermalConductivity_VAPChung(state_325_050[i]);
      lambda_chungErrorWeight_325_050[i] = Media.NH3_Water.thermalConductivity_VAPChungPureErrorWeight(state_325_050[i]);

      state_325_100[i] = Medium.setState_pTX(100e5,325+273.15,{x1[i],1-x1[i]});
      lambda_polingLin_325_100[i] = Media.NH3_Water.thermalConductivity_VAPLinearMolePoling(                                               state_325_100[i]);
      lambda_wilke_325_100[i] = Media.NH3_Water.thermalConductivity_VAPWilke(                                               state_325_100[i]);
      lambda_chung_325_100[i] = Media.NH3_Water.thermalConductivity_VAPChung(state_325_100[i]);
      lambda_chungErrorWeight_325_100[i] = Media.NH3_Water.thermalConductivity_VAPChungPureErrorWeight(state_325_100[i]);

      state_425_100[i] = Medium.setState_pTX(100e5,475+273.15,{x1[i],1-x1[i]});
      lambda_polingLin_425_100[i] = Media.NH3_Water.thermalConductivity_VAPLinearMolePoling(                                               state_425_100[i]);
      lambda_wilke_425_100[i] = Media.NH3_Water.thermalConductivity_VAPWilke(                                               state_425_100[i]);
      lambda_chung_425_100[i] = Media.NH3_Water.thermalConductivity_VAPChung(state_425_100[i]);
      lambda_chungErrorWeight_425_100[i] = Media.NH3_Water.thermalConductivity_VAPChungPureErrorWeight(state_425_100[i]);

      state_525_100[i] = Medium.setState_pTX(100e5,525+273.15,{x1[i],1-x1[i]});
      lambda_polingLin_525_100[i] = Media.NH3_Water.thermalConductivity_VAPLinearMolePoling(                                               state_525_100[i]);
      lambda_wilke_525_100[i] = Media.NH3_Water.thermalConductivity_VAPWilke(                                               state_525_100[i]);
      lambda_chung_525_100[i] = Media.NH3_Water.thermalConductivity_VAPChung(state_525_100[i]);
      lambda_chungErrorWeight_525_100[i] = Media.NH3_Water.thermalConductivity_VAPChungPureErrorWeight(state_525_100[i]);

    end for;

  end vapor_conductivity_sweep;
end TestCore;
end TransportPropsTest;
end AmmoniaWater;
