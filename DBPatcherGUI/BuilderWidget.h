#pragma once

#include <QWidget>

class QSqlQueryModel;
class ObjectNameCompleter;

// Namespace required by Qt for loading .ui form file
namespace Ui
{
	class BuilderWidget;
}

// Class implementing graphical interface for build list creation and launching Builder module
class BuilderWidget : public QWidget
{
	Q_OBJECT

public:
	BuilderWidget(QWidget *parent = nullptr);
	~BuilderWidget();
private:
	// Pointer to ui object required by Qt for loading .ui form file
	// Ui class is created in editor, and its elements are available through this pointer
	Ui::BuilderWidget *ui;
	// Pointer to the schema list model, which can be filled with query execution
	QSqlQueryModel *schema_list_model;
	// Completer object which provides auto-completion of object name user's input
	ObjectNameCompleter *name_completer;
	void AddScripts(const QString &input);
	bool CheckConnection();
	void InitScriptInput();
	void InitCompleter();
	bool StartPatchBuild(const QString &path);
signals:
	void ConnectionRequested();
	void ItemCountChanged();
public slots:
	void OnConnected();
	void OnDisconnectionStarted();
private slots:
	void OnAddButtonClicked();
	void OnBuildButtonClicked();
	void OnExplorerButtonClicked();
	void OnMoveUpButtonClicked();
	void OnMoveDownButtonClicked();
	void OnRemoveButtonClicked();
	void OnClearButtonClicked();
	void OnItemSelectionChanged();
	void OnCurrentTypeChanged(int type);
	void OnCurrentSchemaChanged(const QString &schema);
	void OnNameTextChanged(const QString &input);
	void OnItemCountChanged();
};