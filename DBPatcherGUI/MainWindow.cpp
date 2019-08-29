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
#include <QScrollBar>

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
	ui->tab_widget->setCurrentWidget(ui->builder_tab);
	ReadSettings();
	log_output_device->SetTextEdit(ui->log_text_edit);
	log_output_device->open(QIODevice::WriteOnly);
	InstallerHandler::SetOutputDevice(*log_output_device);
	BuilderHandler::SetOutputDevice(*log_output_device);

	connect_action = new QAction(QIcon(":/images/addDatabase.svg"), "Connect to database...", this);
	disconnect_action = new QAction(QIcon(":/images/removeDatabase.svg"), "Disconnect", this);
	disconnect_action->setDisabled(true);
	database_information = new QLabel("Connect to database!", this);

	ui->database_menu->addAction(connect_action);
	ui->database_menu->addAction(disconnect_action);

	connect_action->setShortcut(QKeySequence("Ctrl+O"));
	disconnect_action->setShortcut(QKeySequence("Ctrl+W"));

	ui->view_menu->addAction(QIcon(":/images/hammer.svg"),"Build", [=]() { ui->tab_widget->setCurrentWidget(ui->builder_tab); }, QKeySequence("Ctrl+B"));
	ui->view_menu->addAction(QIcon(":/images/install.svg"), "Install", [=]() { ui->tab_widget->setCurrentWidget(ui->installer_tab); }, QKeySequence("Ctrl+I"));
	ui->view_menu->addAction("Settings...", [=]() { settings_window->OpenSettingsDialog(settings); });
	ui->view_menu->addAction("About...", [=]()
	{
		QMessageBox::about(this, "PostgreSQL database patcher",
			"PostgreSQL database patcher. "
			"Developed by: Ekaterina Vinnik, Victor Khovanov, Timur Sirkin, Maxim Parshin, Daria Larionova, Nikolay Bazhulin. "
			"Icons made by: Vitaly Gorbachev, Smashicons, Pixelmeetup, Freepik, Roundicons, Good Ware from www.flaticon.com and www.icons8.com. 2019");
	});

	ui->main_tool_bar->addAction(connect_action);
	ui->main_tool_bar->addWidget(database_information);

	connect(login_window, &LoginWindow::ConnectButtonClicked, this, &MainWindow::OnDialogConnectButtonClicked);
	connect(connect_action, &QAction::triggered, this, &MainWindow::OnConnectionRequested);
	connect(disconnect_action, &QAction::triggered, this, &MainWindow::OnDisconnectButtonClicked);
	connect(ui->builder_tab, &BuilderWidget::ConnectionRequested, this, &MainWindow::OnConnectionRequested);
	connect(ui->installer_tab, &InstallerWidget::ConnectionRequested, this, &MainWindow::OnConnectionRequested);
	connect(this, &MainWindow::Connected, ui->builder_tab, &BuilderWidget::OnConnected);
	connect(this, &MainWindow::DisconnectionStarted, ui->builder_tab, &BuilderWidget::OnDisconnectionStarted);
	connect(this, &MainWindow::DisconnectionStarted, ui->installer_tab, &InstallerWidget::OnDisconnectionStarted);
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
	QString error_message = "";

	if (DatabaseProvider::Connect(login_window->GetDatabaseInput(), login_window->GetUsernameInput()
		, login_window->GetPasswordInput(), login_window->GetHostInput(), login_window->GetPortInput(), error_message))
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
		ui->log_text_edit->append(error_message);
		ui->log_text_edit->verticalScrollBar()->setValue(ui->log_text_edit->verticalScrollBar()->maximum());
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
	const auto dialog_result = QMessageBox::question(this, "Disconnect from the database", "Do you want to disconnect from the database?"
		, QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);

	if (dialog_result == QMessageBox::Cancel)
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