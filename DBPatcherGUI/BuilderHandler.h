#pragma once

#include <QObject>

class QString;
class QIODevice;
class QProcess;

// Class for Builder module process management
class BuilderHandler : public QObject
{
	Q_OBJECT

public:
	BuilderHandler() = delete;
	static void setOutputDevice(QIODevice &newDevice);
	static bool buildPatch(const QString &database, const QString &user, const QString &password,
		const QString &server, int port, const QString &patchDir, const QString &buildListDir);
private:
	// Name of Builder module program file
	const static QString program;
	// Path to templates file
	const static QString templatesPath;
	// Device for builder log output
	static QIODevice *outputDevice;
};