#pragma once

#include <QTreeWidget>

// Class implementing graphical interface for list of patch objects
class PatchListWidget : public QTreeWidget
{
	Q_OBJECT

public:

	enum ColumnIndexes
	{
		typeColumn,
		schemaColumn,
		nameColumn
	};

	PatchListWidget(QWidget *parent = nullptr);
	// QStringList itemList() const;
	bool itemExists(int typeIndex, const QString &schema, const QString &name);
	void add(int typeIndex, const QString &schema, const QString &name, bool isDraggable);
private:
	void dropEvent(QDropEvent *event) override;
};