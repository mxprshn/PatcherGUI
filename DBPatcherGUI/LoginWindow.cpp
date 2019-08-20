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
	clear();

	connect(ui->buttonBox, SIGNAL(accepted()), this, SIGNAL(connectButtonClicked()));
	connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(close()));
	connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(clear()));
}

// Destructor with ui object deleting
LoginWindow::~LoginWindow()
{
	delete ui;
}

// Getter for host input
QString LoginWindow::getHostInput() const
{
	return ui->hostLineEdit->text();
}

// Getter for port input
int LoginWindow::getPortInput() const
{
	return ui->portLineEdit->text().toInt();
}

// Getter for database name input
QString LoginWindow::getDatabaseInput() const
{
	return ui->databaseLineEdit->text();
}

// Getter for username input
QString LoginWindow::getUsernameInput() const
{
	return ui->usernameLineEdit->text();
}

// Getter for password input
QString LoginWindow::getPasswordInput() const
{
	return ui->passwordLineEdit->text();
}

// Sets input lines to default state
void LoginWindow::clear()
{
	ui->hostLineEdit->clear();
	ui->portLineEdit->setText("5432");
	ui->databaseLineEdit->clear();
	ui->usernameLineEdit->clear();
	ui->passwordLineEdit->clear();
}