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

#include "debug.h" // always needs to be first include

#include "x11eventnotifier.h"

#include "x11input.h"
#include "x11inputdevice.h"

#include <KDE/KApplication>
#include <KDE/KDebug>

#include <QtGui/QX11Info>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>
#include <X11/extensions/Xrandr.h>

namespace Wacom
{
    class X11EventNotifierPrivate
    {
        public:
            Rotation currentRotation;
            bool     isStarted;
    };
}

using namespace Wacom;

X11EventNotifier::X11EventNotifier()
    : EventNotifier(NULL), d_ptr(new X11EventNotifierPrivate)
{
    Q_D( X11EventNotifier );
    d->currentRotation = 0;
    d->isStarted       = false;
}

X11EventNotifier::X11EventNotifier(const X11EventNotifier& notifier)
    : EventNotifier(NULL), d_ptr(new X11EventNotifierPrivate)
{
    Q_UNUSED(notifier);
    Q_D( X11EventNotifier );
    d->currentRotation = 0;
    d->isStarted       = false;
}

X11EventNotifier::~X11EventNotifier()
{
    delete d_ptr;
}


X11EventNotifier& X11EventNotifier::operator=(const X11EventNotifier& notifier)
{
    // nothing to do - this class is a singleton and must not be copied
    Q_UNUSED(notifier);
    return *this;
}



X11EventNotifier& X11EventNotifier::instance()
{
    static X11EventNotifier instance;
    return instance;
}




void X11EventNotifier::start()
{
    Q_D (X11EventNotifier);

    if (d->isStarted) {
        return;
    }

    if( KApplication::kApplication() != NULL ) {
        registerForNewDeviceEvent(QX11Info::display());
        KApplication::kApplication()->installX11EventFilter(this);
        d->isStarted = true;
    }
}



void X11EventNotifier::stop()
{
    Q_D (X11EventNotifier);

    if( KApplication::kApplication() != NULL ) {
        KApplication::kApplication()->removeX11EventFilter(this);
        d->isStarted = false;
    }
}



bool X11EventNotifier::x11Event(XEvent* event)
{
    XGenericEventCookie *cookie = &event->xcookie;

    if (cookie->type == GenericEvent && cookie->evtype == XI_HierarchyChanged) {
        handleX11InputEvent(event);

    } else {
        handleX11ScreenEvent(event);
    }

    return QWidget::x11Event(event);
}



void X11EventNotifier::handleX11InputEvent(XEvent* event)
{
    XGenericEventCookie *cookie       = &event->xcookie;
    bool                 ownEventData = XGetEventData(QX11Info::display(), cookie);

    if(cookie->data)
    {
        XIHierarchyEvent *hev  = (XIHierarchyEvent *)cookie->data;
        XIHierarchyInfo  *info = (XIHierarchyInfo *)hev->info;

        for (int i = 0; i < hev->num_info; i++)
        {
            if (info[i].flags & XISlaveRemoved) {
                kDebug() << QString::fromLatin1("X11 device with id '%1' removed.").arg(info[i].deviceid);
                emit tabletRemoved(info[i].deviceid);

            } else if (info[i].flags & XISlaveAdded) {
                kDebug() << QString::fromLatin1("X11 device with id '%1' added.").arg(info[i].deviceid);

                X11InputDevice device (QX11Info::display(), info[i].deviceid, QLatin1String("Unknown X11 Device"));

                if (device.isOpen() && device.isTabletDevice()) {
                    kDebug() << QString::fromLatin1("Wacom tablet device with X11 id '%1' added.").arg(info[i].deviceid);
                    emit tabletAdded(info[i].deviceid);
                }
            }
        }

        // only free event data if I own the resource if a different resource called XGetEventData the data will not be set free again here
        if(ownEventData) {
            XFreeEventData(QX11Info::display(), cookie);
        }
    }
    else {
        kDebug() << "Error couldn't retrieve XGetEventData";
    }
}



void X11EventNotifier::handleX11ScreenEvent(XEvent* event)
{
    Q_D( X11EventNotifier );

    int m_eventBase;
    int m_errorBase;

    XRRQueryExtension(QX11Info::display(), &m_eventBase, &m_errorBase);

    if (event->type == m_eventBase + RRScreenChangeNotify) {

        XRRUpdateConfiguration(event);
        Rotation old_r = d->currentRotation;

        XRRRotations(QX11Info::display(), DefaultScreen(QX11Info::display()), &(d->currentRotation));

        if (old_r != d->currentRotation) {
            ScreenRotation newRotation = ScreenRotation::NONE;

            switch (d->currentRotation) {
                    case RR_Rotate_0:
                        newRotation = ScreenRotation::NONE;
                        break;
                    case RR_Rotate_90:
                        newRotation = ScreenRotation::CCW;
                        break;
                    case RR_Rotate_180:
                        newRotation = ScreenRotation::HALF;
                        break;
                    case RR_Rotate_270:
                        newRotation = ScreenRotation::CW;
                        break;
                    default:
                        kError() << QString::fromLatin1("FIXME: Unsupported screen rotation '%1'.").arg(d->currentRotation);
                        return;
            }

            kDebug() << QString::fromLatin1("XRandr screen rotation detected: '%1'.").arg(newRotation.key());
            emit screenRotated(newRotation);
        }
    }
}



int X11EventNotifier::registerForNewDeviceEvent(Display* display)
{
    XIEventMask evmask;
    unsigned char mask[2] = { 0, 0 };

    XISetMask(mask, XI_HierarchyChanged);
    evmask.deviceid = XIAllDevices;
    evmask.mask_len = sizeof(mask);
    evmask.mask = mask;

    XISelectEvents(display, DefaultRootWindow(display), &evmask, 1);

    //register RandR events
    int rrmask = RRScreenChangeNotifyMask;

    XRRSelectInput(display, DefaultRootWindow(display), 0);
    XRRSelectInput(display, DefaultRootWindow(display), rrmask);

    return 0;
}
