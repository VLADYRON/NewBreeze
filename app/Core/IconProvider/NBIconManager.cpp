/*
	*
	* NBIconManager.cpp - Icon Manager class for NewBreeze
	*
*/

#include "NBIconManager.hpp"

#if QT_VERSION >= 0x050000
	#include <QtConcurrent>
#endif

/* Theme Database */
static QSettings mdb( NBXdg::userDir( NBXdg::XDG_CACHE_HOME ) + "NewBreeze/mimetypes.db", QSettings::NativeFormat );
static QSettings idb( NBXdg::userDir( NBXdg::XDG_CACHE_HOME ) + "NewBreeze/icons.db", QSettings::NativeFormat );

static inline QStringList listFiles( QString path ) {

	struct dirent **fileList;
	int entries = scandir( path.toLocal8Bit().data(), &fileList, NULL, NULL );
	if ( entries > 0 ) {
		QStringList files;
		for( int i = 0; i < entries; i++ ) {
			/* Ignore . and .. */
			struct dirent *entry = fileList[ i ];
			if ( not strcmp( entry->d_name, "." ) or not strcmp( entry->d_name, ".." ) )
				continue;

			files << path + QString::fromLocal8Bit( entry->d_name );
		}

		return files;
	}

	return QStringList();
};

NBIconManager *NBIconManager::iMgr = NULL;

NBIconManager* NBIconManager::instance() {

	/* If the Plugin Manager has already been init, then return the instance */
	if ( iMgr and iMgr->init )
		return iMgr;

	/* Init our plugin manager */
	iMgr = new NBIconManager();

	/* If init succeeds, we set the init flag and return */
	iMgr->init = true;

	return iMgr;
};

NBIconManager::NBIconManager() {

	/* Set the default theme path */
	iconThemePath = QString( "/usr/share/icons/" );

	/* Generate the theme database */
	generateThemeDatabase();
};

QStringList NBIconManager::iconsForFile( QString mName, QString file ) {

	if ( mName.isEmpty() ) {
		if ( file.isEmpty() )
			return QStringList() << ":/icons/unknwon.png";

		mName = mimeDb.mimeTypeForFile( file ).name();
	}

	/* If we have a directory */
	if ( not mName.compare( "inode/directory" ) ) {				// QString.compare(...) returns 0 for a match

		if ( not file.endsWith( "/" ) )
			file += "/";

		/* User defined directory icon from @path/.directory */
		QSettings settPath( QDir( file ).filePath( ".directory" ), QSettings::NativeFormat );
		QString icoStr = settPath.value( "Desktop Entry/Icon" ).toString();
		if ( hasIcon( icoStr ) )
			return icon( icoStr );

		else if ( exists( icoStr ) )
			return QStringList() << icoStr;

		/* EncFS Encrypted/Decrypted Folder */
		QSettings settPrnt( QDir( dirName( file ) ).filePath( ".directory" ), QSettings::NativeFormat );
		if ( settPrnt.allKeys().contains( "EncFS/" + baseName( file ) ) )
			return QStringList() << ":/icons/folder-locked.png";

		Q_FOREACH( QString key, settPrnt.allKeys() ) {
			if ( settPrnt.value( key ).toString() == baseName( file ) ) {
				/* If it is mounted, we return folder-unlocked */
				Q_FOREACH( NBDeviceInfo info, NBDeviceManager::allVirtualMounts() ) {
					if ( info.mountPoint() == file )
						return QStringList() << ":/icons/folder-unlocked.png";
				}

				/* Otherwise we return folder-locked */
				return QStringList() << ":/icons/folder-locked.png";
			}
		}

		return mdb.value( "inode/directory" ).toStringList();
	}

	// If it is a desktop file
	else if ( mName == QString( "application/x-desktop" ) ) {

		QSettings dSettings( file, QSettings::NativeFormat );
		QString icoStr = dSettings.value( "Desktop Entry/Icon" ).toString();
		return ( hasIcon( icoStr ) ? QStringList() << icoStr : mdb.value( mName ).toStringList() );
	}

	/* This is an image file */
	else if ( mName.startsWith( "image/" ) ) {
		if ( Settings->General.ImagePreviews ) {
			/* DJVU issue comes only when we have to show previews */
			if ( mName.contains( "djv" ) )
				return mdb.value( mName ).toStringList();

			return QStringList() << QDir( thumbsDir ).absoluteFilePath( MD5( file ) );
		}
	}

	return mdb.value( mName ).toStringList();
};

