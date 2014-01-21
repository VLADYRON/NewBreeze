/*
	*
	* NBDeleteManager.cpp - NewBreeze File/Folder Deleter Class
	*
*/

#include <NBDeleteManager.hpp>
#include <NBTools.hpp>

NBDeleteManager::NBDeleteManager( QObject *parent, bool trash ) : QObject( parent ) {

	thread = new QThread;
	deleter = new NBDeleter;
	trasher = new NBTrasher;

	if ( trash == true ) {
		trasher->moveToThread( thread );
		connect(
			trasher, SIGNAL( completed( QStringList, QStringList ) ),
			this, SLOT( failureHandler( QStringList, QStringList ) )
		);
	}

	else {
		deleter->moveToThread( thread );
		connect(
			deleter, SIGNAL( completed( QStringList, QStringList ) ),
			this, SLOT( failureHandler( QStringList, QStringList ) )
		);
	}

	thread->start();
};

void NBDeleteManager::deleteFromDisk( QStringList deleteList ) {

	QMetaObject::invokeMethod( deleter, "deleteFilesAndFolders", Qt::QueuedConnection, Q_ARG( QStringList, deleteList ) );
};

void NBDeleteManager::sendToTrash( QStringList trashList ) {

	QMetaObject::invokeMethod( trasher, "trashFilesAndFolders", Qt::QueuedConnection, Q_ARG( QStringList, trashList ) );
};

void NBDeleteManager::failureHandler( QStringList failedFiles, QStringList failedDirs ) {

	emit deleteOperationComplete( failedFiles, failedDirs );
};

NBDeleter::NBDeleter( QObject *parent ) : QObject( parent ) {

};

void NBDeleter::removeDir( QString dirName ) {

	QDir dir( dirName);

    if ( dir.exists( dirName ) ) {
		QDir::Filters dFilters = QDir::NoDotAndDotDot | QDir::System | QDir::Hidden | QDir::AllDirs | QDir::Files;
        Q_FOREACH( QFileInfo info, dir.entryInfoList( dFilters, QDir::DirsFirst ) ) {
            if ( info.isDir() )
                removeDir( info.absoluteFilePath() );

            else
                removeFile( info.absoluteFilePath() );
        }

        if ( not dir.rmdir( dirName ) )
			failedDirs << dirName;
    }
};

void NBDeleter::removeFile( QString fileName ) {

	if ( not QFile::remove( fileName ) )
		failedFiles << fileName;
};

void NBDeleter::deleteFilesAndFolders( QStringList deleteList ) {

	foreach( QString item, deleteList ) {
		if ( QFileInfo( item ).isDir() )
			removeDir( item );

		else
			removeFile( item );
	}

	emit completed( failedFiles, failedDirs );
};

NBTrasher::NBTrasher( QObject *parent ) : QObject( parent ) {

};

void NBTrasher::trashFilesAndFolders( QStringList trashList ) {

	QString trashLoc = NBXdg::trashLocation( trashList.at( 0 ) );

	foreach( QString item, trashList ) {
		/* Get the trashed path: $TRASH/files/filename */
		QString newPath = trashLoc + "/files/" + baseName( item );
		QString delTime = QDateTime::currentDateTime().toString( "yyyyMMddThh:mm:ss" );

		/* If it exists, add a date time to it to make it unique */
		if ( access( qPrintable( newPath ), R_OK ) == 0 )
			newPath += delTime;

		qDebug() << item << newPath;

		/* Try trashing it. If it fails, intimate the user */
		if ( rename( qPrintable( item ), qPrintable( newPath ) ) ) {
			qDebug() << "Error" << errno << ": Failed to trash " << item << ":" << strerror( errno );
			failedFiles << item;
		}

		/* If it succeeds, we write the meta data */
		else {
			QFile metadata( trashLoc + "/info/" + baseName( newPath ) + ".trashinfo" );
			metadata.open( QIODevice::WriteOnly );
			metadata.write(
				QString(
					"[Trash Info]\n"
					"Path=%1\n"
					"DeletionDate=%2\n"
				).arg( item ).arg( delTime ).toLocal8Bit()
			);
			metadata.close();

			/* An ugly hack: Shortcut for TrashModel listing */
			QSettings trashInfo( "NewBreeze", "TrashInfo" );
			trashInfo.setValue( baseName( item ), QStringList() << item << delTime << newPath );
		}
	}

	emit completed( failedFiles, failedDirs );
};
