#include "Messagebox.h"
using namespace UICommon;
#include <QDialogButtonBox>
#include <QPushButton>
#include <QLayout>
#include <QCheckBox>
#include <QPointer>
#include <QLabel>
#include <QAbstractButton>
#include <QTextDocument>
#include <QApplication>
#include <QStyle>
#include <QCloseEvent>
#include <QDesktopWidget>
#include <QHBoxLayout>
#include <QStyleOptionButton>
#include <QClipboard>
#include <QDebug>
#include <QAccessible>
#include <QAccessibleEvent>
#include <QFont>
#include "UICommonSizeDefine.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

namespace UICommon {
enum DetailButtonLabel { ShowLabel = 0, HideLabel = 1 };

class DetailButton : public QPushButton
{
public:
    DetailButton(QWidget *parent) : QPushButton(label(ShowLabel), parent)
    {
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    }

    QString label(DetailButtonLabel label) const
    {
        return label == ShowLabel ? QStringLiteral("显示详情...") : QStringLiteral("隐藏详情...");
    }

    void setLabel(DetailButtonLabel lbl)
    {
        setText(label(lbl));
    }

    QSize sizeHint() const Q_DECL_OVERRIDE
    {
        ensurePolished();
        QStyleOptionButton opt;
        initStyleOption(&opt);
        const QFontMetrics fm = fontMetrics();
        opt.text = label(ShowLabel);
        QSize sz = fm.size(Qt::TextShowMnemonic, opt.text);
        QSize ret = style()->sizeFromContents(QStyle::CT_PushButton, &opt, sz, this).
                expandedTo(QApplication::globalStrut());
        opt.text = label(HideLabel);
        sz = fm.size(Qt::TextShowMnemonic, opt.text);
        ret = ret.expandedTo(style()->sizeFromContents(QStyle::CT_PushButton, &opt, sz, this).
                             expandedTo(QApplication::globalStrut()));
        return ret;
    }
};

class CMessageBoxPrivate :public QObject
{
public:
    CMessageBoxPrivate(CMessageBox* mb)
        :pMessageBox(mb)
        ,escapeButton(0),
          defaultButton(0),
          checkbox(0),
          clickedButton(0),
          detailsButton(0),
      #ifndef QT_NO_TEXTEDIT
          detailsText(0),
      #endif
          autoAddOkButton(true),
          detectedEscapeButton(0),
          informativeLabel(0)//,
        //options(QMessageDialogOptions::create()) { }
    {
    }

    void init(const QString &title = QString(), const QString &text = QString());
    void updateSize();
    void detectEscapeButton();
    void setupLayout();
    void retranslateStrings();
    int layoutMinimumWidth();
    int execReturnCode(QAbstractButton *button);

    static QPixmap standardIcon(QMessageBox::Icon icon, CMessageBox *mb);

    void buttonClicked(QAbstractButton *);
    void clicked(QMessageBox::StandardButton button, QMessageBox::ButtonRole role);


    CMessageBox *pMessageBox;
    QLabel *label;
    QMessageBox::Icon icon;
    QLabel *iconLabel;
    QDialogButtonBox *buttonBox;
    QList<QAbstractButton *> customButtonList;
    QAbstractButton *escapeButton;
    QPushButton *defaultButton;
    QCheckBox *checkbox;
    QAbstractButton *clickedButton;
    DetailButton *detailsButton;
#ifndef QT_NO_TEXTEDIT
    QMessageBoxDetailsText *detailsText;
#endif
    bool autoAddOkButton;
    QAbstractButton *detectedEscapeButton;
    QLabel *informativeLabel;
    QPointer<QObject> receiverToDisconnectOnClose;
    QByteArray memberToDisconnectOnClose;
    QByteArray signalToDisconnectOnClose;
    // QSharedPointer<QMessageDialogOptions> options;
};

void CMessageBoxPrivate::init(const QString &title /* = QString() */,
                                      const QString &text /* = QString() */)
{
    pMessageBox->setOKButtonVisible(false);
    pMessageBox->setCancelButtonVisible(false);

    label = new QLabel;
    label->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    label->setObjectName(QLatin1String("qt_msgbox_label"));
    label->setTextInteractionFlags(Qt::TextInteractionFlags(pMessageBox->style()->styleHint(QStyle::SH_MessageBox_TextInteractionFlags, 0, pMessageBox)));
    label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    label->setOpenExternalLinks(true);
    iconLabel = new QLabel(pMessageBox);
    iconLabel->setObjectName(QLatin1String("qt_msgboxex_icon_label"));
    iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	//iconLabel->setFixedSize(QSize(SysDpiZoom(40),SysDpiZoom(30)));
	//iconLabel->setProperty("MessageTip", true);

    buttonBox = new QDialogButtonBox;
	buttonBox->layout()->setSpacing(nSizeDefine_12);
    buttonBox->setObjectName(QLatin1String("qt_msgbox_buttonbox"));
    buttonBox->setCenterButtons(pMessageBox->style()->styleHint(QStyle::SH_MessageBox_CenterButtons, 0, pMessageBox));
	buttonBox->setStyleSheet("QDialogButtonBox{button-layout:1}");
	QObject::connect(buttonBox, SIGNAL(clicked(QAbstractButton*)),
                     pMessageBox, SLOT(slot_buttonClicked(QAbstractButton*)));
    setupLayout();
    if (!title.isEmpty() || !text.isEmpty()) {
        pMessageBox->setWindowTitle(title);
        pMessageBox->setText(text);
    }
    pMessageBox->setModal(true);
#ifdef Q_OS_MAC
    QFont f = pMessageBox->font();
    f.setBold(true);
    label->setFont(f);
#endif
    icon = QMessageBox::NoIcon;
}

void CMessageBoxPrivate::setupLayout()
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);
    mainLayout->addStretch(0);
    QGridLayout *grid = new QGridLayout;
    grid->setContentsMargins(SysDpiZoom(24), SysDpiZoom(30), SysDpiZoom(24), SysDpiZoom(15));
    grid->setVerticalSpacing(0/*SysDpiZoom(16)*/);
	grid->setHorizontalSpacing(SysDpiZoom(16));
    bool hasIcon = iconLabel->pixmap() && !iconLabel->pixmap()->isNull();
    //    if (hasIcon)
    //        grid->addWidget(iconLabel, 0, 0, 2, 1, Qt::AlignVCenter);
    //    if(label)
    //        grid->addWidget(label,0,2,2,2,Qt::AlignVCenter);
    iconLabel->setVisible(hasIcon);
//#ifdef Q_OS_MAC
//    QSpacerItem *indentSpacer = new QSpacerItem(14, 1, QSizePolicy::Fixed, QSizePolicy::Fixed);
//#else
//    QSpacerItem *indentSpacer = new QSpacerItem(hasIcon ? 7 : 15, 1, QSizePolicy::Fixed, QSizePolicy::Fixed);
//#endif
//    grid->addItem(indentSpacer, 0, hasIcon ? 1 : 0, 2, 1);

    if (informativeLabel) {
#ifndef Q_OS_MAC
        informativeLabel->setContentsMargins(0, SysDpiZoom(7), 0, SysDpiZoom(7));
#endif
        if (hasIcon)
            grid->addWidget(iconLabel, 0, 0, Qt::AlignTop);
        grid->setVerticalSpacing(SysDpiZoom(8));

        if(label){
            grid->addWidget(label, 0, hasIcon ? 2 : 1);
        }
        grid->addWidget(informativeLabel, 1, hasIcon ? 2 : 1);
    }
    else
    {
		if (hasIcon)
			grid->addWidget(iconLabel, 0, 0, Qt::AlignVCenter);
        if(label){
			grid->addWidget(label, 0, hasIcon ? 2 : 1, Qt::AlignVCenter);
        }
    }
