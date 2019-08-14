#include "PatchListElement.h"

// Constructor
PatchListElement::PatchListElement(int type, const QString &name, const QString &schema
	, const QStringList &parameters)
	: type(type)
	, name(name)
	, schema(schema)
	, parameters(parameters)
{
}

// Getter for type
int PatchListElement::getType() const
{
	return type;
}

// Getter for name
QString PatchListElement::getName() const
{
	return name;
}

// Getter for schema
QString PatchListElement::getSchema() const
{
	return schema;
}

// Getter for parameters
QStringList PatchListElement::getParameters() const
{
	return parameters;
}