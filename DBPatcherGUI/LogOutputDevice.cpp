#include "LogOutputDevice.h"

#include <QTextEdit>
#include <QScrollBar>
#include <QTextCodec>

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
		// It should be possibly fixed on Linux
		textEdit->append(QTextCodec::codecForName("Windows-1251")->toUnicode(data));
		textEdit->verticalScrollBar()->setValue(textEdit->verticalScrollBar()->maximum());
	}

	return len;
}