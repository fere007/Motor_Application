/*
 * - Minimale Qt-Bridge zwischen C++-Backend und QML-Frontend
 * - Achtung: Q_OBJECT und QML_ELEMENT sind Pflicht, damit QML die Klasse kennt
 */
#pragma once
#include <QObject>
#include <QtQml/qqmlregistration.h>

class MotorBackend : public QObject {
    Q_OBJECT
    QML_ELEMENT

public:
    explicit MotorBackend(QObject *parent = nullptr);
};