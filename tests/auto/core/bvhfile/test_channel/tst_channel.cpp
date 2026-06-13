#include <QtTest/QtTest>
#include "channel.h"

class TestChannel : public QObject {
    Q_OBJECT

private slots:
    void testDefaultConstruction();
    void testConstructionWithName();
    void testNameToType();
    void testTypeToName();
    void testChannelValue();
};

void TestChannel::testDefaultConstruction()
{
    Channel ch;
    QCOMPARE(ch.name, QString(""));
    QCOMPARE(ch.type, Channel::Unknown);
    QCOMPARE(ch.value, 0.0);
}

void TestChannel::testConstructionWithName()
{
    Channel ch("Xposition");
    QCOMPARE(ch.name, QString("Xposition"));
    QCOMPARE(ch.type, Channel::Xposition);
    QCOMPARE(ch.value, 0.0);
}

void TestChannel::testNameToType()
{
    QCOMPARE(Channel::nameToType("Xposition"), Channel::Xposition);
    QCOMPARE(Channel::nameToType("Yposition"), Channel::Yposition);
    QCOMPARE(Channel::nameToType("Zposition"), Channel::Zposition);
    QCOMPARE(Channel::nameToType("Xrotation"), Channel::Xrotation);
    QCOMPARE(Channel::nameToType("Yrotation"), Channel::Yrotation);
    QCOMPARE(Channel::nameToType("Zrotation"), Channel::Zrotation);
    QCOMPARE(Channel::nameToType("Unknown"), Channel::Unknown);
}

void TestChannel::testTypeToName()
{
    QCOMPARE(Channel::typeToName(Channel::Xposition), QString("Xposition"));
    QCOMPARE(Channel::typeToName(Channel::Yposition), QString("Yposition"));
    QCOMPARE(Channel::typeToName(Channel::Zposition), QString("Zposition"));
    QCOMPARE(Channel::typeToName(Channel::Xrotation), QString("Xrotation"));
    QCOMPARE(Channel::typeToName(Channel::Yrotation), QString("Yrotation"));
    QCOMPARE(Channel::typeToName(Channel::Zrotation), QString("Zrotation"));
    QCOMPARE(Channel::typeToName(Channel::Unknown), QString("Unknown"));
}

void TestChannel::testChannelValue()
{
    Channel ch("Xrotation");
    ch.value = 45.5;
    QCOMPARE(ch.value, 45.5);
}

QTEST_MAIN(TestChannel)
#include "tst_channel.moc"
