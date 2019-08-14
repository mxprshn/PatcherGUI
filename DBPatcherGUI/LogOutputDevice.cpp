#include "LogOutputDevice.h"

#include <QTextEdit>
#include <QScrollBar>

// Constructor
LogOutputDevice::LogOutputDevice(QObject *parent)
	: QIODevice(parent)
	, textEdit(nullptr)
{
}

// Sets new QTextEdit for output
void LogOutputDevice::setTextEdit(QTextEdit *textEdit)
{
	this->textEdit = textEdit;
}

// Reimplements QTextEdit reading virtual method
qint64 LogOutputDevice::readData(char *data, qint64 maxlen)
{
	return 0;
}

// Reimplements QTextEdit writing virtual method
qint64 LogOutputDevice::writeData(const char *data, qint64 len)
{
	if (textEdit)
	{
		textEdit->append(data);
		textEdit->verticalScrollBar()->setValue(textEdit->verticalScrollBar()->maximum());
	}
	return len;
}