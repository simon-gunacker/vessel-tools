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
    QString infoFile = QCoreApplication::applicationDirPath() + "/info/quantifier_info.txt";
	QFile file(infoFile);
	bool ok = file.open(QIODevice::ReadOnly | QIODevice::Text);
	if (!ok) {
		ui->textEdit->append("The information file is missing:");
		ui->textEdit->append(infoFile);
    } else {
        QTextStream in(&file);
        QString line = in.readLine();
        while (!line.isNull()) {
            ui->textEdit->append(line);
            line = in.readLine();
        }
        file.close();
        ui->textEdit->moveCursor(QTextCursor::Start);
    }
    imageViewer = NULL;
    fpout = NULL;
    is_ready = false;
    is_amfile = false;
    is_closefile = false;
    is_resultfile = false;
    ui->pushButton_vessels->setEnabled(false);
    ui->groupBox_slice->setEnabled(false);
    checkReady();
    reset();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::amFileSelecter()
{
	ui->labelResult->setText("");
    amFileName = QFileDialog::getOpenFileName(this,
        tr("Input Amira file"), ".", tr("Amira Files (*.am)"));
    ui->labelAmFile->setText(amFileName);
    is_amfile = true;
    checkReady();
    reset();
}

void MainWindow::closeFileSelecter()
{
	ui->labelResult->setText("");
    closeFileName = QFileDialog::getOpenFileName(this,
        tr("Input close image binary file"), ".", tr("Bin Files (*.bin)"));
    ui->labelCloseFile->setText(closeFileName);
    is_closefile = true;
    checkReady();
    reset();
}

void MainWindow::resultFileSelecter()
{
    ui->labelResult->setText("");
    resultFileName = QFileDialog::getSaveFileName(this,
        tr("Result file"), ".", tr("Result Files (*.out)"));
    ui->labelResultFile->setText(resultFileName);
    is_resultfile = true;
    checkReady();
    reset();
}

void MainWindow::voxelChanged()
{
//    reset();
    checkReady();
    if (isSetup()) {
        computeVolume();
    }
}

void MainWindow::sliceChanged()
{
    is_slice = ui->checkBoxSlice->isChecked();
    ui->checkBoxAverage->setChecked(!is_slice);
    is_average = ui->checkBoxAverage->isChecked();
    ui->lineEdit_intercept->setEnabled(is_slice);
    ui->groupBox_centre->setEnabled(is_average&&ui->radioButton_centre->isChecked());
    ui->groupBox_range->setEnabled(is_average&&ui->radioButton_range->isChecked());
}

void MainWindow::averageChanged()
{
    is_average = ui->checkBoxAverage->isChecked();
    ui->checkBoxSlice->setChecked(!is_average);
    is_slice = ui->checkBoxSlice->isChecked();
    ui->lineEdit_intercept->setEnabled(is_slice);
    ui->groupBox_centre->setEnabled(is_average&&ui->radioButton_centre->isChecked());
    ui->groupBox_range->setEnabled(is_average&&ui->radioButton_range->isChecked());
}

void MainWindow::on_radioButton_centre_toggled(bool checked)
{
    if (checked) {
        ui->groupBox_centre->setEnabled(true);
        ui->groupBox_range->setEnabled(false);
    } else {
        ui->groupBox_centre->setEnabled(false);
        ui->groupBox_range->setEnabled(true);
    }
}

void MainWindow::checkBox_x()
{
    if (ui->checkBox_xfull->isChecked()) {
        ui->lineEdit_x1->setEnabled(false);
        ui->lineEdit_x2->setEnabled(false);
    } else {
        ui->lineEdit_x1->setEnabled(true);
        ui->lineEdit_x2->setEnabled(true);
    }
}

