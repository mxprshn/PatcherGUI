#pragma once

#include <QTreeWidget>

// Class implementing graphical interface for dependency list
class DependencyListWidget : public QTreeWidget
{
	Q_OBJECT

public:

	enum ColumnIndexes
	{
		typeColumn,
		schemaColumn,
		nameColumn,
		statusColumn
	};

	enum CheckStatus
	{
		waitingForCheck,
		satisfied,
		notSatisfied
	};

	DependencyListWidget(QWidget *parent = nullptr);
	bool SetCheckStatus(const QBitArray &check_result);
	void Add(int type_index, const QString &schema, const QString &name);
	void Clear();
	void ClearCheck();
	int GetCheckedCount() const;
	bool GetAreAllSatisfied() const;
private:
	// Amount of checked (marked) dependencies in list
	int checked_count;
	// Flag showing if all dependencies are satisfied
	bool are_all_satisfied;
	// Hash for icon file paths
	static const QHash<int, QString> status_icons;
signals:
	void ItemCheckChanged();
private slots:
	void OnItemClicked(QTreeWidgetItem *item, int column);
};
