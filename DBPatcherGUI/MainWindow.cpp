#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "BuilderWidget.h"
#include "InstallerWidget.h"
#include "LoginWindow.h"
#include "SettingsWindow.h"
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
	, log_output_device(new LogOutputDevice(this))
	, login_window(new LoginWindow(this))
	, settings_window(new SettingsWindow(this))
	, settings("spbu-dreamteam", "Patcher")
{
	ui->setupUi(this);
	ReadSettings();
	log_output_device->SetTextEdit(ui->logTextEdit);
	log_output_device->open(QIODevice::WriteOnly);
	InstallerHandler::SetOutputDevice(*log_output_device);
	BuilderHandler::SetOutputDevice(*log_output_device);

	connect_action = new QAction(QIcon(":/images/addDatabase.svg"), "Connect to database...", this);
	disconnect_action = new QAction(QIcon(":/images/removeDatabase.svg"), "Disconnect", this);
	disconnect_action->setDisabled(true);
	database_information = new QLabel("Connect to database!", this);

	ui->databaseMenu->addAction(connect_action);
	ui->databaseMenu->addAction(disconnect_action);

	connect_action->setShortcut(QKeySequence("Ctrl+O"));
	disconnect_action->setShortcut(QKeySequence("Ctrl+W"));

	ui->viewMenu->addAction(QIcon(":/images/hammer.svg"),"Build", [=]() { ui->tabWidget->setCurrentWidget(ui->builderTab); }, QKeySequence("Ctrl+B"));
	ui->viewMenu->addAction(QIcon(":/images/install.svg"), "Install", [=]() { ui->tabWidget->setCurrentWidget(ui->installerTab); }, QKeySequence("Ctrl+I"));
	ui->viewMenu->addAction("Settings...", [=]() { settings_window->OpenSettingsDialog(settings); });
	ui->viewMenu->addAction("About...", [=]()
	{
		QMessageBox::about(this, "PostgreSQL database patcher",
			"PostgreSQL database patcher. "
			"Developed by: Ekaterina Vinnik, Victor Khovanov, Timur Sirkin, Maxim Parshin, Daria Larionova, Nikolay Bazhulin. "
			"Icons made by: Vitaly Gorbachev, Smashicons, Pixelmeetup, Freepik, Roundicons, Good Ware from www.flaticon.com and www.icons8.com. 2019");
	});

	ui->mainToolBar->addAction(connect_action);
	ui->mainToolBar->addWidget(database_information);

	connect(login_window, SIGNAL(connectButtonClicked()), this, SLOT(onDialogConnectButtonClicked()));
	connect(connect_action, SIGNAL(triggered()), this, SLOT(onConnectionRequested()));
	connect(disconnect_action, SIGNAL(triggered()), this, SLOT(onDisconnectButtonClicked()));
	connect(ui->builderTab, SIGNAL(connectionRequested()), this, SLOT(onConnectionRequested()));
	connect(ui->installerTab, SIGNAL(connectionRequested()), this, SLOT(onConnectionRequested()));
	connect(this, SIGNAL(connected()), ui->builderTab, SLOT(onConnected()));
	connect(this, SIGNAL(disconnectionStarted()), ui->builderTab, SLOT(onDisconnectionStarted()));
	connect(this, SIGNAL(disconnectionStarted()), ui->installerTab, SLOT(onDisconnectionStarted()));
	connect(settings_window, &SettingsWindow::SaveButtonClicked, [&]()
	{
		settings_window->SaveSettings(settings);
		ReadSettings();
	});
}

// Destructor with ui object deleting and database disconnection
MainWindow::~MainWindow()
{
	if (DatabaseProvider::IsConnected())
	{
		emit DisconnectionStarted();
		DatabaseProvider::Disconnect();
	}

	delete ui;
}

// Handles click of OK button on input dialog
// Launches database connection and sets appropriate interface elements
void MainWindow::OnDialogConnectButtonClicked()
{
	QString errorMessage = "";

	if (DatabaseProvider::Connect(login_window->GetDatabaseInput(), login_window->GetUsernameInput()
		, login_window->GetPasswordInput(), login_window->GetHostInput(), login_window->GetPortInput(), errorMessage))
	{
		database_information->setText("Connected to \"" + DatabaseProvider::Database() + "\" as \""
			+ DatabaseProvider::User() + "\"");
		connect_action->setDisabled(true);
		disconnect_action->setEnabled(true);
		login_window->Clear();
		login_window->close();
		emit Connected();
	}
	else
	{
		ui->logTextEdit->append(errorMessage);
		QApplication::beep();
		QMessageBox::warning(this, "Connection error"
				, "Connection error. See log for details", QMessageBox::Ok, QMessageBox::Ok);
	}
}

// Handles connection requests from other widgets
void MainWindow::OnConnectionRequested()
{
	login_window->show();
}

// Handles disconnect button click
// Launches database disconnection and sets appropriate interface elements
void MainWindow::OnDisconnectButtonClicked()
{
	const auto dialogResult = QMessageBox::question(this, "Disconnect from the database", "Do you want to disconnect from the database?"
		, QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);

	if (dialogResult == QMessageBox::Cancel)
	{
		return;
	}

	emit DisconnectionStarted();
	DatabaseProvider::Disconnect();
	database_information->setText("Connect to database!");
	connect_action->setEnabled(true);
	disconnect_action->setDisabled(true);
}

// Reads saved settings for the application
void MainWindow::ReadSettings()
{
	BuilderHandler::SetTemplatesFile(settings.value("templates", "Templates.ini").toString());
}