void MainWindow::checkBox_y()
{
    if (ui->checkBox_yfull->isChecked()) {
        ui->lineEdit_y1->setEnabled(false);
        ui->lineEdit_y2->setEnabled(false);
    } else {
        ui->lineEdit_y1->setEnabled(true);
        ui->lineEdit_y2->setEnabled(true);
    }
}

void MainWindow::checkBox_z()
{
    if (ui->checkBox_zfull->isChecked()) {
        ui->lineEdit_z1->setEnabled(false);
        ui->lineEdit_z2->setEnabled(false);
    } else {
        ui->lineEdit_z1->setEnabled(true);
        ui->lineEdit_z2->setEnabled(true);
    }
}

void MainWindow::checkReady()
{
    bool voxelOK;
    voxelsize[0] = ui->lineEdit_xvoxel->text().toDouble();
    voxelsize[1] = ui->lineEdit_yvoxel->text().toDouble();
    voxelsize[2] = ui->lineEdit_zvoxel->text().toDouble();
    if (voxelsize[0] > 0 && voxelsize[1] > 0 && voxelsize[2] > 0)
        voxelOK = true;
    else
        voxelOK = false;
    if ((voxelOK || isSetup()) && is_amfile && is_closefile && is_resultfile) {
        is_ready = true;
        ui->pushButton_setup->setEnabled(true);
    } else {
        is_ready = false;
        ui->pushButton_setup->setEnabled(false);
        ui->groupBox_slice->setEnabled(false);
        ui->pushButton_vessels->setEnabled(false);
    }
}

void MainWindow::reader()
{
    doSetup();
    if (isSetup()) {
        ui->groupBox_slice->setEnabled(true);
        ui->pushButton_vessels->setEnabled(true);
    } else {
        is_ready = false;
        ui->pushButton_setup->setEnabled(false);
        ui->groupBox_slice->setEnabled(false);
        ui->pushButton_vessels->setEnabled(false);
    }
}

void MainWindow::computeVolume()
{
    int err;
    int ntvoxels;
    double volume;
    QString volumestr, countstr;
    bool voxelOK;

    if (!is_ready) {
        return;
    }
    if (!isSetup()) {
        return;
    }
    voxelsize[0] = ui->lineEdit_xvoxel->text().toDouble();
    voxelsize[1] = ui->lineEdit_yvoxel->text().toDouble();
    voxelsize[2] = ui->lineEdit_zvoxel->text().toDouble();
    if (voxelsize[0] > 0 && voxelsize[1] > 0 && voxelsize[2] > 0)
        voxelOK = true;
    else
        voxelOK = false;
    if (!voxelOK) return;
    err = getVolume(&volume,&ntvoxels);
    countstr = QString::number(ntvoxels);
    ui->lineEdit_ntvoxels->setText(countstr);
    fprintf(fpout,"Total voxel count: %d\n",ntvoxels);
    volume = 1.0e-9*volume;   // convert um3 -> mm3
    volumestr = QString::number(volume,'f',3);
    ui->lineEdit_volume->setText(volumestr);
    fprintf(fpout,"Total volume (mm3): %7.3f\n",volume);
}

void MainWindow::computeArea()
{
    int err;
    double area;
    int axis, islice, npixels;
    QString areastr;

    if (!is_ready) {
        return;
    }
    if (ui->radioButton_xaxis->isChecked())
        axis = 0;
    else if (ui->radioButton_yaxis->isChecked())
        axis = 1;
    else if (ui->radioButton_zaxis->isChecked())
        axis = 2;
    islice = ui->lineEdit_intercept->text().toInt();
    if (ui->radioButton_slicemicrons->isChecked())
        islice = (int)(islice/voxelsize[axis]);
    err = checkSlice(axis,islice);
    if (err == 0)
        err = getArea(axis,islice,&npixels,&area);
    else
        area = 0;
    area = 1.0e-6*area;   // convert um2 -> mm2
    areastr = QString::number(area,'f',3);
    ui->lineEdit_area->setText(areastr);
    ui->lineEdit_count->setText("");
    ui->lineEdit_density->setText("");
}

