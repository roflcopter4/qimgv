#pragma once

#include <QMetaType>
#include <QString>

struct Script {
    Script() = default;
    explicit Script(QString path, bool blocking = false);

    QString command;
    bool    blocking = false;
};

Q_DECLARE_METATYPE(Script)
