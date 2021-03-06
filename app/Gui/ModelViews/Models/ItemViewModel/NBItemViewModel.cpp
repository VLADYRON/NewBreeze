/*
	*
	* NBItemViewModel.cpp - NewBreeze NBItemViewModel Class
	*
*/

#include "NBItemViewModel.hpp"

static QMutex mutex;
QStringList lambdaUseFilterList;

inline int matchesFilter( QStringList filters, QString text ) {

	Q_FOREACH( QString filter, filters )
		if ( text.contains( QRegExp( filter, Qt::CaseInsensitive, QRegExp::Wildcard ) ) )
			return 1;

	return 0;
};

inline int scandirCallback( const struct dirent* entry ) {

	/* Always filter . and .. */
	if ( not strcmp( entry->d_name, "." ) or not strcmp( entry->d_name, ".." ) )
		return 0;

	/* If we are not to show hidden files/folders, filter all names starting with . */
	if ( not Settings->General.ShowHidden and not strncmp( entry->d_name, ".", 1 ) )
		return 0;

	/* Name filter */
	if ( lambdaUseFilterList.count() ) {
		/* Do not filter folders, if the flag is not set */
		if ( not Settings->General.FilterFolders and ( entry->d_type == DT_DIR ) )
			return 1;

		/* If the name matches any one of the nameFilters, show it */
		return matchesFilter( lambdaUseFilterList, entry->d_name );
	}

	/* If no filter, select all */
	return 1;
};

NBItemViewModel::NBItemViewModel( QObject *parent ) : QAbstractItemModel( parent ) {

	/* Do we show the special directories */
	mModelDataType = NBItemViewModel::FileSystem;
	mVirtualData = false;

	/* Categrorization enabled by default */
	mCategorizationEnabled = Settings->General.Grouping;

	/* Useless swtiches */
	currentLoadStatus.loading = false;
	currentLoadStatus.stopLoading = false;

	prevSort.column = Settings->General.SortColumn;
	prevSort.cs = Settings->General.SortCase;
	prevSort.categorized = Settings->General.Grouping;

	oldRoots.clear();
	curIndex = 0;
	prevFolder = Settings->Session.LastDir;

	/* Root node */
	rootNode = new NBItemViewNode();

	quickDataGatherer = new NBQuickFileInfoGatherer();

	/* NBFileSystemWatcher */
	watcher = new NBFileSystemWatcher();
	connect( watcher, SIGNAL( nodeCreated( QString ) ), this, SLOT( handleNodeCreated( QString ) ) );
	connect( watcher, SIGNAL( nodeChanged( QString ) ), this, SLOT( handleNodeChanged( QString ) ) );
	connect( watcher, SIGNAL( nodeDeleted( QString ) ), this, SLOT( handleNodeDeleted( QString ) ) );
	connect( watcher, SIGNAL( nodeRenamed( QString, QString ) ), this, SLOT( handleNodeRenamed( QString, QString ) ) );
	connect( watcher, SIGNAL( watchPathDeleted() ), this, SLOT( loadHome() ) );

	thumbnailer = new NBThumbnailer();
	connect( thumbnailer, SIGNAL( updateNode( QString ) ), this, SLOT( nodeUpdated( QString ) ) );
};

NBItemViewModel::~NBItemViewModel() {

};

bool NBItemViewModel::isCategorizationEnabled() {

	return mCategorizationEnabled;
};

void NBItemViewModel::setCategorizationEnabled( bool enabled ) {

	mCategorizationEnabled = enabled;
	sort( prevSort.column, prevSort.cs, enabled );
};

int NBItemViewModel::rowCount( const QModelIndex & parent ) const {

	if ( parent.column() > 0 )
		return 0;

	if ( not parent.isValid() )
		return rootNode->childCount();

	else
		return static_cast<NBItemViewNode*>( parent.internalPointer() )->childCount();
};

int NBItemViewModel::rowCount( QString mCategory ) const {

	if ( not rootNode->categoryList().contains( mCategory ) or mCategory.isEmpty() or mCategory.isNull() )
		return 0;

	return categoryRowMap[ mCategory ].count();
};

int NBItemViewModel::categoryCount() const {

	return rootNode->categoryCount();
};

int NBItemViewModel::columnCount( const QModelIndex & parent ) const {

	Q_UNUSED( parent );
	return 7;
};

Qt::ItemFlags NBItemViewModel::flags( const QModelIndex & index ) const {

	if ( not index.isValid() )
		return Qt::NoItemFlags;

	if ( isDir( nodePath( index ) ) )
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;

	else
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
};

