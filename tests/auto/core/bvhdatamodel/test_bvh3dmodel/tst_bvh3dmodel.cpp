#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QFile>
#include <QSignalSpy>
#include <QVector3D>
#include <memory>

#include "bvh3dmodel.h"
#include "bvhfile.h"
#include "bvhjointlistmodel.h"
#include "bvhbonelistmodel.h"

#ifndef TEST_BVH_DIR
#define TEST_BVH_DIR "bvhfiles"
#endif

class TestBvh3DModel : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();

    void testAttachInvalid();
    void testSkeletonMetadataCounts();
    void testJointPositionsFrame0();
    void testJointPositionsFrame1();
    void testBoneLengthAndRotation();
    void testInterpolation();
    void testListModelRoles();
    void testSetFrameClamps();
    void testSkeletonColors();

private:
    QString m_debugAxesPath;
    std::shared_ptr<BvhFile> m_bvhFile;
};

void TestBvh3DModel::initTestCase()
{
    m_debugAxesPath = QString(TEST_BVH_DIR) + QStringLiteral("/debug_axes.bvh");
    QVERIFY2(QFile::exists(m_debugAxesPath),
             qPrintable(QStringLiteral("Missing test data: ") + m_debugAxesPath));

    m_bvhFile = std::make_shared<BvhFile>();
    QVERIFY(m_bvhFile->load(m_debugAxesPath));
    QVERIFY(m_bvhFile->isValid());
}

void TestBvh3DModel::testAttachInvalid()
{
    Bvh3DModel model;
    QSignalSpy failedSpy(&model, &Bvh3DModel::attachFailed);

    QVERIFY(!model.attach(nullptr));
    QCOMPARE(failedSpy.size(), 1);
    QVERIFY(!model.isValid());
    QCOMPARE(model.jointCount(), 0);
    QCOMPARE(model.boneCount(), 0);

    auto invalidFile = std::make_shared<BvhFile>();
    QVERIFY(!model.attach(invalidFile));
    QCOMPARE(failedSpy.size(), 2);
}

void TestBvh3DModel::testSkeletonMetadataCounts()
{
    Bvh3DModel model(m_bvhFile);

    QCOMPARE(model.jointCount(), 7);
    QCOMPARE(model.boneCount(), 6);
    QCOMPARE(model.jointModel()->rowCount(), 7);
    QCOMPARE(model.boneModel()->rowCount(), 6);
}

void TestBvh3DModel::testJointPositionsFrame0()
{
    Bvh3DModel model(m_bvhFile);
    model.setFrame(0);

    const auto& joints = model.joints();
    auto findJoint = [&joints](const QString& name) -> QVector3D {
        for (const Bvh3DJoint& joint : joints) {
            if (joint.name == name) {
                return joint.position;
            }
        }
        return {};
    };

    const QVector3D root = findJoint(QStringLiteral("Root"));
    const QVector3D xAxis = findJoint(QStringLiteral("Xaxis"));
    const QVector3D xEnd = findJoint(QStringLiteral("Xaxis_EndSite"));
    const QVector3D yAxis = findJoint(QStringLiteral("Yaxis"));
    const QVector3D zAxis = findJoint(QStringLiteral("Zaxis"));

    QVERIFY(qFuzzyCompare(root.x(), 0.0f));
    QVERIFY(qFuzzyCompare(root.y(), 0.0f));
    QVERIFY(qFuzzyCompare(root.z(), 0.0f));

    QVERIFY(qFuzzyCompare(xAxis.x(), 50.0f));
    QVERIFY(qFuzzyCompare(xEnd.x(), 60.0f));
    QVERIFY(qFuzzyCompare(yAxis.y(), 50.0f));
    QVERIFY(qFuzzyCompare(zAxis.z(), 50.0f));
}

void TestBvh3DModel::testJointPositionsFrame1()
{
    Bvh3DModel model(m_bvhFile);
    model.setFrame(1);

    const auto& joints = model.joints();
    auto findJoint = [&joints](const QString& name) -> QVector3D {
        for (const Bvh3DJoint& joint : joints) {
            if (joint.name == name) {
                return joint.position;
            }
        }
        return {};
    };

    const QVector3D root = findJoint(QStringLiteral("Root"));
    const QVector3D xAxis = findJoint(QStringLiteral("Xaxis"));

    QVERIFY(qFuzzyCompare(root.x(), 10.0f));
    QVERIFY(qFuzzyCompare(xAxis.x(), 60.0f));
}

