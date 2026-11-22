/*
 * - Helles 2-Spalten-Dashboard mit weißen Eingabefeldern und sauberer Scrollbar
 * - Steuerung (grüner IDE-Start-Button, Reset, Status) liegt fest oben rechts über den Plots
 * - Achtung: Explizite background/contentItem-Styles verhindern unsichtbaren Text im Windows-Style
 */
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MotorApp

ApplicationWindow {
    id: root
    width: 1280
    height: 800
    visible: true
    visibility: Window.Maximized
    title: "Motor Application"
    color: "#f8fafc"

    font.family: "Segoe UI"
    font.pixelSize: 13

    MotorBackend {
        id: backend
    }

    property bool isRunning: false
    readonly property bool isPmsm: motorTypeCombo.currentText === "PMSM"
    readonly property bool isBatchMode: simModeCombo.currentText === "Zeitgesteuert (Batch)"

    // --- Wiederverwendbare helle Karte ---
    component SectionCard: Frame {
        property string heading: ""
        default property alias content: innerCol.data

        Layout.fillWidth: true
        padding: 14
        background: Rectangle {
            color: "#ffffff"
            radius: 8
            border.color: "#e2e8f0"
            border.width: 1
        }

        ColumnLayout {
            id: innerCol
            anchors.fill: parent
            spacing: 10

            Label {
                text: heading
                font.pixelSize: 14
                font.weight: Font.DemiBold
                color: "#0f172a"
                visible: heading !== ""
            }
        }
    }

    // --- Weißes Textfeld mit sauberem Rahmen ---
    component CleanTextField: TextField {
        Layout.fillWidth: true
        color: "#0f172a"
        leftPadding: 10
        rightPadding: 10
        topPadding: 6
        bottomPadding: 6
        background: Rectangle {
            color: "#ffffff"
            radius: 5
            border.color: parent.activeFocus ? "#2563eb" : "#cbd5e1"
            border.width: parent.activeFocus ? 1.5 : 1
        }
    }

    // --- Sauber gestylter Button (funktioniert in jedem Qt-Style) ---
    component ActionButton: Button {
        property color bgColor: "#ffffff"
        property color hoverColor: "#f1f5f9"
        property color textColor: "#0f172a"
        property color borderColor: "#cbd5e1"

        leftPadding: 14
        rightPadding: 14
        topPadding: 7
        bottomPadding: 7

        contentItem: Text {
            text: parent.text
            font: parent.font
            color: parent.textColor
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        background: Rectangle {
            implicitHeight: 34
            radius: 6
            color: parent.pressed ? Qt.darker(parent.bgColor, 1.1)
                 : parent.hovered ? parent.hoverColor
                 : parent.bgColor
            border.color: parent.borderColor
            border.width: 1
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16

        // ================= LINKE SPALTE: KONFIGURATION =================
        ScrollView {
            id: leftScroll
            Layout.preferredWidth: 380
            Layout.fillHeight: true
            clip: true
            contentWidth: availableWidth
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

            ScrollBar.vertical: ScrollBar {
                parent: leftScroll
                x: leftScroll.width - width
                y: 0
                height: leftScroll.height
                policy: ScrollBar.AsNeeded
                contentItem: Rectangle {
                    implicitWidth: 6
                    radius: 3
                    color: parent.pressed ? "#64748b" : "#cbd5e1"
                }
                background: Rectangle {
                    color: "transparent"
                }
            }

            ColumnLayout {
                width: leftScroll.availableWidth - 12
                spacing: 12

                SectionCard {
                    heading: "Motortyp"

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        ComboBox {
                            id: motorTypeCombo
                            Layout.fillWidth: true
                            model: ["DC", "PMSM"]
                        }

                        ComboBox {
                            id: connectionCombo
                            Layout.preferredWidth: 115
                            model: ["Stern", "Delta"]
                            visible: root.isPmsm
                        }
                    }
                }

                SectionCard {
                    heading: "Ansteuerung (PMSM)"
                    visible: root.isPmsm

                    ComboBox {
                        id: commutationCombo
                        Layout.fillWidth: true
                        model: ["Blockkommutierung (Rechteck)", "Sinuskommutierung", "Field Oriented (FOC)"]
                    }

                    ComboBox {
                        id: sensorCombo
                        Layout.fillWidth: true
                        model: ["Hall-Sensoren", "Encoder", "Ideal"]
                    }
                }

                SectionCard {
                    heading: "Simulationsmodus"

                    ComboBox {
                        id: simModeCombo
                        Layout.fillWidth: true
                        model: ["Live (Interaktiv)", "Zeitgesteuert (Batch)"]
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        visible: root.isBatchMode

                        Label { text: "Dauer [s]:"; color: "#334155" }
                        CleanTextField {
                            id: durationField
                            text: "5.0"
                            validator: DoubleValidator { bottom: 0.001 }
                        }
                    }

                    ActionButton {
                        text: "Lastprofil laden (CSV)..."
                        Layout.fillWidth: true
                        visible: root.isBatchMode
                        onClicked: console.log("Lastprofil-Dialog öffnen")
                    }
                }

                SectionCard {
                    heading: "Motorparameter"

                    GridLayout {
                        Layout.fillWidth: true
                        columns: 2
                        columnSpacing: 12
                        rowSpacing: 8

                        Label { text: "R [Ohm]:"; color: "#334155" }
                        CleanTextField { id: paramR; text: "2.0" }

                        Label { text: "L [H]:"; color: "#334155" }
                        CleanTextField { id: paramL; text: "0.0005" }

                        Label { text: "Kt [Nm/A]:"; color: "#334155" }
                        CleanTextField { id: paramKt; text: "0.1" }

                        Label { text: "Ke [V/(rad/s)]:"; color: "#334155" }
                        CleanTextField { id: paramKe; text: "0.1" }

                        Label { text: "J [kg*m²]:"; color: "#334155" }
                        CleanTextField { id: paramJ; text: "0.01" }

                        Label { text: "Polpaare:"; color: "#334155"; visible: root.isPmsm }
                        SpinBox {
                            id: paramPolePairs
                            from: 1; to: 32; value: 1
                            visible: root.isPmsm
                            Layout.fillWidth: true
                        }
                    }
                }

                SectionCard {
                    heading: "Betriebsparameter"

                    Label { text: "Spannung V_dc: " + voltageSlider.value.toFixed(1) + " V"; color: "#334155" }
                    Slider {
                        id: voltageSlider
                        Layout.fillWidth: true
                        from: 0.0; to: 48.0; value: 12.0; stepSize: 0.5
                    }

                    Label { text: "Reibung B: " + frictionSlider.value.toFixed(4) + " Nms"; color: "#334155" }
                    Slider {
                        id: frictionSlider
                        Layout.fillWidth: true
                        from: 0.0; to: 0.01; value: 0.001; stepSize: 0.0001
                    }

                    Label { text: "Lastmoment T_load: " + loadSlider.value.toFixed(2) + " Nm"; color: "#334155" }
                    Slider {
                        id: loadSlider
                        Layout.fillWidth: true
                        from: 0.0; to: 2.0; value: 0.0; stepSize: 0.05
                    }
                }
            }
        }

        // ================= RECHTE SPALTE: TOOLBAR + PLOTS =================
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 12

            // --- IDE-Steuerleiste (immer sichtbar) ---
            Rectangle {
                Layout.fillWidth: true
                height: 52
                color: "#ffffff"
                radius: 8
                border.color: "#e2e8f0"
                border.width: 1

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 14
                    anchors.rightMargin: 14
                    spacing: 10

                    // Status-Punkt & Text
                    Rectangle {
                        width: 10
                        height: 10
                        radius: 5
                        color: root.isRunning ? "#16a34a" : "#94a3b8"
                    }
                    Label {
                        text: root.isRunning ? "Simulation läuft (" + motorTypeCombo.currentText + ")" : "Bereit"
                        font.weight: Font.Medium
                        color: "#334155"
                    }

                    Item { Layout.fillWidth: true } // Abstandhalter nach rechts

                    ActionButton {
                        text: root.isRunning ? "■  Stopp" : "▶  Start"
                        font.weight: Font.DemiBold
                        bgColor: root.isRunning ? "#dc2626" : "#16a34a"
                        hoverColor: root.isRunning ? "#b91c1c" : "#15803d"
                        borderColor: root.isRunning ? "#dc2626" : "#16a34a"
                        textColor: "#ffffff"
                        onClicked: {
                            root.isRunning = !root.isRunning
                            console.log(root.isRunning ? "Start" : "Stopp", "|", motorTypeCombo.currentText)
                        }
                    }

                    ActionButton {
                        text: "↺  Reset"
                        onClicked: {
                            root.isRunning = false
                            console.log("Reset")
                        }
                    }
                }
            }

            // --- Plot 1: Drehzahl ---
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "#ffffff"
                radius: 8
                border.color: "#e2e8f0"
                border.width: 1

                Label {
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.margins: 14
                    text: "Drehzahl [RPM]"
                    font.pixelSize: 14
                    font.weight: Font.DemiBold
                    color: "#0f172a"
                }

                Label {
                    anchors.centerIn: parent
                    text: "Plot-Bereich (Drehzahl)"
                    color: "#94a3b8"
                }
            }

            // --- Plot 2: Ströme ---
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "#ffffff"
                radius: 8
                border.color: "#e2e8f0"
                border.width: 1

                Label {
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.margins: 14
                    text: "Phasenströme [A]"
                    font.pixelSize: 14
                    font.weight: Font.DemiBold
                    color: "#0f172a"
                }

                Label {
                    anchors.centerIn: parent
                    text: "Plot-Bereich (Ströme)"
                    color: "#94a3b8"
                }
            }
        }
    }
}