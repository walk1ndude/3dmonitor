import QtQuick 2.0
import Qt3D 2.0
import Qt3D.Shapes 2.0

Quad {
    id: meshFace
    scale: 200
    property real halfScale: scale / 2
    effect: programLayout
    cullFaces: "CullBackFaces"
}
