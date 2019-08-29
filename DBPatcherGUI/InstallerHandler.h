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
	static void SetOutputDevice(QIODevice &new_device);
	static bool InstallPatch(const QString &database, const QString &user, const QString &password,
		const QString &server, int port, const QString &path);
	static QBitArray CheckDependencies(const QString &database, const QString &user, const QString &password,
		const QString &server, int port, const QString &path, bool &is_successful);
private:
	// Name of Installer module program file
	const static QString program;
	// Device for installer log output
	static QIODevice *output_device;
};
