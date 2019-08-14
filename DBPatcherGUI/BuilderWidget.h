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
	QSqlQueryModel *schemaListModel;
	// Completer object which provides autocompletion of object name user's input 
	ObjectNameCompleter *nameCompleter;
	void addScripts(const QString &input);
	bool checkConnection();
	void initScriptInput();
	void initCompleter();
	bool startPatchBuild(const QString &path);
signals:
	void connectionRequested();
	void itemCountChanged();
private slots:
	void onAddButtonClicked();
	void onBuildButtonClicked();
	void onExplorerButtonClicked();
	void onMoveUpButtonClicked();
	void onMoveDownButtonClicked();
	void onRemoveButtonClicked();
	void onClearButtonClicked();
	void onItemSelectionChanged();
	void onCurrentTypeChanged(int type);
	void onCurrentSchemaChanged(const QString &schema);
	void onNameTextChanged(const QString &input);
	void onItemCountChanged();
	void onConnected();
	void onDisconnectionStarted();
};