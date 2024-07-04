#include "MenuItem.h"

MenuItem::MenuItem(QWidget *parent)
    : QWidget(parent),
      mIconWidget(new IconButton(this)),
      spacer (new QSpacerItem(3, 1, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed)),
      mLayout(new QHBoxLayout(this))
{
    mLayout->setContentsMargins(6, 0, 8, 0);
    mLayout->setSpacing(2);

    setAccessibleName(u"MenuItem"_s);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    mTextLabel.setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    mShortcutLabel.setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    mIconWidget->setMinimumSize(26, 26); // 5px padding from stylesheet
    mIconWidget->installEventFilter(this);
    mIconWidget->setAttribute(Qt::WA_TransparentForMouseEvents);
    mIconWidget->setAccessibleName(u"MenuItemIcon"_s);

    mTextLabel.setAccessibleName(u"MenuItemText"_s);
    mShortcutLabel.setAccessibleName(u"MenuItemShortcutLabel"_s);

    mLayout->addWidget(mIconWidget);
    mLayout->addWidget(&mTextLabel);
    mLayout->addSpacerItem(spacer);
    mLayout->addWidget(&mShortcutLabel);
    mLayout->setStretch(1, 1);
    setLayout(mLayout);
}

MenuItem::~MenuItem() = default;

void MenuItem::setText(QString const &text)
{
    mTextLabel.setText(text);
}

QString MenuItem::text() const
{
    return mTextLabel.text();
}

void MenuItem::setShortcutText(QString const &text)
{
    mShortcutLabel.setText(text);
    adjustSize();
}

QString MenuItem::shortcut() const
{
    return mShortcutLabel.text();
}

void MenuItem::setIconPath(QString const &path)
{
    mIconWidget->setIconPath(path);
}

void MenuItem::setPassthroughClicks(bool mode)
{
    passthroughClicks = mode;
}

void MenuItem::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void MenuItem::onPress()
{
}

void MenuItem::onClick()
{
}

void MenuItem::mousePressEvent(QMouseEvent *event)
{
    onPress();
    QWidget::mousePressEvent(event);
    if (!passthroughClicks)
        event->accept();
}

void MenuItem::mouseReleaseEvent(QMouseEvent *event)
{
    onClick();
    QWidget::mouseReleaseEvent(event);
    if (!passthroughClicks)
        event->accept();
}
