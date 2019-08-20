#pragma once

#include <QStringList>

class QString;

// Database object class
class PatchListElement
{
public:
	PatchListElement(int type, const QString &name, const QString &schema, const QStringList &parameters);
	int getType() const;
	QString getName() const;
	QString getSchema() const;
	QStringList getParameters() const;
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