//    if (checkbox) {
//        grid->addWidget(checkbox, informativeLabel ? 2 : 1, hasIcon ? 2 : 1, 1, 1, Qt::AlignLeft);
//#ifdef Q_OS_MAC
//        grid->addItem(new QSpacerItem(1, 15, QSizePolicy::Fixed, QSizePolicy::Fixed), grid->rowCount(), 0);
//#else
//        grid->addItem(new QSpacerItem(1, 7, QSizePolicy::Fixed, QSizePolicy::Fixed), grid->rowCount(), 0);
//#endif
//    }
    if (detailsText)
        grid->addWidget(detailsText, grid->rowCount(), 0, 1, grid->columnCount());
    //grid->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding), grid->rowCount(), 0, 1, grid->columnCount());
    grid->setSizeConstraint(QLayout::SetNoConstraint);
    mainLayout->addLayout(grid);
    mainLayout->addStretch(0);
	pMessageBox->bottomLayout()->setSpacing(SysDpiZoom(12));
	pMessageBox->bottomLayout()->addWidget(buttonBox);
	pMessageBox->bottomLayout()->setContentsMargins(SysDpiZoom(18), SysDpiZoom(5), SysDpiZoom(24), SysDpiZoom(16));
    pMessageBox->setLayout(mainLayout);

    retranslateStrings();
    //updateSize();
}

int CMessageBoxPrivate::layoutMinimumWidth()
{
    pMessageBox->layout()->activate();
    return pMessageBox->layout()->totalMinimumSize().width();
}

void CMessageBoxPrivate::updateSize()
{
    if (!pMessageBox->isVisible())
        return;

    QSize screenSize = QApplication::desktop()->availableGeometry(QCursor::pos()).size();
    int hardLimit = qMin(screenSize.width() - 480, 1000); // can never get bigger than this
    // on small screens allows the messagebox be the same size as the screen
    if (screenSize.width() <= 1024)
        hardLimit = screenSize.width();
#ifdef Q_OS_MAC
    int softLimit = qMin(screenSize.width() / 2, 320);
#else
    // note: ideally on windows, hard and soft limits but it breaks compat
    int softLimit = qMin(screenSize.width() / 2, 400);
#endif

    if (informativeLabel)
        informativeLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    label->setWordWrap(false); // makes the label return min size
    int width = layoutMinimumWidth();
    label->setWordWrap(true);
    if (width > softLimit) {
        label->setWordWrap(true);
        width = qMax(softLimit, layoutMinimumWidth());

        if (width > hardLimit) {
            //label->d_func()->ensureTextControl();
            //if (QWidgetTextControl *control = label->d_func()->control) {
            //    QTextOption opt = control->document()->defaultTextOption();
            //    opt.setWrapMode(QTextOption::WrapAnywhere);
            //    control->document()->setDefaultTextOption(opt);
            //}
            width = hardLimit;
        }
    }

    if (informativeLabel) {
        label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
        QSizePolicy policy(QSizePolicy::Minimum, QSizePolicy::Preferred);
        policy.setHeightForWidth(true);
        informativeLabel->setSizePolicy(policy);
        width = qMax(width, layoutMinimumWidth());
        if (width > hardLimit) { // longest word is really big, so wrap anywhere
            //informativeLabel->d_func()->ensureTextControl();
            //if (QWidgetTextControl *control = informativeLabel->d_func()->control) {
            //    QTextOption opt = control->document()->defaultTextOption();
            //    opt.setWrapMode(QTextOption::WrapAnywhere);
            //    control->document()->setDefaultTextOption(opt);
            //}
            width = hardLimit;
        }
        policy.setHeightForWidth(label->wordWrap());
        label->setSizePolicy(policy);
    }

    QFontMetrics fm(QApplication::font("QMdiSubWindowTitleBar"));
    int windowTitleWidth = qMin(fm.width(pMessageBox->windowTitle()) + SysDpiZoom(50), hardLimit);
    if (windowTitleWidth > width)
        width = windowTitleWidth;

    pMessageBox->layout()->activate();
    int height = (pMessageBox->layout()->hasHeightForWidth())
            ? pMessageBox->layout()->totalHeightForWidth(width)
            : pMessageBox->layout()->totalMinimumSize().height();

    height += SysDpiZoom(10);/// TODO 明细框现实不全
    height += SysDpiZoom(30) + SysDpiZoom(54); // 头部和底部的高度
    if (width < pMessageBox->minimumWidth())
        width = pMessageBox->minimumWidth();
    if (height < pMessageBox->minimumHeight())
        height = pMessageBox->minimumHeight();
    //pMessageBox->setFixedSize(width, height);
    QCoreApplication::removePostedEvents(pMessageBox, QEvent::LayoutRequest);
}

void CMessageBoxPrivate::detectEscapeButton()
{
    if (escapeButton) { // escape button explicitly set
        detectedEscapeButton = escapeButton;
        return;
    }

    // Cancel button automatically becomes escape button
    detectedEscapeButton = buttonBox->button(QDialogButtonBox::Cancel);
    if (detectedEscapeButton)
        return;

    // If there is only one button, make it the escape button
    const QList<QAbstractButton *> buttons = buttonBox->buttons();
    if (buttons.count() == 1) {
        detectedEscapeButton = buttons.first();
        return;
    }

    // if the message box has one RejectRole button, make it the escape button
    for (auto *button : buttons) {
        if (buttonBox->buttonRole(button) == QDialogButtonBox::RejectRole) {
            if (detectedEscapeButton) { // already detected!
                detectedEscapeButton = 0;
                break;
            }
            detectedEscapeButton = button;
        }
    }
    if (detectedEscapeButton)
        return;

    // if the message box has one NoRole button, make it the escape button
    for (auto *button : buttons) {
        if (buttonBox->buttonRole(button) == QDialogButtonBox::NoRole) {
            if (detectedEscapeButton) { // already detected!
                detectedEscapeButton = 0;
                break;
            }
            detectedEscapeButton = button;
        }
    }
}

QPixmap CMessageBoxPrivate::standardIcon(QMessageBox::Icon icon,
                                                 CMessageBox *mb)
{   
	int iconSize = nSizeDefine_48;
	QIcon tmpIcon(":/dark/images/CommonCtrl/MessageBox/Tip.svg");
    if (!tmpIcon.isNull()) {
        QWindow *window = Q_NULLPTR;
        if (mb) {
            window = mb->windowHandle();
            if (!window) {
                if (const QWidget *nativeParent = mb->nativeParentWidget())
                    window = nativeParent->windowHandle();
            }
        }
        return tmpIcon.pixmap(window, QSize(iconSize, iconSize));
    }
    return QPixmap();
}
void CMessageBoxPrivate::retranslateStrings()
{
#ifndef QT_NO_TEXTEDIT
    if (detailsButton)
        detailsButton->setLabel(detailsText->isHidden() ? ShowLabel : HideLabel);
#endif
}

