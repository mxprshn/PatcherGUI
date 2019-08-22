#pragma once

#include <QStringList>

class QString;

// Database object class
class PatchListElement
{
public:
	PatchListElement(int type, const QString &name, const QString &schema, const QStringList &parameters);
	int GetType() const;
	QString GetName() const;
	QString GetSchema() const;
	QStringList GetParameters() const;
private:
	// Type index
	int type;
	// Object name
	QString name;
	// Object schema
	QString schema;
	// List of object parameters
	QStringList parameters;
};