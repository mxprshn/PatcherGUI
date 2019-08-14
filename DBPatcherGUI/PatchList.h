#pragma once

#include <QList>

class PatchListElement;

// Class implementing list of database objects
class PatchList
{
public:
	PatchList();
	~PatchList();
	PatchList(const PatchList &other);
	PatchList& operator=(const PatchList &other);
	void add(int typeIndex, const QString &schemaName, const QString &name, const QStringList &parameters = {});
	int count() const;
	QList<PatchListElement*>::const_iterator begin() const;
	QList<PatchListElement*>::const_iterator end() const;
	void clear();
private:
	void swap(PatchList &other);
	// Internal storage of database objects
	QList<PatchListElement*> *elements;
};