int CMessageBoxPrivate::execReturnCode(QAbstractButton *button)
{
    int ret = buttonBox->standardButton(button);
    if (ret == QMessageBox::NoButton) {
        ret = customButtonList.indexOf(button); // if button == 0, correctly sets ret = -1
    }
    return ret;
}


void CMessageBoxPrivate::buttonClicked(QAbstractButton *button)
{
#ifndef QT_NO_TEXTEDIT
    if (detailsButton && detailsText && button == detailsButton) {
        detailsButton->setLabel(detailsText->isHidden() ? HideLabel : ShowLabel);
        detailsText->setHidden(!detailsText->isHidden());
        updateSize();
    }
    else
#endif
    {
        clickedButton = button;
        pMessageBox->done(execReturnCode(button)); // does not trigger closeEvent
        emit pMessageBox->buttonClicked(button);

        if (receiverToDisconnectOnClose) {
            QObject::disconnect(pMessageBox, signalToDisconnectOnClose, receiverToDisconnectOnClose,
                                memberToDisconnectOnClose);
            receiverToDisconnectOnClose = 0;
        }
        signalToDisconnectOnClose.clear();
        memberToDisconnectOnClose.clear();
    }
}

void CMessageBoxPrivate::clicked(QMessageBox::StandardButton button, QMessageBox::ButtonRole role)
{
    Q_UNUSED(role);
    pMessageBox->done(button);
}
}

CMessageBox::CMessageBox(QWidget *parent)
    : CFramelessDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint)
    ,d(new CMessageBoxPrivate(this))
{
	setFixedSize(SysDpiZoom(408), SysDpiZoom(166));
    d->init();
}

/*!
    Constructs a message box with the given \a icon, \a title, \a
    text, and standard \a buttons. Standard or custom buttons can be
    added at any time using addButton(). The \a parent and \a f
    arguments are passed to the QDialog constructor.

    The message box is an \l{Qt::ApplicationModal} {application modal}
    dialog box.

    On \macos, if \a parent is not 0 and you want your message box
    to appear as a Qt::Sheet of that parent, set the message box's
    \l{setWindowModality()} {window modality} to Qt::WindowModal
    (default). Otherwise, the message box will be a standard dialog.

    \sa setWindowTitle(), setText(), setIcon(), setStandardButtons()
*/
CMessageBox::CMessageBox(QMessageBox::Icon icon,
                                         const QString &title,
                                         const QString &text,
                                         QMessageBox::StandardButtons buttons, QWidget *parent,
                                         Qt::WindowFlags f)
    : CFramelessDialog(parent, f | Qt::X11BypassWindowManagerHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint)
    , d(new CMessageBoxPrivate(this))
{
	setFixedWidth(SysDpiZoom(408));
    //setMinimumHeight(SysDpiZoom(166));
    d->init(title, text);
    setIcon(icon);

    if (buttons != QMessageBox::NoButton)
        setStandardButtons(buttons);
}

/*!
    Destroys the message box.
*/
CMessageBox::~CMessageBox()
{
    delete d;
    d = nullptr;
}

/*!
    \since 4.2

    Adds the given \a button to the message box with the specified \a
    role.

    \sa removeButton(), button(), setStandardButtons()
*/
void CMessageBox::addButton(QAbstractButton *button, QMessageBox::ButtonRole role)
{
    if (!button)
        return;
    removeButton(button);
    d->buttonBox->addButton(button, (QDialogButtonBox::ButtonRole)role);
    d->customButtonList.append(button);
    d->autoAddOkButton = false;
}

/*!
    \since 4.2
    \overload

    Creates a button with the given \a text, adds it to the message box for the
    specified \a role, and returns it.
*/
QPushButton *CMessageBox::addButton(const QString& text, QMessageBox::ButtonRole role)
{
    QPushButton *pushButton = new QPushButton(text);
    addButton(pushButton, role);
    d->updateSize();
    return pushButton;
}

/*!
    \since 4.2
    \overload

    Adds a standard \a button to the message box if it is valid to do so, and
    returns the push button.

    \sa setStandardButtons()
*/
QPushButton *CMessageBox::addButton(QMessageBox::StandardButton button)
{
    QPushButton *pushButton = d->buttonBox->addButton((QDialogButtonBox::StandardButton)button);
    if (pushButton)
        d->autoAddOkButton = false;
    retranslateButtonUi(button);

	pushButton->setFixedSize(UICommon_SmallBtnWidth, UICommon_CtrlHeight);

    return pushButton;
}

/*!
    \since 4.2

    Removes \a button from the button box without deleting it.

    \sa addButton(), setStandardButtons()
*/
void CMessageBox::removeButton(QAbstractButton *button)
{
    d->customButtonList.removeAll(button);
    if (d->escapeButton == button)
        d->escapeButton = 0;
    if (d->defaultButton == button)
        d->defaultButton = 0;
    d->buttonBox->removeButton(button);
    d->updateSize();
}

/*!
    \property QMessageBox::StandardButtons
    \brief collection of standard buttons in the message box
    \since 4.2

    This property controls which standard buttons are used by the message box.

    By default, this property contains no standard buttons.

    \sa addButton()
*/
void CMessageBox::setStandardButtons(QMessageBox::StandardButtons buttons)
{
    d->buttonBox->setStandardButtons(QDialogButtonBox::StandardButtons(int(buttons)));

    QList<QAbstractButton *> buttonList = d->buttonBox->buttons();
    if (!buttonList.contains(d->escapeButton))
        d->escapeButton = 0;
    if (!buttonList.contains(d->defaultButton))
        d->defaultButton = 0;
    d->autoAddOkButton = false;
    d->updateSize();
}

QMessageBox::StandardButtons CMessageBox::standardButtons() const
{
    return QMessageBox::StandardButtons(int(d->buttonBox->standardButtons()));
}

/*!
    \since 4.2

    Returns the standard button enum value corresponding to the given \a button,
    or NoButton if the given \a button isn't a standard button.

    \sa button(), standardButtons()
*/
QMessageBox::StandardButton CMessageBox::standardButton(QAbstractButton *button) const
{
    return (QMessageBox::StandardButton)d->buttonBox->standardButton(button);
}

/*!
    \since 4.2

    Returns a pointer corresponding to the standard button \a which,
    or 0 if the standard button doesn't exist in this message box.

    \sa standardButtons, standardButton()
*/
QAbstractButton *CMessageBox::button(QMessageBox::StandardButton which) const
{
    return d->buttonBox->button(QDialogButtonBox::StandardButton(which));
}

/*!
    \since 4.2

    Returns the button that is activated when escape is pressed.

    By default, QMessageBox attempts to automatically detect an
    escape button as follows:

    \list 1
    \li If there is only one button, it is made the escape button.
    \li If there is a \l Cancel button, it is made the escape button.
    \li On \macos only, if there is exactly one button with the role
       CFramelessMessageBox::RejectRole, it is made the escape button.
    \endlist

    When an escape button could not be automatically detected, pressing
    \uicontrol Esc has no effect.

    \sa addButton()
*/
QAbstractButton *CMessageBox::escapeButton() const
{
    return d->escapeButton;
}

