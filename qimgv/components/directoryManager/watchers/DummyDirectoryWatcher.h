#pragma once

#include "DirectoryWatcher.h"

class DirectoryManager;

class DummyDirectoryWatcher : public DirectoryWatcher
{
    Q_OBJECT

  public:
    explicit DummyDirectoryWatcher(DirectoryManager *parent);
};

using DirectoryWatcherImplementation = DummyDirectoryWatcher;
