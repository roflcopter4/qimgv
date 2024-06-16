#include "PathSelectorMenuItem.h"

PathSelectorMenuItem::PathSelectorMenuItem(QWidget *parent)
    : MenuItem(parent)
{
    setFocusPolicy(Qt::NoFocus);
    setIconPath(u":/res/icons/common/menuitem/folder16.png"_s);
    mIconWidget->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    connect(mIconWidget, &IconButton::clicked, this, &PathSelectorMenuItem::onClicked);
}

void PathSelectorMenuItem::onClicked()
{
    QFileDialog dialog;
    dialog.setDirectory(mDirectory);
    dialog.setWindowTitle(u"Select directory"_s);
    dialog.setWindowModality(Qt::ApplicationModal);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly);
    dialog.setOption(QFileDialog::DontResolveSymlinks);
    connect(&dialog, &QFileDialog::fileSelected, this, &PathSelectorMenuItem::setDirectory);
    dialog.exec();
}

QString PathSelectorMenuItem::directory() const
{
    return mDirectory;
}

QString PathSelectorMenuItem::path() const
{
    return mPath;
}

void PathSelectorMenuItem::setDirectory(QString const &path)
{
    mDirectory = path;
    mPath      = path;
    QString stripped;
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    stripped = mDirectory.split(u'/').last();
#else
    stripped = mDirectory.split(u'/', Qt::SkipEmptyParts).last();
#endif
    mTextLabel.setText(stripped);
}

void PathSelectorMenuItem::onPress()
{
    if (!mDirectory.isEmpty())
        emit directorySelected(mDirectory);
}
