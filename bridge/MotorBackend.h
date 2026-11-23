#pragma once
#include <QObject>
#include <QtQml/qqmlregistration.h>

class MotorBackend : public QObject {
    Q_OBJECT
    QML_ELEMENT

public:
    explicit MotorBackend(QObject *parent = nullptr);
};