#include <QtTest/QtTest>
#include <QVector3D>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include "../../../src/core/bvhfile.h"

static void appendDebugLog(const QString& message)
{
    QFile logFile(QDir::current().filePath("test_bvhfile_debug.txt"));
    if (logFile.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&logFile);
        out << message << "\n";
        logFile.close();
    }
}

static QString createTemporaryBvhFile(const QString& content)
{
    QString tempPath = QDir::temp().filePath("test_bvhfile_temp.bvh");
    QFile file(tempPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        appendDebugLog("Failed to open temp file: " + tempPath);
        return QString();
    }
    file.write(content.toUtf8());
    file.close();
    appendDebugLog("Created temp BVH: " + tempPath);
    return tempPath;
}

class TestBvhFile : public QObject {
    Q_OBJECT

private slots:
    void testLoadSimpleBvh();
    void testHierarchyParsing();
    void testMotionParsing();
    void testSetCurrentFrame();
    void testNodeByName();
    void testAllNodes();
    void testChannelIndices();
};

void TestBvhFile::testLoadSimpleBvh()
{
    appendDebugLog("testLoadSimpleBvh start");
    const QString bvhContent = R"(HIERARCHY
ROOT Hips
{
  OFFSET 0.0 0.0 0.0
  CHANNELS 6 Xposition Yposition Zposition Zrotation Xrotation Yrotation
  JOINT Chest
  {
    OFFSET 0.0 10.0 0.0
    CHANNELS 3 Zrotation Xrotation Yrotation
    End Site
    {
      OFFSET 0.0 5.0 0.0
    }
  }
}
MOTION
Frames: 2
Frame Time: 0.0333333
0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0
1.0 2.0 3.0 0.0 0.0 0.0 0.0 0.0 0.0
)";

    QString path = createTemporaryBvhFile(bvhContent);
    QVERIFY(!path.isEmpty());

    BvhFile bvh;
    bool loaded = bvh.load(path);

    QVERIFY(loaded);
    QVERIFY(bvh.isValid());
    appendDebugLog("testLoadSimpleBvh end");
}

void TestBvhFile::testHierarchyParsing()
{
    appendDebugLog("testHierarchyParsing start");
    const QString bvhContent = R"(HIERARCHY
ROOT Hips
{
  OFFSET 0.0 0.0 0.0
  CHANNELS 6 Xposition Yposition Zposition Zrotation Xrotation Yrotation
  JOINT Chest
  {
    OFFSET 0.0 10.0 0.0
    CHANNELS 3 Zrotation Xrotation Yrotation
  }
}
MOTION
Frames: 0
Frame Time: 0.0333333
)";

    QString path = createTemporaryBvhFile(bvhContent);
    QVERIFY(!path.isEmpty());

    BvhFile bvh;
    bool loaded = bvh.load(path);
    appendDebugLog(QString("Hierarchy parsing loaded=%1").arg(loaded));
    QVERIFY(loaded);

    BvhNode* root = bvh.getRootNode();
    appendDebugLog(QString("Hierarchy parsing root=%1").arg(root ? root->getName() : QString("null")));
    QVERIFY(root != nullptr);
    QCOMPARE(root->getName(), QString("Hips"));
    appendDebugLog(QString("Hierarchy parsing channelCount=%1").arg(root->getChannelCount()));
    QCOMPARE(root->getChannelCount(), 6);
    appendDebugLog(QString("Hierarchy parsing childrenCount=%1").arg(root->getChildren().size()));
    QCOMPARE(root->getChildren().size(), 1);

    BvhNode* chest = root->getChildren()[0];
    appendDebugLog(QString("Hierarchy parsing chest=%1").arg(chest ? chest->getName() : QString("null")));
    QCOMPARE(chest->getName(), QString("Chest"));
    QCOMPARE(chest->getChannelCount(), 3);
    appendDebugLog("testHierarchyParsing end");
}

void TestBvhFile::testMotionParsing()
{
    appendDebugLog("testMotionParsing start");
    const QString bvhContent = R"(HIERARCHY
ROOT Hips
{
  OFFSET 0.0 0.0 0.0
  CHANNELS 6 Xposition Yposition Zposition Zrotation Xrotation Yrotation
}
MOTION
Frames: 2
Frame Time: 0.0333333
1.0 2.0 3.0 4.0 5.0 6.0
7.0 8.0 9.0 10.0 11.0 12.0
)";

    QString path = createTemporaryBvhFile(bvhContent);
    QVERIFY(!path.isEmpty());

    BvhFile bvh;
    bool loaded = bvh.load(path);
    QVERIFY(loaded);

    QCOMPARE(bvh.getFrameCount(), 2);
    QVERIFY(qFuzzyCompare(bvh.getFrameTime(), 0.0333333));
    appendDebugLog("testMotionParsing end");
}

