#pragma once

#include <QObject>

class QBitArray;
class QString;
class QIODevice;

// Class for Installer module process management
class InstallerHandler : QObject
{
	Q_OBJECT

public:
	InstallerHandler() = delete;
	static void setOutputDevice(QIODevice &newDevice);
	static bool installPatch(const QString &database, const QString &user, const QString &password,
		const QString &server, int port, const QString &path);
	static QBitArray checkDependencies(const QString &database, const QString &user, const QString &password,
		const QString &server, int port, const QString &path, bool &isSuccessful);
private:
	// Name of Installer module program file
	const static QString program;
	// Device for installer log output
	static QIODevice *outputDevice;
};
