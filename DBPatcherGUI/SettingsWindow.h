#pragma once

#include <QDialog>
#include <QSettings>

namespace Ui
{
	class SettingsWindow;
};

class SettingsWindow : public QDialog
{
	Q_OBJECT

public:
	SettingsWindow(QWidget *parent = nullptr);
	~SettingsWindow();
	void openSettingsDialog(const QSettings &settings);
	void saveSettings(QSettings &settings);
private:
	Ui::SettingsWindow *ui;
signals:
	void saveButtonClicked();
};