/*!
    \since 4.2

    Sets the button that gets activated when the \uicontrol Escape key is
    pressed to \a button.

    \sa addButton(), clickedButton()
*/
void CMessageBox::setEscapeButton(QAbstractButton *button)
{   
    if (d->buttonBox->buttons().contains(button))
        d->escapeButton = button;
}

/*!
    \since 4.3

    Sets the buttons that gets activated when the \uicontrol Escape key is
    pressed to \a button.

    \sa addButton(), clickedButton()
*/
void CMessageBox::setEscapeButton(QMessageBox::StandardButton button)
{
    setEscapeButton(d->buttonBox->button(QDialogButtonBox::StandardButton(button)));
}

/*!
    \since 4.2

    Returns the button that was clicked by the user,
    or 0 if the user hit the \uicontrol Esc key and
    no \l{setEscapeButton()}{escape button} was set.

    If exec() hasn't been called yet, returns 0.

    Example:

    \snippet code/src_gui_dialogs_qmessagebox.cpp 3

    \sa standardButton(), button()
*/
QAbstractButton *CMessageBox::clickedButton() const
{
    return d->clickedButton;
}

/*!
    \since 4.2

    Returns the button that should be the message box's
    \l{QPushButton::setDefault()}{default button}. Returns 0
    if no default button was set.

    \sa addButton(), QPushButton::setDefault()
*/
QPushButton *CMessageBox::defaultButton() const
{
    return d->defaultButton;
}

/*!
    \since 4.2

    Sets the message box's \l{QPushButton::setDefault()}{default button}
    to \a button.

    \sa addButton(), QPushButton::setDefault()
*/
void CMessageBox::setDefaultButton(QPushButton *button)
{
    if (!d->buttonBox->buttons().contains(button))
        return;
    d->defaultButton = button;
    button->setDefault(true);
    button->setFocus();
	button->setProperty("BrandBlue", true);
}

/*!
    \since 4.3

    Sets the message box's \l{QPushButton::setDefault()}{default button}
    to \a button.

    \sa addButton(), QPushButton::setDefault()
*/
void CMessageBox::setDefaultButton(QMessageBox::StandardButton button)
{
    setDefaultButton(d->buttonBox->button(QDialogButtonBox::StandardButton(button)));
}

/*! \since 5.2

    Sets the checkbox \a cb on the message dialog. The message box takes ownership of the checkbox.
    The argument \a cb can be 0 to remove an existing checkbox from the message box.

    \sa checkBox()
*/

void CMessageBox::setCheckBox(QCheckBox *cb)
{
    if (cb == d->checkbox)
        return;

    if (d->checkbox) {
        d->checkbox->hide();
        layout()->removeWidget(d->checkbox);
        if (d->checkbox->parentWidget() == this) {
            d->checkbox->setParent(0);
            d->checkbox->deleteLater();
        }
    }
    d->checkbox = cb;
    if (d->checkbox) {
        QSizePolicy sp = d->checkbox->sizePolicy();
        sp.setHorizontalPolicy(QSizePolicy::MinimumExpanding);
        d->checkbox->setSizePolicy(sp);
    }
    d->setupLayout();
}


/*! \since 5.2

    Returns the checkbox shown on the dialog. This is 0 if no checkbox is set.
    \sa setCheckBox()
*/

QCheckBox* CMessageBox::checkBox() const
{
    return d->checkbox;
}

/*!
  \property CFramelessMessageBox::text
  \brief the message box text to be displayed.

  The text will be interpreted either as a plain text or as rich text,
  depending on the text format setting (\l CFramelessMessageBox::textFormat).
  The default setting is Qt::AutoText, i.e., the message box will try
  to auto-detect the format of the text.

  The default value of this property is an empty string.

  \sa textFormat, CFramelessMessageBox::informativeText, CFramelessMessageBox::detailedText
*/
QString CMessageBox::text() const
{
    return d->label->text();
}

void CMessageBox::setText(const QString &text)
{
    d->label->setText(text);
    d->label->setWordWrap(d->label->textFormat() == Qt::RichText
                          || (d->label->textFormat() == Qt::AutoText && Qt::mightBeRichText(text)));
    d->updateSize();
}

/*!
    \enum QMessageBox::Icon

    This enum has the following values:

    \value NoIcon the message box does not have any icon.

    \value Question an icon indicating that
    the message is asking a question.

    \value Information an icon indicating that
    the message is nothing out of the ordinary.

    \value Warning an icon indicating that the
    message is a warning, but can be dealt with.

    \value Critical an icon indicating that
    the message represents a critical problem.

*/

/*!
    \property QMessageBox::Icon
    \brief the message box's icon

    The icon of the message box can be specified with one of the
    values:

    \list
    \li CFramelessMessageBox::NoIcon
    \li CFramelessMessageBox::Question
    \li CFramelessMessageBox::Information
    \li CFramelessMessageBox::Warning
    \li CFramelessMessageBox::Critical
    \endlist

    The default is CFramelessMessageBox::NoIcon.

    The pixmap used to display the actual icon depends on the current
    \l{QWidget::style()} {GUI style}. You can also set a custom pixmap
    for the icon by setting the \l{QMessageBox::IconPixmap} {icon
    pixmap} property.

    \sa iconPixmap
*/
QMessageBox::Icon CMessageBox::icon() const
{
    return d->icon;
}

void CMessageBox::setIcon(QMessageBox::Icon icon)
{
    setIconPixmap(CMessageBoxPrivate::standardIcon((QMessageBox::Icon)icon,
                                                           this));
    d->icon = icon;
}

/*!
    \property QMessageBox::IconPixmap
    \brief the current icon

    The icon currently used by the message box. Note that it's often
    hard to draw one pixmap that looks appropriate in all GUI styles;
    you may want to supply a different pixmap for each platform.

    By default, this property is undefined.

    \sa icon
*/
QPixmap CMessageBox::iconPixmap() const
{

    if (d->iconLabel && d->iconLabel->pixmap())
        return *d->iconLabel->pixmap();
    return QPixmap();
}

void CMessageBox::setIconPixmap(const QPixmap &pixmap)
{
    d->iconLabel->setPixmap(pixmap);
    d->icon = QMessageBox::NoIcon;
    d->setupLayout();
}

/*!
    \property CFramelessMessageBox::textFormat
    \brief the format of the text displayed by the message box

    The current text format used by the message box. See the \l
    Qt::TextFormat enum for an explanation of the possible options.

    The default format is Qt::AutoText.

    \sa setText()
*/
Qt::TextFormat CMessageBox::textFormat() const
{
    return d->label->textFormat();
}

void CMessageBox::setTextFormat(Qt::TextFormat format)
{
    d->label->setTextFormat(format);
    d->label->setWordWrap(format == Qt::RichText
                          || (format == Qt::AutoText && Qt::mightBeRichText(d->label->text())));
    d->updateSize();
}

/*!
    \property CFramelessMessageBox::textInteractionFlags
    \since 5.1

    Specifies how the label of the message box should interact with user
    input.

    The default value depends on the style.

    \sa QStyle::SH_MessageBox_TextInteractionFlags
*/