void MainWindow::getCentredRanges()
{
    double x0, y0, z0, xw, yw, zw, xr, yr, zr, xfac, yfac, zfac;
    QString x0str, y0str, z0str, xsizestr, ysizestr, zsizestr;

    x0str = ui->lineEdit_xcentre->text();
    y0str = ui->lineEdit_ycentre->text();
    z0str = ui->lineEdit_zcentre->text();
    xsizestr = ui->lineEdit_xsize->text();
    ysizestr = ui->lineEdit_ysize->text();
    zsizestr = ui->lineEdit_zsize->text();
    x0 = x0str.toDouble();
    y0 = y0str.toDouble();
    z0 = z0str.toDouble();
    xw = xsizestr.toDouble();
    yw = ysizestr.toDouble();
    zw = zsizestr.toDouble();
    xr = xw/2;
    yr = yw/2;
    zr = zw/2;
    if (ui->radioButton_rangevoxels->isChecked()) {
        xfac = 1;
        yfac = 1;
        zfac = 1;
    } else {
        xfac = 1/voxelsize[0];
        yfac = 1/voxelsize[1];
        zfac = 1/voxelsize[2];
    }
    range_x1 = (int)(xfac*(x0 - xr));
    range_x2 = (int)(xfac*(x0 + xr));
    range_y1 = (int)(yfac*(y0 - yr));
    range_y2 = (int)(yfac*(y0 + yr));
    range_z1 = (int)(zfac*(z0 - zr));
    range_z2 = (int)(zfac*(z0 + zr));
    range[0][0] = (int)(xfac*(x0 - xr));
    range[0][1] = (int)(xfac*(x0 + xr));
    range[1][0] = (int)(yfac*(y0 - yr));
    range[1][1] = (int)(yfac*(y0 + yr));
    range[2][0] = (int)(zfac*(z0 - zr));
    range[2][1] = (int)(zfac*(z0 + zr));
}

//---------------------------------------------------------
// Note: range[][] is 1-based!
//---------------------------------------------------------
void MainWindow::getAveragingRanges() {
    double x1, y1, z1, x2, y2, z2, xfac, yfac, zfac;
    QString x1str, y1str, z1str, x2str, y2str, z2str;

    if (ui->radioButton_rangevoxels->isChecked()) {
        xfac = 1;
        yfac = 1;
        zfac = 1;
    } else {
        xfac = 1/voxelsize[0];
        yfac = 1/voxelsize[1];
        zfac = 1/voxelsize[2];
    }
    fprintf(fpout,"Voxel factors: %f %f %f\n",xfac,yfac,zfac);
    if (!ui->checkBox_xfull->isChecked()) {
        x1str = ui->lineEdit_x1->text();
        x2str = ui->lineEdit_x2->text();
        x1 = x1str.toDouble();
        x2 = x2str.toDouble();
        range_x1 = (int)(xfac*x1);
        range_x2 = (int)(xfac*x2);
        range[0][0] = (int)(xfac*x1);
        range[0][1] = (int)(xfac*x2);
    } else {
        range_x1 = 1;
        range_x2 = nvoxels[0];
        range[0][0] = 1;
        range[0][1] = nvoxels[0];
    }
    if (!ui->checkBox_yfull->isChecked()) {
        y1str = ui->lineEdit_y1->text();
        y2str = ui->lineEdit_y2->text();
        y1 = y1str.toDouble();
        y2 = y2str.toDouble();
        range_y1 = (int)(yfac*y1);
        range_y2 = (int)(yfac*y2);
        range[1][0] = (int)(yfac*y1);
        range[1][1] = (int)(yfac*y2);
    } else {
        range_y1 = 1;
        range_y2 = nvoxels[1];
        range[1][0] = 1;
        range[1][1] = nvoxels[1];
    }
    if (!ui->checkBox_zfull->isChecked()) {
        z1str = ui->lineEdit_z1->text();
        z2str = ui->lineEdit_z2->text();
        z1 = z1str.toDouble();
        z2 = z2str.toDouble();
        range_z1 = (int)(zfac*z1);
        range_z2 = (int)(zfac*z2);
        range[2][0] = (int)(zfac*z1);
        range[2][1] = (int)(zfac*z2);
    } else {
        range_z1 = 1;
        range_z2 = nvoxels[2];
        range[2][0] = 1;
        range[2][1] = nvoxels[2];
    }
}

