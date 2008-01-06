/***************************************************************************
 * Copyright (C) 2006 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 *                                                                         *
 * This is free software; you can redistribute it and/or modify            *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation; either version 2, or (at your option)     *
 * any later version.                                                      *
 *                                                                         *
 * This software is distributed in the hope that it will be useful,        *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with this package; see the file COPYING.  If not, write to        *
 * the Free Software Foundation, Inc., 51 Franklin Street - Fifth Floor,   *
 * Boston, MA 02110-1301, USA.                                             *
 ***************************************************************************/

#ifndef __WIRE_H
#define __WIRE_H

#include "item.h"
#include "wireline.h"

#include <QtCore/QList>

//!\brief forward declaration.
class SchematicScene;
class QGraphicsLineItem;
class QRubberBand;
class Port;

typedef QList<WireLine> WireLines;

/*!
 * \brief This class represents a wire on schematic.
 */
class Wire : public QucsItem
{
   public:
      enum { Type = QucsItem::WireType };

      //! A struct to store wire's details.
      struct Store {
            WireLines wLines;
            QPointF port1Pos;
            QPointF port2Pos;
      };

      Wire(const QPointF &startPos, const QPointF &endPos,
           SchematicScene *scene = 0);
      Wire(Port *startPort, Port *endPort, SchematicScene *scene = 0);
      ~Wire();

      //! Return's the wire's ports list.
      QList<Port*> ports() const { return m_ports; }
      //! Return's the list's first member.
      Port* port1() const { return m_ports[0]; }
      //! Returns the list's second member.
      Port* port2() const { return m_ports[1]; }

      void movePort(QList<Port*> *connections, const QPointF& scenePos);
      void movePort1(const QPointF& newLocalPos);
      void movePort2(const QPointF& newLocalPos);

      //! Wire identifier.
      int type() const { return Wire::Type; }

      void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                 QWidget *widget = 0);

      void beginGrabMode();
      void grabMoveBy(qreal dx, qreal dy);
      void endGrabMode();

      //! Returns the internal representation of wires as line's list.
      WireLines wireLines() const { return m_wLines; }
      void setWireLines(const WireLines& wirelines);

      void removeNullLines();

      void saveData(Qucs::XmlWriter *writer) const;
      void loadData(Qucs::XmlReader *reader);

      //! No rotate defined for wires.
      void rotate90(Qucs::AngleDirection dir = Qucs::AntiClockwise) {
         Q_UNUSED(dir);
      }

      //! No mirroring defined for wires.
      void mirrorAlong(Qt::Axis) {}

      void storeState();
      Store storedState() const;

      Store currentState() const;
      void setState(Store state);

      void checkAndConnect(bool pushUndoCommands = true);

   protected:
      void mousePressEvent(QGraphicsSceneMouseEvent *event);
      void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
      void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

   private:
      QRect proxyRect(const WireLine& wline) const;
      QRectF wireLineBound(const WireLine& wline) const;

      void updateProxyWires();
      void deleteProxyWires();

      void updateGeometry();

      int indexForPos(const QPointF& pos) const;

      int m_grabbedIndex; //!< Represents the index of wireline being dragged.

      QList<Port*> m_ports;//!< The ports of wires (always contain only 2 elements).
      WireLines m_wLines;//!< Internal line representation of wires.
      QList<QRubberBand*> m_proxyWires;//!< Represent wires while being dragged
      Wire::Store store; //!< Stores the wire data when needed(undo/redo).
};

#endif //__WIRE_H
