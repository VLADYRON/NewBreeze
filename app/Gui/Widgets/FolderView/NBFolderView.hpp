/*
	*
	* NBFolderView.hpp - NewBreeze FolderView Class Header
	*
*/

#pragma once

#include "Global.hpp"
#include "NBArchiveDialog.hpp"
#include "NBConfirmDeleteDialog.hpp"
#include "NBContextMenu.hpp"
#include "NBCustomActions.hpp"
#include "NBCustomPeek.hpp"
#include "NBDeleteManager.hpp"
#include "NBDeleteProcess.hpp"
#include "NBErrorsDialog.hpp"
#include "NBFileDialog.hpp"
#include "NBItemViewModel.hpp"
#include "NBFolderFlash.hpp"
#include "NBGuiWidgets.hpp"
#include "NBIconProvider.hpp"
#include "NBIconView.hpp"
#include "NBIOProcess.hpp"
#include "NBLogger.hpp"
#include "NBMessageDialog.hpp"
#include "NBNewNodeDialog.hpp"
#include "NBPreviewInterface.hpp"
#include "NBProcessManager.hpp"
#include "NBPropertiesDialog.hpp"
#include "NBRenameDialog.hpp"
#include "NBRunCmdDialog.hpp"

class NBFolderView : public QStackedWidget {
	Q_OBJECT

	public :
		NBFolderView( QWidget *parent = NULL);

		void updateViewMode();

		QModelIndexList getSelection();
		bool hasSelection();

		QAction *peekAct, *moveAct, *copyAct, *pasteAct, *renameAct, *reloadAct, *trashAct, *delAct, *propertiesAct, *permissionsAct;
		QAction *actPrevDir, *actNextDir, *actParDir, *actHomeDir, *showHideDotFiles, *openVTE;
		QAction *actNewDir, *actNewFile;
		QAction *sortByNameAct, *sortByTypeAct, *sortBySizeAct, *sortByDateAct;
		QAction *groupsAct;

		NBIconView *IconView;

		NBItemViewModel *fsModel;
		NBActionsMenu *customMenu;

	private :
		QModelIndex indexAt( QPoint );
		void createAndSetupActions();

		bool moveItems;

		QClipboard *clipBoard;
		NBProcessManager *pMgr;

	public slots :
		void goUp();
		void goBack();
		void goForward();

		void newFolder();
		void newFile();

		void doOpen( QModelIndex );
		void doOpen( QString loc );
		void doOpenHome();
		void doOpenWith();
		void doOpenInNewWindow();
		void doOpenInNewTab();
		void doOpenWithCmd();

		void showFolders();

		void doRename();
		void doPeek();
		void doReload();
		void doToggleHidden();
		void move( QStringList, QString );
		void copy( QStringList, QString );
		void link( QStringList, QString );
		void doSendToTrash();
		void doDelete();

		void selectAll();

		void sortByName();
		void sortByType();
		void sortBySize();
		void sortByDate();

		void openTerminal();
		void showContextMenu( QPoint );

		// Over-riding the setFocus() of QWidget
		void setFocus();

	private slots :
		void updatePeekAct();

		void prepareCopy();
		void prepareMove();
		void prepareIO();

		void handleWatchDogBark( QString path );
		void extract( QString );
		void compress( QStringList );

		void updateProgress( QString, float, float );
		void handleDeleteFailure( QStringList, QStringList );

	Q_SIGNALS :
		void newWindow( QString );
		void newTab( QString );
		void selectionChanged( const QItemSelection&, const QItemSelection& );

		void showProperties();
		void showPermissions();

		void reloadCatalogs();
		void toggleGroups();

		void hideStatusBar();
		void showStatusBar();
};