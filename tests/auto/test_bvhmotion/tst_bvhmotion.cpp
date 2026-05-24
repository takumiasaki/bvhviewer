#include <QtTest/QtTest>
#include "../../../src/core/bvhmotion.h"

class TestBvhMotion : public QObject {
    Q_OBJECT

private slots:
    void testConstruction();
    void testFrameInfo();
    void testChannels();
    void testAllocateFrames();
    void testGetSetChannelValue();
    void testGetSetFrame();
    void testTotalDuration();
};

void TestBvhMotion::testConstruction()
{
    BvhMotion motion;
    QCOMPARE(motion.getFrameCount(), 0);
    QCOMPARE(motion.getFrameTime(), 0.0);
    QCOMPARE(motion.getTotalChannels(), 0);
}

void TestBvhMotion::testFrameInfo()
{
    BvhMotion motion;
    motion.setFrameCount(100);
    motion.setFrameTime(0.0333333);
    motion.setTotalChannels(60);
    
    QCOMPARE(motion.getFrameCount(), 100);
    QCOMPARE(motion.getFrameTime(), 0.0333333);
    QCOMPARE(motion.getTotalChannels(), 60);
}

void TestBvhMotion::testChannels()
{
    BvhMotion motion;
    motion.setTotalChannels(6);
    QCOMPARE(motion.getTotalChannels(), 6);
}

void TestBvhMotion::testAllocateFrames()
{
    BvhMotion motion;
    motion.setFrameCount(10);
    motion.setTotalChannels(6);
    motion.allocateFrames();
    
    // Check that all frames are allocated and initialized to 0.0
    for (int i = 0; i < 10; ++i) {
        QVector<double> frame = motion.getFrame(i);
        QCOMPARE(frame.size(), 6);
        for (int j = 0; j < 6; ++j) {
            QCOMPARE(frame[j], 0.0);
        }
    }
}

void TestBvhMotion::testGetSetChannelValue()
{
    BvhMotion motion;
    motion.setFrameCount(5);
    motion.setTotalChannels(3);
    motion.allocateFrames();
    
    motion.setChannelValue(0, 0, 10.5);
    motion.setChannelValue(0, 1, 20.5);
    motion.setChannelValue(0, 2, 30.5);
    
    QCOMPARE(motion.getChannelValue(0, 0), 10.5);
    QCOMPARE(motion.getChannelValue(0, 1), 20.5);
    QCOMPARE(motion.getChannelValue(0, 2), 30.5);
}

void TestBvhMotion::testGetSetFrame()
{
    BvhMotion motion;
    motion.setFrameCount(3);
    motion.setTotalChannels(2);
    motion.allocateFrames();
    
    QVector<double> frameData;
    frameData.append(1.0);
    frameData.append(2.0);
    
    motion.setFrame(0, frameData);
    
    QVector<double> retrievedFrame = motion.getFrame(0);
    QCOMPARE(retrievedFrame.size(), 2);
    QCOMPARE(retrievedFrame[0], 1.0);
    QCOMPARE(retrievedFrame[1], 2.0);
}

void TestBvhMotion::testTotalDuration()
{
    BvhMotion motion;
    motion.setFrameCount(100);
    motion.setFrameTime(0.0333333);
    
    double duration = motion.getTotalDuration();
    QVERIFY(qFuzzyCompare(duration, 3.33333));
}

QTEST_MAIN(TestBvhMotion)
#include "tst_bvhmotion.moc"