QVariant NBItemViewModel::data( const QModelIndex &index, int role ) const {

	if ( not index.isValid() )
		return QVariant();

	NBItemViewNode *node = static_cast<NBItemViewNode*>( index.internalPointer() );
	switch( role ) {

		case Qt::DisplayRole: {
			if ( ( index.column() >= 0 ) and ( index.column() <= 9  ) )
				return node->data( index.column() );

			else
				return QVariant();
		}

		case Qt::DecorationRole: {
			if ( index.column() == 0 ) {

				/* Icon stored in the node */
				return node->icon();
			}

			return QVariant();
		}

		case Qt::TextAlignmentRole: {
			if ( index.column() == 0 )
				return ( 0x0001 | 0x0080 );

			else if ( index.column() == 1 )
				return ( 0x0002 | 0x0080 );

			else
				return Qt::AlignCenter;
		}

		case Qt::InitialSortOrderRole: {
			return Qt::AscendingOrder;
		}

		case Qt::AccessibleTextRole: {
			return node->data( index.column() );
		}

		case Qt::ToolTipRole: {
			switch( index.column() ) {
				case 0:	{
					if ( mModelDataType == NBItemViewModel::Applications )
						return QString( "<center>%1<br>%2</center>" ).arg( node->data( 0 ).toString() ).arg( node->data( 2 ).toString() );

					else
						return QString( "<center>%1<br>%2</center>" ).arg( node->data( 0 ).toString() ).arg( node->data( 1 ).toString() );

				}
				default: {

					return QString( "<b>%1</b>" ).arg( node->data( index.column() ).toString() );
				}
			}
		}

		case Qt::UserRole + 0: {
			return node->data( 0 );
		}

		case Qt::UserRole + 1: {
			return node->data( 1 );
		}

		case Qt::UserRole + 2: {
			return node->data( 2 );
		}

		case Qt::UserRole + 3: {
			return node->data( 3 );
		}

		case Qt::UserRole + 4: {
			return node->data( 4 );
		}

		case Qt::UserRole + 5: {
			return node->data( 5 );
		}

		case Qt::UserRole + 6: {
			return node->data( 6 );
		}

		case Qt::UserRole + 7: {
			return node->data( 7 );
		}

		case Qt::UserRole + 8: {
			return node->data( 8 );
		}

		case Qt::UserRole + 9: {
			return node->data( 9 );
		}

		default: {
			return QVariant();
		}
	}
};

QVariant NBItemViewModel::headerData( int section, Qt::Orientation orientation, int role ) const {

	QVariantList headerList = QVariantList() << "Name" << "Size" << "Type" << "MimeType" << "Modified" << "Permissions" << "Owner";

	if ( ( orientation == Qt::Horizontal ) and ( role == Qt::DisplayRole ) )
		return headerList.at( section );

	else if ( ( orientation == Qt::Horizontal ) and ( role == Qt::TextAlignmentRole ) )
		if ( section == 0 )
			return ( 0x0001 | 0x0080 );

		else if ( section == 1 )
			return ( 0x0002 | 0x0080 );

		else
			return Qt::AlignCenter;

	else
		return QVariant();
};

bool NBItemViewModel::setData( const QModelIndex &index, QVariant value, int role ) {

	bool ok = rootNode->child( index.row() )->setData( role, value );
	if ( ok )
		emit dataChanged( index, index );

	return ok;
};

bool NBItemViewModel::isRealLocation() const {

	return not mVirtualData;
};

quint64 NBItemViewModel::modelDataType() const {

	return mModelDataType;
};

bool NBItemViewModel::insertNode( QString nodeName ) {

	/*
		*
		* Quick Fix: Append the newly added file at the end of the model.
		* Then sort the model.
		*
	*/

	if ( not exists( mRootPath + nodeName ) )
		return false;

	beginResetModel();
	beginInsertRows( parent(), mChildNames.count(), mChildNames.count() );

	QVariantList data = quickDataGatherer->getQuickFileInfo( mRootPath + nodeName );

	if ( Settings->General.ShowHidden ) {
		if ( mNameFilters.count() ) {
			if ( matchesFilter( mNameFilters, nodeName ) ) {
				rootNode->addChild( new NBItemViewNode( data, getCategory( data ), rootNode ) );
				mChildNames << nodeName;
			}
		}
		else {
			rootNode->addChild( new NBItemViewNode( data, getCategory( data ), rootNode ) );
			mChildNames << nodeName;
		}
	}
	else {
		if ( not nodeName.startsWith( "." ) ) {
			if ( mNameFilters.count() ) {
				if ( matchesFilter( mNameFilters, nodeName ) ) {
					rootNode->addChild( new NBItemViewNode( data, getCategory( data ), rootNode ) );
					mChildNames << nodeName;
				}
			}
			else {
				rootNode->addChild( new NBItemViewNode( data, getCategory( data ), rootNode ) );
				mChildNames << nodeName;
			}
		}
	}
	endInsertRows();

	sort( prevSort.column, prevSort.cs, prevSort.categorized );
	endResetModel();

	thumbnailer->createThumbnails( mRootPath );

	return true;
};

void NBItemViewModel::updateNode( QString nodeName ) {

	QMutexLocker locker( &mutex );
	if ( not exists( mRootPath + nodeName ) )
		return;

	if ( not mChildNames.contains( nodeName ) )
		return;

	NBItemViewNode *node = rootNode->child( nodeName );
	if ( isDir( mRootPath + nodeName ) ) {
		node->setData( 1, nChildren( mRootPath + nodeName ), true );
		node->setData( 1, QString( "%1 items" ).arg( node->data( 1, true ).toLongLong() ), false );
	}

	else {
		node->setData( 1, getSize( mRootPath + nodeName ), true );
		node->setData( 1, formatSize( node->data( 1, true ).toLongLong() ), false );
	}

	sort( prevSort.column, prevSort.cs, prevSort.categorized );

	thumbnailer->createThumbnails( mRootPath );
};

