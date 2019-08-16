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
	LogOutputDevice *logOutputDevice;
	// Database information input dialog
	LoginWindow *loginWindow;
	// Actions shown in main menu
	QAction *connectAction;
	QAction *disconnectAction;
	// Label showing connection information
	QLabel *databaseInformation;
	// Settings dialog
	SettingsWindow *settingsWindow;
	// Settings object
	QSettings settings;
	void readSettings();
signals:
	void connected();
	void disconnectionStarted();
private slots:
	void onDialogConnectButtonClicked();
	void onConnectionRequested();
	void onDisconnectButtonClicked();
};