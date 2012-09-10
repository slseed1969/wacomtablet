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

#ifndef TABLETINFORMATION_H
#define TABLETINFORMATION_H

#include "tabletinfo.h"
#include "devicetype.h"

#include <QtCore/QString>
#include <QtCore/QStringList>

namespace Wacom
{

/**
 * @brief Tablet information gathered by X11 or a tablet database.
 *
 * Contains all tablet information which could be gathered by either querying X11
 * or one of the tablet databases.
 * 
 * DO NOT USE THE MEMBERS OF THIS CLASS, USE THE METHODS! The mebers are only
 * public because D-Bus needs access to them (for now).
 * 
 * When extending this class, don't forget to update the DBusTabletInterface class as well!
 */
class TabletInformation
{
public:

    /**
     * Gets tablet information using the provided tablet information identifier.
     * If the identifier can not be mapped to a TabletInfo enumeration type, an
     * empty string will be returned.
     *
     * @param info The tablet information identifier.
     *
     * @return The requsted information or an empty string.
     *
     * @sa get(const TabletInfo&) const
     */
    const QString& get(const QString& info) const;

    
    /**
     * Gets tablet information.
     *
     * @param info The tablet information identifier.
     *
     * @return The requsted information.
     *
     * @sa get(const QString&) const
     */
    const QString& get(const TabletInfo& info) const;


    /**
     * Returns a list of Xinput device names this tablet supports.
     *
     * @return A list of Xinput device names.
     */
    const QStringList& getDeviceList() const;


    /**
     * Gets a Xinput device name based on the provided device type
     * identifier string. If the device type identifier string can not
     * be resolved to a device type enumeration type, an empty string
     * is returned.
     *
     * @param device The device type to get.
     * 
     * @return The requested device name if set or an empty string.
     *
     * @sa getDeviceName(const DeviceType&) const
     */
    const QString& getDeviceName(const QString& device) const;


    /**
     * Gets a Xinput device name of this tablet.
     *
     * @param device The type of device to get the name from.
     *
     * @return The Xinput device name if set.
     *
     * @sa getDeviceName(const QString&) const
     */
    const QString& getDeviceName(const DeviceType& device) const;


    /**
     * Flag which signals if the pad has hardware buttons.
     *
     * @return True if the tablet pad has hardware buttons, else false.
     */
    bool hasButtons() const;


    /**
     * Checks if the tablet has a given device.
     *
     * @param device The device to check for.
     */
    bool hasDevice(const DeviceType& device) const;


    /**
     * @deprecated Do no longer use this!
     * 
     * Flag if this tablet is available.
     *
     * @return True if the tablet is available, else false.
     */
    bool isAvailable() const;


    /**
     * Sets tablet information based on the given tablet information identifier.
     *
     * @param info  The tablet information identifier.
     * @param value The new value.
     */
    void set (const TabletInfo& info, const QString& value);


    /**
     * @deprecated Do no longer use the available flag!
     *
     * Sets the available-flag.
     *
     * @param value The new value.
     */
    void setAvailable(bool value);


    /**
     * Sets the device list of this tablet.
     *
     * @param list The new device list.
     */
    void setDeviceList(const QStringList& list);


    /**
     * Sets a device name of this tablet.
     *
     * @param device The device name to set.
     * @param name   The new name.
     */
    void setDeviceName(const DeviceType& device, const QString& name);


    /**
     * Sets the flag which signals hardware buttons.
     *
     * @param value The new value.
     */
    void setButtons(bool value);


    /*
     * Do not use these members directly any more! Use the getters/setters!
     * These members should only be used by the D-Bus interface to build the structure
     * returned by getInformation().
     */
    QString     unknown;           //!< A dummy member so we can safely return a const reference.
    QString     xdeviceId;         //!< Xinput device identifier.
    QString     companyId;         //!< The company identifier.
    QString     companyName;       //!< The name of the company who built this device.
    QString     tabletId;          //!< The tablet identifier as four digit hex code.
    QString     tabletName;        //!< The name of the tablet.
    QString     tabletModel;       //!< The tablet model.
    QStringList deviceList;        //!< The device names of all devices this tablet consists of.
    QString     padName;           //!< The name of the pad device, if available.
    QString     stylusName;        //!< The name of the stylus device, if available.
    QString     eraserName;        //!< The name of the eraser device, if available.
    QString     cursorName;        //!< The name of the cursor device, if available.
    QString     touchName;         //!< The name of the touch device, if available.
    bool        isTabletAvailable; //!< A flag to signal if this tablet is available.
    bool        hasPadButtons;     //!< A flag to signal if this tablet has pad buttons.

}; // CLASS
}  // NAMESPACE
#endif // HEADER PROTECTION