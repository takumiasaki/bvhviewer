.pragma library

var defaultBoneTone = -25

function boneColorFromTone(jointColor, tone) {
    if (tone === 0) {
        return jointColor
    }
    if (tone > 0) {
        return Qt.lighter(jointColor, 1.0 + tone / 100.0)
    }
    return Qt.darker(jointColor, 1.0 + (-tone) / 100.0)
}
