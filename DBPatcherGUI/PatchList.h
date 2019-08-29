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
	void Add(int type_index, const QString &schema_name, const QString &name, const QStringList &parameters = {});
	int Count() const;
	QList<PatchListElement*>::const_iterator begin() const;
	QList<PatchListElement*>::const_iterator end() const;
	void Clear();
private:
	void Swap(PatchList &other);
	// Internal storage of database objects
	QList<PatchListElement*> *elements;
};