#pragma once

#include <QDialog>

// Namespace required by Qt for loading .ui form file
namespace Ui
{
	class LoginWindow;
}

// Class implementing dialog for database information user input
class LoginWindow : public QDialog
{
	Q_OBJECT

public:

	LoginWindow(QWidget *parent = nullptr);
	~LoginWindow();
	QString GetHostInput() const;
	int GetPortInput() const;
	QString GetDatabaseInput() const;
	QString GetUsernameInput() const;
	QString GetPasswordInput() const;
private:
	// Pointer to ui object required by Qt for loading .ui form file
	// Ui class is created in editor, and its elements are available through this pointer
	Ui::LoginWindow *ui;
signals:
	void ConnectButtonClicked();
public slots:
	void Clear();
};