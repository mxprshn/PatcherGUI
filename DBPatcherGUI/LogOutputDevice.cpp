#include "LogOutputDevice.h"

#include <QTextEdit>
#include <QScrollBar>
#include <QTextCodec>

// Constructor
LogOutputDevice::LogOutputDevice(QObject *parent)
	: QIODevice(parent)
	, text_edit(nullptr)
{
}

// Sets new QTextEdit for output
void LogOutputDevice::SetTextEdit(QTextEdit *text_edit)
{
	this->text_edit = text_edit;
}

// Re-implements QTextEdit reading virtual method
qint64 LogOutputDevice::readData(char *data, qint64 maxlen)
{
	return 0;
}

// Re-implements QTextEdit writing virtual method
qint64 LogOutputDevice::writeData(const char *data, qint64 len)
{
	if (text_edit)
	{
		// It should be possibly fixed on Linux
		text_edit->append(QTextCodec::codecForName("Windows-1251")->toUnicode(data));
		text_edit->verticalScrollBar()->setValue(text_edit->verticalScrollBar()->maximum());
	}

	return len;
}