bool NBIconManager::hasIcon( QString iName ) {

	return idb.allKeys().contains( iName );
};

QStringList NBIconManager::icon( QString iName ) {

	return idb.value( iName, QStringList() << ":/icons/unknown.png" ).toStringList();
};

void NBIconManager::generateThemeDatabase() {

	/* List the theme inheritence */
	QStringList themes;
	themes << Settings->General.IconTheme;
	QSettings index( iconThemePath + themes[ 0 ] + "/index.theme", QSettings::NativeFormat );
	themes << index.value( "Icon Theme/Inherits" ).toStringList();

	/* Add hicolor to every theme and them remove duplicates */
	themes << "hicolor";
	themes.removeDuplicates();

	/* Get all theme directories */
	QStringList themeDirs;
	Q_FOREACH( QString theme, themes ) {
		QSettings index( iconThemePath + theme + "/index.theme", QSettings::NativeFormat );
		Q_FOREACH( QString dir, index.value( "Icon Theme/Directories" ).toStringList() )
			themeDirs << iconThemePath + theme + "/" + dir + "/";
	}
	themeDirs.removeDuplicates();

	/* Clear our older icon database */
	idb.clear();

	/* Create our icon database */
	int counter = 0;
	Q_FOREACH( QString dir, themeDirs ) {
		Q_FOREACH( QString file, listFiles( dir ) ) {
			if ( not ( file.endsWith( ".png" ) or file.endsWith( ".svg" ) or file.endsWith( ".xpm" ) or file.endsWith( ".svgz" ) ) )
				continue;

			QString base = baseName( file ).replace( ".png", "" ).replace( ".svg", "" ).replace( ".xpm", "" ).replace( ".svgz", "" );
			QStringList paths = idb.value( base, QStringList() ).toStringList();
			paths << file;

			idb.setValue( base, paths );
			counter++;
		}
	}

	qDebug() << "Added" << counter << "icons to the database";

	/* Clear our older mimetype database */
	mdb.clear();

	/* Create our mimetype database */
	counter = 0;
	Q_FOREACH( QMimeType mType, mimeDb.allMimeTypes() ) {
		QString mIcon = mType.iconName();
		QString gmIcon = mType.genericIconName();

		QStringList paths;

		paths << icon( mIcon );
		if ( not paths.count() )
			paths << icon( gmIcon );

		else if ( paths.at( 0 ) == ":/icons/unknown.png" ) {
			paths.clear();
			paths << icon( gmIcon );
		}

		if ( paths.count() )
			mdb.setValue( mType.name(), paths );

		else
			mdb.setValue( mType.name(), QStringList() << ":/icons/unknown.png" );

		counter++;
	}

	/* Sync our database */
	mdb.sync();

	qDebug() << "Added" << counter << "mimetypes to the database";

	/* DJVU Fix */
	mdb.setValue( "image/vnd.djvu+multipage", QStringList() << icon( "x-office-document" ) );

	/* Executable Fix */
	mdb.setValue( "application/x-executable", QStringList() << ":/icons/exec.png" );

	/* Shared Object Fix */
	mdb.setValue( "application/x-sharedlib", QStringList() << ":/icons/binary.png" );

	/* APK Fix */
	mdb.setValue( "application/vnd.android.package-archive", icon( "package-x-generic" ) );

	/* Desktop file Fix */
	mdb.setValue( "application/x-desktop", QStringList() << ":/icons/exec.png" );

	/* DOCX Fix */
	mdb.setValue( "application/vnd.openxmlformats-officedocument.wordprocessingml.document", icon( "application-vnd.ms-word" ) );

	/* PPTX Fix */
	mdb.setValue( "application/vnd.openxmlformats-officedocument.presentationml.presentation", icon( "application-vnd.ms-powerpoint" ) );

	/* XLSX Fix */
	mdb.setValue( "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet", icon( "application-vnd.ms-excel" ) );
};
