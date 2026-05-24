#ifndef BVHMOTION_H
#define BVHMOTION_H

#include <QVector>

class BvhMotion {
public:
    BvhMotion();

    // フレーム情報
    int getFrameCount() const { return frameCount; }
    void setFrameCount(int count) { frameCount = count; }

    double getFrameTime() const { return frameTime; }
    void setFrameTime(double time) { frameTime = time; }

    double getTotalDuration() const { return frameCount * frameTime; }

    // チャンネル情報
    int getTotalChannels() const { return totalChannels; }
    void setTotalChannels(int count) { totalChannels = count; }

    // フレームデータアクセス
    double getChannelValue(int frameIndex, int channelIndex) const;
    void setChannelValue(int frameIndex, int channelIndex, double value);

    QVector<double> getFrame(int frameIndex) const;
    void setFrame(int frameIndex, const QVector<double>& values);

    // フレームリスト管理
    void allocateFrames();
    void addFrame(const QVector<double>& frameData);

private:
    int frameCount;
    double frameTime;
    int totalChannels;

    QVector<QVector<double>> frames;  // frames[frameIndex][channelIndex]
};

#endif // BVHMOTION_H
