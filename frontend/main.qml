/*
 * - Minimales Hauptfenster für den ersten Starttest
 * - Achtung: Dateiname muss großgeschrieben sein (Main.qml) und URI zu CMake passen
 */
import QtQuick
import QtQuick.Controls
import MotorApp

ApplicationWindow {
    width: 640
    height: 480
    visible: true
    title: "Motor Application"

    MotorBackend {
        id: backend
    }

    Text {
        anchors.centerIn: parent
        text: "Motor Application läuft!"
        font.pixelSize: 20
    }
}