#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
	void inFileSelecter();
	void backFileSelecter();
	void outFileSelecter();
	void thresholder();

private:
    Ui::MainWindow *ui;

public:
	QString infileName;
	QString backfileName;
	QString outfileName;


};

#endif // MAINWINDOW_H
