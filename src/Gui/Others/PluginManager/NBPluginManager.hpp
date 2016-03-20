/*
	*
	* NBPluginManager.hpp - NBPluginManager.cpp Header
	*
*/

#pragma once

#include <QtCore>
#include <NBPluginInterface.hpp>
#include <NBPreviewInterface.hpp>
#include <NBMimeDatabase.hpp>
#include <NBXdg.hpp>

typedef struct {
	/* Name of the plugin with full path */
	QString name;

	/* Version of the plugin with full path */
	QString version;

	/* Interface this plugin provides */
	NBPluginInterface::Interface interface;

	/* Type of plugin */
	NBPluginInterface::Type type;

	/* Plugin Context */
	NBPluginInterface::Contexts contexts;

	/* List of mimetypes handled */
	QStringList mimeTypes;
} PluginCapability;

class NBPluginManager {

	public:
		/* Create a global instance of NBPluginManager */
		static NBPluginManager* instance();

		/* Return the plugin path for the given mimetype */
		bool hasPluginForMimeType( QString );

		/* Return the plugin path for the given mimetype */
		QString pluginForMimeType( QString );

		/* Return the plugin path for the given setup */
		QStringList plugins( NBPluginInterface::Interface, NBPluginInterface::Type, NBPluginInterface::Context, QString );

	public Q_SLOTS:
		/* Reload the plugins map */
		void reloadPlugins();

	private:
		NBPluginManager();

		void reloadPeekPlugins();
		void reloadOtherPlugins();

		/* Global Instance */
		static NBPluginManager* pMgr;

		/* Init flag */
		bool init;

		/* MimeType-PluginPath map */
		QMap<QString, QStringList> mimePluginMap;
		QMap<QString, int> pluginPriorityMap;

		QList<PluginCapability*> mPluginCapabilityList;
};