Qt::TextInteractionFlags CMessageBox::textInteractionFlags() const
{
    return d->label->textInteractionFlags();
}

void CMessageBox::setTextInteractionFlags(Qt::TextInteractionFlags flags)
{
    d->label->setTextInteractionFlags(flags);
}

/*!
    \reimp
*/
bool CMessageBox::event(QEvent *e)
{
    bool result = CFramelessDialog::event(e);
    switch (e->type()) {
    case QEvent::LayoutRequest:
        d->updateSize();
        break;
    case QEvent::LanguageChange:
        d->retranslateStrings();
        break;
    default:
        break;
    }
    return result;
}

/*!
    \reimp
*/
void CMessageBox::resizeEvent(QResizeEvent *event)
{
    CFramelessDialog::resizeEvent(event);
}

/*!
    \reimp
*/
void CMessageBox::closeEvent(QCloseEvent *e)
{
    if (!d->detectedEscapeButton) {
        e->ignore();
        return;
    }
    CFramelessDialog::closeEvent(e);
    d->clickedButton = d->detectedEscapeButton;
    setResult(d->execReturnCode(d->detectedEscapeButton));
}

/*!
    \reimp
*/
void CMessageBox::changeEvent(QEvent *ev)
{
    switch (ev->type()) {
    case QEvent::StyleChange:
    {
        if (d->icon != QMessageBox::NoIcon)
            setIcon(d->icon);
        Qt::TextInteractionFlags flags(style()->styleHint(QStyle::SH_MessageBox_TextInteractionFlags, 0, this));
        d->label->setTextInteractionFlags(flags);
        d->buttonBox->setCenterButtons(style()->styleHint(QStyle::SH_MessageBox_CenterButtons, 0, this));
        if (d->informativeLabel)
            d->informativeLabel->setTextInteractionFlags(flags);
        // Q_FALLTHROUGH();
    }
    case QEvent::FontChange:
    case QEvent::ApplicationFontChange:
#ifdef Q_OS_MAC
    {
        QFont f = font();
        f.setBold(true);
        d->label->setFont(f);
    }
#endif
        // Q_FALLTHROUGH();
    default:
        break;
    }
    CFramelessDialog::changeEvent(ev);
}

/*!
    \reimp
*/
void CMessageBox::keyPressEvent(QKeyEvent *e)
{
    if (e->matches(QKeySequence::Cancel)) {
        if (d->detectedEscapeButton) {
#ifdef Q_OS_MAC
            d->detectedEscapeButton->animateClick();
#else
            d->detectedEscapeButton->click();
#endif
        }
        return;
    }


#if !defined(QT_NO_CLIPBOARD) && !defined(QT_NO_SHORTCUT)

#if !defined(QT_NO_TEXTEDIT)
    if (e == QKeySequence::Copy) {
        if (d->detailsText && d->detailsText->isVisible() && d->detailsText->copy()) {
            e->setAccepted(true);
            return;
        }
    } else if (e == QKeySequence::SelectAll && d->detailsText && d->detailsText->isVisible()) {
        d->detailsText->selectAll();
        e->setAccepted(true);
        return;
    }
#endif // !QT_NO_TEXTEDIT

#if defined(Q_OS_WIN)
    if (e == QKeySequence::Copy) {
        const QLatin1String separator("---------------------------\n");
        QString textToCopy;
        textToCopy += separator + windowTitle() + QLatin1Char('\n') + separator // title
                + d->label->text() + QLatin1Char('\n') + separator;       // text

        if (d->informativeLabel)
            textToCopy += d->informativeLabel->text() + QLatin1Char('\n') + separator;

        const QList<QAbstractButton *> buttons = d->buttonBox->buttons();
        for (const auto *button : buttons)
            textToCopy += button->text() + QLatin1String("   ");
        textToCopy += QLatin1Char('\n') + separator;
#ifndef QT_NO_TEXTEDIT
        if (d->detailsText)
            textToCopy += d->detailsText->text() + QLatin1Char('\n') + separator;
#endif
        QApplication::clipboard()->setText(textToCopy);
        return;
    }
#endif // Q_OS_WIN

#endif // !QT_NO_CLIPBOARD && !QT_NO_SHORTCUT

#ifndef QT_NO_SHORTCUT
    if (!(e->modifiers() & (Qt::AltModifier | Qt::ControlModifier | Qt::MetaModifier))) {
        int key = e->key() & ~Qt::MODIFIER_MASK;
        if (key) {
            const QList<QAbstractButton *> buttons = d->buttonBox->buttons();
            for (auto *pb : buttons) {
                QKeySequence shortcut = pb->shortcut();
                if (!shortcut.isEmpty() && key == int(shortcut[0] & ~Qt::MODIFIER_MASK)) {
                    pb->animateClick();
                    return;
                }
            }
        }
    }
#endif
    CFramelessDialog::keyPressEvent(e);
}

/*!
    \overload

    Opens the dialog and connects its finished() or buttonClicked() signal to
    the slot specified by \a receiver and \a member. If the slot in \a member
    has a pointer for its first parameter the connection is to buttonClicked(),
    otherwise the connection is to finished().

    The signal will be disconnected from the slot when the dialog is closed.
*/
void CMessageBox::open(QObject *receiver, const char *member)
{   
    const char *signal = member && strchr(member, '*') ? SIGNAL(buttonClicked(QAbstractButton*))
                                                       : SIGNAL(finished(int));
    connect(this, signal, receiver, member);
    d->signalToDisconnectOnClose = signal;
    d->receiverToDisconnectOnClose = receiver;
    d->memberToDisconnectOnClose = member;
    CFramelessDialog::open();
}

/*!
    \since 4.5

    Returns a list of all the buttons that have been added to the message box.

    \sa buttonRole(), addButton(), removeButton()
*/
QList<QAbstractButton *> CMessageBox::buttons() const
{   
    return d->buttonBox->buttons();
}

/*!
    \since 4.5

    Returns the button role for the specified \a button. This function returns
    \l InvalidRole if \a button is 0 or has not been added to the message box.

    \sa buttons(), addButton()
*/
QMessageBox::ButtonRole CMessageBox::buttonRole(QAbstractButton *button) const
{
    return QMessageBox::ButtonRole(d->buttonBox->buttonRole(button));
}

/*!
    \reimp
*/
void CMessageBox::showEvent(QShowEvent *e)
{
    if (d->autoAddOkButton) {
        addButton(QMessageBox::Ok);
    }
    if (d->detailsButton)
        addButton(d->detailsButton, QMessageBox::ActionRole);
    d->detectEscapeButton();
    d->updateSize();
#ifndef QT_NO_ACCESSIBILITY
    QAccessibleEvent event(this, QAccessible::Alert);
    QAccessible::updateAccessibility(&event);
#endif
    setCloseButtonVisible(getCloseButtonVisible() && d->detectedEscapeButton);
    CFramelessDialog::showEvent(e);
}


