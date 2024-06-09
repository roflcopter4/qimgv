#include "FileSystemModelCustom.h"
#include <QFileIconProvider>
#include <QFileSystemModel>

class FileSystemModelCustom::FileIconProvider : public QAbstractFileIconProvider
{
    static QString getFileType(const QFileInfo &info)
    {
        if (info.isRoot())
            return QGuiApplication::translate("QAbstractFileIconProvider", "Drive");
        if (info.isFile())
            return QGuiApplication::translate("QAbstractFileIconProvider", "File");
        if (info.isDir())
#ifdef Q_OS_WIN
            return QGuiApplication::translate("QAbstractFileIconProvider", "File Folder", "Match Windows Explorer");
#else
            return QGuiApplication::translate("QAbstractFileIconProvider", "Folder", "All other platforms");
#endif
        if (info.isSymLink())
#ifdef Q_OS_MACOS
            return QGuiApplication::translate("QAbstractFileIconProvider", "Alias", "macOS Finder");
#else
            return QGuiApplication::translate("QAbstractFileIconProvider", "Shortcut", "All other platforms");
#endif
        return QGuiApplication::translate("QAbstractFileIconProvider", "Unknown");
    }

  public:
    FileIconProvider()           = default;
    ~FileIconProvider() override = default;

    QString type(QFileInfo const &info) const override
    {
        return getFileType(info);
    }

    QIcon icon(IconType) const override
    {
        return {};
    }

    QIcon icon(const QFileInfo &) const override
    {
        return {};
    }
};


FileSystemModelCustom::FileSystemModelCustom(QObject *parent)
    : QFileSystemModel(parent),
      iconProvider(new FileIconProvider())
{
    setOptions(Option::DontWatchForChanges | Option::DontResolveSymlinks);
    setIconProvider(iconProvider.get());

    auto dpr      = qApp->devicePixelRatio();
    auto iconPath = QS(":/res/icons/common/menuitem/folder16.png");

    if (dpr >= 1.0 + 0.001)
        iconPath.replace(QS("."), QS("@2x."));
    folderIcon.load(iconPath);
    ImageLib::recolor(this->folderIcon, settings->colorScheme().icons);

    connect(settings, &Settings::settingsChanged,
            [this]() { ImageLib::recolor(this->folderIcon, settings->colorScheme().icons); });
}

FileSystemModelCustom::~FileSystemModelCustom()
{
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