void MainWindow::getRanges()
{
    if (ui->radioButton_range->isChecked()) {
        fprintf(fpout,"getAveragingRanges\n");
        getAveragingRanges();
    } else {
        fprintf(fpout,"getCentredRanges\n");
        getCentredRanges();
    }
    checkRanges();
    fprintf(fpout,"Ranges:\n");
    for (int i=0; i<3; i++)
        fprintf(fpout,"axis: %d  %d %d\n",i,range[i][0],range[i][1]);
}

void MainWindow::checkRanges()
{
    int axis;

    for (axis=0; axis<3; axis++) {
        range[axis][0] = MAX(range[axis][0],1);
        range[axis][1] = MIN(range[axis][1],nvoxels[axis]);
    }
}

void MainWindow::computeVessels()
{
    int err;
    double slicearea, totlen, totvol, darea, areafraction;
    int axis, islice, nvessels, nvesselpixels, nslicepixels, density, MVD, w, h, nbranchpts;
    char *axisstr;
    QString areastr, countstr, densitystr, MVDstr, fractionstr;

    if (!is_ready) {
        return;
    }
    if (is_slice) {
        if (ui->radioButton_xaxis->isChecked()) {
            axis = 0;
            axisstr = "X";
            w = nvoxels[1];
            h = nvoxels[2];
            darea = voxelsize[1]*voxelsize[2];
        } else if (ui->radioButton_yaxis->isChecked()) {
            axis = 1;
            axisstr = "Y";
            w = nvoxels[2];
            h = nvoxels[0];
            darea = voxelsize[0]*voxelsize[2];
        } else if (ui->radioButton_zaxis->isChecked()) {
            axis = 2;
            axisstr = "Z";
            w = nvoxels[0];
            h = nvoxels[1];
            darea = voxelsize[0]*voxelsize[1];
        }
        islice = ui->lineEdit_intercept->text().toInt();
        fprintf(fpout,"\nComputing histology for a slice: axis: %s  islice: %d\n",axisstr,islice);
        if (ui->radioButton_slicemicrons->isChecked())
            islice = (int)(islice/voxelsize[axis]);
        err = checkSlice(axis,islice);
        if (err == 0) {
            if (imageViewer) delete imageViewer;
            imageViewer = new ImageViewer(w,h);
            err = SliceHistology(axis, islice, &nvessels, &nvesselpixels, &nslicepixels, &slicearea);
            imageViewer->paintLabel();
            imageViewer->show();
//            areafraction = (count*darea)/area;
            areafraction = (double)nvesselpixels/nslicepixels;
        } else {
            slicearea = 0;
            nvessels = 0;
            areafraction = 0;
        }
    } else {
        fprintf(fpout,"Computing average histology\n");
        getRanges();
        err = VolumeHistology(&nvessels,&slicearea);
        err = branching(&nbranchpts, &totlen, &totvol);
    }
    slicearea = 1.0e-6*slicearea;   // convert um2 -> mm2
    areastr = QString::number(slicearea,'f',3);
    ui->lineEdit_area->setText(areastr);
    countstr = QString::number(nvessels);
    ui->lineEdit_count->setText(countstr);
    if (slicearea > 0) {
        density = (int)(nvessels/slicearea + 0.5);
        MVD = (int)((nvessels*0.74/slicearea) + 0.5);
    } else {
        density = 0;
        MVD = 0;
    }
    densitystr = QString::number(density);
    ui->lineEdit_density->setText(densitystr);
    MVDstr = QString::number(MVD);
    ui->lineEdit_MVD->setText(MVDstr);
    fractionstr = QString::number(100*areafraction,'f',3);
    ui->lineEdit_fraction->setText(fractionstr);
    fprintf(fpout,"Slice pixels    : %8d\n",nslicepixels);
    fprintf(fpout,"Slice area (mm2): %f\n",(float)slicearea);
    fprintf(fpout,"Vessel count:     %6d\n",nvessels);
    fprintf(fpout,"Vessel pixels:    %6d\n",nvesselpixels);
    fprintf(fpout,"Count/area:       %6d\n",density);
    fprintf(fpout,"MVD:              %6d\n",MVD);
    fprintf(fpout,"Area percentage:  %8.3f\n",100*areafraction);
}