bool NBItemViewModel::removeNode( QString nodeName ) {

	if ( not mChildNames.contains( nodeName ) )
		return false;

	mChildNames.removeAll( nodeName );
	NBItemViewNode *removedNode = rootNode->child( nodeName );

	beginResetModel();
	beginRemoveRows( parent(), removedNode->row(), removedNode->row() );

	rootNode->removeChild( removedNode );

	endRemoveRows();
	endResetModel();

	if ( mCategorizationEnabled )
		recategorize();

	sort( prevSort.column, prevSort.cs, prevSort.categorized );

	return false;
};

void NBItemViewModel::nodeUpdated( QString nodeName ) {

	NBItemViewNode *node = rootNode->child( baseName( nodeName ) );
	node->updateIcon();

	emit dataChanged( index( nodeName ), index( nodeName ) );
};

QModelIndex NBItemViewModel::index( int row, int column, const QModelIndex &parent ) const {

    if (row < 0 || column < 0)
        return QModelIndex();

    if ( not ( ( row < rowCount( parent ) ) and ( column < columnCount( parent ) ) ) )
		return QModelIndex();

	NBItemViewNode *parentNode;
	if ( not parent.isValid() )
		parentNode = rootNode;

	else
		parentNode = (NBItemViewNode *)parent.internalPointer();

	NBItemViewNode *childNode = parentNode->child( row );
	if ( childNode )
		return createIndex( row, column, childNode );

	else
		return QModelIndex();
};

QModelIndex NBItemViewModel::index( QString name, const QModelIndex &parent ) const {

	if ( not mChildNames.contains( name ) )
		return QModelIndex();

	NBItemViewNode *parentNode;
	if ( not parent.isValid() )
		parentNode = rootNode;

	else
		parentNode = (NBItemViewNode *)parent.internalPointer();

	int row = parentNode->child( name )->row();
	NBItemViewNode *childNode = parentNode->child( name );

	if ( childNode )
		return createIndex( row, 0, childNode );

	return QModelIndex();
};

QModelIndex NBItemViewModel::parent( const QModelIndex &index ) const {

	if ( not index.isValid() )
		return QModelIndex();

	NBItemViewNode *childNode = ( NBItemViewNode * )index.internalPointer();
	NBItemViewNode *parentNode = childNode->parent();

	if ( parentNode == rootNode )
		return QModelIndex();

	return createIndex( parentNode->row(), 0, parentNode );
};

QString NBItemViewModel::category( const QModelIndex &index ) const {

	if ( not index.isValid() )
		return QString( "" );

	NBItemViewNode *childItem = ( NBItemViewNode * )index.internalPointer();
	return childItem->category();
};

int NBItemViewModel::categoryIndex( const QModelIndex &index ) const {

	if ( not index.isValid() )
		return -1;

	NBItemViewNode *childItem = ( NBItemViewNode * )index.internalPointer();
	return childItem->categoryIndex();
};

QStringList NBItemViewModel::categories() const {

	return rootNode->categoryList();
};

QPixmap NBItemViewModel::pixmapForCategory( QString categoryName, bool folded ) const {

	switch ( mModelDataType ) {

		case NBItemViewModel::Applications: {

			if ( categoryName == "Accessories" )
				return QIcon::fromTheme( "applications-accessories" ).pixmap( 24, 24 );

			else if ( categoryName == "Development" )
				return QIcon::fromTheme( "applications-development" ).pixmap( 24, 24 );

			else if ( categoryName == "Education" )
				return QIcon::fromTheme( "applications-education" ).pixmap( 24, 24 );

			else if ( categoryName == "Games" )
				return QIcon::fromTheme( "applications-games" ).pixmap( 24, 24 );

			else if ( categoryName == "Graphics" )
				return QIcon::fromTheme( "applications-graphics" ).pixmap( 24, 24 );

			else if ( categoryName == "Internet" )
				return QIcon::fromTheme( "applications-internet" ).pixmap( 24, 24 );

			else if ( categoryName == "Multimedia" )
				return QIcon::fromTheme( "applications-multimedia" ).pixmap( 24, 24 );

			else if ( categoryName == "Office" )
				return QIcon::fromTheme( "applications-office" ).pixmap( 24, 24 );

			else if ( categoryName == "Science and Math" )
				return QIcon::fromTheme( "applications-science" ).pixmap( 24, 24 );

			else if ( categoryName == "Settings" )
				return QIcon::fromTheme( "preferences-system" ).pixmap( 24, 24 );

			else if ( categoryName == "System" )
				return QIcon::fromTheme( "applications-system" ).pixmap( 24, 24 );

			else
				return QIcon::fromTheme( "preferences-other" ).pixmap( 24, 24 );
		}

		case NBItemViewModel::Catalogs: {

			if ( categoryName == "Documents" )
				return QIcon( ":/icons/documents.png" ).pixmap( 16, 16 );

			else if ( categoryName == "Music" )
				return QIcon::fromTheme( "folder-sound" ).pixmap( 16, 16 );

			else if ( categoryName == "Pictures" )
				return QIcon::fromTheme( "folder-image" ).pixmap( 16, 16 );

			else if ( categoryName == "Videos" )
				return QIcon::fromTheme( "folder-video" ).pixmap( 16, 16 );

			else
				return QIcon( ":/icons/catalogs-alt.png" ).pixmap( 16, 16 );
		}

		case NBItemViewModel::SuperStart: {
			if ( categoryName == "Places  " )
				return QIcon( ":/icons/folder.png" ).pixmap( 16, 16 );

			else if ( categoryName == "Computer  " )
				return QIcon( ":/icons/comp.png" ).pixmap( 16, 16 );

			else
				return QIcon( ":/icons/files.png" ).pixmap( 16, 16 );
		}

		default: {
			QIcon icon = QIcon::fromTheme( mCategoryIconMap[ categoryName ] );
			if ( icon.isNull() ) {
				if ( folded )
					return QIcon::fromTheme( "arrow-right" ).pixmap( 16, 16 );

				else
					return QIcon::fromTheme( "arrow-down" ).pixmap( 16, 16 );
			}
			else {

				return icon.pixmap( 16, 16 );
			}
		}
	}
};