static QMessageBox::StandardButton showNewMessageBox(QWidget *parent,
                                                     QMessageBox::Icon icon,
                                                     const QString& title, const QString& text,
                                                     QMessageBox::StandardButtons buttons,
                                                     QMessageBox::StandardButton defaultButton)
{
    CMessageBox msgBox(icon, title, text, QMessageBox::NoButton, parent);
    QDialogButtonBox *buttonBox = msgBox.findChild<QDialogButtonBox*>();
    Q_ASSERT(buttonBox != 0);

    uint mask = QMessageBox::FirstButton;
    while (mask <= QMessageBox::LastButton) {
        uint sb = buttons & mask;
        mask <<= 1;
        if (!sb)
            continue;
        QPushButton *button = msgBox.addButton((QMessageBox::StandardButton)sb);
        // Choose the first accept role as the default
        if (msgBox.defaultButton())
            continue;
        if ((defaultButton == QMessageBox::NoButton && buttonBox->buttonRole(button) == QDialogButtonBox::AcceptRole)
                || (defaultButton != QMessageBox::NoButton && sb == uint(defaultButton)))
            msgBox.setDefaultButton(button);
    }
    if (msgBox.exec() == -1)
        return QMessageBox::Cancel;
    return msgBox.standardButton(msgBox.clickedButton());
}


//主要解决将不同软件集合到vsp中，每个软件使用不同的风格
static QMessageBox::StandardButton showNewMessageBox(QWidget *parent,
                                                     QMessageBox::Icon icon,
                                                     const QString& title, const QString& text,
                                                     char* propertyName, bool propertyVal,
                                                     QMessageBox::StandardButtons buttons,
                                                     QMessageBox::StandardButton defaultButton,
                                                     bool bShowCloseButton = true, const QString& text2=QString(),
													 bool bInformationProperty = false)
{
    CMessageBox msgBox(icon, title, text, QMessageBox::NoButton, parent);
	msgBox.setInformativeText(text2);
	if (bInformationProperty)
		msgBox.setInformationProperty(propertyName, propertyVal);
	else
		msgBox.setProperty(propertyName, propertyVal);
    QDialogButtonBox *buttonBox = msgBox.findChild<QDialogButtonBox*>();
    Q_ASSERT(buttonBox != 0);

    if(!bShowCloseButton)
        msgBox.setCloseButtonVisible(bShowCloseButton);

    uint mask = QMessageBox::FirstButton;
    while (mask <= QMessageBox::LastButton) {
        uint sb = buttons & mask;
        mask <<= 1;
        if (!sb)
            continue;
        QPushButton *button = msgBox.addButton((QMessageBox::StandardButton)sb);
        // Choose the first accept role as the default
        if (msgBox.defaultButton())
            continue;
        if ((defaultButton == QMessageBox::NoButton && buttonBox->buttonRole(button) == QDialogButtonBox::AcceptRole)
                || (defaultButton != QMessageBox::NoButton && sb == uint(defaultButton)))
            msgBox.setDefaultButton(button);
    }

#ifdef Q_OS_LINUX
    if(parent)
        msgBox.move((parent->width()-msgBox.width())/2, (parent->height()-msgBox.height())/2);
#endif

    if (msgBox.exec() == -1)
        return QMessageBox::Cancel;
    return msgBox.standardButton(msgBox.clickedButton());
}
/*!
    \since 4.2

    Opens an information message box with the given \a title and
    \a text in front of the specified \a parent widget.

    The standard \a buttons are added to the message box.
    \a defaultButton specifies the button used when \uicontrol Enter is pressed.
    \a defaultButton must refer to a button that was given in \a buttons.
    If \a defaultButton is CFramelessMessageBox::NoButton, QMessageBox
    chooses a suitable default automatically.

    Returns the identity of the standard button that was clicked. If
    \uicontrol Esc was pressed instead, the \l{Default and Escape Keys}
    {escape button} is returned.

    The message box is an \l{Qt::ApplicationModal}{application modal}
    dialog box.

    \warning Do not delete \a parent during the execution of the dialog.
             If you want to do this, you should create the dialog
             yourself using one of the QMessageBox constructors.

    \sa question(), warning(), critical()
*/
QMessageBox::StandardButton CMessageBox::information(
        QWidget *parent,
        const QString &title,
        const QString& text,
        QMessageBox::StandardButtons buttons,
        QMessageBox::StandardButton defaultButton)
{
    return showNewMessageBox(parent, QMessageBox::Information, title, text, buttons,
                             defaultButton);
}

QMessageBox::StandardButton CMessageBox::information(QWidget *parent, const QString &title,
                                                             const QString &text, char *propertyName,
                                                             bool propertyVal, QMessageBox::StandardButtons buttons,
                                                             QMessageBox::StandardButton defaultButton, bool bShowCloseButton, const QString& text2)
{
    return showNewMessageBox(parent, QMessageBox::Information, title, text, propertyName, propertyVal, buttons,
                             defaultButton,bShowCloseButton, text2);
}


/*!
    \since 4.2

    Opens a question message box with the given \a title and \a
    text in front of the specified \a parent widget.

    The standard \a buttons are added to the message box. \a
    defaultButton specifies the button used when \uicontrol Enter is
    pressed. \a defaultButton must refer to a button that was given in \a buttons.
    If \a defaultButton is CFramelessMessageBox::NoButton, QMessageBox
    chooses a suitable default automatically.

    Returns the identity of the standard button that was clicked. If
    \uicontrol Esc was pressed instead, the \l{Default and Escape Keys}
    {escape button} is returned.

    The message box is an \l{Qt::ApplicationModal} {application modal}
    dialog box.

    \warning Do not delete \a parent during the execution of the dialog.
             If you want to do this, you should create the dialog
             yourself using one of the QMessageBox constructors.

    \sa information(), warning(), critical()
*/
QMessageBox::StandardButton CMessageBox::question(
        QWidget *parent,
        const QString &title,
        const QString& text,
        QMessageBox::StandardButtons buttons,
        QMessageBox::StandardButton defaultButton)
{
    return showNewMessageBox(parent, QMessageBox::Question, title, text, buttons, defaultButton);
}

QMessageBox::StandardButton CMessageBox::question(QWidget *parent, const QString &title,
                                                          const QString &text, char *propertyName,
                                                          bool propertyVal, QMessageBox::StandardButtons buttons,
                                                          QMessageBox::StandardButton defaultButton, bool bShowCloseButton, const QString& text2)
{
    return showNewMessageBox(parent, QMessageBox::Question, title, text, propertyName,
                             propertyVal, buttons, defaultButton,bShowCloseButton, text2);
}

/*!
    \since 4.2

    Opens a warning message box with the given \a title and \a
    text in front of the specified \a parent widget.

    The standard \a buttons are added to the message box. \a
    defaultButton specifies the button used when \uicontrol Enter is
    pressed. \a defaultButton must refer to a button that was given in \a buttons.
    If \a defaultButton is CFramelessMessageBox::NoButton, QMessageBox
    chooses a suitable default automatically.

    Returns the identity of the standard button that was clicked. If
    \uicontrol Esc was pressed instead, the \l{Default and Escape Keys}
    {escape button} is returned.

    The message box is an \l{Qt::ApplicationModal} {application modal}
    dialog box.

    \warning Do not delete \a parent during the execution of the dialog.
             If you want to do this, you should create the dialog
             yourself using one of the QMessageBox constructors.

    \sa question(), information(), critical()
*/
QMessageBox::StandardButton CMessageBox::warning(
        QWidget *parent,
        const QString &title,
        const QString& text,
        QMessageBox::StandardButtons buttons,
        QMessageBox::StandardButton defaultButton)
{
    return showNewMessageBox(parent, QMessageBox::Warning, title, text, buttons, defaultButton);
}

