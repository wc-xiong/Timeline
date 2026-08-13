#pragma once
#include "UICommonGlobal.h"
#include "Framelessdialog.h"
#include <QDialog>
#include <QMessageBox>
#include <QString>
#include <QTextEdit>
#include <QMenu>
#include <QContextMenuEvent>
#include <QVBoxLayout>

QT_BEGIN_NAMESPACE
class QLabel;
class QMessageBoxPrivate;
class QAbstractButton;
class QCheckBox;
QT_END_NAMESPACE

namespace UICommon{
#ifndef QT_NO_TEXTEDIT
	class QMessageBoxDetailsText : public QWidget
	{
		Q_OBJECT
	public:
		class TextEdit : public QTextEdit
		{
		public:
			TextEdit(QWidget *parent = 0) : QTextEdit(parent) { }
#ifndef QT_NO_CONTEXTMENU
			void contextMenuEvent(QContextMenuEvent * e) Q_DECL_OVERRIDE
			{
				QMenu *menu = createStandardContextMenu();
				menu->setAttribute(Qt::WA_DeleteOnClose);
				menu->popup(e->globalPos());
			}
#endif // QT_NO_CONTEXTMENU
		};

		QMessageBoxDetailsText(QWidget *parent = 0)
			: QWidget(parent)
			, copyAvailable(false)
		{
			QVBoxLayout *layout = new QVBoxLayout;
			layout->setMargin(0);
			QFrame *line = new QFrame(this);
			line->setFrameShape(QFrame::HLine);
			line->setFrameShadow(QFrame::Sunken);
			layout->addWidget(line);
			textEdit = new TextEdit();
			textEdit->setFixedHeight(100);
			textEdit->setFocusPolicy(Qt::NoFocus);
			textEdit->setReadOnly(true);
			layout->addWidget(textEdit);
			setLayout(layout);

			connect(textEdit, SIGNAL(copyAvailable(bool)),
				this, SLOT(textCopyAvailable(bool)));
		}
		void setText(const QString &text) { textEdit->setPlainText(text); }
		QString text() const { return textEdit->toPlainText(); }

		bool copy()
		{
#ifdef QT_NO_CLIPBOARD
			return false;
#else
			if (!copyAvailable)
				return false;
			textEdit->copy();
			return true;
#endif
		}

		void selectAll()
		{
			textEdit->selectAll();
		}

	private slots:
		void textCopyAvailable(bool available)
		{
			copyAvailable = available;
		}

	private:
		bool copyAvailable;
		TextEdit *textEdit;
	};
#endif // QT_NO_TEXTEDIT
    class CMessageBoxPrivate;
    class UICOMMON_EXPORT CMessageBox : public CFramelessDialog
    {
        Q_OBJECT
			Q_PROPERTY(QString text READ text WRITE setText)
			Q_PROPERTY(QMessageBox::Icon icon READ icon WRITE setIcon)
			Q_PROPERTY(QPixmap iconPixmap READ iconPixmap WRITE setIconPixmap)
			Q_PROPERTY(Qt::TextFormat textFormat READ textFormat WRITE setTextFormat)
			Q_PROPERTY(QMessageBox::StandardButtons standardButtons READ standardButtons WRITE setStandardButtons)
#ifndef QT_NO_TEXTEDIT
			Q_PROPERTY(QString detailedText READ detailedText WRITE setDetailedText)
#endif
			Q_PROPERTY(QString informativeText READ informativeText WRITE setInformativeText)
			Q_PROPERTY(Qt::TextInteractionFlags textInteractionFlags READ textInteractionFlags WRITE setTextInteractionFlags)

