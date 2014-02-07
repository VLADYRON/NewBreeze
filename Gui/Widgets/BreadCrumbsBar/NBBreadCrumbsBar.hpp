/*
	*
	* NBBreadCrumbsBar.hpp - NBBreadCrumbsBar.cpp Header
	*
*/

#pragma once
#ifndef NBBREADCRUMBSBAR_HPP
#define NBBREADCRUMBSBAR_HPP

#include <Global.hpp>
#include <NBGuiWidgets.hpp>
#include <NBTools.hpp>

class NBBreadCrumbMenu : public QLabel {
	Q_OBJECT

	public:
		NBBreadCrumbMenu( QWidget *parent = 0, QString path = QString() );

	private:
		void createMenu();

		QDir cwd;
		NBMenu *menu;

	private slots:
		void onMenuHidden();
		void onMenuItemClicked();

	protected:
		void mousePressEvent( QMouseEvent * );

	signals:
		void openThisLocation( QString );
};

class NBBreadCrumb : public QLabel {
	Q_OBJECT

	public:
		NBBreadCrumb( QWidget *parent = 0, QString path = QString(), bool current = false );
		QDir cwd;

	protected:
		void mousePressEvent( QMouseEvent * );

	signals:
		void openThisLocation( QString );
};

class NBBreadCrumbsBar : public QWidget {
	Q_OBJECT

	public:
		NBBreadCrumbsBar( QWidget *parent = 0, QString address = 0 );

		bool autoLoadNewPath();
		void setAutoLoadNewPath( bool );

		bool hasTrailingDir();

		void setCurrentDirectory( QString );

	private:
		QDir cwd;
		bool autoLoad;
		bool hasTrailing;

	public slots:
		void loadPath( QString );

	private slots:
		void handleCrumbAndMenuSignal( QString );

	signals:
		void openLocation( QString );
};

#endif
