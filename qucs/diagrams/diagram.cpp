/***************************************************************************
                          diagram.cpp  -  description
                             -------------------
    begin                : Thu Oct 2 2003
    copyright            : (C) 2003, 2004 by Michael Margraf
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

using namespace std;

#include "diagram.h"
#include "qucs.h"

#include <stdlib.h>
#include <math.h>
#include <float.h>

#if HAVE_IEEEFP_H
# include <ieeefp.h>
#endif

#include <qtextstream.h>
#include <qmessagebox.h>
#include <qregexp.h>
#include <qdatetime.h>

#ifdef __MINGW32__
# define finite(x) _finite(x)
#endif

#define INVALID_STR QObject::tr(" <invalid>")

Diagram::Diagram(int _cx, int _cy)
{
  cx = _cx;  cy = _cy;

  xAxis.numGraphs = ylAxis.numGraphs = yrAxis.numGraphs = 0;
  xAxis.min = xAxis.low =
  ylAxis.min = ylAxis.low = yrAxis.min = yrAxis.low = 0.0;
  xAxis.max = xAxis.up =
  ylAxis.max = ylAxis.up = yrAxis.max = yrAxis.up = 1.0;
  xAxis.GridOn = ylAxis.GridOn = true;
  yrAxis.GridOn = false;

  Type = isDiagram;
  isSelected = false;
  GridPen = QPen(QPen::lightGray,0);
  Graphs.setAutoDelete(true);
  Arcs.setAutoDelete(true);
  Lines.setAutoDelete(true);
  Texts.setAutoDelete(true);
}

Diagram::~Diagram()
{
}

// ------------------------------------------------------------
void Diagram::paint(ViewPainter *p)
{
  // paint all lines
  for(Line *pl = Lines.first(); pl != 0; pl = Lines.next()) {
    p->Painter->setPen(pl->style);
    p->drawLine(cx+pl->x1, cy-pl->y1, cx+pl->x2, cy-pl->y2);
  }

  // paint all arcs
  for(Arc *pa = Arcs.first(); pa != 0; pa = Arcs.next()) {
    p->Painter->setPen(pa->style);
    p->drawArc(cx+pa->x, cy-pa->y, pa->w, pa->h, pa->angle, pa->arclen);
  }

  QSize s;
  Graph *pg;
  int   x, y;
  if(Name[0] != 'T') {   // no graph within tabulars
    // draw all graphs
    for(pg = Graphs.first(); pg != 0; pg = Graphs.next())
      pg->paint(p, cx, cy);

    p->map(cx+(x2>>1), cy+y1, &x, &y);
    if(xAxis.Label.isEmpty()) {
      // write all x labels ----------------------------------------
      for(pg = Graphs.first(); pg != 0; pg = Graphs.next()) {
	DataX *pD = pg->cPointsX.getFirst();
	if(!pD) continue;
	p->Painter->setPen(pg->Color);
	s = p->Painter->fontMetrics().size(0, pD->Var);
	p->Painter->drawText(x-(s.width()>>1), y, pD->Var);
	y += p->LineSpacing;
      }
    }
    else {
      // write x label text -----------------------------------------
      p->Painter->setPen(Qt::black);
      s = p->Painter->fontMetrics().size(0, xAxis.Label);
      p->Painter->drawText(x-(s.width()>>1), y, xAxis.Label);
    }


    QWMatrix wm = p->Painter->worldMatrix();
    p->Painter->setWorldMatrix(QWMatrix(0.0,-1.0,1.0,0.0, 0, 0));
    p->map(cx-x1, cy-(y2>>1), &x, &y);
    if(ylAxis.Label.isEmpty()) {
      // draw left y-label for all graphs ------------------------------
      for(pg = Graphs.first(); pg != 0; pg = Graphs.next()) {
        if(pg->yAxisNo != 0)  continue;
        p->Painter->setPen(pg->Color);
	if(pg->Points) {
	  s = p->Painter->fontMetrics().size(0, pg->Var);
	  p->Painter->drawText(-y-(s.width()>>1), x, pg->Var);
	}
	else {     // if no data => <invalid>
	  s = p->Painter->fontMetrics().size(0, pg->Var+INVALID_STR);
	  p->Painter->drawText(-y-(s.width()>>1), x, pg->Var+INVALID_STR);
	}
	x -= p->LineSpacing;
      }
    }
    else {
	p->Painter->setPen(Qt::black);
	s = p->Painter->fontMetrics().size(0, ylAxis.Label);
	p->Painter->drawText(-y-(s.width()>>1), x, ylAxis.Label);
    }

    p->Painter->setWorldMatrix(QWMatrix(0.0,1.0,-1.0,0.0, 0, 0));
    p->map(cx+x3, cy-(y2>>1), &x, &y);
    if(yrAxis.Label.isEmpty()) {
      // draw right y-label for all graphs ------------------------------
      for(pg = Graphs.first(); pg != 0; pg = Graphs.next()) {
        if(pg->yAxisNo != 1)  continue;
        p->Painter->setPen(pg->Color);
	if(pg->Points) {
	  s = p->Painter->fontMetrics().size(0, pg->Var);
	  p->Painter->drawText(y-(s.width()>>1), -x, pg->Var);
	}
	else {     // if no data => <invalid>
	  s = p->Painter->fontMetrics().size(0, pg->Var+INVALID_STR);
	  p->Painter->drawText(y-(s.width()>>1), -x, pg->Var+INVALID_STR);
	}
	x += p->LineSpacing;
      }
    }
    else {
	p->Painter->setPen(Qt::black);
	s = p->Painter->fontMetrics().size(0, yrAxis.Label);
	p->Painter->drawText(y-(s.width()>>1), -x, yrAxis.Label);
    }
    p->Painter->setWorldMatrix(wm);
    p->Painter->setWorldXForm(false);
  }

  p->Painter->setPen(QPen(QPen::black,1));
  // write whole text
  for(Text *pt = Texts.first(); pt != 0; pt = Texts.next())
    p->drawText(pt->s, cx+pt->x, cy-pt->y);

  // draw markers last, so they are at the top of painting layers
  for(pg = Graphs.first(); pg != 0; pg = Graphs.next())
    for(Marker *pm = pg->Markers.first(); pm != 0; pm = pg->Markers.next())
      pm->paint(p, cx, cy);


  if(isSelected) {
    p->Painter->setPen(QPen(QPen::darkGray,3));
    p->drawRect(cx-5, cy-y2-5, x2+10, y2+10);
    p->Painter->setPen(QPen(QPen::darkRed,2));
    p->drawResizeRect(cx, cy-y2);  // markers for changing the size
    p->drawResizeRect(cx, cy);
    p->drawResizeRect(cx+x2, cy-y2);
    p->drawResizeRect(cx+x2, cy);
  }
}

// ------------------------------------------------------------
void Diagram::paintScheme(QPainter *p)
{
  p->drawRect(cx, cy-y2, x2, y2);
}

// ------------------------------------------------------------
int Diagram::calcDiagram()
{
  return 0;
}

// ------------------------------------------------------------
void Diagram::calcData(Graph *g, int valid)
{
  if(g->Points != 0) { free(g->Points);  g->Points = 0; }
  if(Name[0] == 'T')  return;   // no graph within tabulars
  if((valid | g->yAxisNo) == 0)  return;


  if(g->countY == 0) return;
  int Size = ((2*(g->cPointsX.getFirst()->count) + 1) * g->countY) + 4;
  int *p = (int*)malloc( Size*sizeof(int) );  // create memory for points
  int *p_end;
  g->Points = p_end = p;
  p_end += Size - 5;   // limit of buffer

  int i, z;
  double *px;
  double *py = g->cPointsY;
  Axis *pa;
  if(g->yAxisNo == 0)  pa = &ylAxis;
  else  pa = &yrAxis;

  double Stroke=10.0, Space=10.0; // length of strokes and spaces in pixel
  switch(g->Style) {
    case 0:   // solid line
      for(i=g->countY; i>0; i--) {
	px = g->cPointsX.getFirst()->Points;
	for(z=g->cPointsX.getFirst()->count; z>0; z--) {
	  calcCoordinate(*px, *py, *(py+1), p, p+1, pa);
	  px++;
	  py += 2;
	  p  += 2;
	}
	*(p++) = -10;
      }
      *p = -100;
      return;
    case 1: Stroke = 10.0; Space =  6.0;  break;   // dash line
    case 2: Stroke =  2.0; Space =  4.0;  break;   // dot line
    case 3: Stroke = 24.0; Space =  8.0;  break;   // long dash line
  }

  int dx, dy, xtmp, ytmp, tmp;
  double alpha, dist;
  int Flag;    // current state: 1=stroke, 0=space
  for(i=g->countY; i>0; i--) {
    Flag = 1;
    dist = -Stroke;
    px = g->cPointsX.getFirst()->Points;
    calcCoordinate(*px, *py, *(py+1), &xtmp, &ytmp, pa);
    px++;  py += 2;
    *(p++) = xtmp;
    *(p++) = ytmp;
    for(z=g->cPointsX.getFirst()->count-1; z>0; z--) {
      dx = xtmp;
      dy = ytmp;
      calcCoordinate(*px, *py, *(py+1), &xtmp, &ytmp, pa);
      px++;  py += 2;
      dx = xtmp - dx;
      dy = ytmp - dy;
      dist += sqrt(double(dx*dx + dy*dy)); // distance between points
      if(Flag == 1) if(dist <= 0) {
	if(p >= p_end) {  // need to enlarge memory block ?
	  Size += 256;
	  tmp = p - g->Points;
	  p = p_end = g->Points = (int*)realloc(g->Points, Size*sizeof(int));
	  p += tmp;
	  p_end += Size - 5;
	}

	*(p++) = xtmp;    // if stroke then save points
	*(p++) = ytmp;
	continue;
      }
      alpha = atan2(double(dy), double(dx));   // slope for interpolation
      while(dist > 0) {   // stroke or space finished ?
	if(p >= p_end) {  // need to enlarge memory block ?
	  Size += 256;
	  tmp = p - g->Points;
	  p = p_end = g->Points = (int*)realloc(g->Points, Size*sizeof(int));
	  p += tmp;
	  p_end += Size - 5;
	}

	*(p++) = xtmp - int(dist*cos(alpha) + 0.5); // linearly interpolate
        *(p++) = ytmp - int(dist*sin(alpha) + 0.5);

         if(Flag == 0) {
            dist -= Stroke;
            if(dist <= 0) {
               *(p++) = xtmp;  // don't forget point after ...
               *(p++) = ytmp;  // ... interpolated point
            }
         }
         else {
            dist -= Space;
            *(p++) = -2;  // value for interrupt stroke
         }
         Flag ^= 1; // toggle between stroke and space
      }

    } // of x loop
    *(p++) = -10;
  } // of y loop
  *p = -100;

}

// -------------------------------------------------------
void Diagram::Bounding(int& _x1, int& _y1, int& _x2, int& _y2)
{
  _x1 = cx-x1;
  _y1 = cy-y2;
  _x2 = cx+x2;
  _y2 = cy+y1;
}

// -------------------------------------------------------
bool Diagram::getSelected(int x_, int y_)
{
  if(x_ >= cx-x1) if(x_ <= cx+x2) if(y_ >= cy-y2) if(y_ <= cy+y1)
    return true;

  return false;
}

// ------------------------------------------------------------
// Checks if the resize area was clicked. If so return "true" and sets
// x1/y1 and x2/y2 to the border coordinates to draw a rectangle.
bool Diagram::ResizeTouched(int x, int y)
{
  if(x < cx-5) return false;
  if(x > cx+x2+5) return false;
  if(y < cy-y2-5) return false;
  if(y > cy+5) return false;

  State = 0;
  if(x < cx+5) State = 1;
  else  if(x <= cx+x2-5) return false;
  if(y > cy-5) State |= 2;
  else  if(y >= cy-y2+5) return false;

  return true;
}

// --------------------------------------------------------------------------
void Diagram::loadGraphData(const QString& defaultDataSet)
{
  ylAxis.numGraphs = yrAxis.numGraphs = 0;
  ylAxis.min = yrAxis.min = xAxis.min =  DBL_MAX;
  ylAxis.max = yrAxis.max = xAxis.max = -DBL_MAX;

  for(Graph *pg = Graphs.first(); pg != 0; pg = Graphs.next())
    loadVarData(defaultDataSet);  // load data, determine max/min values

  if(ylAxis.min > ylAxis.max) {
    ylAxis.min = 0.0;
    ylAxis.max = 1.0;
  }
  if(yrAxis.min > yrAxis.max) {
    yrAxis.min = 0.0;
    yrAxis.max = 1.0;
  }
  if(xAxis.min > xAxis.max) {
    xAxis.min = 0.0;
    xAxis.max = 1.0;
  }
  if((Name == "Polar") || (Name == "Smith")) {  // one axis only
    if(ylAxis.min > yrAxis.min)  ylAxis.min = yrAxis.min;
    if(ylAxis.max < yrAxis.max)  ylAxis.max = yrAxis.max;
  }

  updateGraphData();
}

// ------------------------------------------------------------------------
// Calculate diagram again without reading dataset from file.
void Diagram::recalcGraphData()
{
  ylAxis.min = yrAxis.min = xAxis.min =  DBL_MAX;
  ylAxis.max = yrAxis.max = xAxis.max = -DBL_MAX;

  int z;
  double x, y, *p;
  // get maximum and minimum values
  for(Graph *pg = Graphs.first(); pg != 0; pg = Graphs.next()) {
    DataX *pD=pg->cPointsX.getFirst();
    if(pD == 0) continue;
    p = pD->Points;
    for(z=pD->count; z>0; z--) { // check every x coordinate (1. dimension)
      x = *(p++);
      if(x > xAxis.max) xAxis.max = x;
      if(x < xAxis.min) xAxis.min = x;
    }

    Axis *pa;
    if(pg->yAxisNo == 0)  pa = &ylAxis;
    else  pa = &yrAxis;
    p = pg->cPointsY;
    for(z=pg->countY*pD->count; z>0; z--) {  // check every y coordinate
      x = *(p++);
      y = *(p++);
      if(fabs(y) >= 1e-250) x = sqrt(x*x+y*y);
      if(x > pa->max) pa->max = x;
      if(x < pa->min) pa->min = x;
    }
  }

  if(xAxis.min > xAxis.max) {
    xAxis.min = 0.0;
    xAxis.max = 1.0;
  }
  if(ylAxis.min > ylAxis.max) {
    ylAxis.min = 0.0;
    ylAxis.max = 1.0;
  }
  if(yrAxis.min > yrAxis.max) {
    yrAxis.min = 0.0;
    yrAxis.max = 1.0;
  }
  if((Name == "Polar") || (Name == "Smith")) {  // one axis only
    if(ylAxis.min > yrAxis.min)  ylAxis.min = yrAxis.min;
    if(ylAxis.max < yrAxis.max)  ylAxis.max = yrAxis.max;
  }

  updateGraphData();
}

// ------------------------------------------------------------------------
void Diagram::updateGraphData()
{
  int valid = calcDiagram();   // do not calculate graph data if invalid

  for(Graph *pg = Graphs.first(); pg != 0; pg = Graphs.next()) {
    calcData(pg, valid);   // calculate graph coordinates

    for(Marker *pm = pg->Markers.first(); pm != 0; pm = pg->Markers.next())
      pm->createText();
  }
}

// --------------------------------------------------------------------------
bool Diagram::loadVarData(const QString& fileName)
{
  Graph *g = Graphs.current();
  g->countY = 0;
  g->cPointsX.clear();
  if(g->cPointsY != 0) { delete[] g->cPointsY;  g->cPointsY = 0; }
  if(g->Var.isEmpty()) return false;

  QFile file;
  QString Variable;
  int pos = g->Var.find(':');
  if(pos < 0) {
    file.setName(fileName);
    Variable = g->Var;
  }
  else {
    file.setName(g->Var.left(pos)+".dat");
    Variable = g->Var.mid(pos+1);
  }

  file.setName(QucsWorkDir.filePath(file.name()));
  if(!file.open(IO_ReadOnly)) {
    QMessageBox::critical(0, QObject::tr("Error"),
                 QObject::tr("Cannot load dataset: ")+file.name());
    return false;
  }

  // *****************************************************************
  // To strongly speed up the file read operation the whole file is
  // read into the memory in one piece.
  QTextStream ReadWhole(&file);
  QString FileString = ReadWhole.read();
  file.close();


  QString Line, tmp, Var;
  // *****************************************************************
  // look for variable name in data file  ****************************
  int i=0, j=0, k=0;   // i and j must not be used temporarily !!!
  i = FileString.find('<')+1;
  while(i > 0) {
    j = FileString.find('>', i);
    Line = FileString.mid(i, j-i);
    i = FileString.find('<', j)+1;
    if(Line.left(3) == "dep") {
      tmp = Line.section(' ', 1, 1);
      if(Variable != tmp) continue; // found variable with name sought for ?

      k = 2;
      tmp = Line.section(' ',k,k);
      while(!tmp.isEmpty()) {
        g->cPointsX.append(new DataX(tmp));  // name of independet variable
        k++;
        tmp = Line.section(' ',k,k);
      }
      break;
    }
    if(Line.left(5) == "indep") {
      tmp = Line.section(' ', 1, 1);
      if(Variable != tmp) continue;  // found variable with name sought for ?
      break;
    }
  }

  if(i <= 0)  return false;   // return if data name was not found

  // *****************************************************************
  // get independent variable ****************************************
  bool ok=true, ok2=true;
  double *p;
  int counting = 0;
  if(g->cPointsX.isEmpty()) {    // create independent variable by myself ?
    tmp = Line.section(' ', 2, 2);  // get number of points
    counting = tmp.toInt(&ok);
    g->cPointsX.append(new DataX("number", 0, counting));
    if(!ok) {
      QMessageBox::critical(0, QObject::tr("Error"),
                   QObject::tr("Cannot get size of independent data \"")+
		   Variable+"\"");
      return false;
    }

    p = new double[counting];  // memory of new independent variable
    g->countY = 1;
    g->cPointsX.current()->Points = p;
    for(int z=1; z<=counting; z++)  *(p++) = double(z);
    xAxis.min = 1.0;
    xAxis.max = double(counting);
  }
  else {  // ...................................
    // get independent variables from data file
    g->countY = 1;
    for(DataX *pD = g->cPointsX.last(); pD!=0; pD = g->cPointsX.prev()) {
      if(pD == g->cPointsX.getFirst()) {
	xAxis.min = DBL_MAX;    // only count the first independent variable
	xAxis.max = -DBL_MAX;
      }
      counting = loadIndepVarData(pD->Var, FileString);
      g->countY *= counting;
      if(counting <= 0) {     // failed to load independent variable ?
        g->cPointsX.clear();
        return false;  // error message was already created
      }
    }
    g->countY /= counting;
  }


  // *****************************************************************
  // get dependent variables *****************************************
  counting  *= g->countY;
  p = new double[2*counting]; // memory for dependent variables
  g->cPointsY = p;
  Axis *pa;
  if(g->yAxisNo == 0)  pa = &ylAxis;   // for which axis
  else  pa = &yrAxis;
  (pa->numGraphs)++;    // count graphs

  double x, y;
  QRegExp WhiteSpace("\\s");
  QRegExp noWhiteSpace("\\S");
  i = FileString.find(noWhiteSpace, j+1);
  j = FileString.find(WhiteSpace, i);
  Line = FileString.mid(i, j-i);
  for(int z=counting; z>0; z--) {
    k = Line.find('j');
    if(k < 0) {
      x = Line.toDouble(&ok);
      y = 0.0;
    }
    else {
      tmp = Line.mid(k);  // imaginary part
      tmp.at(0) = Line.at(k-1);   // copy sign over "j"
      y = tmp.toDouble(&ok);
      Line = Line.left(k-1);  // real part
      x = Line.toDouble(&ok2);
    }
    if((!ok) || (!ok2)) {
      QMessageBox::critical(0, QObject::tr("Error"),
		QObject::tr("Too few dependent data \"") + Variable+"\"");
      g->cPointsX.clear();
      delete[] g->cPointsY;  g->cPointsY = 0;
      g->countY = 0;
      return false;
    }
    *(p++) = x;
    *(p++) = y;
    if(fabs(y) >= 1e-250) x = sqrt(x*x+y*y);
    if(finite(x)) {
      if(x > pa->max) pa->max = x;
      if(x < pa->min) pa->min = x;
    }

    i = FileString.find(noWhiteSpace, j);
    j = FileString.find(WhiteSpace, i);
    Line = FileString.mid(i, j-i);
  }

  return true;
}

// --------------------------------------------------------------------------
// Reads the data of an independent variable. Returns the number of points.
int Diagram::loadIndepVarData(const QString& var, const QString& FileString)
{
  QString Line, tmp;

  int i=0, j=0;
  i = FileString.find('<')+1;
  if(i > 0)
  do {    // look for variable name in data file
    j = FileString.find('>', i);
    Line = FileString.mid(i, j-i);
    i = FileString.find('<', j)+1;
    if(Line.left(5) == "indep") {
      tmp = Line.section(' ', 1, 1);
      if(var == tmp) break;     // found variable with name sought for ?
    }
  } while(i > 0);

  if(i <= 0) {
    QMessageBox::critical(0, QObject::tr("Error"),
	QObject::tr("Independent data \"")+var+QObject::tr("\" not found"));
    return -1;
  }

  bool ok;
  tmp = Line.section(' ', 2, 2);  // get number of points
  int n = tmp.toInt(&ok);
  if(!ok) {
    QMessageBox::critical(0, QObject::tr("Error"),
	QObject::tr("Cannot get size of independent data \"")+var+"\"");
    return -1;
  }

  double *p = new double[n];     // memory for new independent variable
  DataX *pD = Graphs.current()->cPointsX.current();
  pD->Points = p;
  pD->count  = n;

  double x;
  QRegExp WhiteSpace("\\s");
  QRegExp noWhiteSpace("\\S");
  i = FileString.find(noWhiteSpace, j+1);
  j = FileString.find(WhiteSpace, i);
  Line = FileString.mid(i, j-i);
  for(int z=0; z<n; z++) {
    x = Line.toDouble(&ok);  // get number
    if(!ok) {
      QMessageBox::critical(0, QObject::tr("Error"),
		 QObject::tr("Too few independent data \"") + var + "\"");
      return -1;
    }
    *(p++) = x;
    if(finite(x)) {
      if(x > xAxis.max) xAxis.max = x;
      if(x < xAxis.min) xAxis.min = x;
    }

    i = FileString.find(noWhiteSpace, j);
    j = FileString.find(WhiteSpace, i);
    Line = FileString.mid(i, j-i);
  }

  return n;   // return number of independent data
}

// ------------------------------------------------------------
void Diagram::setCenter(int x, int y, bool relative)
{
  if(relative) {
    cx += x;  cy += y;
  }
  else {
    cx = x;  cy = y;
  }
}

// -------------------------------------------------------
void Diagram::getCenter(int& x, int& y)
{
  x = cx + (x2 >> 1);
  y = cy - (y2 >> 1);
}

// ------------------------------------------------------------
Diagram* Diagram::newOne()
{
  return new Diagram();
}

// ------------------------------------------------------------
QString Diagram::save()
{
  QString s = "<"+Name+" "+QString::number(cx)+" "+QString::number(cy)+" ";
  s += QString::number(x2)+" "+QString::number(y2)+" ";
  if(xAxis.GridOn) s+= "1 ";
  else s += "0 ";
  s += GridPen.color().name() + " " + QString::number(GridPen.style());
  if(xAxis.log) s+= " 1";  else s += " 0";
//  if(ylAxis.log) s+= "1";   else s += "0";
  char c = '0';
  if(ylAxis.log)  c |= 1;
  if(yrAxis.log)  c |= 2;
  s += c;
  s += " \""+xAxis.Label+"\" \""+ylAxis.Label+"\" \""+yrAxis.Label+"\">\n";

  for(Graph *pg=Graphs.first(); pg != 0; pg=Graphs.next())
    s += pg->save()+"\n";

  s += "  </"+Name+">";
  return s;
}

// ------------------------------------------------------------
bool Diagram::load(const QString& Line, QTextStream *stream)
{
  bool ok;
  QString s = Line;

  if(s.at(0) != '<') return false;
  if(s.at(s.length()-1) != '>') return false;
  s = s.mid(1, s.length()-2);   // cut off start and end character

  QString n;
  n  = s.section(' ',1,1);    // cx
  cx = n.toInt(&ok);
  if(!ok) return false;

  n  = s.section(' ',2,2);    // cy
  cy = n.toInt(&ok);
  if(!ok) return false;

  n  = s.section(' ',3,3);    // x2
  x2 = n.toInt(&ok);
  if(!ok) return false;

  n  = s.section(' ',4,4);    // y2
  y2 = n.toInt(&ok);
  if(!ok) return false;

  n  = s.section(' ',5,5);    // GridOn
  xAxis.GridOn = ylAxis.GridOn = n.at(0) != '0';

  n  = s.section(' ',6,6);    // color for GridPen
  QColor co;
  co.setNamedColor(n);
  GridPen.setColor(co);
  if(!GridPen.color().isValid()) return false;

  n  = s.section(' ',7,7);    // line style
  GridPen.setStyle((Qt::PenStyle)n.toInt(&ok));
  if(!ok) return false;

  char c;
  n  = s.section(' ',8,8);    // xlog, ylog
  xAxis.log = n.at(0) != '0';
  c = n.at(1).latin1();
  ylAxis.log = ((c - '0') & 1) == 1;
  yrAxis.log = ((c - '0') & 2) == 2;

  xAxis.Label = s.section('"',1,1);    // xLabel
  ylAxis.Label = s.section('"',3,3);   // yLabel left
  yrAxis.Label = s.section('"',5,5);   // yLabel right

  Graph *pg;
  // .......................................................
  // load graphs of the diagram
  while(!stream->atEnd()) {
    s = stream->readLine();
    s = s.stripWhiteSpace();
    if(s.isEmpty()) continue;

    if(s == ("</"+Name+">")) return true;  // found end tag ?
    if(s.section(' ', 0,0) == "<Mkr") {

      // .......................................................
      // load markers of the diagram
      pg = Graphs.current();
      if(!pg)  return false;
      Marker *pm = new Marker(this, pg);
      if(!pm->load(s)) {
	delete pm;
	return false;
      }
      pg->Markers.append(pm);
      continue;
    }

    pg = new Graph();
    if(!pg->load(s)) {
      delete pg;
      return false;
    }
    Graphs.append(pg);
  }

  return false;   // end tag missing
}