int NBItemViewModel::indexListCountForCategory( QString mCategory ) const {

	if ( not rootNode->categoryList().contains( mCategory ) or mCategory.isEmpty() or mCategory.isNull() )
		return 0;

	return categoryRowMap[ mCategory ].count();
};

QModelIndexList NBItemViewModel::indexListForCategory( QString mCategory ) const {

	QModelIndexList mList;

	if ( not rootNode->categoryList().contains( mCategory ) or mCategory.isEmpty() or mCategory.isNull() )
		return mList;

	foreach( int row, categoryRowMap[ mCategory ] ) {
		mList << index( row, 0, QModelIndex() );
	}

	return mList;
};

int NBItemViewModel::indexListCountForCategory( int mCategoryIdx ) const {

	return indexListCountForCategory( rootNode->categoryList().at( mCategoryIdx ) );
};

QModelIndexList NBItemViewModel::indexListForCategory( int mCategoryIdx ) const {

	return indexListForCategory( rootNode->categoryList().at( mCategoryIdx ) );
};

QModelIndexList NBItemViewModel::categorySiblings( QModelIndex idx ) const {

	if ( not idx.isValid() )
		return QModelIndexList();

	QModelIndexList mList = indexListForCategory( category( idx ) );
	mList.removeAll( idx );

	return mList;
};

int NBItemViewModel::indexInCategory( QModelIndex idx ) const {

	return indexListForCategory( category( idx ) ).indexOf( idx );
};

Qt::DropActions NBItemViewModel::supportedDragActions() const {

	return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
};

Qt::DropActions NBItemViewModel::supportedDropActions() const {

	return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
};

Qt::ItemFlags NBItemViewModel::flags( const QModelIndex index ) const {

	if ( isReadable( nodePath( index ) ) )
		return Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled;

	else
		return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
};

QStringList NBItemViewModel::mimeTypes() const {

	QStringList types;
	types << "text/uri-list";
	return types;
};

bool NBItemViewModel::filter( Filters filter ) {

	Q_UNUSED( filter );

	return false;
};

void NBItemViewModel::setFilter( Filters filter, bool on ) {

	Q_UNUSED( filter );
	Q_UNUSED( on );

	return;
};

QStringList NBItemViewModel::nameFilters() {

	return mNameFilters;
};

void NBItemViewModel::setNameFilters( QStringList filters ) {

	mNameFilters.clear();
	mNameFilters << filters;

	setupModelData();
};

void NBItemViewModel::clearNameFilters() {

	mNameFilters.clear();
	setupModelData();
};

void NBItemViewModel::sort( int column, bool cs, bool categorized ) {

	prevSort.column = column;
	prevSort.cs = cs;
	prevSort.categorized = categorized;

	recategorize();
	rootNode->sort( column, cs, false );

	// Create a map of categoryIndex versus rows
	categoryRowMap.clear();
	foreach( NBItemViewNode *item, rootNode->children() )
		categoryRowMap[ item->category() ] << item->row();

	emit layoutChanged();
};

void NBItemViewModel::reload() {

	setupModelData();
};

bool NBItemViewModel::rename( QString oldName, QString newName ) {

	// Do nothing: This should never happen
	if ( mVirtualData )
		return true;

	/* If the file @oldName is not from the current directory */
	if ( dirName( oldName ) != mRootPath ) {
		insertNode( baseName( newName ) );
		return true;
	}

	/* If the file @newName is not in the current dir */
	else if ( dirName( newName ) != mRootPath ) {
		removeNode( baseName( oldName ) );
		return true;
	}

	else {
		/* Same folder */
		removeNode( baseName( oldName ) );
		insertNode( baseName( newName ) );

		return true;
	}

	/* This can never happen! */
	return false;
};

QString NBItemViewModel::nodeName( const QModelIndex idx ) const {

	return idx.data( 0 ).toString();
};

