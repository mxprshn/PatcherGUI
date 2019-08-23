#include "LoginWindow.h"
#include "ui_LoginWindow.h"

#include <QLineEdit>

// Widget constructor, taking pointer to parent widget
// When parent widget is being deleted, all its children are deleted automatically
LoginWindow::LoginWindow(QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::LoginWindow)
{
	ui->setupUi(this);
	Clear();
	setWindowFlag(Qt::WindowContextHelpButtonHint, false);

	connect(ui->button_box, &QDialogButtonBox::accepted, this, &LoginWindow::ConnectButtonClicked);
	connect(ui->button_box, &QDialogButtonBox::rejected, this, &LoginWindow::close);
	connect(ui->button_box, &QDialogButtonBox::rejected, this, &LoginWindow::Clear);
}

// Destructor with ui object deleting
LoginWindow::~LoginWindow()
{
	delete ui;
}

// Getter for host input
QString LoginWindow::GetHostInput() const
{
	return ui->host_line_edit->text();
}

// Getter for port input
int LoginWindow::GetPortInput() const
{
	return ui->port_line_edit->text().toInt();
}

// Getter for database name input
QString LoginWindow::GetDatabaseInput() const
{
	return ui->database_line_edit->text();
}

// Getter for username input
QString LoginWindow::GetUsernameInput() const
{
	return ui->username_line_edit->text();
}

// Getter for password input
QString LoginWindow::GetPasswordInput() const
{
	return ui->password_line_edit->text();
}

// Sets input lines to default state
void LoginWindow::Clear()
{
	ui->host_line_edit->clear();
	ui->port_line_edit->setText("5432");
	ui->database_line_edit->clear();
	ui->username_line_edit->clear();
	ui->password_line_edit->clear();
	ui->host_line_edit->setFocus();
}