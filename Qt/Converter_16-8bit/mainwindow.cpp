// compresser
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QFile>
#include <QTextStream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
	ui->textEdit->setReadOnly(true);
    QString infoFile = QCoreApplication::applicationDirPath() + "/info/converter_16-8bit_info.txt";
	QFile file(infoFile);
	bool ok = file.open(QIODevice::ReadOnly | QIODevice::Text);
	if (!ok) {
		ui->textEdit->append("The information file is missing:");
		ui->textEdit->append(infoFile);
		return;
	}
	QTextStream in(&file);
	QString line = in.readLine();
	while (!line.isNull()) {
		ui->textEdit->append(line);
		line = in.readLine();
	}
	file.close();
	ui->textEdit->moveCursor(QTextCursor::Start);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::inputFileSelecter()
{
	ui->labelResult->setText("");
	inputFileName = QFileDialog::getOpenFileName(this,
        tr("Input 16-bit TIFF file"), ".", tr("TIFF Files (*.tif)"));
	ui->labelInputFile->setText(inputFileName);
}

void MainWindow::outputFileSelecter()
{
	ui->labelResult->setText("");
	outputFileName = QFileDialog::getSaveFileName(this,
        tr("Output 8-bit TIFF file"), ".", tr("TIFF Files (*.tif)"));
	ui->labelOutputFile->setText(outputFileName);
}

void MainWindow::converter()
{
	int res;
    QString qstr, resultstr;
    char cmdstr[2048];

    qstr = QCoreApplication::applicationDirPath() + "/exec/convert_16-8bit ";
	qstr += inputFileName;
	qstr += " ";
	qstr += outputFileName;
    qstr += " ";
    qstr += ui->lineEdit_nbits->text();
    qstr += " ";
    qstr += ui->lineEdit_thresh->text();

	if (qstr.size()>(int)sizeof(cmdstr)-1) {
		printf("Failed to convert qstr->cmdstr since qstr didn't fit\n");
		resultstr = "FAILED: cmdstr not big enough for the command";
		ui->labelResult->setText(resultstr);
		return;
	}
    strcpy(cmdstr, qstr.toLocal8Bit().constData());

	ui->labelResult->setText(cmdstr);
	res = system(cmdstr);
	if (res == 0)
		resultstr = "SUCCESS";
	else if (res == 1)
		resultstr = "FAILED: wrong number of arguments";
	else if (res == 2)
		resultstr = "FAILED: Read error on input file";
	else if (res == 3)
		resultstr = "FAILED: Write error on output file";
    ui->labelResult->setText(resultstr);
}

