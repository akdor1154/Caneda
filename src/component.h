/***************************************************************************
 * Copyright (C) 2007 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2012 by Pablo Daniel Pareja Obregon                       *
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

#ifndef QCOMPONENT_H
#define QCOMPONENT_H

#include "cgraphicsitem.h"
#include "property.h"

namespace Caneda
{
    // Forward declarations
    class PropertiesGroup;
    class PortData;

    //! Shareable component's data
    struct ComponentData : public QSharedData
    {
        ComponentData() {}

        QString name;
        QString filename;
        QString labelPrefix;
        QString displayText;
        QString description;
        QString library;
        QList<PortData*> ports; //! List of component's ports
        PropertyMap propertyMap; //! Properties the user can modify (in the properties dialog)
    };

    typedef QSharedDataPointer<ComponentData> ComponentDataPtr;

    /*!
     * \brief Represents the component on schematic.
     *
     * This component can either be directly loaded from an xml doc or
     * manually set data if it requires.
     *
     * This class uses ComponentsCache as a cache to render the component.
     * To be able to render the symbol registered to ComponentsCache, the
     * symbol must be previously registered (by the library class).
     */
    class Component : public QObject, public CGraphicsItem
    {
        Q_OBJECT

    public:
        enum { Type = CGraphicsItem::ComponentType };

        Component(CGraphicsScene *scene = 0);
        Component(const QSharedDataPointer<ComponentData>& other,
                  CGraphicsScene *scene = 0);
        ~Component();

        //! Used for component identification at runtime.
        int type() const { return Component::Type; }

        //! Returns name of the component.
        QString name() const { return d->name; }

        //! Returns label prefix of component.
        QString labelPrefix() const { return d->labelPrefix; }
        QString labelSuffix() const;

        //! Represents model of component, which is infact a property.
        QString model() const { return property("model"); }

        //! Returns string to be displayed in sidebar.
        QString displayText() const { return d->displayText; }

        //! Returns a helpful text corresponding to component.
        QString description() const { return d->description; }

        //! Returns the library to which this component belongs.
        QString library() const { return d->library; }

        //! Returns the property map (actually copy of property map).
        PropertyMap propertyMap() const { return d->propertyMap; }
        void setPropertyMap(const PropertyMap& propMap);

        //! Returns property group of the component.
        PropertiesGroup* propertyGroup() const { return m_propertyGroup; }
        void updatePropertyGroup();
        void createPropertyGroup();

        QString property(const QString& propName) const;
        bool setProperty(const QString& propName, const QString &value);
        void setPropertyVisible(const QString& propName, bool visibility);

        //! Returns the label of the component in the form {label_prefix}{number_suffix}
        QString label() const { return property("label"); }
        bool setLabel(const QString& _label);

        static Component* loadComponent(Caneda::XmlReader *reader, CGraphicsScene *scene);
        void loadData(Caneda::XmlReader *reader);
        void saveData(Caneda::XmlWriter *writer) const;

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *);

        Component *copy(CGraphicsScene *scene = 0) const;
        void copyDataTo(Component *comp) const;

        //! \reimp Reimplemented to return rtti info.
        bool isComponent() const { return true; }
        //! \reimp Reimplemented to return rtti info.
        bool isWire() const { return false; }

        int launchPropertyDialog(Caneda::UndoOption opt);

    protected:
        QRectF adjustedBoundRect(const QRectF& rect);
        QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    public slots:
        void updateBoundingRect();

    private:
        void init();

        //! \brief Component Shared datas
        QSharedDataPointer<ComponentData> d;
        //! \brief Property group (ie properties display of this component)
        PropertiesGroup *m_propertyGroup;
    };

} // namespace Caneda

#endif //COMPONENT_H
