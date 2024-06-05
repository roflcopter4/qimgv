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
    directoryManager/watchers/DummyWatcher.cpp
    directoryManager/watchers/WatcherEvent.cpp
    directoryManager/watchers/WatcherWorker.cpp

)

if(APPLE)
    #target_sources(qimgv PRIVATE
    #    directorymanager/watchers/linux/linuxfsevent.cpp
    #    directorymanager/watchers/linux/linuxwatcher.cpp
    #    directorymanager/watchers/linux/linuxworker.cpp)
elseif(UNIX)
    target_sources(qimgv PRIVATE
        directoryManager/watchers/linux/LinuxFsEvent.cpp
        directoryManager/watchers/linux/LinuxWatcher.cpp
        directoryManager/watchers/linux/LinuxWorker.cpp)
elseif(WIN32)
    target_sources(qimgv PRIVATE
        directoryManager/watchers/windows/WindowsWatcher.cpp
        directoryManager/watchers/windows/WindowsWorker.cpp)
endif()
