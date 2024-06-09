## SOURCES FOR QIMGV COMPONENTS

target_sources(qimgv PRIVATE
    DirectoryModel.cpp
    DirectoryPresenter.cpp

    scriptManager/ScriptManager.cpp
    actionManager/ActionManager.cpp

    cache/Cache.cpp
    cache/CacheItem.cpp
    cache/ThumbnailCache.cpp

    loader/Loader.cpp
    loader/LoaderRunnable.cpp

    scaler/Scaler.cpp
    scaler/ScalerRunnable.cpp

    thumbnailer/Thumbnailer.cpp
    thumbnailer/ThumbnailerRunnable.cpp

    directoryManager/DirectoryManager.cpp

    directoryManager/watchers/DirectoryWatcher.cpp
    directoryManager/watchers/DummyDirectoryWatcher.cpp
    directoryManager/watchers/DirectoryWatcherEvent.cpp
    directoryManager/watchers/DirectoryWatcherWorker.cpp

)

if(APPLE)
    #target_sources(qimgv PRIVATE
    #    directorymanager/watchers/linux/LinuxFsEvent.cpp
    #    directorymanager/watchers/linux/LinuxDirectoryWatcher.cpp
    #    directorymanager/watchers/linux/LinuxDirectoryWorker.cpp)
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    target_sources(qimgv PRIVATE
        directoryManager/watchers/linux/LinuxFsEvent.cpp
        directoryManager/watchers/linux/LinuxDirectoryWatcher.cpp
        directoryManager/watchers/linux/LinuxDirectoryWorker.cpp)
elseif(WIN32)
    target_sources(qimgv PRIVATE
        directoryManager/watchers/windows/WindowsDirectoryWatcher.cpp
        directoryManager/watchers/windows/WindowsDirectoryWorker.cpp)
endif()