QString NBItemViewModel::nodePath( const QModelIndex idx ) const {

	switch ( mModelDataType ) {

		case NBItemViewModel::Applications: {

			return idx.data( Qt::UserRole + 9 ).toString();
		}

		case NBItemViewModel::SuperStart: {

			QString path = idx.data( Qt::UserRole + 2 ).toString();
			if ( exists( path ) )
				return path;

			else
				return idx.data( Qt::UserRole + 7 ).toString();
		}

		case NBItemViewModel::Catalogs:
		case NBItemViewModel::FileSystem:
			return idx.data( Qt::UserRole + 7 ).toString();
	}
};

QString NBItemViewModel::nodePath( const QString path ) const {

	QModelIndex idx = index( path );
	if ( not idx.isValid() )
		return mRootPath + path;

	return nodePath( idx );
};

QFileInfo NBItemViewModel::nodeInfo( const QModelIndex idx ) const {

	switch ( mModelDataType ) {

		case NBItemViewModel::Applications: {

			return QFileInfo( idx.data( Qt::UserRole + 9 ).toString() );
		}

		case NBItemViewModel::SuperStart: {

			QString path = idx.data( Qt::UserRole + 2 ).toString();
			if ( exists( path ) )
				return QFileInfo( path );

			else
				return QFileInfo( idx.data( Qt::UserRole + 7 ).toString() );
		}

		case NBItemViewModel::Catalogs:
		case NBItemViewModel::FileSystem:
			return QFileInfo( idx.data( Qt::UserRole + 7 ).toString() );
	}
};

QString NBItemViewModel::rootPath() const {

	return mRootPath;
};

void NBItemViewModel::setRootPath( QString path ) {

	if ( path.startsWith( "/" ) )
		prevFolder = path;

	mRootPath = path;

	if ( path.startsWith( "NB://SuperStart" ) ) {
		mVirtualData = true;
		mModelDataType = (quint64)NBItemViewModel::SuperStart;
	}

	else if ( path.startsWith( "NB://Applications" ) ) {
		mVirtualData = true;
		mModelDataType = (quint64)NBItemViewModel::Applications;
	}

	else if ( path.startsWith( "NB://Catalogs" ) ) {
		mVirtualData = true;
		mModelDataType = (quint64)NBItemViewModel::Catalogs;
	}

	else if ( path.startsWith( "NB://Folders" ) ) {
		mVirtualData = false;
		mModelDataType = (quint64)NBItemViewModel::FileSystem;

		if ( not prevFolder.isEmpty() )
			mRootPath = prevFolder;

		else
			mRootPath = Settings->Session.LastDir;

		thumbnailer->createThumbnails( mRootPath );
	}

	else {
		mVirtualData = false;
		mModelDataType = (quint64)NBItemViewModel::FileSystem;

		mRootPath = ( path.endsWith( "/" ) ? path : path + "/" );
		thumbnailer->createThumbnails( mRootPath );
	}

	/* Navigation: If we are in the middle, remove all 'forawrd' roots */
	if ( oldRoots.count() )
		oldRoots.erase( oldRoots.begin() + curIndex + 1, oldRoots.end() );

	/* Append this root to navigation list */
	oldRoots << mRootPath;
	curIndex = oldRoots.count() - 1;

	/* chdir( mRootPath ) to set the correct link to /pwd/<pid>/cwd */
	chdir( mRootPath.toLocal8Bit().constData() );

	/* We have set per-folder settings */
	QSettings sett( mRootPath + ".directory", QSettings::NativeFormat );

	/* Check per folder view settings */
	// Settings->General.ShowHidden = sett.value( "NewBreeze/Hidden", false ).toBool();

	prevSort.column = sett.value( "NewBreeze/SortColumn", Settings->General.SortColumn ).toInt();
	prevSort.cs = sett.value( "NewBreeze/SortCase", Settings->General.SortCase ).toBool();
	prevSort.categorized = sett.value( "NewBreeze/Grouping", Settings->General.Grouping ).toBool();
	mCategorizationEnabled = prevSort.categorized;

	/* Delete the old node */
	delete rootNode;

	/* Create a fresh root node */
	rootNode = new NBItemViewNode( quickDataGatherer->getQuickFileInfo( path ), "" );

	/* Stop loading thumbs, or even nodes */
	if ( currentLoadStatus.loading )
		currentLoadStatus.stopLoading = true;

	/* If the root path is /dev/, then stop the watcher */
	if ( mRootPath.startsWith( "/dev/" ) )
		watcher->stopWatch();

	/* For all the other folders, we can happily start the watcher */
	else
		watcher->startWatch( path );

	setupModelData();
};

