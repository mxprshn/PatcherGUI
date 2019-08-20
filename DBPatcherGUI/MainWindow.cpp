#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "BuilderWidget.h"
#include "InstallerWidget.h"
#include "LoginWindow.h"
#include "LogOutputDevice.h"
#include "InstallerHandler.h"
#include "BuilderHandler.h"
#include "DatabaseProvider.h"

#include <QMessageBox>
#include <QLabel>

// Widget constructor, taking pointer to parent widget
// When parent widget is being deleted, all its children are deleted automatically
MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
	, logOutputDevice(new LogOutputDevice(this))
	, loginWindow(new LoginWindow(this))
{
	ui->setupUi(this);
	logOutputDevice->setTextEdit(ui->logTextEdit);
	logOutputDevice->open(QIODevice::WriteOnly);
	InstallerHandler::setOutputDevice(*logOutputDevice);
	BuilderHandler::setOutputDevice(*logOutputDevice);

	connectAction = new QAction(QIcon(":/images/addDatabase.svg"), "Connect to database...", this);
	disconnectAction = new QAction(QIcon(":/images/removeDatabase.svg"), "Disconnect", this);
	disconnectAction->setDisabled(true);
	databaseInformation = new QLabel("Connect to database!", this);

	ui->databaseMenu->addAction(connectAction);
	ui->databaseMenu->addAction(disconnectAction);

	connectAction->setShortcut(QKeySequence("Ctrl+O"));
	disconnectAction->setShortcut(QKeySequence("Ctrl+W"));

	ui->viewMenu->addAction(QIcon(":/images/hammer.svg"),"Build", [=]() { ui->tabWidget->setCurrentWidget(ui->builderTab); }, QKeySequence("Ctrl+B"));
	ui->viewMenu->addAction(QIcon(":/images/install.svg"), "Install", [=]() { ui->tabWidget->setCurrentWidget(ui->installerTab); }, QKeySequence("Ctrl+I"));
	ui->viewMenu->addAction("About...", [=]()
	{
		QMessageBox::about(this, "PostgreSQL database patcher",
			"PostgreSQL database patcher. "
			"Developed by: Ekaterina Vinnik, Victor Khovanov, Timur Sirkin, Maxim Parshin, Daria Larionova, Nikolay Bazhulin. "
			"Icons made by: Vitaly Gorbachev, Smashicons, Pixelmeetup, Freepik, Roundicons, Good Ware from www.flaticon.com and www.icons8.com. 2019");
	});

	ui->mainToolBar->addAction(connectAction);
	ui->mainToolBar->addWidget(databaseInformation);

	connect(loginWindow, SIGNAL(connectButtonClicked()), this, SLOT(onDialogConnectButtonClicked()));
	connect(connectAction, SIGNAL(triggered()), this, SLOT(onConnectionRequested()));
	connect(disconnectAction, SIGNAL(triggered()), this, SLOT(onDisconnectButtonClicked()));
	connect(ui->builderTab, SIGNAL(connectionRequested()), this, SLOT(onConnectionRequested()));
	connect(ui->installerTab, SIGNAL(connectionRequested()), this, SLOT(onConnectionRequested()));
	connect(this, SIGNAL(connected()), ui->builderTab, SLOT(onConnected()));
	connect(this, SIGNAL(disconnectionStarted()), ui->builderTab, SLOT(onDisconnectionStarted()));
}

// Destructor with ui object deleting and database disconnection
MainWindow::~MainWindow()
{
	if (DatabaseProvider::isConnected())
	{
		emit disconnectionStarted();
		DatabaseProvider::disconnect();
	}

	delete ui;
}

// Handles click of OK button on input dialog
// Launches database connection and sets appropriate interface elements
void MainWindow::onDialogConnectButtonClicked()
{
	QString errorMessage = "";

	if (DatabaseProvider::connect(loginWindow->getDatabaseInput(), loginWindow->getUsernameInput()
		, loginWindow->getPasswordInput(), loginWindow->getHostInput(), loginWindow->getPortInput(), errorMessage))
	{
		databaseInformation->setText("Connected to \"" + DatabaseProvider::database() + "\" as \""
			+ DatabaseProvider::user() + "\"");
		connectAction->setDisabled(true);
		disconnectAction->setEnabled(true);
		loginWindow->clear();
		loginWindow->close();
		emit connected();
	}
	else
	{
		ui->logTextEdit->append(errorMessage);
		QApplication::beep();
		QMessageBox::warning(this, "Connection error"
				, "Connection error. See log for detailed information.", QMessageBox::Ok, QMessageBox::Ok);
	}
}

// Handles connection requests from other widgets
void MainWindow::onConnectionRequested()
{
	loginWindow->show();
}

// Handles disconnect button click
// Launches database disconnection and sets appropriate interface elements
void MainWindow::onDisconnectButtonClicked()
{
	const auto dialogResult = QMessageBox::question(this, "Remove item", "Are you sure to disconnect from database?"
		, QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);

	if (dialogResult == QMessageBox::Cancel)
	{
		return;
	}

	emit disconnectionStarted();
	DatabaseProvider::disconnect();
	databaseInformation->setText("Connect to database!");
	connectAction->setEnabled(true);
	disconnectAction->setDisabled(true);
}