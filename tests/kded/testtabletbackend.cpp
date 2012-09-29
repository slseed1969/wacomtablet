/*
 * Copyright 2012 Alexander Maret-Huskinson <alex@maret.de>
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

#include "testtabletbackend.moc"

#include "kdedtestutils.h"
#include "tabletinfo.h"
#include "tabletinformation.h"


using namespace Wacom;

void TestTabletBackend::initTestCase()
{
    m_tabletInformation.set (TabletInfo::CompanyId,   TabletInfo::CompanyId.key());
    m_tabletInformation.set (TabletInfo::CompanyName, TabletInfo::CompanyName.key());
    m_tabletInformation.set (TabletInfo::CursorName,  TabletInfo::CursorName.key());
    m_tabletInformation.set (TabletInfo::EraserName,  TabletInfo::EraserName.key());
    m_tabletInformation.set (TabletInfo::PadName,     TabletInfo::PadName.key());
    m_tabletInformation.set (TabletInfo::StylusName,  TabletInfo::StylusName.key());
    m_tabletInformation.set (TabletInfo::TabletId,    TabletInfo::TabletId.key());
    m_tabletInformation.set (TabletInfo::TabletModel, TabletInfo::TabletModel.key());
    m_tabletInformation.set (TabletInfo::TabletName,  TabletInfo::TabletName.key());
    m_tabletInformation.set (TabletInfo::TouchName,   TabletInfo::TouchName.key());
    m_tabletInformation.setAvailable(true);
    m_tabletInformation.setButtons(true);

    m_eraserXinputAdaptor    = new PropertyAdaptorMock<XinputProperty>();
    m_eraserXsetwacomAdaptor = new PropertyAdaptorMock<XsetwacomProperty>();

    m_padXinputAdaptor       = new PropertyAdaptorMock<XinputProperty>();
    m_padXsetwacomAdaptor    = new PropertyAdaptorMock<XsetwacomProperty>();

    m_stylusXinputAdaptor    = new PropertyAdaptorMock<XinputProperty>();
    m_stylusXsetwacomAdaptor = new PropertyAdaptorMock<XsetwacomProperty>();

    m_tabletBackend = new TabletBackend(m_tabletInformation);
    m_tabletBackend->addAdaptor(DeviceType::Eraser, m_eraserXsetwacomAdaptor);
    m_tabletBackend->addAdaptor(DeviceType::Eraser, m_eraserXinputAdaptor);
    m_tabletBackend->addAdaptor(DeviceType::Pad, m_padXinputAdaptor);
    m_tabletBackend->addAdaptor(DeviceType::Pad, m_padXsetwacomAdaptor);
    m_tabletBackend->addAdaptor(DeviceType::Stylus, m_stylusXsetwacomAdaptor);
    m_tabletBackend->addAdaptor(DeviceType::Stylus, m_stylusXinputAdaptor);
}



void TestTabletBackend::testGetInformation()
{
    TabletInformation tabletInformation = m_tabletBackend->getInformation();
    KdedTestUtils::assertTabletInformation(m_tabletInformation, tabletInformation);
}



void TestTabletBackend::testSetDeviceProfile()
{
    // cleanup
    m_eraserXinputAdaptor->m_properties.clear();
    m_eraserXsetwacomAdaptor->m_properties.clear();

    // create simple device profile
    DeviceProfile profile;

    profile.setProperty(Property::Button5, Property::Button5.key());
    profile.setProperty(Property::CursorAccelAdaptiveDeceleration, Property::CursorAccelAdaptiveDeceleration.key());

    m_tabletBackend->setProfile(DeviceType::Eraser, profile);

    // check values
    QVERIFY(m_eraserXsetwacomAdaptor->m_properties.contains(Property::Button5.key()));
    QVERIFY(!m_eraserXsetwacomAdaptor->m_properties.contains(Property::CursorAccelAdaptiveDeceleration.key()));
    QCOMPARE(Property::Button5.key(), m_eraserXsetwacomAdaptor->m_properties.value(Property::Button5.key()));
    
    QVERIFY(m_eraserXinputAdaptor->m_properties.contains(Property::CursorAccelAdaptiveDeceleration.key()));
    QVERIFY(!m_eraserXinputAdaptor->m_properties.contains(Property::Button5.key()));
    QCOMPARE(Property::CursorAccelAdaptiveDeceleration.key(), m_eraserXinputAdaptor->m_properties.value(Property::CursorAccelAdaptiveDeceleration.key()));
}



void TestTabletBackend::testSetProfile()
{
    // cleanup
    m_eraserXinputAdaptor->m_properties.clear();
    m_eraserXsetwacomAdaptor->m_properties.clear();
    m_stylusXinputAdaptor->m_properties.clear();
    m_stylusXsetwacomAdaptor->m_properties.clear();

    // create test data
    TabletProfile tabletProfile;
    DeviceProfile eraserProfile;
    DeviceProfile stylusProfile;

    eraserProfile.setDeviceType(DeviceType::Eraser);
    eraserProfile.setProperty(Property::Button8, Property::Button8.key());

    stylusProfile.setDeviceType(DeviceType::Stylus);
    stylusProfile.setProperty(Property::CursorAccelVelocityScaling, Property::CursorAccelVelocityScaling.key());

    tabletProfile.setName(QLatin1String("MyProfile"));
    tabletProfile.setDevice(eraserProfile);
    tabletProfile.setDevice(stylusProfile);

    m_tabletBackend->setProfile(tabletProfile);

    // check values
    QVERIFY(m_eraserXsetwacomAdaptor->m_properties.contains(Property::Button8.key()));
    QVERIFY(!m_eraserXinputAdaptor->m_properties.contains(Property::Button8.key()));
    QVERIFY(!m_stylusXinputAdaptor->m_properties.contains(Property::Button8.key()));
    QVERIFY(!m_stylusXsetwacomAdaptor->m_properties.contains(Property::Button8.key()));
    QCOMPARE(Property::Button8.key(), m_eraserXsetwacomAdaptor->m_properties.value(Property::Button8.key()));
    
    QVERIFY(m_stylusXinputAdaptor->m_properties.contains(Property::CursorAccelVelocityScaling.key()));
    QVERIFY(!m_stylusXsetwacomAdaptor->m_properties.contains(Property::CursorAccelVelocityScaling.key()));
    QVERIFY(!m_eraserXinputAdaptor->m_properties.contains(Property::CursorAccelVelocityScaling.key()));
    QVERIFY(!m_eraserXsetwacomAdaptor->m_properties.contains(Property::CursorAccelVelocityScaling.key()));
    QCOMPARE(Property::CursorAccelVelocityScaling.key(), m_stylusXinputAdaptor->m_properties.value(Property::CursorAccelVelocityScaling.key()));
}



void TestTabletBackend::testSetProperty()
{
    // cleanup
    m_eraserXinputAdaptor->m_properties.clear();
    m_eraserXsetwacomAdaptor->m_properties.clear();
    m_padXinputAdaptor->m_properties.clear();
    m_padXsetwacomAdaptor->m_properties.clear();
    m_stylusXinputAdaptor->m_properties.clear();
    m_stylusXsetwacomAdaptor->m_properties.clear();
    
    // setting a property on an unsupported device type should fail
    QVERIFY(!m_tabletBackend->setProperty(DeviceType::Touch, Property::Gesture, Property::Gesture.key()));

    // these properties should only be set on the XsetwacomAdaptorMock as they are unsupported by Xinput
    QVERIFY(m_tabletBackend->setProperty(DeviceType::Eraser, Property::Mode, Property::Mode.key()));
    QVERIFY(m_tabletBackend->setProperty(DeviceType::Stylus, Property::Mode, Property::Mode.key()));

    // make sure only the xsetwacom adaptor mocks contain the property which was just set
    QVERIFY(!m_eraserXinputAdaptor->m_properties.contains(Property::Mode.key()));
    QVERIFY(m_eraserXsetwacomAdaptor->m_properties.contains(Property::Mode.key()));
    QCOMPARE(Property::Mode.key(), m_eraserXsetwacomAdaptor->m_properties.value(Property::Mode.key()));

    QVERIFY(!m_stylusXinputAdaptor->m_properties.contains(Property::Mode.key()));
    QVERIFY(m_stylusXsetwacomAdaptor->m_properties.contains(Property::Mode.key()));
    QCOMPARE(Property::Mode.key(), m_stylusXsetwacomAdaptor->m_properties.value(Property::Mode.key()));

    QVERIFY(!m_padXinputAdaptor->m_properties.contains(Property::Mode.key()));
    QVERIFY(!m_padXsetwacomAdaptor->m_properties.contains(Property::Mode.key()));
    
    // set a value on both adaptors
    QVERIFY(m_tabletBackend->setProperty(DeviceType::Pad, Property::Button1, Property::Button1.key()));
    QVERIFY(m_tabletBackend->setProperty(DeviceType::Pad, Property::CursorAccelProfile, Property::CursorAccelProfile.key()));

    // make sure each adaptor only contains the property which belongs to him
    QVERIFY(!m_padXinputAdaptor->m_properties.contains(Property::Button1.key()));
    QVERIFY(!m_padXsetwacomAdaptor->m_properties.contains(Property::CursorAccelProfile.key()));
    QVERIFY(m_padXinputAdaptor->m_properties.contains(Property::CursorAccelProfile.key()));
    QVERIFY(m_padXsetwacomAdaptor->m_properties.contains(Property::Button1.key()));

    QCOMPARE(Property::CursorAccelProfile.key(), m_padXinputAdaptor->m_properties.value(Property::CursorAccelProfile.key()));
    QCOMPARE(Property::Button1.key(), m_padXsetwacomAdaptor->m_properties.value(Property::Button1.key()));


    // make sure the values we just set can be get again
    QCOMPARE(Property::Mode.key(), m_tabletBackend->getProperty(DeviceType::Eraser, Property::Mode));
    QCOMPARE(Property::Mode.key(), m_tabletBackend->getProperty(DeviceType::Stylus, Property::Mode));
    QCOMPARE(Property::Button1.key(), m_tabletBackend->getProperty(DeviceType::Pad, Property::Button1));
    QCOMPARE(Property::CursorAccelProfile.key(), m_tabletBackend->getProperty(DeviceType::Pad, Property::CursorAccelProfile));
}



void TestTabletBackend::cleanupTestCase()
{
    delete m_tabletBackend;
}
