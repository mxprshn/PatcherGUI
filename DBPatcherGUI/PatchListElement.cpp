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
int PatchListElement::GetType() const
{
	return type;
}

// Getter for name
QString PatchListElement::GetName() const
{
	return name;
}

// Getter for schema
QString PatchListElement::GetSchema() const
{
	return schema;
}

// Getter for parameters
QStringList PatchListElement::GetParameters() const
{
	return parameters;
}