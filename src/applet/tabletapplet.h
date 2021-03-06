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

#ifndef TABLETAPPLET_H
#define TABLETAPPLET_H

#include "tabletinformation.h"

//Qt include
#include <QtCore/QObject>
#include <QtCore/QString>

class QGraphicsWidget;

namespace Wacom
{
class WacomTabletSettings;
class TabletAppletPrivate;

/**
  * This class provides the plasma applet with the widget content.
  *
  * It handles the signal/slot communication and shows either the details for the tablet config
  * and thus a list of all available profiles together with the small possibility to change the pad rotation and
  * pen mode (absolute/relative) or just a simple error message if no tablet is connected or the daemon is not running in the background.
  */
class TabletApplet : public QObject
{
    Q_OBJECT
public:
    /**
      * Creates a new TabletApplet.
      *
      * @param tabletSettings the parent widget
      */
    explicit TabletApplet(WacomTabletSettings *tabletSettings);

    /**
      * Destroy this applet
      */
    ~TabletApplet();

    /**
      * Returns the related QGraphicsWidget.
      *
      * @return the widget created with buildDialog()
      */
    QGraphicsWidget *dialog();

    /**
      * Shows an error icon and message when no tablet is connected or a DBus failure has occurred.
      *
      * @param msg the massage shown in the applet
      */
    void showError(const QString& title, const QString & msg);

    /**
      * Shows the widget with the detail information again.
      * Used to hide the error message again.
      */
    void showApplet();

public Q_SLOTS:
    /**
     * Connectes the DBus interfaces for the /Tablet and /Device interface
     * Prints an error message when one of the interface is not valid
     */
    void onDBusConnected();

    /**
      * Shows an error that the D-Bus interface is not available.
      */
    void onDBusDisconnected();

    /**
      * Updates the widget content and internal values.
      *
      * Called when the tablet daemon detects that a new tablet was added or
      * the existing tablet has been removed.
      */
    void updateWidget();

    /**
      * Updates the list off all available profiles.
      *
      * Called when the tablet device changes or when a profile is created/deleted
      * via the kcmodule.
      */
    void updateProfile();

    /**
      * Selects the current used profile in the KComboBox.
      *
      * Called via DBus from the tablet daemon to change the current used profile
      *
      * @param name the current used profile name
      */
    void setProfile(const QString & name);

    /**
      * Tells the tablet daemon to switch the profile via DBus
      *
      * Called when a new profile is selected via the KCombobox
      *
      * @param name the selected profile name
      */
    void switchProfile(const QString & name);

    void switchTablet(const QString &tablet);

    /**
      * Rotates the tablet pad back to normal
      *
      * Calls the daemon via DBus to change the pad rotation
      */
    void rotateNorm();

    /**
      * Rotates the tablet pad clockwise
      *
      * Calls the daemon via DBus to change the pad rotation
      */
    void rotateCw();

    /**
      * Rotates the tablet pad counter clockwise
      *
      * Calls the daemon via DBus to change the pad rotation
      */
    void rotateCcw();

    /**
      * Rotates the tablet pad 180°
      *
      * Calls the daemon via DBus to change the pad rotation
      */
    void rotateHalf();

    /**
      * Sets the stylus and eraser mode to absolute
      *
      * Called when selected from the Radiobuttons
      *
      * @param state state of the radiobutton when clicked
      */
    void selectAbsoluteMode(bool state);

    /**
      * Sets the stylus and eraser mode to relative
      *
      * Called when selected from the Radiobuttons
      *
      * @param state state of the radiobutton when clicked
      */
    void selectRelativeMode(bool state);

    /**
      * Sets the Touch mode on
      *
      * @param state if true touch i eneabled, if false touch is disabled
      */
    void setTouchModeOn(bool state);

    /**
      * Sets the Touch mode off
      *
      * @param state if true touch i eneabled, if false touch is disabled
      */
    void setTouchModeOff(bool state);

    /**
      * Called via DBus from the daemon when Solid detects a new tablet.
      *
      * Updates the @c m_applet TabletApplet with the new values.
      */
    void onTabletAdded(const QString &tabletId);

    /**
      * Called via DBus from the daemon when Solid detects that the connected tablet has been removed.
      *
      * Shows an error messages in the @c m_applet TabletApplet to indicate the change.
      */
    void onTabletRemoved(const QString &tabletId);

private:
    void showNoTabletWidget();

    /**
      * Build the basic frame of the applet.
      *
      * This contains just the title and another widget as placeholder for the
      * info and error dialog. This way around it is easily possible to change the content
      * of the applet based on the current tablet detection.
      *
      * @see buildInfoDialog()
      * @see buildErrorDialog()
      */
    void buildDialog();

    /**
      * This part of the dialogue creates the actual applet.
      *
      * The profile selection box as well as a small groupbox with the pad rotation push buttons
      * and pen mode radio button.
      */
    void buildConfigDialog();

    /**
      * Creates the widget for the error message.
      *
      * @see showError()
      */
    void buildErrorDialog();


    Q_DECLARE_PRIVATE (TabletApplet)
    TabletAppletPrivate* const d_ptr;

}; // CLASS
}  // NAMESPACE
#endif // HEADER PROTECTION