void TestBvhFile::testSetCurrentFrame()
{
    appendDebugLog("testSetCurrentFrame start");
    const QString bvhContent = R"(HIERARCHY
ROOT Hips
{
  OFFSET 0.0 0.0 0.0
  CHANNELS 6 Xposition Yposition Zposition Zrotation Xrotation Yrotation
}
MOTION
Frames: 2
Frame Time: 0.0333333
1.0 2.0 3.0 4.0 5.0 6.0
7.0 8.0 9.0 10.0 11.0 12.0
)";

    QString path = createTemporaryBvhFile(bvhContent);
    QVERIFY(!path.isEmpty());

    BvhFile bvh;
    bool loaded = bvh.load(path);
    QVERIFY(loaded);

    bvh.setCurrentFrame(0);
    BvhNode* hips = bvh.getRootNode();
    QVERIFY(hips != nullptr);
    QCOMPARE(hips->getChannelValue(0), 1.0);
    QCOMPARE(hips->getChannelValue(1), 2.0);
    QCOMPARE(hips->getChannelValue(2), 3.0);

    bvh.setCurrentFrame(1);
    QCOMPARE(hips->getChannelValue(0), 7.0);
    QCOMPARE(hips->getChannelValue(1), 8.0);
    QCOMPARE(hips->getChannelValue(2), 9.0);
    appendDebugLog("testSetCurrentFrame end");
}

void TestBvhFile::testNodeByName()
{
    appendDebugLog("testNodeByName start");
    const QString bvhContent = R"(HIERARCHY
ROOT Hips
{
  OFFSET 0.0 0.0 0.0
  CHANNELS 6 Xposition Yposition Zposition Zrotation Xrotation Yrotation
  JOINT Chest
  {
    OFFSET 0.0 10.0 0.0
    CHANNELS 3 Zrotation Xrotation Yrotation
  }
}
MOTION
Frames: 1
Frame Time: 0.0333333
0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0
)";

    QString path = createTemporaryBvhFile(bvhContent);
    QVERIFY(!path.isEmpty());

    BvhFile bvh;
    bool loaded = bvh.load(path);
    QVERIFY(loaded);

    BvhNode* hips = bvh.getNodeByName("Hips");
    QVERIFY(hips != nullptr);
    QCOMPARE(hips->getName(), QString("Hips"));

    BvhNode* chest = bvh.getNodeByName("Chest");
    QVERIFY(chest != nullptr);
    QCOMPARE(chest->getName(), QString("Chest"));

    BvhNode* unknown = bvh.getNodeByName("Unknown");
    QCOMPARE(unknown, nullptr);
    appendDebugLog("testNodeByName end");
}

void TestBvhFile::testAllNodes()
{
    appendDebugLog("testAllNodes start");
    const QString bvhContent = R"(HIERARCHY
ROOT Hips
{
  OFFSET 0.0 0.0 0.0
  CHANNELS 6 Xposition Yposition Zposition Zrotation Xrotation Yrotation
  JOINT Chest
  {
    OFFSET 0.0 10.0 0.0
    CHANNELS 3 Zrotation Xrotation Yrotation
  }
  JOINT RightLeg
  {
    OFFSET 5.0 0.0 0.0
    CHANNELS 3 Zrotation Xrotation Yrotation
  }
}
MOTION
Frames: 1
Frame Time: 0.0333333
0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0
)";

    QString path = createTemporaryBvhFile(bvhContent);
    QVERIFY(!path.isEmpty());

    BvhFile bvh;
    bool loaded = bvh.load(path);
    QVERIFY(loaded);

    QCOMPARE(bvh.getAllNodes().size(), 3);
    appendDebugLog("testAllNodes end");
}

void TestBvhFile::testChannelIndices()
{
    appendDebugLog("testChannelIndices start");
    const QString bvhContent = R"(HIERARCHY
ROOT Hips
{
  OFFSET 0.0 0.0 0.0
  CHANNELS 6 Xposition Yposition Zposition Zrotation Xrotation Yrotation
  JOINT Chest
  {
    OFFSET 0.0 10.0 0.0
    CHANNELS 3 Zrotation Xrotation Yrotation
  }
}
MOTION
Frames: 1
Frame Time: 0.0333333
0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0
)";

    QString path = createTemporaryBvhFile(bvhContent);
    QVERIFY(!path.isEmpty());

    BvhFile bvh;
    bool loaded = bvh.load(path);
    QVERIFY(loaded);

    QCOMPARE(bvh.getTotalChannels(), 9);

    BvhNode* hips = bvh.getRootNode();
    QCOMPARE(hips->getChannelStartIndex(), 0);

    BvhNode* chest = bvh.getNodeByName("Chest");
    QCOMPARE(chest->getChannelStartIndex(), 6);
    appendDebugLog("testChannelIndices end");
}

QTEST_MAIN(TestBvhFile)
#include "tst_bvhfile.moc"
