/***************************************************************************
                       mosfet_depl.cpp  -  description
                             -------------------
    begin                : Fri Jun 4 2004
    copyright            : (C) 2003 by Michael Margraf
    email                : michael.margraf@alumni.tu-berlin.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "mosfet_depl.h"


MOSFET_depl::MOSFET_depl()
{
  Description = QObject::tr("depletion MOS field-effect transistor");

  Lines.append(new Line(-14,-13,-14, 13,QPen(QPen::darkBlue,3)));
  Lines.append(new Line(-30,  0,-14,  0,QPen(QPen::darkBlue,2)));
  Lines.append(new Line(-10,-16,-10, 16,QPen(QPen::darkBlue,3)));

  Lines.append(new Line(-10,-11,  0,-11,QPen(QPen::darkBlue,2)));
  Lines.append(new Line(  0,-11,  0,-30,QPen(QPen::darkBlue,2)));
  Lines.append(new Line(-10, 11,  0, 11,QPen(QPen::darkBlue,2)));
  Lines.append(new Line(  0,  0,  0, 30,QPen(QPen::darkBlue,2)));
  Lines.append(new Line(-10,  0,  0,  0,QPen(QPen::darkBlue,2)));

  Lines.append(new Line( -9,  0, -4, -5,QPen(QPen::darkBlue,2)));
  Lines.append(new Line( -9,  0, -4,  5,QPen(QPen::darkBlue,2)));

  Ports.append(new Port(-30,  0));
  Ports.append(new Port(  0,-30));
  Ports.append(new Port(  0, 30));

  x1 = -30; y1 = -30;
  x2 =   4; y2 =  30;

  tx = x2+4;
  ty = y1+4;
  Model = "DMOSFET";
  Name  = "T";

  // this must be the first property in the list !!!
  Props.append(new Property("Type", "nfet", false,
	QObject::tr("polarity (nfet,pfet)")));
  Props.append(new Property("Vt0", "-1.0 V", true,
	QObject::tr("zero-bias threshold voltage")));
  Props.append(new Property("Kp", "2e-5", true,
	QObject::tr("transconductance coefficient in A/m^2")));
  Props.append(new Property("Gamma", "0.0", false,
	QObject::tr("bulk threshold")));
  Props.append(new Property("Phi", "0.6 V", false,
	QObject::tr("surface potential")));
  Props.append(new Property("Lambda", "0.0", true,
	QObject::tr("channel-length modulation parameter")));
  Props.append(new Property("Rd", "0.0 Ohm", false,
	QObject::tr("drain ohmic resistance")));
  Props.append(new Property("Rs", "0.0 Ohm", false,
	QObject::tr("source ohmic resistance")));
  Props.append(new Property("Rg", "0.0 Ohm", false,
	QObject::tr("gate ohmic resistance")));
  Props.append(new Property("Is", "1e-14 A", false,
	QObject::tr("bulk junction saturation current")));
  Props.append(new Property("N", "1.0", false,
	QObject::tr("bulk junction emission coefficient")));
  Props.append(new Property("W", "1 um", false,
	QObject::tr("channel width")));
  Props.append(new Property("L", "1 um", false,
	QObject::tr("channel length")));
  Props.append(new Property("Ld", "0.0", false,
	QObject::tr("lateral diffusion length")));
  Props.append(new Property("Tox", "0.1 um", false,
	QObject::tr("oxide thickness")));
  Props.append(new Property("Cgso", "0.0", false,
	QObject::tr("gate-source overlap capacitance per meter of "
		    "channel width in F/m")));
  Props.append(new Property("Cgdo", "0.0", false,
	QObject::tr("gate-drain overlap capacitance per meter of "
		    "channel width in F/m")));
  Props.append(new Property("Cgbo", "0.0", false,
	QObject::tr("gate-bulk overlap capacitance per meter of "
		    "channel length in F/m")));
  Props.append(new Property("Cbd", "0.0 F", false,
	QObject::tr("zero-bias bulk-drain junction capacitance")));
  Props.append(new Property("Cbs", "0.0 F", false,
	QObject::tr("zero-bias bulk-source junction capacitance")));
  Props.append(new Property("Pb", "0.8 V", false,
	QObject::tr("bulk junction potential")));
  Props.append(new Property("Mj", "0.5", false,
	QObject::tr("bulk junction bottom grading coefficient")));
  Props.append(new Property("Fc", "0.5", false,
	QObject::tr("bulk junction forward-bias depletion capacitance "
		    "coefficient")));
  Props.append(new Property("Tt", "0.0 ps", false,
	QObject::tr("bulk transit time")));
  Props.append(new Property("Nsub", "0.0", false,
	QObject::tr("substrate bulk doping density in 1/cm^3")));
  Props.append(new Property("Nss", "0.0", false,
	QObject::tr("surface state density in 1/cm^2")));
  Props.append(new Property("Tpg", "1", false,
	QObject::tr("gate material type: 0 = alumina; -1 = same as bulk; "
		    "1 = opposite to bulk")));
  Props.append(new Property("Uo", "600.0", false,
	QObject::tr("surface mobility in cm^2/Vs")));
  Props.append(new Property("Kf", "0.0", false,
	QObject::tr("flicker noise coefficient")));
  Props.append(new Property("Af", "1.0", false,
	QObject::tr("flicker noise exponent")));
  Props.append(new Property("Ffe", "1.0", false,
	QObject::tr("flicker noise frequency exponent")));
  Props.append(new Property("Temp", "26.85", false,
	QObject::tr("simulation temperature in degree Celsius")));
}

MOSFET_depl::~MOSFET_depl()
{
}

Component* MOSFET_depl::newOne()
{
  return new MOSFET_depl();
}

Component* MOSFET_depl::info(QString& Name, char* &BitmapFile, bool getNewOne)
{
  Name = QObject::tr("depletion MOSFET");
  BitmapFile = "dmosfet";

  if(getNewOne)  return new MOSFET_depl();
  return 0;
}
