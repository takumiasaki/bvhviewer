#include <QtTest/QtTest>
#include <QVector3D>
#include <QMatrix4x4>
#include "../../../src/core/bvhnode.h"

class TestBvhNode : public QObject {
    Q_OBJECT

private slots:
    void testConstruction();
    void testAddChild();
    void testOffset();
    void testChannels();
    void testChannelValue();
    void testEndSite();
    void testLocalTransformIdentity();
    void testLocalTransformWithPosition();
    void testLocalTransformWithRotation();
    void testWorldPosition();
};

void TestBvhNode::testConstruction()
{
    BvhNode node("TestNode");
    QCOMPARE(node.getName(), QString("TestNode"));
    QCOMPARE(node.getParent(), nullptr);
    QCOMPARE(node.getChildren().size(), 0);
}

void TestBvhNode::testAddChild()
{
    BvhNode parent("Parent");
    BvhNode* child = new BvhNode("Child");
    
    parent.addChild(child);
    
    QCOMPARE(parent.getChildren().size(), 1);
    QCOMPARE(parent.getChildren()[0]->getName(), QString("Child"));
    QCOMPARE(child->getParent()->getName(), QString("Parent"));
}

void TestBvhNode::testOffset()
{
    BvhNode node("Test");
    QVector3D offset(1.0f, 2.0f, 3.0f);
    node.setOffset(offset);
    
    QCOMPARE(node.getOffset(), offset);
}

void TestBvhNode::testChannels()
{
    BvhNode node("Test");
    Channel ch1("Xposition");
    Channel ch2("Yrotation");
    
    node.addChannel(ch1);
    node.addChannel(ch2);
    
    QCOMPARE(node.getChannelCount(), 2);
    QCOMPARE(node.getChannels()[0].name, QString("Xposition"));
    QCOMPARE(node.getChannels()[1].name, QString("Yrotation"));
}

void TestBvhNode::testChannelValue()
{
    BvhNode node("Test");
    Channel ch("Xposition");
    node.addChannel(ch);
    
    node.setChannelValue(0, 42.5);
    QCOMPARE(node.getChannelValue(0), 42.5);
}

void TestBvhNode::testEndSite()
{
    BvhNode node("Test");
    QVector3D endSiteOffset(0.0f, 5.0f, 0.0f);
    
    node.setEndSite(endSiteOffset);
    
    QVERIFY(node.hasEndSiteInfo());
    QCOMPARE(node.getEndSiteOffset(), endSiteOffset);
}

void TestBvhNode::testLocalTransformIdentity()
{
    BvhNode node("Test");
    node.updateLocalTransform();
    
    QMatrix4x4 identity;
    identity.setToIdentity();
    
    // Check if local transform is identity (no offset, no rotation)
    QCOMPARE(node.getLocalTransform(), identity);
}

void TestBvhNode::testLocalTransformWithPosition()
{
    BvhNode node("Test");
    node.setOffset(QVector3D(1.0f, 2.0f, 3.0f));
    
    Channel xpos("Xposition");
    xpos.value = 10.0;
    node.addChannel(xpos);
    
    node.updateLocalTransform();
    
    QVector3D pos = node.getLocalTransform().column(3).toVector3D();
    QCOMPARE(pos.x(), 11.0f);  // offset.x + position.x
    QCOMPARE(pos.y(), 2.0f);   // offset.y (no Yposition)
    QCOMPARE(pos.z(), 3.0f);   // offset.z (no Zposition)
}

void TestBvhNode::testLocalTransformWithRotation()
{
    BvhNode node("Test");
    
    Channel yrot("Yrotation");
    yrot.value = 90.0;  // 90 degrees around Y axis
    node.addChannel(yrot);
    
    node.updateLocalTransform();
    
    QMatrix4x4 transform = node.getLocalTransform();
    
    // After 90 degree rotation around Y, X axis should point in -Z direction
    QVector4D xAxisRotated = transform * QVector4D(1, 0, 0, 0);
    QVERIFY(qFuzzyCompare(xAxisRotated.z(), -1.0f));
}

void TestBvhNode::testWorldPosition()
{
    BvhNode node("Test");
    node.setOffset(QVector3D(1.0f, 2.0f, 3.0f));
    node.updateLocalTransform();
    node.setWorldTransform(node.getLocalTransform());
    
    QVector3D worldPos = node.getWorldPosition();
    QCOMPARE(worldPos, QVector3D(1.0f, 2.0f, 3.0f));
}

QTEST_MAIN(TestBvhNode)
#include "tst_bvhnode.moc"
