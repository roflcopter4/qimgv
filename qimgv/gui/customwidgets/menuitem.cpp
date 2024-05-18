#include "menuitem.h"

MenuItem::MenuItem(QWidget *parent)
    : QWidget(parent)
{
    mLayout.setContentsMargins(6,0,8,0);
    mLayout.setSpacing(2);

    setAccessibleName(QS("MenuItem"));
    this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    mTextLabel.setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    mShortcutLabel.setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    mIconWidget.setMinimumSize(26, 26); // 5px padding from stylesheet

    mIconWidget.installEventFilter(this);

    spacer = new QSpacerItem(3, 1, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    mIconWidget.setAttribute(Qt::WA_TransparentForMouseEvents, true);
    mIconWidget.setAccessibleName(QS("MenuItemIcon"));
    mTextLabel.setAccessibleName(QS("MenuItemText"));
    mShortcutLabel.setAccessibleName(QS("MenuItemShortcutLabel"));
    mLayout.addWidget(&mIconWidget);
    mLayout.addWidget(&mTextLabel);
    mLayout.addSpacerItem(spacer);
    mLayout.addWidget(&mShortcutLabel);
    mLayout.setStretch(1,1);

    setLayout(&mLayout);
}

MenuItem::~MenuItem() {
    delete spacer;
}

void MenuItem::setText(QString const &text) {
    this->mTextLabel.setText(text);
}

QString MenuItem::text() const
{
    return mTextLabel.text();
}

void MenuItem::setShortcutText(QString const &text) {
    this->mShortcutLabel.setText(text);
    this->adjustSize();
}

QString MenuItem::shortcut() const
{
    return mShortcutLabel.text();
}

void MenuItem::setIconPath(QString const &path) {
    mIconWidget.setIconPath(path);
}

void MenuItem::setPassthroughClicks(bool mode) {
    passthroughClicks = mode;
}

void MenuItem::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event)
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void MenuItem::onPress() {
}

void MenuItem::onClick() {
}

void MenuItem::mousePressEvent(QMouseEvent *event) {
    onPress();
    QWidget::mousePressEvent(event);
    if(!passthroughClicks)
        event->accept();
}

void MenuItem::mouseReleaseEvent(QMouseEvent *event) {
    onClick();
    QWidget::mouseReleaseEvent(event);
    if(!passthroughClicks)
        event->accept();
}
