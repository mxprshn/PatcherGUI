#pragma once

#include <QDialog>
#include <QSettings>

// Namespace required by Qt for loading .ui form file
namespace Ui
{
	class SettingsWindow;
};

// Class implementing dialog for settings editing
class SettingsWindow : public QDialog
{
	Q_OBJECT

public:
	SettingsWindow(QWidget *parent = nullptr);
	~SettingsWindow();
	void OpenSettingsDialog(const QSettings &settings);
	void SaveSettings(QSettings &settings);
private:
	Ui::SettingsWindow *ui;
signals:
	void SaveButtonClicked();
private slots:
	void OnExplorerButtonClicked();
	void OnDefaultButtonClicked();
};
