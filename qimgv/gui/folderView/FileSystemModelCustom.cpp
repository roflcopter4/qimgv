#include "FileSystemModelCustom.h"
#include <QFileIconProvider>
#include <QFileSystemModel>

class FileSystemModelCustom::FileIconProvider final : public QAbstractFileIconProvider
{
#define appTr QGuiApplication::translate

    // This is copied from the Qt source.
    static QString getFileType(QFileInfo const &info)
    {
        if (info.isRoot())
            return appTr("QAbstractFileIconProvider", "Drive");
        if (info.isFile())
            return appTr("QAbstractFileIconProvider", "File");
        if (info.isDir())
#ifdef Q_OS_WIN
            return appTr("QAbstractFileIconProvider", "File Folder", "Match Windows Explorer");
#else
            return appTr("QAbstractFileIconProvider", "Folder", "All other platforms");
#endif
        if (info.isSymLink())
#ifdef Q_OS_MACOS
            return appTr("QAbstractFileIconProvider", "Alias", "macOS Finder");
#else
            return appTr("QAbstractFileIconProvider", "Shortcut", "All other platforms");
#endif
        return appTr("QAbstractFileIconProvider", "Unknown");
    }

  public:
    FileIconProvider()           = default;
    ~FileIconProvider() override = default;
    DELETE_COPY_MOVE_ROUTINES(FileIconProvider);

    ND QString type(QFileInfo const &info) const override
    {
        return getFileType(info);
    }

    ND QIcon icon(IconType) const override
    {
        return {};
    }

    ND QIcon icon(QFileInfo const &) const override
    {
        return {};
    }

#undef appTr
};

/****************************************************************************************/

FileSystemModelCustom::FileSystemModelCustom(QObject *parent)
    : QFileSystemModel(parent),
      iconProvider(new FileIconProvider())
{
    setOptions(Option::DontResolveSymlinks);
    setIconProvider(iconProvider.get());

    auto dpr      = qApp->devicePixelRatio();
    auto iconPath = u":/res/icons/common/menuitem/folder16.png"_s;

    if (dpr >= 1.0 + 0.001)
        iconPath.replace(u'.', u"@2x."_s);
    folderIcon.load(iconPath);
    ImageLib::recolor(folderIcon, settings->colorScheme().icons);

    connect(settings, &Settings::settingsChanged, this, &FileSystemModelCustom::onSettingsChanged);
}

FileSystemModelCustom::~FileSystemModelCustom() = default;

void FileSystemModelCustom::onSettingsChanged()
{
    ImageLib::recolor(folderIcon, settings->colorScheme().icons);
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