    public:
        explicit CMessageBox(QWidget *parent = Q_NULLPTR);
        CMessageBox(QMessageBox::Icon icon,
            const QString &title,
            const QString &text,
            QMessageBox::StandardButtons buttons = QMessageBox::NoButton,
            QWidget *parent = Q_NULLPTR,
            Qt::WindowFlags flags = Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
        ~CMessageBox();

        void addButton(QAbstractButton *button, QMessageBox::ButtonRole role);
        QPushButton *addButton(const QString &text, QMessageBox::ButtonRole role);
        QPushButton *addButton(QMessageBox::StandardButton button);
        void removeButton(QAbstractButton *button);

        using QDialog::open;
        void open(QObject *receiver, const char *member);

        QList<QAbstractButton *> buttons() const;
        QMessageBox::ButtonRole buttonRole(QAbstractButton *button) const;

        void setStandardButtons(QMessageBox::StandardButtons buttons);
        QMessageBox::StandardButtons standardButtons() const;
        QMessageBox::StandardButton standardButton(QAbstractButton *button) const;
        QAbstractButton *button(QMessageBox::StandardButton which) const;

        QPushButton *defaultButton() const;
        void setDefaultButton(QPushButton *button);
        void setDefaultButton(QMessageBox::StandardButton button);

        QAbstractButton *escapeButton() const;
        void setEscapeButton(QAbstractButton *button);
        void setEscapeButton(QMessageBox::StandardButton button);

        QAbstractButton *clickedButton() const;

        QString text() const;
        void setText(const QString &text);

        QMessageBox::Icon icon() const;
        void setIcon(QMessageBox::Icon);

        QPixmap iconPixmap() const;
        void setIconPixmap(const QPixmap &pixmap);

        Qt::TextFormat textFormat() const;
        void setTextFormat(Qt::TextFormat format);

        void setTextInteractionFlags(Qt::TextInteractionFlags flags);
        Qt::TextInteractionFlags textInteractionFlags() const;

        void setCheckBox(QCheckBox *cb);
        QCheckBox* checkBox() const;

        static QMessageBox::StandardButton information(QWidget *parent, const QString &title,
            const QString &text, QMessageBox::StandardButtons buttons = QMessageBox::Ok,
            QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);
        static QMessageBox::StandardButton information(QWidget *parent, const QString &title,
            const QString &text, char *propertyName, bool propertyVal, QMessageBox::StandardButtons buttons = QMessageBox::Ok,
            QMessageBox::StandardButton defaultButton = QMessageBox::NoButton, bool bShowCloseButton = true, const QString& text2 = QString());

        static QMessageBox::StandardButton question(QWidget *parent, const QString &title,
            const QString &text, QMessageBox::StandardButtons buttons = QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No),
            QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

        static QMessageBox::StandardButton question(QWidget *parent, const QString &title,
            const QString &text, char *propertyName, bool propertyVal, QMessageBox::StandardButtons buttons = QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No),
            QMessageBox::StandardButton defaultButton = QMessageBox::NoButton, bool bShowCloseButton = true, const QString& text2 = QString());

        static QMessageBox::StandardButton warning(QWidget *parent, const QString &title,
            const QString &text, QMessageBox::StandardButtons buttons = QMessageBox::Ok,
            QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);
        static QMessageBox::StandardButton warning(QWidget *parent, const QString &title, const QString& text,
            char *propertyName, bool propertyVal, QMessageBox::StandardButtons buttons = QMessageBox::Ok,
            QMessageBox::StandardButton defaultButton = QMessageBox::NoButton, bool bShowCloseButton = true, const QString& text2 = QString(), bool bInformationProperty = false);

        static QMessageBox::StandardButton critical(QWidget *parent, const QString &title,
            const QString &text, QMessageBox::StandardButtons buttons = QMessageBox::Ok,
            QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);
        static QMessageBox::StandardButton critical(QWidget *parent, const QString &title,
            const QString &text, char *propertyName, bool propertyVal, QMessageBox::StandardButtons buttons = QMessageBox::Ok,
            QMessageBox::StandardButton defaultButton = QMessageBox::NoButton, bool bShowCloseButton = true, const QString& text2 = QString());

        static void about(QWidget *parent, const QString &title, const QString &text);
        static void aboutQt(QWidget *parent, const QString &title = QString());

        QString informativeText() const;
        void setInformativeText(const QString &text);
		void setInformationProperty(const char * porperty, bool propertyVal);

#ifndef QT_NO_TEXTEDIT
        QString detailedText() const;
        void setDetailedText(const QString &text);
#endif

        void setWindowTitle(const QString &title);
        void setWindowModality(Qt::WindowModality windowModality);
        /**
            * @brief 实时翻译时，刷新UI文字显示
            */
        void retranslateButtonUi(QMessageBox::StandardButton button);
    Q_SIGNALS:
        void buttonClicked(QAbstractButton *button);

    protected:
        bool event(QEvent *e) Q_DECL_OVERRIDE;
        void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
        void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
        void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
        void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
        void changeEvent(QEvent *event) Q_DECL_OVERRIDE;
        private slots:
        void slot_buttonClicked(QAbstractButton *);
        void slot_clicked(QMessageBox::StandardButton button, QMessageBox::ButtonRole role);
    private:
        Q_DISABLE_COPY(CMessageBox)
        Q_DECLARE_PRIVATE(CMessageBox)

        CMessageBoxPrivate *d;
    };
}// namespace UICommon
