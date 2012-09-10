/*
 * Copyright 2009, 2010 Jörg Ehrichs <joerg.ehichs@gmx.de>
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

#ifndef TABLETHANDLER_H
#define TABLETHANDLER_H

#include "tablethandlerinterface.h"
#include "tabletinformation.h"
#include "property.h"
#include "tabletrotation.h"

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>

namespace Wacom
{
class TabletProfile;
class TabletHandlerPrivate;

class TabletHandler : public TabletHandlerInterface
{
    Q_OBJECT

public:

    TabletHandler();
    virtual ~TabletHandler();

    /**
     * @see TabletHandler::getProperty(const QString&, const QString&) const
     */
    QString getProperty(const QString& device, const Property& property) const;


    /**
      * returns the current value for a specific tablet setting
      * This is forwarded to the right backend specified by m_curDevice
      *
      * @param device name of the tablet device we set. Internal name of the pad/stylus/eraser/cursor
      * @param param the parameter we are looking for
      *
      * @return the value as string
      */
    QString getProperty(const QString& device, const QString& param) const;


    /**
      * @brief Reads a list of all available profiles from the profile manager.
      *
      * @return the list of all available profiles
      */
    QStringList listProfiles() const;


    /**
      * @brief Applies a profile to the tablet device
      *
      * The profile must be known to the profile manager, otherwise a
      * notification error is displayed.
      *
      * @param profile The name of the profile to apply.
      */
    void setProfile(const QString& profile);


    /**
      * Sets the configuration of @p param from @p device with @p value
      * This is forwarded to the right backend specified by m_curDevice
      *
      * @param device   The name of the device to set the property on.
      * @param property The property key as returned by Property::key()
      * @param value    New value of the parameter
      */
    void setProperty(const QString & device, const QString& property, const QString & value);


    /**
     * @see TabletHandler::setProperty(const QString&, const QString&, const QString&)
     */
    void setProperty(const QString& device, const Property & property, const QString& value);


    /**
      * Toggles the stylus/eraser to absolute/relative mode
      */
    void togglePenMode();


    /**
      * Toggles the touch tool on/off
      */
    void toggleTouch();




public Q_SLOTS:
    /**
      * @brief Handles the connection of a new tablet device.
      * 
      * This slot has to be connected to the X device event notifier and
      * executed when a new tablet device is plugged in.
      *
      * @param deviceid The device id as reported by X11.
      */
    void onDeviceAdded(int deviceid);

    /**
      * @brief Handles the removal of a tablet device.
      *
      * This slot has to be connected to the X device event notifier and
      * executed when a tablet is disconnected from the system.
      *
      * @param deviceid The device id as reported by X11.
      */
    void onDeviceRemoved(int deviceid);

    /**
     * @brief Handles rotating the tablet.
     *
     * This slot has to be connected to the X event notifier and executed
     * when the screen is rotated.
     *
     * @param screenRotation The screen rotation.
     */
    void onScreenRotated(TabletRotation screenRotation);



Q_SIGNALS:
    /**
     * Emitted if the user should be notified.
     */
    void notify (const QString& eventId, const QString& title, const QString& message);


    /**
      * Emitted when the profile of the current tablet is changed.
      *
      * @param profile The name of the new active profile.
      */
    void profileChanged(const QString& profile);


    /**
      * Emitted when a new tablet is connected or if the currently active tablet changes.
      */
    void tabletAdded(const TabletInformation& info);


    /**
      * Emitted when the currently active tablet is removed.
      */
    void tabletRemoved();


private:

    /**
      * resets all device information
      */
    void clearTabletInformation();


    /**
      * Tablet Device detection. Finds a connected tablet via X11 and tries
      * to collect as much information as possible.
      *
      * @return @c true if detection worked
      *         @c false if a failure happened
      */
    bool detectTablet();


    /**
      * Sets the backend for the settings based on the name in the company list data file
      * Support is only available for xsetwacom at the moment, will hopefully change in the future
      *
      * @param backendName name of the used backend as specified in the device overview list
      */
    void selectDeviceBackend(const QString & backendName);


    Q_DECLARE_PRIVATE(TabletHandler)
    TabletHandlerPrivate *const d_ptr; /**< d-pointer for this class */

}; // CLASS
}  // NAMESPACE
#endif // HEADER PROTECTION