int MainWindow::checkSlice(int axis, int islice)
{
    if (islice < 1 || islice > nvoxels[axis]) {
        // need a message to the user
        return 1;
    } else {
        return 0;
    }
}

void MainWindow::doSetup()
{
    int err;
    int um[3];
    QString numstr, resultstr;
    char input_amfile[1024], close_file[1024], result_file[1024];

    this->setCursor( QCursor( Qt::WaitCursor ) );
    ui->labelResult->setText("Doing setup()");
    strcpy(input_amfile, amFileName.toAscii().constData());
    strcpy(close_file, closeFileName.toAscii().constData());
    strcpy(result_file, resultFileName.toAscii().constData());
    if (fpout) fclose(fpout);
    fpout = fopen(result_file,"w");
    if (!isSetup()) {
        err = setup(input_amfile,close_file,result_file,voxelsize);
        resultstr = QString::number(err);
//        ui->labelResult->setText(resultstr);
        if (err != 0) {
            this->setCursor( QCursor( Qt::ArrowCursor ) );
            return;
        }
    }
    getCloseSize(nvoxels);
    numstr = QString::number(nvoxels[0]);
    ui->lineEdit_nx->setText(numstr);
    numstr = QString::number(nvoxels[1]);
    ui->lineEdit_ny->setText(numstr);
    numstr = QString::number(nvoxels[2]);
    ui->lineEdit_nz->setText(numstr);
    um[0] = (int)(nvoxels[0]*voxelsize[0] + 0.5);
    numstr = QString::number(um[0]);
    ui->lineEdit_umx->setText(numstr);
    um[1] = (int)(nvoxels[1]*voxelsize[1] + 0.5);
    numstr = QString::number(um[1]);
    ui->lineEdit_umy->setText(numstr);
    um[2] = (int)(nvoxels[2]*voxelsize[2] + 0.5);
    numstr = QString::number(um[2]);
    ui->lineEdit_umz->setText(numstr);
    fprintf(fpout,"\nVoxel size: %6.2f x %6.2f x %6.2f\n",voxelsize[0],voxelsize[1], voxelsize[2]);
    fprintf(fpout,"Image size (voxels): %6d x %6d x %6d\n",nvoxels[0],nvoxels[1], nvoxels[2]);
    fprintf(fpout,"Image size (um): %6d x %6d x %6d\n",um[0],um[1],um[2]);
    computeVolume();
    this->setCursor( QCursor( Qt::ArrowCursor ) );
    //	res = system(cmdstr);
//	if (res == 0)
//		resultstr = "SUCCESS";
//	else if (res == 1)
//		resultstr = "FAILED: wrong number of arguments";
//	else if (res == 2)
//		resultstr = "FAILED: Read error on input file";
//	else if (res == 3)
//		resultstr = "FAILED: Write error on output file";
//	else if (res == 4)
//		resultstr = "FAILED: out of memory";
//	ui->labelResult->setText(resultstr);
}