void NBItemViewModel::goBack() {

	if ( canGoBack() ) {
		curIndex--;

		mRootPath = oldRoots.at( curIndex );

		if ( mRootPath.startsWith( "NB://SuperStart" ) ) {
			mVirtualData = true;
			mModelDataType = (quint64)NBItemViewModel::SuperStart;
		}

		else if ( mRootPath.startsWith( "NB://Applications" ) ) {
			mVirtualData = true;
			mModelDataType = (quint64)NBItemViewModel::Applications;
		}

		else if ( mRootPath.startsWith( "NB://Catalogs" ) ) {
			mVirtualData = true;
			mModelDataType = (quint64)NBItemViewModel::Catalogs;
		}

		else {
			mVirtualData = false;
			mModelDataType = (quint64)NBItemViewModel::FileSystem;
		}

		delete rootNode;
		rootNode = new NBItemViewNode( quickDataGatherer->getQuickFileInfo( mRootPath ), "" );

		setupModelData();
	}
};

void NBItemViewModel::goForward() {

	if ( canGoForward() ) {
		curIndex++;

		mRootPath = oldRoots.at( curIndex );

		if ( mRootPath.startsWith( "NB://SuperStart" ) ) {
			mVirtualData = true;
			mModelDataType = (quint64)NBItemViewModel::SuperStart;
		}

		else if ( mRootPath.startsWith( "NB://Applications" ) ) {
			mVirtualData = true;
			mModelDataType = (quint64)NBItemViewModel::Applications;
		}

		else if ( mRootPath.startsWith( "NB://Catalogs" ) ) {
			mVirtualData = true;
			mModelDataType = (quint64)NBItemViewModel::Catalogs;
		}

		else {
			mVirtualData = false;
			mModelDataType = (quint64)NBItemViewModel::FileSystem;
		}

		delete rootNode;
		rootNode = new NBItemViewNode( quickDataGatherer->getQuickFileInfo( mRootPath ), "" );

		setupModelData();
	}
};

void NBItemViewModel::goUp() {

	if ( canGoUp() ) {
		QString newRoot = mRootPath.section( "/", 0, -3 );
		newRoot = ( newRoot.endsWith( "/" ) ? newRoot : newRoot + "/" );
		setRootPath( newRoot );
	}
};

void NBItemViewModel::goHome() {

	if ( Settings->General.SpecialOpen and Settings->General.SuperStart )
		setRootPath( "NB://SuperStart" );

	else if ( Settings->General.SpecialOpen and Settings->General.OpenWithCatalog )
		setRootPath( "NB://Catalogs" );

	else
		setRootPath( QDir::homePath() );
};

bool NBItemViewModel::canGoBack() const {

	return ( curIndex > 0 );
};

bool NBItemViewModel::canGoForward() const {

	return ( curIndex < ( oldRoots.count() - 1 ) ? true : false);
};

bool NBItemViewModel::canGoUp() const {

	if ( oldRoots.at( curIndex ).startsWith( "NB://" ) )
		return false;

	return ( mRootPath != "/" );
};

QString NBItemViewModel::previousDir() const {

	if ( canGoBack() )
		return oldRoots.at( curIndex - 1 );

	else
		return QString( "" );
};

QString NBItemViewModel::nextDir() const {

	if ( canGoForward() )
		return oldRoots.at( curIndex + 1 );

	else
		return QString( "" );
};

QString NBItemViewModel::currentDir() const {

	return mRootPath;
};

QString NBItemViewModel::parentDir() const {

	QString path = mRootPath.section( "/", 0, -3 );
	return ( path.endsWith( "/" ) ? path : path + "/" );
};

QString NBItemViewModel::lastOpenedFolder() const {

	return prevFolder;
};

void NBItemViewModel::setupModelData() {

	switch( mModelDataType ) {
		case NBItemViewModel::SuperStart: {
			setupSuperStartData();
			return;
		}

		case NBItemViewModel::Applications: {
			setupApplicationsData();
			return;
		}

		case NBItemViewModel::Catalogs: {
			setupCatalogData();
			return;
		}

		case NBItemViewModel::FileSystem: {
			setupFileSystemData();
			thumbnailer->createThumbnails( mRootPath );
			return;
		}
	}
};

void NBItemViewModel::setupFileSystemData() {

	mChildNames.clear();
	rootNode->clearChildren();
	currentLoadStatus.loading = true;

	/* These are useful only when we do not refresh or change into some directory. */
	lastUpdatedNodes.clear();
	lastUpdatedTimes.clear();
	delayedUpdateList.clear();

	emit directoryLoading( mRootPath );

	beginResetModel();

	/* Get file list */
	struct dirent **fileList;
	lambdaUseFilterList << mNameFilters;
	int numFiles = scandir( mRootPath.toStdString().c_str(), &fileList, scandirCallback, NULL );
	lambdaUseFilterList.clear();

	/* Add the files to the model */
	if ( numFiles >= 0 ) {
		for( int i = 0; i < numFiles; i++ ) {
			QString _nodeName = QString::fromLocal8Bit( fileList[ i ]->d_name );
			QVariantList data = quickDataGatherer->getQuickFileInfo( mRootPath + _nodeName );
			QString _category = getCategory( data );
			rootNode->addChild( new NBItemViewNode( data, _category, rootNode ) );
			mChildNames << _nodeName;
			mCategoryIconMap[ _category ] = data.at( 2 ).toString();
			free( fileList[ i ] );
		}
		free( fileList );
	}

	endResetModel();

	/* We make all the categories visible by default, except the saved ones */
	QSettings dirSett( mRootPath + ".directory", QSettings::NativeFormat );

	/* Sort the contents */
	sort( prevSort.column, prevSort.cs, prevSort.categorized );

	/* Switch off the loading flag */
	currentLoadStatus.loading = false;

	/* Emit directory loaded */
	emit directoryLoaded( mRootPath );
};

