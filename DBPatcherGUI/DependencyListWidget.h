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
	bool setCheckStatus(const QBitArray &checkResult);
	void add(int typeIndex, const QString &schema, const QString &name);
	void clear();
	int getCheckedCount() const;
	bool getAreAllSatisfied() const;
private:
	// Amount of checked (marked) dependencies in list
	int checkedCount;
	// Flag showing if all dependencies are satisfied
	bool areAllSatisfied;
	// Hash for icon file paths
	static const QHash<int, QString> statusIcons;
signals:
	void itemCheckChanged();
private slots:
	void onItemClicked(QTreeWidgetItem *item, int column);
};
