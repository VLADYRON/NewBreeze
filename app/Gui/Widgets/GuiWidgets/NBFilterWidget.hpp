/*
	*
	* NBNBFilterWidget.hpp - NBNBFilterWidget.cpp header for NewBreeze
	*
*/

#pragma once

#include "Global.hpp"

class NBFilterWidget : public QWidget {
	Q_OBJECT

	public:
		NBFilterWidget( QWidget *parent );

		void clear();

	private:
		void setupUI();

		QLineEdit *findLE;
		QLabel *findLbl;

	public slots:
		void setFocus();
		void show();
		void hide();

		void showHide();

	protected:
		void keyPressEvent( QKeyEvent* );

	Q_SIGNALS:
		void search( QString );
		void shown();
		void hidden();
};
