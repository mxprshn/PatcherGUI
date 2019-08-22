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
	static void SetOutputDevice(QIODevice &new_device);
	static bool BuildPatch(const QString &database, const QString &user, const QString &password,
		const QString &server, int port, const QString &patch_dir, const QString &build_list_dir);
	static void SetTemplatesFile(const QString &path);
private:
	// Name of Builder module program file
	const static QString program;
	// Path to templates file
	static QString templates_path;
	// Device for builder log output
	static QIODevice *output_device;
};