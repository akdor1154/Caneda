/***************************************************************************
 * Copyright (C) 2013-2014 by Pablo Daniel Pareja Obregon                  *
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

#include "portsymbol.h"

#include "cgraphicsitem.h"
#include "propertygroup.h"
#include "settings.h"
#include "xmlutilities.h"

#include "dialogs/propertydialog.h"

#include <QStyleOptionGraphicsItem>

namespace Caneda
{
    /*!
     * \brief Constructs a port symbol item.
     *
     * \param scene CGraphicsScene on which the port symbol should be added.
     *
     * \sa init()
     */
    PortSymbol::PortSymbol(CGraphicsScene *scene) : CGraphicsItem(0, scene)
    {
        // Create a port with the default name
        init(QString("label"));
    }

    /*!
     * \brief Constructs a port symbol item.
     *
     * \param label Label of the port. Special ports as GND, should pass the
     *        corresponding label.
     * \param scene CGraphicsScene on which the port symbol should be added.
     *
     * \sa init()
     */
    PortSymbol::PortSymbol(const QString &label, CGraphicsScene *scene) : CGraphicsItem(0, scene)
    {
        init(label);
    }

    //! \brief Destructor.
    PortSymbol::~PortSymbol()
    {
        qDeleteAll(m_ports);
    }

    /*!
     * \brief Initialize the ports parameters
     *
     * This method initializes the main port parameters. It is done separately
     * from the contructors methods to allow reutilization of the code in
     * different existing contructors.
     *
     * \param label Label of the port. Special ports as GND, should pass the
     *        corresponding label.
     */
    void PortSymbol::init(const QString &label)
    {
        // Set component flags
        setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
        setFlag(ItemSendsGeometryChanges, true);
        setFlag(ItemSendsScenePositionChanges, true);

        // Create port properties and add port label
        Property _label("label", label, QObject::tr("Port Label"), true);
        properties = new PropertyGroup();
        properties->setParentItem(this);
        properties->addProperty("label", _label);
        properties->setTransform(transform().inverted());

        // Create ports
        m_ports << new Port(this, mapFromScene(pos()));

        updateGeometry();
    }

    //! \brief Draw port symbol
    void PortSymbol::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
    {
        // Save pen
        QPen savedPen = painter->pen();

        // Set global pen settings
        Settings *settings = Settings::instance();
        if(option->state & QStyle::State_Selected) {
            painter->setPen(QPen(settings->currentValue("gui/selectionColor").value<QColor>(),
                                 settings->currentValue("gui/lineWidth").toInt()));
        }
        else {
            painter->setPen(QPen(settings->currentValue("gui/lineColor").value<QColor>(),
                                 settings->currentValue("gui/lineWidth").toInt()));
        }

        // Draw the port symbol
        painter->drawPath(m_symbol);

        // Restore pen
        painter->setPen(savedPen);
    }

    //! \brief Updates the geometry of the port symbol.
    void PortSymbol::updateGeometry()
    {
        m_symbol = QPainterPath();

        // Define the port symbol acording to its label
        if(properties->propertyValue("label").toLower() == "ground" ||
                properties->propertyValue("label").toLower() == "gnd") {

            m_symbol.lineTo(0,10);
            m_symbol.moveTo(-10,10);
            m_symbol.lineTo(10,10);
            m_symbol.moveTo(-5,15);
            m_symbol.lineTo(5,15);
            m_symbol.moveTo(-2,20);
            m_symbol.lineTo(2,20);
        }
        else {
            m_symbol.addRoundRect(-10, -10, 20, 20, 25, 25);
        }

        properties->setPos(m_symbol.boundingRect().bottomLeft());

        QRectF boundRect = m_symbol.boundingRect();
        setShapeAndBoundRect(m_symbol, boundRect);

        update();
    }

    //! \brief Returns a copy of port symbol item parented to scene \a scene.
    PortSymbol* PortSymbol::copy(CGraphicsScene *scene) const
    {
        PortSymbol *portSymbol = new PortSymbol(scene);
        portSymbol->properties->setPropertyMap(properties->propertyMap());
        PortSymbol::copyDataTo(portSymbol);
        return portSymbol;
    }

    void PortSymbol::copyDataTo(PortSymbol *portSymbol) const
    {
        CGraphicsItem::copyDataTo(static_cast<CGraphicsItem*>(portSymbol));
        portSymbol->updateGeometry();
        portSymbol->update();
    }

    //! \copydoc CGraphicsItem::launchPropertyDialog()
    int PortSymbol::launchPropertyDialog(Caneda::UndoOption)
    {
        int retVal = properties->launchPropertyDialog();
        updateGeometry();

        return retVal;
    }

    /*!
     * \brief Convenience static method to load a PortSymbol saved as xml.
     *
     * This method loads a PortSymbol saved as xml. Once the PortSymbol is
     * created, its data is filled using the loadData() method.
     *
     * \param reader The xmlreader used to read xml data.
     * \param scene CGraphicsScene to which PortSymbol should be parented to.
     * \return Returns new PortSymbol pointer on success and null on failure.
     *
     * \sa loadData()
     */
    PortSymbol* PortSymbol::loadPortSymbol(Caneda::XmlReader *reader, CGraphicsScene *scene)
    {
        PortSymbol *retVal = new PortSymbol(scene);
        retVal->loadData(reader);

        return retVal;
    }

    //! \brief Saves data to xml \a writer.
    void PortSymbol::saveData(Caneda::XmlWriter *writer) const
    {
        writer->writeStartElement("port");

        writer->writeAttribute("name", properties->propertyValue("label"));
        writer->writePointAttribute(pos(), "pos");

        writer->writeEndElement(); // < /port>
    }

    //! \brief Loads portSymbol from xml \a reader.
    void PortSymbol::loadData(Caneda::XmlReader *reader)
    {
        Q_ASSERT(reader->isStartElement() && reader->name() == "port");

        QString label = reader->attributes().value("name").toString();
        properties->setPropertyValue("label", label);
        setPos(reader->readPointAttribute("pos"));
        updateGeometry();

        // Read until end of element
        reader->readUnknownElement();
    }

} // namespace Caneda
