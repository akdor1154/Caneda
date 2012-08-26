/***************************************************************************
 * Copyright (C) 2009-2012 by Pablo Daniel Pareja Obregon                  *
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

#include "xmlsymbol.h"

#include "cgraphicsscene.h"
#include "global.h"
#include "library.h"
#include "symboldocument.h"

#include "paintings/painting.h"

#include "xmlutilities/transformers.h"
#include "xmlutilities/xmlutilities.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QString>

namespace Caneda
{
    //! Constructor
    XmlSymbol::XmlSymbol(SymbolDocument *doc) :
        m_symbolDocument(doc)
    {
    }

    bool XmlSymbol::save()
    {
        CGraphicsScene *scene = cGraphicsScene();
        if(!scene) {
            return false;
        }

        QString text = saveText();

        if(text.isEmpty()) {
            qDebug("Looks buggy! Null data to save! Was this expected?");
        }

        QFile file(fileName());
        if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(0, QObject::tr("Error"),
                    QObject::tr("Cannot save document!"));
            return false;
        }
        QTextStream stream(&file);
        stream << text;
        file.close();

        return true;
    }

    bool XmlSymbol::load()
    {
        CGraphicsScene *scene = cGraphicsScene();
        if(!scene) {
            return false;
        }

        QFile file(fileName());
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::critical(0, QObject::tr("Error"),
                    QObject::tr("Cannot load document ")+fileName());
            return false;
        }

        QTextStream stream(&file);

        bool result = loadFromText(stream.readAll());

        file.close();
        return result;
    }

    /*!
     * \brief Reads component data from component description xml file.
     *
     * \param reader XmlReader responsible for reading xml data.
     * \param path The path of the xml file being processed.
     * \param d (Output variable) The data ptr where data should be uploaded.
     */
    bool XmlSymbol::readComponentData(Caneda::XmlReader *reader, const QString& path,
                                      QSharedDataPointer<ComponentData> &d)
    {
        QXmlStreamAttributes attributes = reader->attributes();

        Q_ASSERT(reader->isStartElement() && reader->name() == "component");

        // Check version compatibility first.
        Q_ASSERT(Caneda::checkVersion(attributes.value("version").toString()));

        // Get name
        d->name = attributes.value("name").toString();
        Q_ASSERT(!d->name.isEmpty());

        // Get label
        d->labelPrefix = attributes.value("label").toString();
        Q_ASSERT(!d->labelPrefix.isEmpty());

        // Read the component body
        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isEndElement()) {
                break;
            }

            if(reader->isStartElement()) {
                // Read display text
                if(reader->name() == "displaytext") {
                    d->displayText = reader->readLocaleText(Caneda::localePrefix());
                    Q_ASSERT(reader->isEndElement());
                }

                // Read description
                else if(reader->name() == "description") {
                    d->description = reader->readLocaleText(Caneda::localePrefix());
                    Q_ASSERT(reader->isEndElement());
                }

                // Read schematic
                else if(reader->name() == "schematics") {
                    if(readSchematics(reader, path, d)==false) {
                        d = static_cast<ComponentData*>(0);
                        return false;
                    }
                }

                // Read ports
                else if(reader->name() == "ports") {
                    while(!reader->atEnd()) {
                        reader->readNext();

                        if(reader->isEndElement()) {
                            Q_ASSERT(reader->name() == "ports");
                            break;
                        }

                        if(reader->isStartElement() && reader->name() == "port") {
                            //TODO: Read ports information
                            reader->readFurther();
                            Q_ASSERT(reader->isEndElement() && reader->name() == "port");
                        }
                    }
                }

                // Read properties
                else if(reader->name() == "properties") {
                    readComponentProperties(reader,d);
                }

                else {
                    reader->readUnknownElement();
                }
            }
        }

        if(reader->hasError()) {
            d = static_cast<ComponentData*>(0);
            return false;
        }
        return true;
    }

    SymbolDocument* XmlSymbol::symbolDocument() const
    {
        return m_symbolDocument;
    }

    CGraphicsScene* XmlSymbol::cGraphicsScene() const
    {
        return m_symbolDocument ? m_symbolDocument->cGraphicsScene() : 0;
    }

    QString XmlSymbol::fileName() const
    {
        return m_symbolDocument ? m_symbolDocument->fileName() : QString();
    }

    QString XmlSymbol::saveText()
    {
        QString retVal;
        Caneda::XmlWriter *writer = new Caneda::XmlWriter(&retVal);
        writer->setAutoFormatting(true);

        //Fist we start the document and write current version
        writer->writeStartDocument();
        writer->writeDTD(QString("<!DOCTYPE caneda>"));
        writer->writeStartElement("caneda");
        writer->writeAttribute("version", Caneda::version());

        //Write all view details
        writer->writeStartElement("component");
        QFileInfo info(fileName());
        writer->writeAttribute("name", info.baseName());
        writer->writeAttribute("label", "comp");

        writer->writeStartElement("displaytext");
        writer->writeLocaleText("C", "User created component");
        //   TODO: When available use this to save user defined displaytext
        //   writer->writeLocaleText("C", scene->displayText());
        writer->writeEndElement(); //</displaytext>

        writer->writeStartElement("description");
        writer->writeLocaleText("C", "User created component based on user symbol");
        //   TODO: When available use this to save user defined description
        //   writer->writeLocaleText("C", scene->description());
        writer->writeEndElement(); //</description>

        writer->writeStartElement("symbol");
        writer->writeAttribute("name", "userdefined");


        //Now we copy all the elements and properties in the schematic
        QList<QGraphicsItem*> items = cGraphicsScene()->items();
        QList<Painting*> paintings = filterItems<Painting>(items, RemoveItems);
        if(!paintings.isEmpty()) {
            foreach(Painting *p, paintings) {
                p->saveData(writer);
            }
        }
        //Finally we finish the document
        writer->writeEndElement(); //</symbol>

        //TODO Write properties
        writer->writeStartElement("properties");
        writer->writeEndElement(); //</properties>

        writer->writeEndDocument(); //</caneda>

        delete writer;
        return retVal;
    }

    bool XmlSymbol::loadFromText(const QString& text)
    {
        Caneda::XmlReader *reader = new Caneda::XmlReader(text.toUtf8());
        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isStartElement()) {
                if(reader->name() == "caneda" &&
                        Caneda::checkVersion(reader->attributes().value("version").toString())) {

                    while(!reader->atEnd()) {
                        reader->readNext();
                        if(reader->isEndElement()) {
                            Q_ASSERT(reader->name() == "caneda");
                            break;
                        }

                        reader->readNext();

                        if(!reader->isStartElement() || reader->name() != "component") {
                            reader->raiseError(QObject::tr("Malformatted file"));
                            break;
                        }

                        while(!reader->atEnd()) {
                            reader->readNext();
                            if(reader->isEndElement()) {
                                Q_ASSERT(reader->name() == "component");
                                break;
                            }
                            reader->readNext();

                            if(reader->name() == "displaytext") {
                                // TODO: Implement this.
                                reader->readUnknownElement();
                            }
                            else if(reader->name() == "description") {
                                // TODO: Implement this.
                                reader->readUnknownElement();
                            }
                            else if(reader->name() == "symbol") {

                                while(!reader->atEnd()) {
                                    reader->readNext();

                                    if(reader->isEndElement()) {
                                        Q_ASSERT(reader->name() == "symbol");
                                        break;
                                    }

                                    if(reader->isStartElement()) {
                                        if(reader->name() == "painting") {
                                            Painting::loadPainting(reader, cGraphicsScene());
                                        }
                                        else {
                                            qWarning() << "Error: Found unknown painting type" <<
                                                          reader->name().toString();
                                            reader->readUnknownElement();
                                            reader->raiseError(QObject::tr("Malformatted file"));
                                        }
                                    }
                                }

                            }
                            else if(reader->name() == "properties") {
                                // TODO: Implement this.
                                reader->readUnknownElement();
                            }
                            else {
                                reader->readUnknownElement();
                            }
                        }
                    }
                }
                else {
                    reader->raiseError(QObject::tr("Not a caneda file or probably malformatted file"));
                }
            }
        }

        if(reader->hasError()) {
            QMessageBox::critical(0, QObject::tr("Xml parse error"), reader->errorString());
            delete reader;
            return false;
        }

        delete reader;
        return true;
    }

    /*!
     * \brief Read schematics section of component description xml file
     *
     * \param reader XmlReader responsible for reading xml data.
     * \param path The path of the xml file being processed.
     * \param d (Output variable) The data ptr where data should be uploaded.
     */
    bool XmlSymbol::readSchematics(Caneda::XmlReader *reader, const QString& svgPath,
                                   QSharedDataPointer<ComponentData> &d)
    {
        /* list of symbols */
        QStringList parsedSymbols;

        /* get default value */
        QString defaultSchematic =
            reader->attributes().value("default").toString();

        Q_ASSERT(reader->isStartElement() && reader->name() == "schematics");
        Q_ASSERT(!defaultSchematic.isEmpty());

        /* read different schematic */
        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isEndElement()) {
                break;
            }

            if(reader->isStartElement()) {
                if(reader->name() == "schematic") {
                    QString schName =
                        reader->attributes().value("name").toString();

                    Q_ASSERT(!schName.isEmpty());

                    parsedSymbols << schName;
                    if(!readSchematic(reader, svgPath, d)) {
                        return false;
                    }
                }
                else {
                    Q_ASSERT(!sizeof("Unknown element in schematics element"));
                }
            }
        }

        /* check if default is present */
        Q_ASSERT(parsedSymbols.contains(defaultSchematic));

        /* add symbols to property list */
        QString symbolDescription =
            QObject::tr("Represents the current symbol of component.");
        QVariant defValue(defaultSchematic);
        Q_ASSERT(defValue.convert(QVariant::String));
        Property symb("symbol", symbolDescription, QVariant::String, false,
                false, defValue, parsedSymbols);
        d->propertyMap.insert("symbol", symb);

        return true;
    }

    /*!
     * \brief Reads component properties data from component description xml file.
     *
     * \param reader XmlReader responsible for reading xml data.
     * \param d (Output variable) The data ptr where data should be uploaded.
     */
    void XmlSymbol::readComponentProperties(Caneda::XmlReader *reader,
                                            QSharedDataPointer<ComponentData> &d)
    {
        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isEndElement()) {
                break;
            }

            else if(reader->isStartElement()) {
                if(reader->name() == "property") {
                    Property prop = PropertyFactory::createProperty(reader);
                    d->propertyMap.insert(prop.name(), prop);
                }
                /* default */
                else {
                    Q_ASSERT(!sizeof("unknow element in properties element"));
                }
            }
        }
    }

    /*!
     * \brief Reads the schematic tag of component description xml file.
     *
     * \param reader XmlReader responsible for reading xml data.
     * \param path The path of the xml file being processed.
     * \param d (Output variable) The data ptr where data should be uploaded.
     */
    bool XmlSymbol::readSchematic(Caneda::XmlReader *reader, const QString& svgPath,
                                  QSharedDataPointer<ComponentData> &d)
    {
        Q_ASSERT(reader->isStartElement() && reader->name() == "schematic");

        QString schName = reader->attributes().value("name").toString();
        QString schType = reader->attributes().value("href").toString();
        bool readok;

        // If external svg file
        if(!schType.isEmpty()) {
            QFile svgFile(svgPath + "/" + schType);
            if(!svgFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                return false;
            }

            QByteArray svgContent(svgFile.readAll());
            if(svgContent.isEmpty()) {
                return false;
            }

            readok = readSchematicSvg(svgContent, schName, d);
            if(!readok) {
                return false;
            }
        }

        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isEndElement()) {
                break;
            }

            if(reader->isStartElement()) {
                // Internal svg
                if(reader->name() == "svg") {
                    Q_ASSERT(schType.isEmpty());
                    QByteArray svgContent = reader->readXmlFragment().toUtf8();
                    // TODO: return error
                    readok = readSchematicSvg(svgContent, schName, d);
                    if(!readok) {
                        return false;
                    }
                }
                else if(reader->name() == "port") {
                    readSchematicPort(reader, schName, d);
                }
                else {
                    Q_ASSERT(!sizeof("Unknown element in schematic element"));
                }
            }
        }
        return true;
    }

    /*!
     * \brief Reads the schematic port tag of component description xml file.
     *
     * \param reader XmlReader responsible for reading xml data.
     * \param schName Schematic name
     * \param d (Output variable) The data ptr where data should be uploaded.
     */
    void XmlSymbol::readSchematicPort(Caneda::XmlReader *reader, const QString & schName,
                                      QSharedDataPointer<ComponentData> &d)
    {
        Q_ASSERT(reader->isStartElement());
        QXmlStreamAttributes attribs = reader->attributes();
        bool ok;

        qreal x = attribs.value("x").toString().toDouble(&ok);
        Q_ASSERT(ok);

        qreal y = attribs.value("y").toString().toDouble(&ok);
        Q_ASSERT(ok);

        QString portName = attribs.value("name").toString();
        d->schematicPortMap[schName] <<
            new PortData(QPointF(x, y), portName);

        // Read until end element as all data we require is already obtained.
        reader->readUnknownElement();
    }

    /*!
     * \brief Read an svg schematic
     *
     * \param svgContent svg content as utf8
     * \param schName Schematic name
     * \param d (Output variable) The data ptr where data should be uploaded.
     * \todo Check errors
     */
    bool XmlSymbol::readSchematicSvg(const QByteArray &svgContent,
                                     const QString &schName,
                                     QSharedDataPointer<ComponentData> &d)
    {
        // Process using xslt
        Caneda::QXmlStreamReaderExt QXmlSvg(svgContent, 0,
                Caneda::transformers::defaultInstance()->componentsvg());

        LibraryManager *libraryManager = LibraryManager::instance();
        QString symbolId = d.constData()->name + "/" + schName;
        libraryManager->registerComponent(symbolId, QXmlSvg.constData());
        if(QXmlSvg.hasError()) {
            qWarning() << "Could not read svg file" << schName << ": " << QXmlSvg.errorString();
            return false;
        }

        return true;;
    }


} // namespace Caneda