void TestBvh3DModel::testBoneLengthAndRotation()
{
    Bvh3DModel model(m_bvhFile);
    model.setFrame(0);

    const auto& bones = model.bones();
    QVERIFY(!bones.isEmpty());

    const Bvh3DBone& rootToX = bones.first();
    QVERIFY(qFuzzyCompare(rootToX.length, 50.0f));

    const float quatLength = rootToX.rotation.length();
    QVERIFY(qFuzzyCompare(quatLength, 1.0f));
}

void TestBvh3DModel::testInterpolation()
{
    Bvh3DModel model(m_bvhFile);

    const double frameTime = m_bvhFile->getFrameTime();
    model.setPoseAtTime(frameTime * 0.5);

    const auto& joints = model.joints();
    auto findJoint = [&joints](const QString& name) -> QVector3D {
        for (const Bvh3DJoint& joint : joints) {
            if (joint.name == name) {
                return joint.position;
            }
        }
        return {};
    };

    const QVector3D root = findJoint(QStringLiteral("Root"));
    QVERIFY(qFuzzyCompare(root.x(), 5.0f));
    QVERIFY(qFuzzyCompare(root.y(), 0.0f));
}

void TestBvh3DModel::testListModelRoles()
{
    Bvh3DModel model(m_bvhFile);
    model.setFrame(0);

    const QModelIndex firstJoint = model.jointModel()->index(0, 0);
    QVERIFY(firstJoint.isValid());
    QCOMPARE(firstJoint.data(BvhJointListModel::NameRole).toString(), QStringLiteral("Root"));
    QVERIFY(firstJoint.data(BvhJointListModel::IsEndSiteRole).toBool() == false);

    const QModelIndex endSiteJoint = model.jointModel()->index(2, 0);
    QVERIFY(endSiteJoint.isValid());
    QCOMPARE(endSiteJoint.data(BvhJointListModel::NameRole).toString(),
             QStringLiteral("Xaxis_EndSite"));
    QVERIFY(endSiteJoint.data(BvhJointListModel::IsEndSiteRole).toBool());

    const QModelIndex firstBone = model.boneModel()->index(0, 0);
    QVERIFY(firstBone.isValid());
    QVERIFY(firstBone.data(BvhBoneListModel::LengthRole).toFloat() > 0.0f);
}

void TestBvh3DModel::testSetFrameClamps()
{
    Bvh3DModel model(m_bvhFile);

    model.setFrame(999);
    QCOMPARE(model.currentFrame(), 2);

    model.setFrame(-5);
    QCOMPARE(model.currentFrame(), 0);
}

void TestBvh3DModel::testSkeletonColors()
{
    Bvh3DModel model(m_bvhFile);

    const QColor jointColor(QStringLiteral("#67C1FF"));
    const QColor customColor(QStringLiteral("#FF9F5E"));

    model.setJointColor(jointColor);
    model.setBoneTone(-25);
    model.setCustomBoneColor(customColor);

    model.setBoneColorMode(Bvh3DModel::Custom);
    QCOMPARE(model.boneColorMode(), Bvh3DModel::Custom);
    QCOMPARE(model.boneColor(), customColor);
    QCOMPARE(model.customBoneColor(), customColor);

    model.setJointColor(QColor(QStringLiteral("#112233")));
    QCOMPARE(model.boneColor(), customColor);

    model.setBoneColorMode(Bvh3DModel::SameAsJoint);
    QCOMPARE(model.boneColorMode(), Bvh3DModel::SameAsJoint);
    QCOMPARE(model.boneColor(), model.jointColor());
    QCOMPARE(model.boneTone(), -25);

    model.setBoneColorMode(Bvh3DModel::ToneOffset);
    QCOMPARE(model.boneTone(), -25);
    QVERIFY(model.boneColor() != model.jointColor());

    model.setBoneColorMode(Bvh3DModel::Custom);
    QCOMPARE(model.boneColor(), customColor);

    model.setBoneColorMode(Bvh3DModel::SameAsJoint);
    model.setBoneColorMode(Bvh3DModel::Custom);
    model.setBoneColorMode(Bvh3DModel::ToneOffset);
    QCOMPARE(model.boneTone(), -25);

    model.setJointColor(jointColor);
    model.setBoneColorMode(Bvh3DModel::SameAsJoint);
    QCOMPARE(model.boneColorMode(), Bvh3DModel::SameAsJoint);
    QCOMPARE(model.jointColor(), jointColor);
    QCOMPARE(model.boneColor(), jointColor);
}

QTEST_MAIN(TestBvh3DModel)
#include "tst_bvh3dmodel.moc"
