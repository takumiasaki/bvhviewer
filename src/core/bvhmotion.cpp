#include "bvhmotion.h"

BvhMotion::BvhMotion()
    : frameCount(0), frameTime(0.0), totalChannels(0)
{
}

double BvhMotion::getChannelValue(int frameIndex, int channelIndex) const
{
    if (frameIndex < 0 || frameIndex >= frames.size()) {
        return 0.0;
    }
    if (channelIndex < 0 || channelIndex >= frames[frameIndex].size()) {
        return 0.0;
    }
    return frames[frameIndex][channelIndex];
}

void BvhMotion::setChannelValue(int frameIndex, int channelIndex, double value)
{
    if (frameIndex < 0 || frameIndex >= frames.size()) {
        return;
    }
    if (channelIndex < 0 || channelIndex >= frames[frameIndex].size()) {
        return;
    }
    frames[frameIndex][channelIndex] = value;
}

QVector<double> BvhMotion::getFrame(int frameIndex) const
{
    if (frameIndex < 0 || frameIndex >= frames.size()) {
        return QVector<double>();
    }
    return frames[frameIndex];
}

void BvhMotion::setFrame(int frameIndex, const QVector<double>& values)
{
    if (frameIndex < 0 || frameIndex >= frames.size()) {
        return;
    }
    if (values.size() != totalChannels) {
        return;
    }
    frames[frameIndex] = values;
}

void BvhMotion::allocateFrames()
{
    frames.resize(frameCount);
    for (int i = 0; i < frameCount; ++i) {
        frames[i].resize(totalChannels);
        frames[i].fill(0.0);
    }
}

void BvhMotion::addFrame(const QVector<double>& frameData)
{
    if (frameData.size() == totalChannels) {
        frames.append(frameData);
    }
}