void NBItemViewModel::setupSuperStartData() {

	mChildNames.clear();
	rootNode->clearChildren();
	currentLoadStatus.loading = true;

	emit directoryLoading( mRootPath );

	QSettings superStart( "NewBreeze", "SuperStart" );

	beginResetModel();

	Q_FOREACH( NBDeviceInfo info, NBDeviceManager::allMounts() ) {
		QVariantList data;
		data << "SuperStart" << 0 << ":/icons/" + info.deviceType() + ".png";
		data << info.displayName();
		data << QString( "%1% used" ).arg( info.bytesUsed() * 100 / info.bytesTotal() );
		data << info.mountPoint();

		if ( info.mountPoint() == "/" )
			data[ 3 ] = "FileSystem";

		rootNode->addChild( new NBItemViewNode( data, "Computer  ", rootNode ) );
		mChildNames << info.displayName();
	};

	/* Bookmarked Folders */
	superStart.beginGroup( "Places" );
	foreach( QString key, superStart.childKeys() ) {
		QString location = superStart.value( key ).toString();
		if ( not exists( location ) )
			continue;

		QVariantList data = quickDataGatherer->getQuickFileInfo( location );
		data[ 0 ] = "dirss";				// dirss Needed for getting the right icon
		data[ 3 ] = key;
		rootNode->addChild( new NBItemViewNode( data, "Places  ", rootNode ) );
		mChildNames << key;
	}
	superStart.endGroup();

	/* Recent Applications */
	superStart.beginGroup( "Files" );
	foreach( QString key, superStart.childKeys() ) {
		QString path = superStart.value( key ).toString();
		QVariantList data = quickDataGatherer->getQuickFileInfo( path );
		data[ 3 ] = key;
		rootNode->addChild( new NBItemViewNode( data, "Files  ", rootNode ) );
		mChildNames << key;
	}
	superStart.endGroup();

	endResetModel();

	sort( prevSort.column, prevSort.cs, prevSort.categorized );

	currentLoadStatus.loading = false;

	emit directoryLoaded( mRootPath );
};

void NBItemViewModel::setupApplicationsData() {

	mChildNames.clear();
	rootNode->clearChildren();
	currentLoadStatus.loading = true;

	emit directoryLoading( mRootPath );

	beginResetModel();
	NBAppEngine *appEngine = NBAppEngine::instance();
	foreach( NBAppFile app, appEngine->allDesktops().toQList() ) {
		if ( ( app.value( NBAppFile::Type ) != "Application" ) or ( app.value( NBAppFile::NoDisplay ).toBool() ) )
			continue;

		QVariantList data;

		/* Special Data */
		data << "Application" << 0 << app.value( NBAppFile::Icon );

		/* Normal Data */
		data << app.value( NBAppFile::Name );												/* Qt::UserRole + 0 */
		data << app.value( NBAppFile::Exec );												/* Qt::UserRole + 1 */
		data << app.value( NBAppFile::Comment );											/* Qt::UserRole + 2 */
		data << app.execArgs();																/* Qt::UserRole + 3 */
		data << app.value( NBAppFile::Icon );												/* Qt::UserRole + 4 */
		data << app.value( NBAppFile::WorkPath );											/* Qt::UserRole + 5 */
		data << app.value( NBAppFile::MimeTypes );											/* Qt::UserRole + 6 */
		data << app.value( NBAppFile::TerminalMode );										/* Qt::UserRole + 7 */
		data << app.value( NBAppFile::Categories );											/* Qt::UserRole + 8 */
		data << app.filePath();																/* Qt::UserRole + 9 */

		rootNode->addChild( new NBItemViewNode( data, app.category(), rootNode ) );
	}
	endResetModel();

	sort( prevSort.column, prevSort.cs, prevSort.categorized );

	currentLoadStatus.loading = false;

	emit directoryLoaded( mRootPath );
};

void NBItemViewModel::setupCatalogData() {

	mChildNames.clear();
	rootNode->clearChildren();
	currentLoadStatus.loading = true;

	QStringList dirs;

	emit directoryLoading( mRootPath );

	QSettings catalogsSettings( "NewBreeze", "Catalogs" );
	/* Default Catalogs */
	beginResetModel();
	foreach( QString ctlg, catalogsSettings.childKeys() ) {
		QStringList locations = catalogsSettings.value( ctlg ).toStringList();
		foreach( QString location, locations ) {
			if ( not exists( location ) )
				continue;

			QVariantList data = quickDataGatherer->getQuickFileInfo( location );
			rootNode->addChild( new NBItemViewNode( data, ctlg, rootNode ) );
			mChildNames << data.at( 3 ).toString();
			dirs << location;
		}
	}

	/* Custom Catalogs */
	catalogsSettings.beginGroup( "Custom" );
	foreach( QString ctlg, catalogsSettings.childKeys() ) {
		QStringList locations = catalogsSettings.value( ctlg ).toStringList();
		foreach( QString location, locations ) {
			if ( not exists( location ) )
				continue;

			QVariantList data = quickDataGatherer->getQuickFileInfo( location );
			rootNode->addChild( new NBItemViewNode( data, ctlg, rootNode ) );
			mChildNames << data.at( 3 ).toString();
			dirs << location;
		}
	}
	catalogsSettings.endGroup();
	endResetModel();

	sort( prevSort.column, prevSort.cs, prevSort.categorized );

	currentLoadStatus.loading = false;

	emit directoryLoaded( mRootPath );
};

