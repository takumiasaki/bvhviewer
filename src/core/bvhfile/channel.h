#ifndef CHANNEL_H
#define CHANNEL_H

#include <QString>

class Channel {
public:
    enum Type {
        Xposition,
        Yposition,
        Zposition,
        Xrotation,
        Yrotation,
        Zrotation,
        Unknown
    };

    Channel();
    explicit Channel(const QString& name);

    static Type nameToType(const QString& name);
    static QString typeToName(Type type);

    QString name;
    Type type;
    double value;
};

#endif // CHANNEL_H
