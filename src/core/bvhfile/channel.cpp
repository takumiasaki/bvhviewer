#include "channel.h"

Channel::Channel()
    : name(""), type(Unknown), value(0.0)
{
}

Channel::Channel(const QString& name)
    : name(name), type(nameToType(name)), value(0.0)
{
}

Channel::Type Channel::nameToType(const QString& name)
{
    if (name == "Xposition") return Xposition;
    if (name == "Yposition") return Yposition;
    if (name == "Zposition") return Zposition;
    if (name == "Xrotation") return Xrotation;
    if (name == "Yrotation") return Yrotation;
    if (name == "Zrotation") return Zrotation;
    return Unknown;
}

QString Channel::typeToName(Type type)
{
    switch (type) {
    case Xposition: return "Xposition";
    case Yposition: return "Yposition";
    case Zposition: return "Zposition";
    case Xrotation: return "Xrotation";
    case Yrotation: return "Yrotation";
    case Zrotation: return "Zrotation";
    case Unknown:
    default:
        return "Unknown";
    }
}
