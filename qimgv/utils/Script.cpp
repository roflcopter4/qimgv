#include "Script.h"


Script::Script(QString path, bool blocking)
    : command(std::move(path)),
      blocking(blocking)
{}
