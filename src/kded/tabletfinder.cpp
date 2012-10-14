/*
 * This file is part of the KDE wacomtablet project. For copyright
 * information and license terms see the AUTHORS and COPYING files
 * in the top-level directory of this distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "debug.h"
#include "tabletfinder.h"
#include "tabletdatabase.h"
#include "x11tabletfinder.h"

#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>

using namespace Wacom;

namespace Wacom
{
    class TabletFinderPrivate 
    {
        public:
            typedef QList<TabletInformation> TabletInformationList;

            TabletInformationList tabletList;

    }; // CLASS
} // NAMESPACE


TabletFinder::TabletFinder() : QObject(NULL), d_ptr(new TabletFinderPrivate)
{
}

TabletFinder::TabletFinder(const TabletFinder& finder) : QObject(NULL), d_ptr(new TabletFinderPrivate)
{
    // nothing to do - this class is a singleton and must not be copied

    // prevent compiler warning about unused parameter at least for debug builds.
    assert(&finder != NULL); 
}

TabletFinder::~TabletFinder()
{
    delete d_ptr;
}

TabletFinder& TabletFinder::operator=(const TabletFinder& finder)
{
    // nothing to do - this class is a singleton and must not be copied

    // prevent compiler warning about unused parameter at least for debug builds.
    assert(&finder != NULL);
    return *this;
}



TabletFinder& TabletFinder::instance()
{
    static TabletFinder instance;
    return instance;
}



bool TabletFinder::scan()
{
    Q_D(TabletFinder);

    X11TabletFinder       x11tabletFinder;
    QMap<QString,QString> buttonMap;

    if (x11tabletFinder.scanDevices()) {
        d->tabletList = x11tabletFinder.getTablets();

        TabletFinderPrivate::TabletInformationList::Iterator iter;

        for (iter = d->tabletList.begin() ; iter != d->tabletList.end() ; ++iter) {
            // lookup device information and button map
            lookupInformation(*iter);

            // emit tablet added signal
            emit tabletAdded(*iter);
        }

        return true;
    }

    return false;
}



void TabletFinder::onX11TabletAdded(int deviceId)
{
    Q_D(TabletFinder);

    // check if we already know this tablet
    for (int i = 0 ; i < d->tabletList.size() ; ++i) {
        if (d->tabletList.at(i).hasDevice(deviceId)) {
            // we already know this tablet
            return;
        }
    }

    // scan for tablet devices
    X11TabletFinder x11TabletFinder;

    if (!x11TabletFinder.scanDevices()) {
        return;
    }

    // check if the device id can be found
    foreach (const TabletInformation& info, x11TabletFinder.getTablets()) {
        if (info.hasDevice(deviceId)) {
            // tablet found - lookup additional information
            TabletInformation tabletInfo = info;
            lookupInformation(tabletInfo);

            // add tablet to the list of known tablets and emit added signal
            d->tabletList.append(tabletInfo);
            emit tabletAdded(tabletInfo);
            return;
        }
    }
}



void TabletFinder::onX11TabletRemoved(int deviceId)
{
    Q_D(TabletFinder);

    // check if we know this tablet
    TabletFinderPrivate::TabletInformationList::iterator iter;

    for (iter = d->tabletList.begin() ; iter != d->tabletList.end() ; ++iter) {
        if (iter->hasDevice(deviceId)) {
            TabletInformation info = *iter;
            d->tabletList.erase(iter);
            emit tabletRemoved(info);
            return;
        }
    }
}



bool TabletFinder::lookupInformation(TabletInformation& info)
{
    TabletDatabase        tabletDatabase;
    QMap<QString,QString> buttonMap;

    if (!tabletDatabase.lookupDevice(info, info.get (TabletInfo::TabletId))) {
        kDebug() << "Could not find device in database: " << info.get (TabletInfo::TabletId);
        return false;
    }

    if (tabletDatabase.lookupButtonMapping(buttonMap, info.get (TabletInfo::CompanyId), info.get (TabletInfo::TabletId))) {
        info.setButtonMap(buttonMap);
    }

    return true;
}

