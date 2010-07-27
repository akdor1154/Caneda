/***************************************************************************
 * Copyright (C) 2010 by Pablo Daniel Pareja Obregon                       *
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

#include "webcontext.h"

#include "singletonowner.h"
#include "webdocument.h"
#include "webview.h"

#include <QFileInfo>
#include <QStringList>

namespace Caneda
{
    WebContext::WebContext(QObject *parent) : IContext(parent)
    {
    }

    WebContext* WebContext::instance()
    {
        static WebContext *instance = 0;
        if (!instance) {
            instance = new WebContext(SingletonOwner::instance());
        }

        return instance;
    }

    WebContext::~WebContext()
    {
    }

    void WebContext::init()
    {
    }

    bool WebContext::canOpen(const QFileInfo& info) const
    {
        QStringList supportedSuffixes;
        supportedSuffixes << "htm";
        supportedSuffixes << "html";

        foreach (const QString &suffix, supportedSuffixes) {
            if (suffix == info.suffix()) {
                return true;
            }
        }

        return false;
    }

    QStringList WebContext::fileNameFilters() const
    {
        return QStringList();
    }

    IDocument* WebContext::newDocument()
    {
        return new WebDocument;
    }

    IDocument* WebContext::open(const QString& fileName, QString *errorMessage)
    {
        WebDocument *document = new WebDocument();
        document->setFileName(fileName);

        if (!document->load(errorMessage)) {
            delete document;
            document = 0;
        }

        return document;
    }

} // namespace Caneda