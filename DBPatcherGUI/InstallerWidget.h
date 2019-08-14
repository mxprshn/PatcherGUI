#pragma once

#include <QWidget>
#include <QDir>

// Namespace required by Qt for loading .ui form file
namespace Ui
{
	class InstallerWidget;
}

// Class implementing graphical interface for patch opening, launching dependency check and launching patch installation
class InstallerWidget : public QWidget
{
	Q_OBJECT

public:
	InstallerWidget(QWidget *parent = nullptr);
	~InstallerWidget();
private:
	// Pointer to ui object required by Qt for loading .ui form file
	// Ui class is created in editor, and its elements are available through this pointer
	Ui::InstallerWidget *ui;
	// Directory of current patch
	QDir patchDir;
	// Flag showing if patch is opened
	bool isPatchOpened;
	bool initPatchList(const QString &path);
	bool initDependencyList(const QString &path);
	void clearCurrentPatch();
	void setReadyToOpen();
	bool checkConnection();
signals:
	void connectionRequested();
private slots:
	void onOpenButtonClicked();
	void onCheckButtonClicked();
	void onInstallButtonClicked();
	void onItemCheckChanged();
};