QMessageBox::StandardButton CMessageBox::warning(
        QWidget *parent,
        const QString &title,
        const QString& text,
        char* propertyName,
        bool propertyVal,
        QMessageBox::StandardButtons buttons,
        QMessageBox::StandardButton defaultButton,
        bool bShowCloseButton, const QString& text2, bool bInformationProperty)
{
    return showNewMessageBox(parent, QMessageBox::Warning, title, text, propertyName,
                             propertyVal, buttons, defaultButton, bShowCloseButton, text2, bInformationProperty);
}

/*!
    \since 4.2

    Opens a critical message box with the given \a title and \a
    text in front of the specified \a parent widget.

    The standard \a buttons are added to the message box. \a
    defaultButton specifies the button used when \uicontrol Enter is
    pressed. \a defaultButton must refer to a button that was given in \a buttons.
    If \a defaultButton is CFramelessMessageBox::NoButton, QMessageBox
    chooses a suitable default automatically.

    Returns the identity of the standard button that was clicked. If
    \uicontrol Esc was pressed instead, the \l{Default and Escape Keys}
    {escape button} is returned.

    The message box is an \l{Qt::ApplicationModal} {application modal}
    dialog box.

    \warning Do not delete \a parent during the execution of the dialog.
             If you want to do this, you should create the dialog
             yourself using one of the QMessageBox constructors.

    \sa question(), warning(), information()
*/
QMessageBox::StandardButton CMessageBox::critical(
        QWidget *parent,
        const QString &title,
        const QString& text,
        QMessageBox::StandardButtons buttons,
        QMessageBox::StandardButton defaultButton)
{
    return showNewMessageBox(parent, QMessageBox::Critical, title, text, buttons, defaultButton);
}

QMessageBox::StandardButton CMessageBox::critical(QWidget *parent, const QString &title,
                                                          const QString &text, char *propertyName,
                                                          bool propertyVal, QMessageBox::StandardButtons buttons,
                                                          QMessageBox::StandardButton defaultButton, bool bShowCloseButton, const QString& text2)
{
    return showNewMessageBox(parent, QMessageBox::Critical, title, text,
                             propertyName, propertyVal, buttons, defaultButton, bShowCloseButton, text2);
}

/*!
    Displays a simple about box with title \a title and text \a
    text. The about box's parent is \a parent.

    about() looks for a suitable icon in four locations:

    \list 1
    \li It prefers \l{QWidget::windowIcon()}{parent->icon()}
    if that exists.
    \li If not, it tries the top-level widget containing \a parent.
    \li If that fails, it tries the \l{QApplication::activeWindow()}{active window.}
    \li As a last resort it uses the Information icon.
    \endlist

    The about box has a single button labelled "OK". On \macos, the
    about box is popped up as a modeless window; on other platforms,
    it is currently application modal.

    \sa QWidget::windowIcon(), QApplication::activeWindow()
*/
void CMessageBox::about(QWidget *parent, const QString &title, const QString &text)
{
#ifdef Q_OS_MAC
    static QPointer<CMessageBox> oldMsgBox;

    if (oldMsgBox && oldMsgBox->text() == text) {
        oldMsgBox->show();
        oldMsgBox->raise();
        oldMsgBox->activateWindow();
        return;
    }
#endif

    CMessageBox *msgBox = new CMessageBox(QMessageBox::Information,title, text ,QMessageBox::Ok, parent
                                                      #ifdef Q_OS_MAC
                                                          , Qt::WindowTitleHint | Qt::WindowSystemMenuHint
                                                      #endif
                                                          );
    msgBox->setAttribute(Qt::WA_DeleteOnClose);
    QIcon icon = msgBox->windowIcon();
    QSize size = icon.actualSize(QSize(SysDpiZoom(64), SysDpiZoom(64)));
    msgBox->setIconPixmap(icon.pixmap(size));

    // should perhaps be a style hint
#ifdef Q_OS_MAC
    oldMsgBox = msgBox;
#if 0
    // ### doesn't work until close button is enabled in title bar
    msgBox->d->autoAddOkButton = false;
#else
    msgBox->d->buttonBox->setCenterButtons(true);
#endif
    msgBox->show();
#else
    msgBox->exec();
#endif
}

/*!
    Displays a simple message box about Qt, with the given \a title
    and centered over \a parent (if \a parent is not 0). The message
    includes the version number of Qt being used by the application.

    This is useful for inclusion in the \uicontrol Help menu of an application,
    as shown in the \l{mainwindows/menus}{Menus} example.

    QApplication provides this functionality as a slot.

    On \macos, the about box is popped up as a modeless window; on
    other platforms, it is currently application modal.

    \sa QApplication::aboutQt()
*/
void CMessageBox::aboutQt(QWidget *parent, const QString &title)
{
#ifdef Q_OS_MAC
    static QPointer<CMessageBox> oldMsgBox;

    if (oldMsgBox) {
        oldMsgBox->show();
        oldMsgBox->raise();
        oldMsgBox->activateWindow();
        return;
    }
#endif

    QString translatedTextAboutQtCaption;
    translatedTextAboutQtCaption = QStringLiteral("<h3>关于 Qt</h3><p>本程序使用 Qt 版本 %1。</p>")
                .arg(QLatin1String(QT_VERSION_STR));
    QString translatedTextAboutQtText;
    translatedTextAboutQtText = QStringLiteral(
                "<p>Qt 是用于跨平台应用程序开发的 C++ 工具包。</p>"
                "<p>更多信息请访问 <a href=\"http://%2/\">%2</a>。</p>"
                "<p>Copyright (C) %1 The Qt Company Ltd and other contributors.</p>"
                "<p>详见 <a href=\"http://%3/\">%3</a>。</p>"
                ).arg(QStringLiteral("2016"),
                      QStringLiteral("qt.io/licensing"),
                      QStringLiteral("qt.io"));
    CMessageBox *msgBox = new CMessageBox(parent);
    msgBox->setAttribute(Qt::WA_DeleteOnClose);
    msgBox->setWindowTitle(title.isEmpty() ? QStringLiteral("关于 Qt") : title);
    msgBox->setText(translatedTextAboutQtCaption);
    msgBox->setInformativeText(translatedTextAboutQtText);

    QPixmap pm(QLatin1String(":/qt-project.org/qmessagebox/images/qtlogo-64.png"));
    if (!pm.isNull())
        msgBox->setIconPixmap(pm);

    // should perhaps be a style hint
#ifdef Q_OS_MAC
    oldMsgBox = msgBox;
#if 0
    // ### doesn't work until close button is enabled in title bar
    msgBox->d->autoAddOkButton = false;
#else
    msgBox->d->buttonBox->setCenterButtons(true);
#endif
    msgBox->show();
#else
    msgBox->exec();
#endif
}

#ifndef QT_NO_TEXTEDIT
/*!
  \property CFramelessMessageBox::detailedText
  \brief the text to be displayed in the details area.
  \since 4.2

  The text will be interpreted as a plain text.

  By default, this property contains an empty string.

  \sa CFramelessMessageBox::text, CFramelessMessageBox::informativeText
*/
QString CMessageBox::detailedText() const
{
    return d->detailsText ?  d->detailsText->text() : QString();
}

