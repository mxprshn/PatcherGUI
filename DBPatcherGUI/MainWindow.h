#pragma once

#include <QMainWindow>
#include <QSettings>

class QAction;
class QLabel;
class LoginWindow;
class SettingsWindow;
class LogOutputDevice;

// Namespace required by Qt for loading .ui form file
namespace Ui
{
	class MainWindow;
}

// Class implementing graphical interface for main application window
class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();
private:
	// Pointer to ui object required by Qt for loading .ui form file
	// Ui class is created in editor, and its elements are available through this pointer
	Ui::MainWindow *ui;
	// Device for log output
	LogOutputDevice *log_output_device;
	// Database information input dialog
	LoginWindow *login_window;
	// Actions shown in main menu
	QAction *connect_action;
	QAction *disconnect_action;
	// Label showing connection information
	QLabel *database_information;
	// Settings dialog
	SettingsWindow *settings_window;
	// Settings object
	QSettings settings;
	void ReadSettings();
signals:
	void Connected();
	void DisconnectionStarted();
private slots:
	void OnDialogConnectButtonClicked();
	void OnConnectionRequested();
	void OnDisconnectButtonClicked();
};