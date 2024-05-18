#include "script.h"


Script::Script(QString _path, bool _blocking)
    : command(std::move(_path)),
      blocking(_blocking)
{}

