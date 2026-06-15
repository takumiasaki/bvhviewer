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

function colorsClose(a, b) {
    return Math.abs(a.r - b.r) < 0.02
            && Math.abs(a.g - b.g) < 0.02
            && Math.abs(a.b - b.b) < 0.02
}

function estimateBoneTone(jointColor, boneColor) {
    for (var tone = -100; tone <= 100; tone += 5) {
        if (colorsClose(boneColorFromTone(jointColor, tone), boneColor)) {
            return tone
        }
    }
    return defaultBoneTone
}