void NBItemViewModel::newWatch( QString path ) {

	/* Path is not empty and exists, start the watch */
	if ( path.count() and exists( path ) )
		watcher->startWatch( path );
};

QString NBItemViewModel::getCategory( QVariantList data ) {

	switch( prevSort.column ) {
		/* Name Sorting */
		case 0: {
			if ( data.at( 0 ).toString() == "dir" )
				return data.at( 3 ).toString().toUpper().at( 0 );

			else
				return data.at( 3 ).toString().toUpper().at( 0 ) + QString( " " );
		}

		/* Size Sorting */
		/* Separate directories from files */
		case 1: {
			/* Folders */
			if ( data.at( 0 ).toString() == "dir" )
				return "Folders";
			/* 0 - 100 kB */
			if ( data.at( 1 ).toLongLong() < 102400 )
				return "Tiny";
			/* 100 kB - 10 MB */
			if ( data.at( 1 ).toLongLong() < 10485760 )
				return "Small";
			/* 10 MB - 100 MB */
			if ( data.at( 1 ).toLongLong() < 104857600 )
				return "Medium";
			/* 100 MB - 1 GB */
			if ( data.at( 1 ).toLongLong() < 1073741824 )
				return "Large";
			/* 0 - 100 kB */
			if ( data.at( 1 ).toLongLong() >= 1073741824 )
				return "Massive";
		}

		/* Type Sorting */
		case 2: {
			if ( data.at( 0 ).toString() == "dir" ) {
				return "Folders";
			}

			if ( data.at( 0 ).toString() == "file" ) {
				return data.at( 5 ).toString();
			}

			if ( data.at( 0 ).toString() == "system" ) {
				return "System Files";
			}
		}

		case 4: {
			QDate date = QDate::fromString( data.at( 7 ).toString(), "ddd, dd MMM, yyyy" );
			if ( date.isValid() ) {
				if ( date == QDate::currentDate() )
					return "Today";

				else if ( date.weekNumber() == QDate::currentDate().weekNumber() )
					return "This Week";

				else if ( date.weekNumber() == QDate::currentDate().weekNumber() - 1 )
					return "Last Week";

				else if ( date.month() == QDate::currentDate().month() )
					return "This Month";

				else if ( date.month()== QDate::currentDate().month() - 1 )
					return "Last Month";

				else
					return date.toString( "MMMM yyyy" );
			}

			return "Uncategorized";
		}

		default: {
			return QString( "Uncategorized" );
		}
	}

	return QString( "Uncategorized" );
};

void NBItemViewModel::recategorize() {

	foreach( NBItemViewNode *node, rootNode->children() )
		if ( not mVirtualData )
			node->setCategory( getCategory( node->allData() ) );

	rootNode->updateCategories();
};

void NBItemViewModel::handleNodeCreated( QString node ) {

	if ( baseName( node ).startsWith( "." ) and not Settings->General.ShowHidden )
		return;

	if ( dirName( node ) == currentDir() )
		insertNode( baseName( node ) );
};

/*
	*
	* We have updated the three function to ignore rapid inotify events triggered
	* due to file copy. When a file is copied, it is rapidly changed. If an event is
	* triggered for the same node in succession, then we quarantine it for a second.
	*
*/
void NBItemViewModel::handleNodeChanged( QString node ) {

	if ( baseName( node ).startsWith( "." ) )
		return;

	if ( dirName( node ) == currentDir() ) {
		qint64 idx = lastUpdatedNodes.indexOf( node );
		if ( idx == -1 ) {
			updateNode( baseName( node ) );
			lastUpdatedNodes << node;
			lastUpdatedTimes << QTime::currentTime();
		}

		/* To handle copying or moving, or any continually changing node */
		else {
			/* Past update was more than ten second ago */
			if ( lastUpdatedTimes.at( idx ).secsTo( QTime::currentTime() ) > 10000 ) {
				lastUpdatedTimes.replace( idx, QTime::currentTime() );
				updateNode( baseName( node ) );
			}
		}
	}
};

void NBItemViewModel::handleNodeDeleted( QString node ) {

	if ( baseName( node ).startsWith( "." ) and not Settings->General.ShowHidden )
		return;

	if ( dirName( node ) == currentDir() )
		removeNode( baseName( node ) );
};

void NBItemViewModel::handleNodeRenamed( QString oldNode, QString newNode ) {

	rename( oldNode, newNode );
};

void NBItemViewModel::loadHome() {

	emit runningHome( currentDir() );
	goHome();
};
