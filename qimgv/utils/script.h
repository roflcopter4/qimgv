#pragma once

#include <QMetaType>
#include <QString>

struct Script {
    Script() = default;;
    Script(QString _path, bool _blocking);
    QString command;
    bool    blocking = false;
};

Q_DECLARE_METATYPE(Script)
