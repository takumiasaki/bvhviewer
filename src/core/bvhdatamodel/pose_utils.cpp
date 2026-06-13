#include "pose_utils.h"

#include "bvhfile.h"
#include "bvhnode.h"

#include <QtMath>
#include <algorithm>

namespace PoseUtils {

namespace {

constexpr float kMinBoneLength = 1e-4f;
constexpr float kDirectionEpsilon = 1e-6f;

QVector<double> interpolateFrameData(const BvhFile* bvhFile, double frameReal)
{
    const int frameCount = bvhFile->getFrameCount();
    if (frameCount <= 0) {
        return {};
    }

    const int totalChannels = bvhFile->getTotalChannels();
    if (frameCount == 1 || totalChannels <= 0) {
        return bvhFile->getFrameData(0);
    }

    const double clampedFrame = std::clamp(frameReal, 0.0, static_cast<double>(frameCount - 1));
    const int f0 = static_cast<int>(std::floor(clampedFrame));
    const int f1 = static_cast<int>(std::ceil(clampedFrame));
    const double alpha = clampedFrame - static_cast<double>(f0);

    const QVector<double> frame0 = bvhFile->getFrameData(f0);
    const QVector<double> frame1 = bvhFile->getFrameData(f1);
    if (frame0.isEmpty()) {
        return frame1;
    }
    if (frame1.isEmpty() || f0 == f1) {
        return frame0;
    }

    QVector<double> blended(totalChannels);
    for (int i = 0; i < totalChannels; ++i) {
        const double v0 = i < frame0.size() ? frame0[i] : 0.0;
        const double v1 = i < frame1.size() ? frame1[i] : v0;
        blended[i] = v0 + (v1 - v0) * alpha;
    }
    return blended;
}

} // namespace

BoneTransform computeBoneTransform(const QVector3D& parentPosition,
                                   const QVector3D& childPosition)
{
    BoneTransform result;
    const QVector3D delta = childPosition - parentPosition;
    const float distance = delta.length();

    if (distance < kDirectionEpsilon) {
        result.position = parentPosition;
        result.rotation = QQuaternion();
        result.length = kMinBoneLength;
        return result;
    }

    const QVector3D direction = delta / distance;
    result.rotation = QQuaternion::rotationTo(QVector3D(0.0f, 1.0f, 0.0f), direction);
    result.length = distance;
    result.position = parentPosition + direction * (distance * 0.5f);
    return result;
}

void applyChannelValues(BvhFile* bvhFile, const QVector<double>& frameData)
{
    if (!bvhFile) {
        return;
    }

    const QList<BvhNode*>& nodes = bvhFile->getAllNodes();
    for (BvhNode* node : nodes) {
        const int startIdx = node->getChannelStartIndex();
        const int channelCount = node->getChannelCount();

        for (int i = 0; i < channelCount; ++i) {
            if (startIdx + i < frameData.size()) {
                node->setChannelValue(i, frameData[startIdx + i]);
            }
        }

        node->updateLocalTransform();
    }

    for (BvhNode* node : nodes) {
        BvhNode* parent = node->getParent();
        if (parent) {
            node->setWorldTransform(parent->getWorldTransform() * node->getLocalTransform());
        } else {
            node->setWorldTransform(node->getLocalTransform());
        }
    }
}

void applyInterpolatedFrame(BvhFile* bvhFile, double frameReal)
{
    if (!bvhFile || !bvhFile->isValid()) {
        return;
    }

    const QVector<double> frameData = interpolateFrameData(bvhFile, frameReal);
    if (frameData.isEmpty()) {
        return;
    }

    applyChannelValues(bvhFile, frameData);
}

} // namespace PoseUtils
