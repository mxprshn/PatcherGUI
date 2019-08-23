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
	QDir patch_dir;
	// Flag showing if patch is opened
	bool is_patch_opened;
	bool InitPatchList(const QString &path);
	bool InitDependencyList(const QString &path);
	void ClearCurrentPatch();
	void SetReadyToOpen();
	bool CheckConnection();
	bool StartDependencyCheck();
signals:
	void ConnectionRequested();
public slots:
	void OnDisconnectionStarted();
private slots:
	void OnOpenButtonClicked();
	void OnCheckButtonClicked();
	void OnInstallButtonClicked();
	void OnItemCheckChanged();
};