void CMessageBox::setDetailedText(const QString &text)
{
    if (text.isEmpty()) {
        if (d->detailsText) {
            d->detailsText->hide();
            d->detailsText->deleteLater();
        }
        d->detailsText = 0;
        removeButton(d->detailsButton);
        if (d->detailsButton) {
            d->detailsButton->hide();
            d->detailsButton->deleteLater();
        }
        d->detailsButton = 0;
    } else {
        if (!d->detailsText) {
            d->detailsText = new QMessageBoxDetailsText(this);
            d->detailsText->hide();
        }
        if (!d->detailsButton) {
            const bool autoAddOkButton = d->autoAddOkButton; // QTBUG-39334, addButton() clears the flag.
            d->detailsButton = new DetailButton(this);
            addButton(d->detailsButton, QMessageBox::ActionRole);
            d->autoAddOkButton = autoAddOkButton;
        }
        d->detailsText->setText(text);
    }
    d->setupLayout();
}
#endif // QT_NO_TEXTEDIT

/*!
  \property CFramelessMessageBox::informativeText

  \brief the informative text that provides a fuller description for
  the message

  \since 4.2

  Infromative text can be used to expand upon the text() to give more
  information to the user. On the Mac, this text appears in small
  system font below the text().  On other platforms, it is simply
  appended to the existing text.

  By default, this property contains an empty string.

  \sa CFramelessMessageBox::text, CFramelessMessageBox::detailedText
*/
QString CMessageBox::informativeText() const
{
    return d->informativeLabel ? d->informativeLabel->text() : QString();
}

void CMessageBox::setInformativeText(const QString &text)
{
    if (text.isEmpty()) {
        if (d->informativeLabel) {
            d->informativeLabel->hide();
            d->informativeLabel->deleteLater();
        }
        d->informativeLabel = 0;
    } else {
        if (!d->informativeLabel) {
            QLabel *label = new QLabel;
            label->setObjectName(QLatin1String("qt_msgbox_informativelabel"));
            label->setTextInteractionFlags(Qt::TextInteractionFlags(style()->styleHint(QStyle::SH_MessageBox_TextInteractionFlags, 0, this)));
            label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
            label->setOpenExternalLinks(true);
            label->setWordWrap(true);
#ifdef Q_OS_MAC
            /// TODO apply a smaller font the information label on the mac
            //label->setFont(qt_app_fonts_hash()->value("QTipLabel"));
#endif
            label->setWordWrap(true);
            d->informativeLabel = label;
        }
        d->informativeLabel->setText(text);
    }
    d->setupLayout();
}

/*!
    \since 4.2

    This function shadows QWidget::setWindowTitle().

    Sets the title of the message box to \a title. On \macos,
    the window title is ignored (as required by the \macos
    Guidelines).
*/
void CMessageBox::setWindowTitle(const QString &title)
{
    //    // Message boxes on the mac do not have a title
    //#ifndef Q_OS_MAC
    //    FramelessDialog::setWindowTitle(title);
    //#else
    //    Q_UNUSED(title);
    //#endif
    CFramelessDialog::setWindowTitle(title);
}


/*!
    \since 4.2

    This function shadows QWidget::setWindowModality().

    Sets the modality of the message box to \a windowModality.

    On \macos, if the modality is set to Qt::WindowModal and the message box
    has a parent, then the message box will be a Qt::Sheet, otherwise the
    message box will be a standard dialog.
*/
void CMessageBox::setWindowModality(Qt::WindowModality windowModality)
{
    CFramelessDialog::setWindowModality(windowModality);

    if (parentWidget() && windowModality == Qt::WindowModal)
        setParent(parentWidget(), Qt::Sheet);
    else
        setParent(parentWidget(), Qt::Dialog);
    setDefaultButton(d_func()->defaultButton);
}

void CMessageBox::slot_buttonClicked(QAbstractButton *pButton)
{
    d->buttonClicked(pButton);
}

void CMessageBox::slot_clicked(QMessageBox::StandardButton button, QMessageBox::ButtonRole role)
{
    d->clicked(button, role);
}

void CMessageBox::retranslateButtonUi(QMessageBox::StandardButton button)
{
    QPushButton *pButton = d->buttonBox->button((QDialogButtonBox::StandardButton)button);
    if (nullptr == pButton)
        return;
    switch (button) {
    case QMessageBox::Ok:
        pButton->setText(QStringLiteral("确定"));
        break;
    case QMessageBox::Save:
        pButton->setText(QStringLiteral("保存"));
        break;
    case QMessageBox::SaveAll:
        pButton->setText(QStringLiteral("保存"));
        break;
    case QMessageBox::Open:
        pButton->setText(QStringLiteral("打开"));
        break;
    case QMessageBox::Yes:
        pButton->setText(QStringLiteral("是"));
        break;
    case QMessageBox::YesToAll:
        pButton->setText(QStringLiteral("是"));
        break;
    case QMessageBox::No:
        pButton->setText(QStringLiteral("否"));
        break;
    case QMessageBox::NoToAll:
        pButton->setText(QStringLiteral("否"));
        break;
    case QMessageBox::Abort:
		pButton->setText(pButton->fontMetrics().elidedText(QStringLiteral("不保存"), Qt::ElideRight, UICommon_SmallBtnWidth - nSizeDefine_8));
		pButton->setToolTip(QStringLiteral("不保存"));
		break;
    case QMessageBox::Retry:
        pButton->setText(QStringLiteral("重试"));
        break;
    case QMessageBox::Ignore:
        pButton->setText(QStringLiteral("忽略"));
        break;
    case QMessageBox::Close:
        pButton->setText(QStringLiteral("关闭"));
        break;
    case QMessageBox::Cancel:
        pButton->setText(QStringLiteral("取消"));
        break;
    case QMessageBox::Discard:
        pButton->setText(QStringLiteral("丢弃"));
        break;
    case QMessageBox::Help:
        pButton->setText(QStringLiteral("帮助"));
        break;
    case QMessageBox::Apply:
        pButton->setText(QStringLiteral("应用"));
        break;
    case QMessageBox::Reset:
        pButton->setText(QStringLiteral("重置"));
        break;
    default:
        break;
    }

	if (button >= QMessageBox::Ok && button <= QMessageBox::YesToAll || button == QMessageBox::Apply || button == QMessageBox::Retry)
	{
		pButton->setStyleSheet("QPushButton {"
			" background-color: rgba(65, 139, 255);"
			" color: rgb(255, 255, 255);}"
			"QPushButton:hover:!pressed {"
			"background-color: rgb(77, 148, 255);}"
			"QPushButton:checked,"
			"QPushButton:pressed{"
			"background-color: rgb(96, 160, 255);}"
			"QPushButton:disable{"
			"background-color: rgb(35, 79, 145);}"
		);
	}


}
//#include "moc_FramelessMessageBox.cpp"
//#include "FramelessMessageBox.moc"

void CMessageBox::setInformationProperty(const char * porperty, bool propertyVal)
{
	if (d->informativeLabel)
		d->informativeLabel->setProperty(porperty, propertyVal);
}
