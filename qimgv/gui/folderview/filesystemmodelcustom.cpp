#include "FileSystemModelCustom.h"

FileSystemModelCustom::FileSystemModelCustom(QObject *parent)
    : QFileSystemModel(parent)
{
    setOptions(Option::DontWatchForChanges);
    setIconProvider(nullptr);

    auto dpr      = qApp->devicePixelRatio();
    auto iconPath = QS(":/res/icons/common/menuitem/folder16.png");

    if (dpr >= 1.0 + 0.001)
        iconPath.replace(QS("."), QS("@2x."));
    folderIcon.load(iconPath);
    ImageLib::recolor(this->folderIcon, settings->colorScheme().icons);

    connect(settings, &Settings::settingsChanged,
            [this]() { ImageLib::recolor(this->folderIcon, settings->colorScheme().icons); });
}

QVariant FileSystemModelCustom::data(QModelIndex const &index, int role) const
{
    if (role == Qt::DecorationRole)
        return folderIcon;
    return QFileSystemModel::data(index, role);
}

Qt::ItemFlags FileSystemModelCustom::flags(QModelIndex const &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags; // 0
        // return Qt::ItemIsDropEnabled;    // Allow drops in the top-level (no parent)
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDropEnabled;
}


#if 0
bool FileSystemModelCustom::event(QEvent *event)
{
    qDebug() << event->type();
    return QFileSystemModel::event(event);

#if 0
    switch (event->type()) {
    case QEvent::Type::MetaCall: {
        std::thread fuck([&](QEvent *ev){QFileSystemModel::event(ev);}, event);
        fuck.detach();
        return false;
        //QThread thr([](){});
        //thr.
        //QThread::run();
        //connect(this, &QObject::finished, wkThrd, &QThread::quit);
    }
    default:
        return QFileSystemModel::event(event);
        break;
    }
#endif
}
#endif
