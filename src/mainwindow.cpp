#include <QtGui>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QShortcut>
#include <QScrollBar>
#include <QHeaderView>
#include <QPixmap>

#include <iostream>

#include <cstdlib>
#include <sys/stat.h>
#include <algorithm>
#include <functional>

#if _WIN32 || _WIN64
#include <direct.h>
#endif
#if __GNUC__
#include <unistd.h>
#endif

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "commands.h"
#include "dialogsettestitem.h"
#include "dialognewseqpreset.h"
#include "dialogshowgraph.h"
#include "dialogshowdevattrib.h"
#include "commonfuncs.h"
#include <smartcard.h>
#include "logger.h"

using namespace std;
using namespace conf;
using namespace smcard;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _pConfig(nullptr),
    _deviceInfoShow(true),
    _test_running(false),
    indexSeqPresUpdateInfo(-1),
    indexSeqCMDUpdateInfo(-1),
    _needReselect(false),
    _curr_win_width(0),
    _curr_win_height(0),
    _customPlot(nullptr),
    _failCheckInfo(false),
    _exclusiveConnect(false)
{
    ui->setupUi(this);

    popUp = new PopUp();

    this->setMinimumSize(this->size());
    this->showMaximized();

    int ret = -1;

    std::string log_path = QDir::currentPath().toStdString();// QDir::homePath().toStdString();

#if _WIN32 || _WIN64
    log_path += "/log_testex";

    ret = _mkdir(log_path.c_str());

    if (!ret || errno == EEXIST)
    {
        log_path += "/testex";
        DEBUG_CONF( log_path.c_str(),
                    Logger::file_on|
                    Logger::screen_on,
                    DBG_DEBUG,
                    DBG_DEBUG);

        QString logPath = Logger::getInstance().getLogFilename();

        // log file name: full_log_path/log_2020_00_00-00-00.log
        int log_name_index = logPath.lastIndexOf("/");

        // remove full_log_path. Use only log name: log_2020_00_00-00-00.log
        logPath.remove(0,log_name_index + 1);

        // create link
        ui->labelFileLog->setText(QString("<a href=") +
                                  Logger::getInstance().getLogFilename() +
                                  QString(">") +
                                  QString("Show log") +
                                  //                                  logPath +
                                  QString("</a>"));

        DEBUG(DBG_DEBUG, "Program started");
    }
#else
    log_path += "/log_testex";

    ret = mkdir(log_path.c_str(), S_IRWXU | S_IRWXG | S_IROTH );

    if (!ret || errno == EEXIST)
    {
        log_path += "/testex";
        DEBUG_CONF( log_path.c_str(),
                    Logger::file_on|
                    Logger::screen_on,
                    DBG_DEBUG,
                    DBG_DEBUG);

        ui->labelFileLog->setText(Logger::getInstance().getLogFilename());
    }
#endif

    // plot
    _customPlot = ui->widgetPlot;
    _customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

    _customPlot->xAxis->setAutoTicks(false);

    _customPlot->xAxis->setRange(0, 1);
    _customPlot->yAxis->setRange(0, 1);

    //_customPlot->hide();
    //ui->pushButtonShowGraph->hide();

    // Config Menu
    loadPresetConf = new QAction(tr("&Load config (Ctrl + O)"), this);
    editPresetConf = new QAction(tr("&Edit current command (Ctrl + E)"), this);
    addPresetConf = new QAction(tr("&Add (Ctrl + N)"), this);
    delItemConf = new QAction(tr("&Delete current command (Ctrl + D)"), this);
    delPresetConf = new QAction(tr("&Del current preset"), this);
    reloadPresetConf = new QAction(tr("&Reload config (Ctrl + R)"), this);

    configMenu = menuBar()->addMenu(tr("&Config"));
    configMenu->addAction(loadPresetConf);
    configMenu->addAction(editPresetConf);
    configMenu->addAction(addPresetConf);
    configMenu->addAction(delItemConf);
    configMenu->addAction(delPresetConf);
    configMenu->addAction(reloadPresetConf);

    // Seq Menu
    startSeqTestConf = new QAction(tr("&Start test (Space)"), this);
    addSeqTestPreset = new QAction(tr("&New sequence"), this);
    delSeqTestPreset = new QAction(tr("&Del current sequence"), this);
    addCmd2SeqTestConf = new QAction(tr("&Add command (+)"), this);
    saveSeqTestConf = new QAction(tr("&Save (Ctrl + P)"), this);
    clearSeqTestConf = new QAction(tr("&Clear sequence"), this);

    seqTestMenu = menuBar()->addMenu(tr("&Test"));
    seqTestMenu->addAction(startSeqTestConf);
    seqTestMenu->addAction(addCmd2SeqTestConf);
    seqTestMenu->addAction(addSeqTestPreset);
    seqTestMenu->addAction(saveSeqTestConf);
    seqTestMenu->addAction(delSeqTestPreset);
    seqTestMenu->addAction(clearSeqTestConf);

    partitionTest = new QAction(tr("&Partition"), this);
    showDeviceAttributes = new QAction(tr("&Reader attributes (Ctrl+I)"), this);

    // About Menu
    aboutAct = new QAction(tr("&About"), this);
    aboutQtAct = new QAction(tr("About &Qt"), this);

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);

    // Sets
    ui->lineEditData->setEnabled(false);
    ui->textEdit->setReadOnly(true);
    ui->lineEditProto->setReadOnly(true);
    ui->progressBarTest->hide();
    ui->checkBoxExclusive->setChecked(false);

    // table commands
    ui->tableWidget->setColumnCount(3);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setShowGrid(true);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget->setHorizontalHeaderLabels(QStringList()
                                               << "Cmd name" << "Result" << "APDU");

    ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);

//    ui->groupBoxDevice->setStyleSheet("QGroupBox  { border: 2px solid gray; border-radius: 3px; }");

    // Key Shortcut
    ui->pushButtonSendAPDU->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Enter));

    QKeySequence ksO(Qt::CTRL + Qt::Key_O);
    QShortcut* shortcutO = new QShortcut(ksO, this);
    QObject::connect(shortcutO, SIGNAL(activated()),
                     this, SLOT(LoadCfg_clicked()));

    QKeySequence ksN(Qt::CTRL + Qt::Key_N);
    QShortcut* shortcutN = new QShortcut(ksN, this);
    QObject::connect(shortcutN, SIGNAL(activated()),
                     this, SLOT(AddItem_clicked()));

    QKeySequence ksE(Qt::CTRL + Qt::Key_E);
    QShortcut* shortcutE = new QShortcut(ksE, this);
    QObject::connect(shortcutE, SIGNAL(activated()),
                     this, SLOT(EditCfg_clicked()));

    QKeySequence ksR(Qt::CTRL + Qt::Key_R);
    QShortcut* shortcutR = new QShortcut(ksR, this);
    QObject::connect(shortcutR, SIGNAL(activated()),
                     this, SLOT(reloadConfiguration_triggered()));

    QKeySequence ksD(Qt::CTRL + Qt::Key_D);
    QShortcut* shortcutD = new QShortcut(ksD, this);
    QObject::connect(shortcutD, SIGNAL(activated()),
                     this, SLOT(DelItem_clicked()));

    QKeySequence ksSpace(/*Qt::CTRL + */Qt::Key_Space);
    QShortcut* shortcutSpace = new QShortcut(ksSpace, this);
    QObject::connect(shortcutSpace, SIGNAL(activated()),
                     this, SLOT(on_pushButtonStartTest_clicked()));

    QKeySequence ksCSpace(Qt::CTRL + Qt::Key_Space);
    QShortcut* shortcutCSpace = new QShortcut(ksCSpace, this);
    QObject::connect(shortcutCSpace, SIGNAL(activated()),
                     this, SLOT(StartTest_Selected_activated()));

    QKeySequence ksP(Qt::CTRL + Qt::Key_S);
    QShortcut* shortcutP = new QShortcut(ksP, this);
    QObject::connect(shortcutP, SIGNAL(activated()),
                     this, SLOT(saveSeqConfiguration_triggered()));

    QKeySequence ksPlus(/*Qt::CTRL + */Qt::Key_Plus);
    QShortcut* shortcutPlus = new QShortcut(ksPlus, this);
    QObject::connect(shortcutPlus, SIGNAL(activated()),
                     this, SLOT(on_pushButtonAddTotable_clicked()));

    QKeySequence ksDel(/*Qt::CTRL + */Qt::Key_Delete);
    QShortcut* shortcutDel = new QShortcut(ksDel, this);
    QObject::connect(shortcutDel, SIGNAL(activated()),
                     this, SLOT(deleteSelectedRows_pressed()));

    QKeySequence ksC(Qt::CTRL + Qt::Key_C);
    QShortcut* shortcutC = new QShortcut(ksC, this);
    QObject::connect(shortcutC, SIGNAL(activated()),
                     this, SLOT(copySelectedRows_pressed()));

    QKeySequence ksV(Qt::CTRL + Qt::Key_V);
    QShortcut* shortcutV = new QShortcut(ksV, this);
    QObject::connect(shortcutV, SIGNAL(activated()),
                     this, SLOT(pasteSelectedRows_pressed()));

    QKeySequence ksI(Qt::CTRL + Qt::Key_I);
    QShortcut* shortcutI = new QShortcut(ksI, this);
    QObject::connect(shortcutI, SIGNAL(activated()),
                     this, SLOT(showDeviceAttrib_triggered()));

    // Connect Slots
    // Config Menu
    connect(addPresetConf, SIGNAL(triggered()),
            this, SLOT(AddItem_clicked()));
    connect(loadPresetConf,  SIGNAL(triggered()),
            this, SLOT(LoadCfg_clicked()));
    connect(editPresetConf,  SIGNAL(triggered()),
            this, SLOT(EditCfg_clicked()));
    connect(delItemConf,  SIGNAL(triggered()),
            this, SLOT(DelItem_clicked()));
    connect(delPresetConf,  SIGNAL(triggered()),
            this, SLOT(DelPreset_clicked()));
    connect(reloadPresetConf,  SIGNAL(triggered()),
            this, SLOT(reloadConfiguration_triggered()));

    // Tools Menu
    connect(showDeviceAttributes,  SIGNAL(triggered()),
            this, SLOT(showDeviceAttrib_triggered()));

    // Seq Menu
    connect(startSeqTestConf,  SIGNAL(triggered()),
            this, SLOT(on_pushButtonStartTest_clicked()));
    connect(addSeqTestPreset,  SIGNAL(triggered()),
            this, SLOT(AddSeqTestItem_clicked()));
    connect(delSeqTestPreset,  SIGNAL(triggered()),
            this, SLOT(DelSeqPreset_clicked()));
    connect(addCmd2SeqTestConf,  SIGNAL(triggered()),
            this, SLOT(on_pushButtonAddTotable_clicked()));
    connect(saveSeqTestConf,  SIGNAL(triggered()),
            this, SLOT(saveSeqConfiguration_triggered()));
    connect(clearSeqTestConf,  SIGNAL(triggered()),
            this, SLOT(clearSeqConfiguration_triggered()));

    // Table Menu
    connect(ui->tableWidget, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(customMenuRequested(QPoint)));
    connect(ui->tableWidget, SIGNAL(cellDoubleClicked(int,int)),
            this, SLOT(tableItemClicked(int,int)));

    // About Menu
    connect(aboutAct,  SIGNAL(triggered()), this, SLOT(about()));
    connect(aboutQtAct,  SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(this, SIGNAL(findDevices_signal()),
            this, SLOT(on_pushButtonFindDevs_clicked()));

    ui->textEdit->installEventFilter(this);

    emit findDevices_signal();

    memset(&ansBuffer,0,sizeof(answerBuffer_t));

    this->showMaximized();
}

MainWindow::~MainWindow()
{
    _test_running = false;

    if (this->_pConfig)
    {
        if (QFile::exists(QString(_pConfig->getConfFilePath()) + QString(".bck")))
        {
            QFile::remove(QString(_pConfig->getConfFilePath()) + QString(".bck"));
        }

        QFile::copy(_pConfig->getConfFilePath(),
                    QString(_pConfig->getConfFilePath()) + QString(".bck"));
    }

    this->_SCard.disconnect();

    if (_pConfig)
    {
        delete _pConfig;
    }

    delete _customPlot;

    delete popUp;

    DEBUG(DBG_DEBUG, "Program stopped");

    delete ui;
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About Menu"),
                       tr("The <b>Menu</b> About "
                          "About menu"));
}

void MainWindow::on_pushButtonFindDevs_clicked()
{
    if (!this->findDevices())
    {
        DEBUG(DBG_WARN, "No Devices" );
    }
}

void MainWindow::LoadCfg_clicked()
{
    if (_test_running)
    {
        return;
    }

    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Load Config"),
                                                    QDir::currentPath(),//homePath(),
                                                    tr("Config Files (*.xml)"));

    if (!_pConfig)
    {
        _pConfig = new Configuration();
    }

    if (fileName.isEmpty())
    {
        return;
    }
    else
    {
        QFile::copy(fileName, QString(fileName + ".bck"));
        _pConfig->setConfFilePath(fileName.toStdString().c_str());

        if (QFile::exists(QString(_pConfig->getConfFilePath()) + QString(".bck")))
        {
            QFile::remove(QString(_pConfig->getConfFilePath()) + QString(".bck"));
        }

        QFile::copy(_pConfig->getConfFilePath(),
                    QString(_pConfig->getConfFilePath()) + QString(".bck"));
    }

    updateLocalConfig();
}

void MainWindow::EditCfg_clicked()
{
    if (_test_running)
    {
        return;
    }

    DialogAddItem d;

    if(!_pConfig )
    {
        return;
    }

    if(!_pConfig->isConfigured())
    {
        return;
    }

    if (ui->comboBoxPreset->currentIndex() < 0 ||
            ui->comboBoxAPDUSelect->currentIndex() < 0)
    {
        return;
    }

    if (!d.setCurrentPreset(_pConfig,
                            &_common_conf,
                            ui->comboBoxPreset->currentIndex(),
                            ui->comboBoxAPDUSelect->currentIndex(),
                            true))
    {
        return;
    }

    d.show();
    d.exec();

    this->savePresetsToConf();
}

void MainWindow::AddItem_clicked()
{
    if (_test_running)
    {
        return;
    }

    DialogAddItem d;

    if(!_pConfig )
    {
        return;
    }

    if(!_pConfig->isConfigured())
    {
        return;
    }

    conf_item_t item;

    if (ui->comboBoxPreset->currentIndex() < 0)
    {
        return;
    }

    if (!d.setCurrentPreset(_pConfig,
                            &_common_conf,
                            ui->comboBoxPreset->currentIndex(),
                            -1,
                            false))
    {
        return;
    }

    d.show();
    d.exec();

    _common_conf.presetCommandsV[ui->comboBoxPreset->currentIndex()].commandConf.push_back(item);

    this->savePresetsToConf();
}

bool MainWindow::updateLocalConfig()
{
    if (!_pConfig)
    {
        QMessageBox messageBox;
        messageBox.critical(this,"Error","Config file missing");
        messageBox.setFixedSize(500,200);
        return false;
    }

    if(!_pConfig->fillPresetsConf(this->_common_conf))
    {
        QMessageBox messageBox;
        messageBox.critical(this,"Error","Read conffile failded");
        messageBox.setFixedSize(500,200);

        return false;
    }

    int preset_index_1 = ui->comboBoxPreset->currentIndex();

    int seq_preset_index = ui->comboBoxSeqPreset->currentIndex();

    ui->comboBoxPreset->clear();

    for (int i = 0;i < _common_conf.presetCommandsV.size();i++)
    {
        ui->comboBoxPreset->addItem(_common_conf.presetCommandsV[i].preset_name.c_str());
    }

    ui->comboBoxSeqPreset->clear();

    for (auto i = 0;i < _common_conf.presetSequenceV.size();i++)
    {
        ui->comboBoxSeqPreset->addItem(QString::number(i + 1) +
                                       QString(") ") +
                                       QString(_common_conf.presetSequenceV[i].seqPresetName.c_str()));
    }

    if (preset_index_1 >= 0 &&
            preset_index_1 < _common_conf.presetSequenceV.size())
    {
        ui->comboBoxPreset->setCurrentIndex(preset_index_1);
    }

    if (seq_preset_index >= 0 &&
            seq_preset_index < _common_conf.presetSequenceV.size())
    {
        ui->comboBoxSeqPreset->setCurrentIndex(seq_preset_index);
    }

    if (ui->tableWidget->rowCount() > 0)
    {
        QString text = "Total: ";
        text += QString::number(ui->tableWidget->rowCount());
        ui->label_33->setText(text);
    }

    ui->label_34->setText("---");

    return true;
}

bool MainWindow::findDevices()
{
    ui->comboBoxDevice->clear();

    uint8_t			ReadersCnt = 0;

#if __GNUC__

    LPTSTR			Readers[MAX_READERS_AVAILABLE];

    SCARDCONTEXT	tmpCardCtx;
    char 			ReadersList[READER_LIST_SIZE];

    DWORD			size = READER_LIST_SIZE;

    LONG res = SCardEstablishContext(SCARD_SCOPE_USER, nullptr, nullptr, &tmpCardCtx);

    if (SCARD_S_SUCCESS != res)
    {
        qDebug() << __FUNCTION__ <<
                    "Failed to establish context: ";

        /*<<
                    pcsc_stringify_error(res);*/
        return false;
    }

    res = SCardListReaders(tmpCardCtx, NULL, (LPTSTR)ReadersList, &size);

    if (tmpCardCtx)
    {
        SCardReleaseContext(tmpCardCtx);
    }

    if (SCARD_S_SUCCESS != res)
    {
        qDebug() << "WARNING: Reader not available";
        ui->textEdit->append("No device");
    }
    else
    {
        char *pReader = ReadersList;

        while ('\0' != *pReader)
        {
            Readers[ReadersCnt++] = pReader;
            pReader = pReader + strlen((const char *)pReader) + 1;

            if (ReadersCnt >= MAX_READERS_AVAILABLE)
            {
                qDebug() << __FUNCTION__ <<
                            "ERROR: Too much Reader, MAX = " <<
                            MAX_READERS_AVAILABLE <<
                            ", available: " <<
                            ReadersCnt;
                return false;
            }
        }
    }

    for (uint8_t i = 0; i < ReadersCnt; i++)
    {
        ui->comboBoxDevice->addItem((char *)Readers[i]);
    }

    if (ReadersCnt > 1)
    {
        ui->comboBoxDevice->setCurrentIndex(0);
    }
#else

    SCARDCONTEXT    hSC;
    LONG            lReturn;
    char *			Readers[MAX_READERS_AVAILABLE];

    lReturn = SCardEstablishContext(SCARD_SCOPE_USER,
                                    NULL,
                                    NULL,
                                    &hSC);

    if (lReturn != SCARD_S_SUCCESS)
    {
        DEBUG(DBG_ERROR, "Failed Establish Context");
        return false;
    }

    LPTSTR          pmszReaders = NULL;
    LPTSTR          pReader;
    DWORD           cch = SCARD_AUTOALLOCATE;

    // Retrieve the list the readers.
    // hSC was set by a previous call to SCardEstablishContext.
    lReturn = SCardListReaders(hSC,
                               NULL,
                               (LPTSTR)&pmszReaders,
                               &cch );

    switch( lReturn )
    {
        case SCARD_E_NO_READERS_AVAILABLE:
            // "Reader is not in groups";
            return false;
        case SCARD_S_SUCCESS:

            pReader = pmszReaders;

            while ( '\0' != *pReader )
            {
                Readers[ReadersCnt++] = encode(pReader, CP_UTF8);
                pReader = pReader + wcslen((wchar_t *)pReader) + 1;
            }

            SCardFreeMemory( hSC, pmszReaders );

            break;

        default:
            return false;
    }

    if (hSC)
        SCardReleaseContext(hSC);

#endif // __GNUC__

    //    int JaCartaReaders = 0;

    for (uint8_t i = 0; i < ReadersCnt; i++)
    {
        ui->comboBoxDevice->addItem((char *)Readers[i]);
    }

    if (ReadersCnt > 1)
    {
        ui->comboBoxDevice->setCurrentIndex(0);
    }

    return true;
}

void MainWindow::on_comboBoxPreset_currentIndexChanged(int index)
{
    if(index < 0)
    {
        return;
    }

    ui->lineEditCLA->clear();
    ui->lineEditINS->clear();
    ui->lineEditP1->clear();
    ui->lineEditP2->clear();
    ui->lineEditLEN->clear();
    ui->lineEditData->clear();

    ui->comboBoxAPDUSelect->clear();

    int num = 0;
    for (std::vector<conf_item_t>::iterator it = _common_conf.presetCommandsV[index].commandConf.begin();
         it != _common_conf.presetCommandsV[index].commandConf.end() ; it++)
    {
        QString desc = QString::number(num);
        desc += " ";
        desc += it->commandDesc.c_str();
        desc += " [";
        desc += QString("%1").arg(it->cla, 2, 16, QChar('0')).toUpper();
        desc += " ";
        desc += QString("%1").arg(it->ins, 2, 16, QChar('0')).toUpper();
        desc += " ";
        desc += QString("%1").arg(it->p1, 2, 16, QChar('0')).toUpper();
        desc += " ";
        desc += QString("%1").arg(it->p2, 2, 16, QChar('0')).toUpper();
        desc += " ";
        desc += QString("%1").arg(it->len, 2, 16, QChar('0')).toUpper();
        desc += "]";

        ui->comboBoxAPDUSelect->addItem(desc);
        num++;
    }

    if (_common_conf.presetCommandsV[index].commandConf.empty())
    {
        ui->lineEditCLA->setEnabled(false);
        ui->lineEditINS->setEnabled(false);
        ui->lineEditP1->setEnabled(false);
        ui->lineEditP2->setEnabled(false);
        ui->lineEditLEN->setEnabled(false);
        ui->lineEditCLA->clear();
        ui->lineEditINS->clear();
        ui->lineEditP1->clear();
        ui->lineEditP2->clear();
        ui->lineEditLEN->clear();
        ui->lineEditData->clear();
    }
    else
    {
        ui->lineEditCLA->setEnabled(true);
        ui->lineEditINS->setEnabled(true);
        ui->lineEditP1->setEnabled(true);
        ui->lineEditP2->setEnabled(true);
        ui->lineEditLEN->setEnabled(true);
    }
}

void MainWindow::on_comboBoxAPDUSelect_currentIndexChanged(int index)
{
    if (index < 0)
    {
        ui->comboBoxAPDUSelect->clear();
        return;
    }

    ui->lineEditCLA->clear();
    ui->lineEditINS->clear();
    ui->lineEditP1->clear();
    ui->lineEditP2->clear();
    ui->lineEditLEN->clear();
    ui->lineEditData->clear();

    int preset_index = ui->comboBoxPreset->currentIndex();

    if (preset_index < 0)
    {
        return;
    }

    QString text = "";

    std::string result = this->n2hexstr(_common_conf.presetCommandsV[preset_index].commandConf[index].cla,
                                        sizeof(_common_conf.presetCommandsV[preset_index].commandConf[index].cla));

    text += _common_conf.presetCommandsV[preset_index].commandConf[index].commandDesc.c_str();

    text += ": ";

    result.erase(result.begin(),result.end() - 2);
    result += " ";
    text += result.c_str();

    result = this->n2hexstr(_common_conf.presetCommandsV[preset_index].commandConf[index].ins,
                            sizeof(_common_conf.presetCommandsV[preset_index].commandConf[index].ins));
    result.erase(result.begin(),result.end() - 2);
    result += " ";
    text += result.c_str();

    result = this->n2hexstr(_common_conf.presetCommandsV[preset_index].commandConf[index].p1,
                            sizeof(_common_conf.presetCommandsV[preset_index].commandConf[index].p1));
    result.erase(result.begin(),result.end() - 2);
    result += " ";
    text += result.c_str();

    result = this->n2hexstr(_common_conf.presetCommandsV[preset_index].commandConf[index].p2,
                            sizeof(_common_conf.presetCommandsV[preset_index].commandConf[index].p2));
    result.erase(result.begin(),result.end() - 2);
    result += " ";
    text += result.c_str();

    result = this->n2hexstr(_common_conf.presetCommandsV[preset_index].commandConf[index].len,
                            sizeof(_common_conf.presetCommandsV[preset_index].commandConf[index].len));
    result.erase(result.begin(),result.end() - 2);
    result += " ";
    text += result.c_str();

    ui->lineEditCLA->setText(QString("%1").arg(_common_conf.presetCommandsV[preset_index].commandConf[index].cla, 2, 16, QChar('0')).toUpper());
    ui->lineEditINS->setText(QString("%1").arg(_common_conf.presetCommandsV[preset_index].commandConf[index].ins, 2, 16, QChar('0')).toUpper());
    ui->lineEditP1->setText(QString("%1").arg(_common_conf.presetCommandsV[preset_index].commandConf[index].p1, 2, 16, QChar('0')).toUpper());
    ui->lineEditP2->setText(QString("%1").arg(_common_conf.presetCommandsV[preset_index].commandConf[index].p2, 2, 16, QChar('0')).toUpper());
    ui->lineEditLEN->setText(QString("%1").arg(_common_conf.presetCommandsV[preset_index].commandConf[index].len, 2, 16, QChar('0')).toUpper());
    ui->lineEditData->setText(_common_conf.presetCommandsV[preset_index].commandConf[index].data_field.c_str());

    if (_common_conf.presetCommandsV[preset_index].commandConf[index].len > 0)
    {
        ui->lineEditData->setEnabled(true);
    }
    else
    {
        ui->lineEditData->setEnabled(false);
    }

    qApp->processEvents();
}

void MainWindow::DelPreset_clicked()
{
    std::string msg_text;

    msg_text = "Remove preset Config: ";
    msg_text += ui->comboBoxPreset->currentText().toStdString();
    msg_text += "?";

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this,
                                  "Warning",
                                  msg_text.c_str(),
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        this->_pConfig->removePreset(this->_common_conf, ui->comboBoxPreset->currentIndex());
        this->savePresetsToConf();
    }
}

void MainWindow::DelSeqPreset_clicked()
{
    if (_test_running)
    {
        return;
    }

    std::string msg_text;

    msg_text = "Remove Seq preset: ";
    msg_text += ui->comboBoxSeqPreset->currentText().toStdString();
    msg_text += "?";

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this,
                                  "Warning",
                                  msg_text.c_str(),
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        this->_pConfig->removeSequence(this->_common_conf,
                                       ui->comboBoxSeqPreset->currentIndex());
        while (ui->tableWidget->rowCount() > 0)
        {
            ui->tableWidget->removeRow(0);
        }

        this->savePresetsToConf();
    }
}

void MainWindow::DelItem_clicked()
{
    if (_test_running)
    {
        return;
    }

    std::string msg_text;

    msg_text = "Remove APDU: ";
    msg_text += ui->comboBoxAPDUSelect->currentText().toStdString();
    msg_text += "?";

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Warning", msg_text.c_str(),
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        if (ui->comboBoxPreset->currentIndex() < 0 ||
                ui->comboBoxAPDUSelect->currentIndex() < 0)
        {
            return;
        }

        this->_pConfig->removePresetItem(this->_common_conf,
                                         ui->comboBoxPreset->currentIndex(),
                                         ui->comboBoxAPDUSelect->currentIndex());
        this->savePresetsToConf();
    }
}

void MainWindow::on_comboBoxDevice_currentIndexChanged(int index)
{
    if (index < 0)
    {
        return;
    }
}

void MainWindow::showDeviceAttrib_triggered()
{

    DialogShowDevAttrib dialDevAttr(0,
                                    &this->_SCard);


    dialDevAttr.show();
    dialDevAttr.exec();
}

void MainWindow::on_pushButtonConnect_clicked()
{
    if (ui->comboBoxDevice->currentIndex() < 0)
    {
        return;
    }

    std::string str;

    if (!this->_SCard.isActive() && ui->pushButtonConnect->text().compare("Disconnect"))
    {
        this->_SCard.setDeviceName(ui->comboBoxDevice->currentText().toStdString());

        if (this->_SCard.connect(ui->checkBoxExclusive->isChecked()))
        {
            ui->pushButtonFindDevs->setEnabled(false);
            ui->pushButtonConnect->setText(tr("Disconnect"));
            ui->comboBoxDevice->setEnabled(false);

            str = "Reader (";
            str += std::to_string(INDEX_DEVICE_1);
            str += "): \"";
            str += this->_SCard.getDeviceName();
            str += "\" Connected";

            ui->textEdit->append(str.c_str());

            DEBUG(DBG_DEBUG, str);

            ui->lineEditProto->setText(QString("T=" + QString::number(this->_SCard.getProto())));
        }
        else
        {
            str = "Reader 1:";
            str += this->_SCard.getSCardErrorDescription(this->_SCard.getLastSCardErrror());

            ui->textEdit->append(str.c_str());
            DEBUG(DBG_DEBUG, str);
        }
    }
    else
    {
        if (this->_SCard.disconnect())
        {
            std::string str = "Reader: ";
            str += "\"";
            str += this->_SCard.getDeviceName();
            str += "\" disconnected (Device 1)";

            ui->textEdit->append(str.c_str());
            DEBUG(DBG_DEBUG, str);

            ui->pushButtonFindDevs->setEnabled(true);
            ui->pushButtonConnect->setText(tr("Connect"));
            ui->comboBoxDevice->setEnabled(true);

            ui->lineEditProto->clear();
        }

        ui->pushButtonFindDevs->setEnabled(true);

        ui->pushButtonConnect->setText(tr("Connect"));
        ui->comboBoxDevice->setEnabled(true);
    }

    this->showCurExecCommand();
}

void MainWindow::on_pushButtonSendAPDU_clicked()
{
    if (!this->_SCard.isActive())
    {
        ui->textEdit->append("Reader 1 Not Connected");
        return;
    }

    if (!_pConfig ||
            !_pConfig->isConfigured() ||
            ui->comboBoxPreset->currentIndex() < 0 ||
            ui->comboBoxAPDUSelect->currentIndex() < 0)
    {
        if (ui->lineEditCLA->text().isEmpty() ||
                ui->lineEditINS->text().isEmpty() ||
                ui->lineEditP1->text().isEmpty() ||
                ui->lineEditP2->text().isEmpty())
        {
            return;
        }
    }

    uint8_t apduBuf[APDU_BUFFER_SIZE] = {0};
    uint32_t apduLength = 0;
    uint8_t rpl[APDU_BUFFER_SIZE];
    uint32_t rLength;

    long rVal = 0;

    ui->lineEditData->setStyleSheet("QLineEdit { background: white;}");

    int base = 16;
    bool ok = false;

    if (ui->lineEditLEN->text().isEmpty())
    {
        ui->lineEditLEN->setText("0");
    }

    apduBuf[CLA_INDEX] = (0xff & ui->lineEditCLA->text().toInt(&ok,base));
    apduBuf[INS_INDEX] = (0xff & ui->lineEditINS->text().toInt(&ok,base));
    apduBuf[P1_INDEX] = (0xff & ui->lineEditP1->text().toInt(&ok,base));
    apduBuf[P2_INDEX] = (0xff & ui->lineEditP2->text().toInt(&ok,base));
    apduBuf[LEN_INDEX] = (0xff & ui->lineEditLEN->text().toInt(&ok,base));

    std::string data = ui->lineEditData->text().toStdString();

    if (apduBuf[LEN_INDEX] != 0x00 && !ui->lineEditData->text().isEmpty())
    {
        for (unsigned int j = 0,index = 5;j < data.size() - 1;j+=2,index++)
        {
            apduBuf[index] = (0xff & ((char2bin(data[j]) << 4) | char2bin(data[j+1])));
        }

        apduLength = APDU_HEAD_LEN + apduBuf[LEN_INDEX];
    }
    else
    {
        apduLength = APDU_HEAD_LEN;
    }

    DEBUG(DBG_DEBUG, "---------------------------------------------");
    ui->textEdit->append("---------------------------------------------");

    QString text;
    text += " ";
    text += ui->comboBoxAPDUSelect->currentText();
    text += " [";
    text += data.c_str();
    text += "]";

    ui->textEdit->append(QString("Single command: " + text));

    DEBUG(DBG_DEBUG, "Single command: " + text.toStdString());

    rVal = this->scardExchange((const char*)apduBuf, apduLength, (char*)rpl, (int*)&rLength);

    if (rVal)
    {
        ui->textEdit->append("APDU Fail");
        DEBUG(DBG_DEBUG,"APDU Fail");
    }

    DEBUG(DBG_DEBUG, "---------------------------------------------");
    ui->textEdit->append("---------------------------------------------");
}

void MainWindow::on_pushButtonClearLog_clicked()
{
    ui->textEdit->clear();
}

bool MainWindow::addItemToTableByIndex(int index)
{
    int indexPres = ui->comboBoxPreset->currentIndex();

    if ( indexPres < 0)
    {
        return false;
    }

    int row = ui->tableWidget->rowCount();

    ui->tableWidget->insertRow(row);

    QString command;
    command += QString("%1").arg(_common_conf.presetCommandsV[indexPres].commandConf[index].cla, 2, 16, QChar('0')).toUpper();
    command += ":";
    command += QString("%1").arg(_common_conf.presetCommandsV[indexPres].commandConf[index].ins, 2, 16, QChar('0')).toUpper();
    command += ":";
    command += QString("%1").arg(_common_conf.presetCommandsV[indexPres].commandConf[index].p1, 2, 16, QChar('0')).toUpper();
    command += ":";
    command += QString("%1").arg(_common_conf.presetCommandsV[indexPres].commandConf[index].p2, 2, 16, QChar('0')).toUpper();
    command += " | ";
    command += QString("%1").arg(_common_conf.presetCommandsV[indexPres].commandConf[index].len, 2, 16, QChar('0')).toUpper();
    command += " | ";

    if (_common_conf.presetCommandsV[indexPres].commandConf[index].len > 0)
    {
        for (unsigned i = 0;i < _common_conf.presetCommandsV[indexPres].commandConf[index].data_field.size()-1;i+=2)
        {
            command += _common_conf.presetCommandsV[indexPres].commandConf[index].data_field.substr(i,2).c_str();

            if (i != _common_conf.presetCommandsV[indexPres].commandConf[index].data_field.size()-2)
            {
                command += ":";
            }
        }
    }


    ui->tableWidget->setItem(row, TABLE_COL_CMD_NAME, new QTableWidgetItem(_common_conf.presetCommandsV[indexPres].commandConf[index].commandDesc.c_str()));
    ui->tableWidget->setItem(row, TABLE_COL_APDU, new QTableWidgetItem(command));

    this->showCurExecCommand();

    return true;
}

void MainWindow::confSendUpdateInfo(int index_seq_pres, int index_seq_command)
{
    indexSeqPresUpdateInfo = index_seq_pres;
    indexSeqCMDUpdateInfo = index_seq_command;

    emit updateInfo();
}

void MainWindow::showCurExecCommand()
{
    if (!_pConfig)
    {
        return;
    }

    if (!_pConfig->isConfigured())
    {
        return;
    }

    if (ui->comboBoxSeqPreset->currentIndex() < 0)
    {
        return;
    }

    if (this->_SCard.isActive())
    {
        for (unsigned i = 0;i < _common_conf.presetSequenceV[ui->comboBoxSeqPreset->currentIndex()].seqTest.size();i++)
        {
            ui->tableWidget->setItem(i, TABLE_COL_DEVICE, new QTableWidgetItem("---"));
        }
    }
}

void MainWindow::setWidgetProperties(bool flag)
{
    ui->pushButtonConnect->setEnabled(flag);
    ui->pushButtonAddTotable->setEnabled(flag);
    ui->pushButtonFindDevs->setEnabled(flag);
    ui->pushButtonSendAPDU->setEnabled(flag);
}

void MainWindow::on_lineEditLEN_editingFinished()
{
    if (ui->lineEditLEN->text().compare("00"))
    {
        ui->lineEditData->setEnabled(true);
    }
    else
    {
        ui->lineEditData->setEnabled(false);
    }
}

void MainWindow::on_pushButtonAddTotable_clicked()
{
    if (_test_running)
    {
        return;
    }

    if (_common_conf.presetSequenceV.size() == 0)
    {
        ui->textEdit->append("Seq preset is missing. Create New Sequnce Preset");
        return;
    }

    int indexCMD = ui->comboBoxAPDUSelect->currentIndex();
    int indexPres = ui->comboBoxPreset->currentIndex();
    int indexSeqPreset = ui->comboBoxSeqPreset->currentIndex();

    if (indexCMD < 0 ||
            indexPres < 0 ||
            indexSeqPreset < 0)
    {
        return;
    }

    test_conf_t test_conf;
    this->_pConfig->setDefaultTestConf(&test_conf);

    test_conf.commandDesc = _common_conf.presetCommandsV[indexPres].commandConf[indexCMD].commandDesc;
    test_conf.cla = _common_conf.presetCommandsV[indexPres].commandConf[indexCMD].cla;
    test_conf.ins = _common_conf.presetCommandsV[indexPres].commandConf[indexCMD].ins;
    test_conf.p1 = _common_conf.presetCommandsV[indexPres].commandConf[indexCMD].p1;
    test_conf.p2 = _common_conf.presetCommandsV[indexPres].commandConf[indexCMD].p2;
    test_conf.len = _common_conf.presetCommandsV[indexPres].commandConf[indexCMD].len;
    test_conf.data_field = _common_conf.presetCommandsV[indexPres].commandConf[indexCMD].data_field;

    QString command;
    command += QString("%1").arg(_common_conf.presetCommandsV[indexPres].commandConf[indexCMD].cla, 2, 16, QChar('0')).toUpper();
    command += ":";
    command += QString("%1").arg(_common_conf.presetCommandsV[indexPres].commandConf[indexCMD].ins, 2, 16, QChar('0')).toUpper();
    command += ":";
    command += QString("%1").arg(_common_conf.presetCommandsV[indexPres].commandConf[indexCMD].p1, 2, 16, QChar('0')).toUpper();
    command += ":";
    command += QString("%1").arg(_common_conf.presetCommandsV[indexPres].commandConf[indexCMD].p2, 2, 16, QChar('0')).toUpper();
    command += " | ";
    command += QString("%1").arg(_common_conf.presetCommandsV[indexPres].commandConf[indexCMD].len, 2, 16, QChar('0')).toUpper();
    command += " | ";

    if (_common_conf.presetCommandsV[indexPres].commandConf[indexCMD].len > 0)
    {
        if (_common_conf.presetCommandsV[indexPres].commandConf[indexCMD].data_field.empty())
        {
            DEBUG(DBG_ERROR, "No Data In command: '" <<
                  _common_conf.presetCommandsV[indexPres].commandConf[indexCMD].commandDesc <<
                  "', but len > 0");

            ui->textEdit->append(QString("No Data In command: '") +
                                 QString(_common_conf.presetCommandsV[indexPres].commandConf[indexCMD].commandDesc.c_str()) +
                                 QString("', but len > 0"));

            return;
        }
        for (unsigned i = 0;i < _common_conf.presetCommandsV[indexPres].commandConf[indexCMD].data_field.size()-1;i+=2)
        {
            command += _common_conf.presetCommandsV[indexPres].commandConf[indexCMD].data_field.substr(i,2).c_str();

            if (i != _common_conf.presetCommandsV[indexPres].commandConf[indexCMD].data_field.size()-2)
            {
                command += ":";
            }
        }
    }

    int row = ui->tableWidget->rowCount();

    ui->tableWidget->insertRow(row);

    ui->tableWidget->setItem(row, TABLE_COL_CMD_NAME,
                             new QTableWidgetItem(_common_conf.presetCommandsV[indexPres].commandConf[indexCMD].commandDesc.c_str()));

    ui->tableWidget->setItem(row, TABLE_COL_APDU, new QTableWidgetItem(command));

    this->showCurExecCommand();

    _common_conf.presetSequenceV[indexSeqPreset].seqTest.push_back(test_conf);

    if (ui->tableWidget->rowCount() > 0)
    {
        QString text = "Total: ";
        text += QString::number(ui->tableWidget->rowCount());
        ui->label_33->setText(text);
    }

    ui->label_34->setText("---");

    this->savePresetsToConf();
}

void MainWindow::on_pushButtonStartTest_clicked()
{
    if (_test_running)
    {
        DEBUG(DBG_DEBUG,"Stop test.");
        _test_running = false;
        this->setWidgetProperties(!_test_running);
        return;
    }

    int column = -1;
    bool both_device = false;

    if (ui->comboBoxDevice->currentIndex() < 0)
    {
        return;
    }

    int indexSeqPres = ui->comboBoxSeqPreset->currentIndex();

    if (indexSeqPres < 0)
    {
        return;
    }

    DEBUG(DBG_DEBUG, "START NEW TEST");
    DEBUG(DBG_DEBUG, _common_conf.presetSequenceV[indexSeqPres].seqPresetName.c_str());
    ui->textEdit->append("START NEW TEST");
    ui->textEdit->append(_common_conf.presetSequenceV[indexSeqPres].seqPresetName.c_str());

    if (this->_SCard.isActive())
    {
        DEBUG(DBG_DEBUG, "Device: " <<
              this->_SCard.getDeviceName());
        column = TABLE_COL_DEVICE;
    }
    else
    {
        DEBUG(DBG_WARN,"No connected readers. Stop test...");
        ui->textEdit->append("No connected readers");
        return;
    }

    this->showCurExecCommand();

    int row = ui->tableWidget->rowCount();

    if(row <= 0)
    {
        return;
    }

    _test_running = true;
    int repeat_cnt = 1;

    this->setWidgetProperties(!_test_running);

    ui->pushButtonStartTest->setText(tr("Stop Test"));
    startSeqTestConf->setText(tr("Stop Test (Space)"));

    qApp->processEvents();

    ui->tableWidget->verticalScrollBar()->setSliderPosition(0);

    double success = 0;
    unsigned i = 0;

    unsigned startTimeTest = getTimeMs();

    QVector<double> testNum(1), timeEx(1);

    testNum[0] = 0;
    timeEx[0] = 0;
    unsigned maxTimeEx = 0;
    unsigned exchange_tm = 0;

    if (_customPlot->graphCount() > 0)
    {
        _customPlot->removeGraph(0);
    }

    _customPlot->show();
    ui->progressBarTest->show();
    ui->progressBarTest->setTextVisible(true);

    ui->comboBoxSeqPreset->setEnabled(false);

    do
    {
        this->showCurExecCommand();
        for (i = 0; _test_running && i < _common_conf.presetSequenceV[indexSeqPres].seqTest.size(); i++)
        {
            ui->label_33->setText(QString("Test # ") +
                                  QString::number(i + 1) +
                                  QString(" of ") +
                                  QString::number(ui->tableWidget->rowCount()));

            DEBUG(DBG_DEBUG, "---------------------------------------------");
            ui->textEdit->append("---------------------------------------------");
            ui->textEdit->append(QString("Test # " +
                                         QString::number(i + 1) +
                                         ":" +
                                         _common_conf.presetSequenceV[indexSeqPres].seqTest[i].commandDesc.c_str()));
            DEBUG(DBG_DEBUG, "Test # " <<
                  (i + 1)
                  << ":"
                  << _common_conf.presetSequenceV[indexSeqPres].seqTest[i].commandDesc);

            qApp->processEvents();

            this->_failCheckInfo = false;

            if (!both_device)
            {
//                ui->tableWidget->item( i, 0)->setBackgroundColor(Qt::lightGray);
//                ui->tableWidget->item( i, 2)->setBackgroundColor(Qt::lightGray);

                qApp->processEvents();

                uint8_t apduBuf[APDU_BUFFER_SIZE] = {0};
                uint32_t apduLength = 0;
                uint8_t rpl[APDU_BUFFER_SIZE];
                uint32_t rLength;

                long rVal = 0;

                apduBuf[CLA_INDEX] = (0xff & _common_conf.presetSequenceV[indexSeqPres].seqTest[i].cla);
                apduBuf[INS_INDEX] = (0xff & _common_conf.presetSequenceV[indexSeqPres].seqTest[i].ins);
                apduBuf[P1_INDEX] = (0xff & _common_conf.presetSequenceV[indexSeqPres].seqTest[i].p1);
                apduBuf[P2_INDEX] = (0xff & _common_conf.presetSequenceV[indexSeqPres].seqTest[i].p2);
                apduBuf[LEN_INDEX] = (0xff & _common_conf.presetSequenceV[indexSeqPres].seqTest[i].len);

                apduLength = APDU_HEAD_LEN;


                if (!_common_conf.presetSequenceV[indexSeqPres].seqTest[i].data_field.empty())
                {
                    std::string data = _common_conf.presetSequenceV[indexSeqPres].seqTest[i].data_field;

                    for (unsigned int j = 0,index = 5;j < data.size() - 1;j+=2,index++)
                    {
                        apduBuf[index] = (0xff & ((char2bin(data[j]) << 4) | char2bin(data[j+1])));
                    }

                    apduBuf[LEN_INDEX] = 0xff & (data.size()/2); // String "FF" means 2 character
                }

                apduLength += apduBuf[LEN_INDEX];

                bool cmdOK = true;
                QString text = "OK";

                unsigned start_tm = getTimeMs();


                rVal = this->scardExchange((const char*)apduBuf,
                                           apduLength,
                                           (char*)rpl,
                                           (int*)&rLength,
                                           exchange_tm);

                unsigned stop_tm = getTimeMs();

                qApp->processEvents();

                testNum.append(i+1);
                timeEx.append(stop_tm - start_tm);

                if (maxTimeEx < (stop_tm - start_tm))
                {
                    maxTimeEx = (stop_tm - start_tm);
                }

                if (!rVal)
                {
                    if ((uint32_t)_common_conf.presetSequenceV[indexSeqPres].seqTest[i].ansLen != rLength)
                    {
                        if (rLength > (uint32_t)_common_conf.presetSequenceV[indexSeqPres].seqTest[i].ansLenMax)
                        {
                            cmdOK = false;
                            DEBUG(DBG_ERROR, "WRONG LEN. LEN > MAX" << "(" << rLength << " >" <<
                                  _common_conf.presetSequenceV[indexSeqPres].seqTest[i].ansLenMax << ")");
                            text = "FAIL";
                            QString desc = "WRONG LEN. LEN > MAX";
                            desc += "(";
                            desc += QString::number(rLength);
                            desc += " > ";
                            desc += QString::number(_common_conf.presetSequenceV[indexSeqPres].seqTest[i].ansLenMax);
                            desc += ")";
                            ui->textEdit->append(desc);
                        }
                        else if (rLength < (uint32_t)_common_conf.presetSequenceV[indexSeqPres].seqTest[i].ansLenMin)
                        {
                            cmdOK = false;

                            DEBUG(DBG_ERROR, "WRONG LEN. LEN > MIN" << "(" << rLength << " >" <<
                                  _common_conf.presetSequenceV[indexSeqPres].seqTest[i].ansLenMin << ")");
                            text = "FAIL";
                            QString desc = "WRONG LEN. LEN < MIN";
                            desc += "(";
                            desc += QString::number(rLength);
                            desc += " < ";
                            desc += QString::number(_common_conf.presetSequenceV[indexSeqPres].seqTest[i].ansLenMin);
                            desc += ")";
                            ui->textEdit->append(desc);
                        }
                        else
                        {
                            DEBUG(DBG_WARN, "WRONG LEN in test. conf: " <<
                                  _common_conf.presetSequenceV[indexSeqPres].seqTest[i].ansLen << ", answer: " << rLength);
                            QString desc = "WRONG LEN in test";
                            desc += "( test:";
                            desc += QString::number(rLength);
                            desc += ", answer: ";
                            desc += QString::number(_common_conf.presetSequenceV[indexSeqPres].seqTest[i].ansLen);
                            desc += " )";
                            ui->textEdit->append(desc);
                        }
                    }

                    if (rpl[rLength - 2] != _common_conf.presetSequenceV[indexSeqPres].seqTest[i].SW1)
                    {
                        cmdOK = false;
                        DEBUG(DBG_ERROR, "WRONG SW1");
                        text = "FAIL";
                        ui->textEdit->append("WRONG SW1");
                    }

                    if (rpl[rLength - 1] != _common_conf.presetSequenceV[indexSeqPres].seqTest[i].SW2)
                    {
                        cmdOK = false;
                        DEBUG(DBG_ERROR, "WRONG SW2");
                        text = "FAIL";
                        ui->textEdit->append("WRONG SW2");
                    }

                    ui->tableWidget->setItem(i, column, new QTableWidgetItem(text));
                    ui->textEdit->append(text);

                    if (cmdOK)
                    {
                        ui->tableWidget->item( i, column)->setBackground(Qt::darkGreen);
                        success++;
                    }
                    else
                    {
                        ui->tableWidget->item( i, column)->setBackground(Qt::darkRed);
                        this->_test_running = false;
                        break;
                    }

                    if (_common_conf.presetSequenceV[indexSeqPres].seqTest[i].resetUSB)
                    {
                        DEBUG(DBG_WARN, "Try reconnect after reset USB");
                        ui->textEdit->append("Try reconnect after reset USB");
                        qApp->processEvents();

                        this->_SCard.reconnect(DEFAULT_RECON_TIMEOUT_MS,
                                               ui->checkBoxExclusive->isChecked());
                    }
                }
                else if (SCARD_E_INVALID_HANDLE != rVal || SCARD_W_RESET_CARD == rVal)
                {
                    if (_common_conf.presetSequenceV[indexSeqPres].seqTest[i].resetUSB)
                    {
                        ui->tableWidget->setItem(i, column, new QTableWidgetItem(text));
                        ui->textEdit->append(text);
                        ui->tableWidget->item( i, column)->setBackground(Qt::darkGreen);
                        success++;
                        DEBUG(DBG_WARN, "Try reconnect after reset USB");
                        ui->textEdit->append("Try reconnect after reset USB");
                        qApp->processEvents();
                        this->_SCard.reconnect(DEFAULT_RECON_TIMEOUT_MS,
                                               ui->checkBoxExclusive->isChecked());
                    }
                    else
                    {
                        ui->tableWidget->setItem(i, column, new QTableWidgetItem("FAIL (TRANS)"));
                        DEBUG(DBG_ERROR, "FAIL (TRANS). Error: " << this->_SCard.getSCardErrorDescription(rVal));
                        ui->textEdit->append(QString("FAIL (TRANS). Error: ") + QString(this->_SCard.getSCardErrorDescription(rVal)));
                        qApp->processEvents();
                        this->_test_running = false;
                        break;
                    }
                }
                else
                {
                    ui->tableWidget->setItem(i, column, new QTableWidgetItem("FAIL (HANDLE)"));
                    DEBUG(DBG_ERROR, "FAIL (HANDLE). Error: " << this->_SCard.getSCardErrorDescription(rVal));
                    ui->textEdit->append(QString("FAIL (HANDLE). Error: ") + QString(this->_SCard.getSCardErrorDescription(rVal)));
                    ui->tableWidget->item( i, column)->setBackground(Qt::darkRed);
                    qApp->processEvents();

                    this->_test_running = false;
                    break;
                }

                if (!(i % 20))
                {
                    ui->tableWidget->verticalScrollBar()->setSliderPosition(i);
                }

//                ui->tableWidget->item( i, 0)->setBackgroundColor(Qt::white);
//                ui->tableWidget->item( i, 2)->setBackgroundColor(Qt::white);

                DEBUG(DBG_DEBUG, "---------------------------------------------");
                ui->textEdit->append("---------------------------------------------");

                qApp->processEvents();
            }

            ui->label_34->setText(QString("Success: ") +
                                  QString::number(success) +
                                  QString(" | Test Count: ") +
                                  QString::number(repeat_cnt));

            if (this->_failCheckInfo)
            {
                this->_test_running = false;
                break;
            }

            ui->progressBarTest->setValue(((float)(i + 1)/(float)ui->tableWidget->rowCount()) * 100);
        }

        if (ui->checkBoxInfinite->isChecked() && this->_test_running)
        {
            DEBUG(DBG_DEBUG, "\n\nRepeat\n\n");
            ui->textEdit->append("\n\nRepeat\n\n");
            repeat_cnt++;
        }

    } while(ui->checkBoxInfinite->isChecked() && this->_test_running);
    unsigned stopTimeTest = getTimeMs();

    ui->comboBoxSeqPreset->setEnabled(true);

    ui->checkBoxInfinite->setChecked(false);

    _customPlot->addGraph();
    _customPlot->graph(0)->setData(testNum, timeEx);

    _customPlot->xAxis->setLabel("test num");
    _customPlot->yAxis->setLabel("time, ms");

    _customPlot->xAxis->setRange(0, ui->tableWidget->rowCount() + 2);
    _customPlot->yAxis->setRange(0, maxTimeEx + (maxTimeEx/100 * 5)); // + 5%

    _customPlot->xAxis->setTickVector(testNum);

    _customPlot->graph(0)->rescaleAxes();

    _customPlot->replot();

    QString txt = "Test Done. Total: ";

    txt += QString::number(ui->tableWidget->rowCount());
    txt +=  " | Count: ";
    txt += QString::number(repeat_cnt);
    txt +=  " | OK: ";
    txt += QString::number(success);
    txt += " | Fail: ";
    txt += QString::number(ui->tableWidget->rowCount() * repeat_cnt - success);

    unsigned difftime = stopTimeTest - startTimeTest;

    txt += " | Time: ";
    txt += QString("%1").arg((difftime/3600000), 2, 10, QChar('0')).toUpper();
    txt += ":";
    difftime %= 3600000;
    txt += QString("%1").arg((difftime/60000), 2, 10, QChar('0')).toUpper();
    txt += ":";
    difftime %= 60000;
    txt += QString("%1").arg((difftime/1000), 2, 10, QChar('0')).toUpper();
    txt += ".";
    difftime %= 1000;
    txt += QString::number(difftime);

    ui->textEdit->append(txt + "\n\n\n");

    DEBUG(DBG_DEBUG, txt.toStdString() << "\n\n\n");

    if (i != _common_conf.presetSequenceV[indexSeqPres].seqTest.size())
    {
        ui->tableWidget->item( i, column)->setBackground(Qt::darkRed);

        if (this->_failCheckInfo)
        {
            ui->tableWidget->item( i, column)->setBackground(Qt::darkRed);
        }
    }

    ui->label_34->setText(txt.remove("Test Done. "));

    _test_running = false;
    this->setWidgetProperties(!_test_running);

    ui->pushButtonStartTest->setText(tr("Start Test"));
    startSeqTestConf->setText(tr("Start Test (Space)"));
}

void MainWindow::StartTest_Selected_activated()
{
    if (_test_running)
    {
        DEBUG(DBG_DEBUG,"Stop test.");
        _test_running = false;
        this->setWidgetProperties(!_test_running);
        return;
    }

    int column = -1;
    bool both_device = false;

    if (ui->comboBoxDevice->currentIndex() < 0)
    {
        return;
    }

    int indexSeqPres = ui->comboBoxSeqPreset->currentIndex();

    if (indexSeqPres < 0)
    {
        return;
    }

    QList<QTableWidgetItem *> itemList = ui->tableWidget->selectedItems();

    if (itemList.empty())
    {
        return;
    }

    std::vector<int> vectIndex;

    for (int i = (itemList.size() - 1); i > 0; i -= ui->tableWidget->columnCount())
    {
        vectIndex.push_back(itemList[i]->row());
    }

    // lambdas not working Hz
    std::sort(vectIndex.begin(), vectIndex.end());


    DEBUG(DBG_DEBUG, "START NEW TEST");
    DEBUG(DBG_DEBUG, _common_conf.presetSequenceV[indexSeqPres].seqPresetName.c_str());
    ui->textEdit->append("START NEW TEST");
    ui->textEdit->append(_common_conf.presetSequenceV[indexSeqPres].seqPresetName.c_str());

    if (this->_SCard.isActive())
    {
        DEBUG(DBG_DEBUG, "Device 1: " <<
              this->_SCard.getDeviceName());
        column = TABLE_COL_DEVICE;
    }
    else
    {
        DEBUG(DBG_WARN,"No connected readers. Stop test...");
        ui->textEdit->append("No connected readers");
        return;
    }

    this->showCurExecCommand();

    int row = ui->tableWidget->rowCount();

    if(row <= 0)
    {
        return;
    }

    _test_running = true;

    this->setWidgetProperties(!_test_running);

    ui->pushButtonStartTest->setText(tr("Stop Test"));
    startSeqTestConf->setText(tr("Stop Test (Space)"));

    qApp->processEvents();

    ui->tableWidget->verticalScrollBar()->setSliderPosition(0);

    double success = 0;
    unsigned i = 0;
    int repeat_cnt = 1;

    unsigned startTimeTest = getTimeMs();

    QVector<double> testNum(1), timeEx(1);

    testNum[0] = 0;
    timeEx[0] = 0;
    unsigned maxTimeEx = 0;
    unsigned exchange_tm = 0;

    if (_customPlot->graphCount() > 0)
    {
        _customPlot->removeGraph(0);
    }

    _customPlot->show();
    //ui->pushButtonShowGraph->show();
    ui->progressBarTest->show();
    ui->progressBarTest->setTextVisible(true);
    ui->comboBoxSeqPreset->setEnabled(false);

    do
    {
        this->showCurExecCommand();
        for (i = 0; _test_running && i < vectIndex.size(); i++)
        {
            ui->label_33->setText(QString("Test # ") +
                                  QString::number(vectIndex[i] + 1) +
                                  QString(" of ") +
                                  QString::number(ui->tableWidget->rowCount()));

            DEBUG(DBG_DEBUG, "---------------------------------------------");
            ui->textEdit->append("---------------------------------------------");
            ui->textEdit->append(QString("Test # " +
                                         QString::number(vectIndex[i] + 1) +
                                         ":" +
                                         _common_conf.presetSequenceV[indexSeqPres].seqTest[vectIndex[i]].commandDesc.c_str()));
            DEBUG(DBG_DEBUG, "Test # " <<
                  (vectIndex[i] + 1)
                  << ":"
                  << _common_conf.presetSequenceV[indexSeqPres].seqTest[vectIndex[i]].commandDesc);

            qApp->processEvents();

            this->_failCheckInfo = false;

            if (!both_device)
            {
//                ui->tableWidget->item( vectIndex[i], 0)->setBackgroundColor(Qt::lightGray);
//                ui->tableWidget->item( vectIndex[i], 2)->setBackgroundColor(Qt::lightGray);

                qApp->processEvents();

                uint8_t apduBuf[APDU_BUFFER_SIZE] = {0};
                uint32_t apduLength = 0;
                uint8_t rpl[APDU_BUFFER_SIZE];
                uint32_t rLength;

                long rVal = 0;

                apduBuf[CLA_INDEX] = (0xff & _common_conf.presetSequenceV[indexSeqPres].seqTest[vectIndex[i]].cla);
                apduBuf[INS_INDEX] = (0xff & _common_conf.presetSequenceV[indexSeqPres].seqTest[vectIndex[i]].ins);
                apduBuf[P1_INDEX] = (0xff & _common_conf.presetSequenceV[indexSeqPres].seqTest[vectIndex[i]].p1);
                apduBuf[P2_INDEX] = (0xff & _common_conf.presetSequenceV[indexSeqPres].seqTest[vectIndex[i]].p2);
                apduBuf[LEN_INDEX] = (0xff & _common_conf.presetSequenceV[indexSeqPres].seqTest[vectIndex[i]].len);

                apduLength = APDU_HEAD_LEN;


                if (!_common_conf.presetSequenceV[indexSeqPres].seqTest[vectIndex[i]].data_field.empty())
                {
                    std::string data = _common_conf.presetSequenceV[indexSeqPres].seqTest[vectIndex[i]].data_field;

                    for (unsigned int j = 0,index = 5;j < data.size() - 1;j+=2,index++)
                    {
                        apduBuf[index] = (0xff & ((char2bin(data[j]) << 4) | char2bin(data[j+1])));
                    }

                    apduBuf[LEN_INDEX] = 0xff & (data.size()/2); // String "FF" means 2 character
                }

                apduLength += apduBuf[LEN_INDEX];


                bool cmdOK = true;
                QString text = "OK";


                rVal = this->scardExchange((const char*)apduBuf,
                                           apduLength,
                                           (char*)rpl,
                                           (int*)&rLength,
                                           exchange_tm);

                testNum.append(vectIndex[i]+1);
                timeEx.append(exchange_tm);

                if (maxTimeEx < (exchange_tm))
                {
                    maxTimeEx = (exchange_tm);
                }

                if (!rVal)
                {
                    if ((uint32_t)_common_conf.presetSequenceV[indexSeqPres].seqTest[vectIndex[i]].ansLen != rLength)
                    {
                        if (rLength > (uint32_t)_common_conf.presetSequenceV[indexSeqPres].seqTest[vectIndex[i]].ansLenMax)
                        {
                            cmdOK = false;
                            DEBUG(DBG_ERROR, "WRONG LEN. LEN > MAX" << "(" << rLength << " >" <<
                                  _common_conf.presetSequenceV[indexSeqPres].seqTest[vectIndex[i]].ansLenMax << ")");
                            text = "FAIL";
                            QString desc = "WRONG LEN. LEN > MAX";
                            desc += "(";
                            desc += QString::number(rLength);
                            desc += " > ";
                            desc += QString::number(_common_conf.presetSequenceV[indexSeqPres].seqTest[vectIndex[i]].ansLenMax);
                            desc += ")";
                            ui->textEdit->append(desc);
                        }
                        else if (rLength < (uint32_t)_common_conf.presetSequenceV[indexSeqPres].seqTest[vectIndex[i]].ansLenMin)
                        {
                            cmdOK = false;

                            DEBUG(DBG_ERROR, "WRONG LEN. LEN > MIN" << "(" << rLength << " >" <<
                                  _common_conf.presetSequenceV[indexSeqPres].seqTest[vectIndex[i]].ansLenMin << ")");
                            text = "FAIL";
                            QString desc = "WRONG LEN. LEN < MIN";
                            desc += "(";
                            desc += QString::number(rLength);
                            desc += " < ";
                            desc += QString::number(_common_conf.presetSequenceV[indexSeqPres].seqTest[vectIndex[i]].ansLenMin);
                            desc += ")";
                            ui->textEdit->append(desc);
                        }
                        else
                        {
                            DEBUG(DBG_WARN, "WRONG LEN in test. conf: " <<
                                  _common_conf.presetSequenceV[indexSeqPres].seqTest[vectIndex[i]].ansLen << ", answer: " << rLength);
                            QString desc = "WRONG LEN in test";
                            desc += "( test:";
                            desc += QString::number(rLength);
                            desc += ", answer: ";
                            desc += QString::number(_common_conf.presetSequenceV[indexSeqPres].seqTest[vectIndex[i]].ansLen);
                            desc += " )";
                            ui->textEdit->append(desc);
                        }
                    }

                    if (rpl[rLength - 2] != _common_conf.presetSequenceV[indexSeqPres].seqTest[vectIndex[i]].SW1)
                    {
                        cmdOK = false;
                        DEBUG(DBG_ERROR, "WRONG SW1");
                        text = "FAIL";
                        ui->textEdit->append("WRONG SW1");
                    }

                    if (rpl[rLength - 1] != _common_conf.presetSequenceV[indexSeqPres].seqTest[vectIndex[i]].SW2)
                    {
                        cmdOK = false;
                        DEBUG(DBG_ERROR, "WRONG SW2");
                        text = "FAIL";
                        ui->textEdit->append("WRONG SW2");
                    }

                    ui->tableWidget->setItem(vectIndex[i], column, new QTableWidgetItem(text));
                    ui->textEdit->append(text);

                    if (cmdOK)
                    {
                        ui->tableWidget->item( vectIndex[i], column)->setBackground(Qt::darkGreen);
                        success++;
                    }
                    else
                    {
                        ui->tableWidget->item( vectIndex[i], column)->setBackground(Qt::darkRed);
                        this->_test_running = false;
                        break;
                    }

                    if (_common_conf.presetSequenceV[indexSeqPres].seqTest[vectIndex[i]].resetUSB)
                    {
                        DEBUG(DBG_WARN, "Try reconnect after reset USB");
                        ui->textEdit->append("Try reconnect after reset USB");
                        qApp->processEvents();

                        this->_SCard.reconnect(DEFAULT_RECON_TIMEOUT_MS,
                                               ui->checkBoxExclusive->isChecked());
                    }
                }
                else if (SCARD_E_INVALID_HANDLE != rVal || SCARD_W_RESET_CARD == rVal)
                {
                    if (_common_conf.presetSequenceV[indexSeqPres].seqTest[vectIndex[i]].resetUSB)
                    {
                        ui->tableWidget->setItem(vectIndex[i], column, new QTableWidgetItem(text));
                        ui->textEdit->append(text);
                        ui->tableWidget->item( vectIndex[i], column)->setBackground(Qt::darkGreen);
                        success++;
                        DEBUG(DBG_WARN, "Try reconnect after reset USB");
                        ui->textEdit->append("Try reconnect after reset USB");
                        qApp->processEvents();
                        this->_SCard.reconnect(DEFAULT_RECON_TIMEOUT_MS,
                                               ui->checkBoxExclusive->isChecked());
                    }
                    else
                    {
                        ui->tableWidget->setItem(vectIndex[i], column, new QTableWidgetItem("FAIL (TRANS)"));
                        DEBUG(DBG_ERROR, "FAIL (TRANS). Error: " << this->_SCard.getSCardErrorDescription(rVal));
                        ui->textEdit->append(QString("FAIL (TRANS). Error: ") + QString(this->_SCard.getSCardErrorDescription(rVal)));
                        qApp->processEvents();
                        this->_test_running = false;
                        break;
                    }
                }
                else
                {
                    ui->tableWidget->setItem(vectIndex[i], column, new QTableWidgetItem("FAIL (HANDLE)"));
                    DEBUG(DBG_ERROR, "FAIL (HANDLE). Error: " << this->_SCard.getSCardErrorDescription(rVal));
                    ui->textEdit->append(QString("FAIL (HANDLE). Error: ") + QString(this->_SCard.getSCardErrorDescription(rVal)));
                    ui->tableWidget->item( vectIndex[i], column)->setBackground(Qt::darkRed);
                    qApp->processEvents();

                    this->_test_running = false;
                    break;
                }

                if (!(i % 20))
                {
                    ui->tableWidget->verticalScrollBar()->setSliderPosition(i);
                }

                DEBUG(DBG_DEBUG, "---------------------------------------------");
                ui->textEdit->append("---------------------------------------------");

//                ui->tableWidget->item( vectIndex[i], 0)->setBackgroundColor(Qt::white);
//                ui->tableWidget->item( vectIndex[i], 2)->setBackgroundColor(Qt::white);

                qApp->processEvents();
            }

            ui->label_34->setText(QString("Success: ") +
                                  QString::number(success) +
                                  QString(" | Test Count: ") +
                                  QString::number(repeat_cnt));

            if (this->_failCheckInfo)
            {
                this->_test_running = false;
                break;
            }

            ui->progressBarTest->setValue(((float)(i + 1)/(float)vectIndex.size()) * 100);
        }

        if (ui->checkBoxInfinite->isChecked() && this->_test_running)
        {
            DEBUG(DBG_DEBUG, "\n\nRepeat\n\n" + QString::number(repeat_cnt).toStdString());
            ui->textEdit->append(QString("\n\nRepeat\n\n") + QString::number(repeat_cnt));
            repeat_cnt++;
        }

    } while(ui->checkBoxInfinite->isChecked() && this->_test_running);
    unsigned stopTimeTest = getTimeMs();

    ui->comboBoxSeqPreset->setEnabled(true);
    ui->checkBoxInfinite->setChecked(false);

    _customPlot->addGraph();
    _customPlot->graph(0)->setData(testNum, timeEx);

    _customPlot->xAxis->setLabel("test num");
    _customPlot->yAxis->setLabel("time, ms");

    _customPlot->xAxis->setRange(0, ui->tableWidget->rowCount() + 2);
    _customPlot->yAxis->setRange(0, maxTimeEx + (maxTimeEx/100 * 5)); // + 5%

    _customPlot->xAxis->setTickVector(testNum);

    _customPlot->graph(0)->rescaleAxes();

    _customPlot->replot();

    QString txt = "Test Done. Total: ";

    txt += QString::number(vectIndex.size());
    txt +=  " | Count: ";
    txt += QString::number(repeat_cnt);
    txt +=  " | OK: ";
    txt += QString::number(success);
    txt += " | Fail: ";
    txt += QString::number(vectIndex.size() * repeat_cnt - success);

    unsigned difftime = stopTimeTest - startTimeTest;

    txt += " | Time: ";
    txt += QString("%1").arg((difftime/3600000), 2, 10, QChar('0')).toUpper();
    txt += ":";
    difftime %= 3600000;
    txt += QString("%1").arg((difftime/60000), 2, 10, QChar('0')).toUpper();
    txt += ":";
    difftime %= 60000;
    txt += QString("%1").arg((difftime/1000), 2, 10, QChar('0')).toUpper();
    txt += ".";
    difftime %= 1000;
    txt += QString::number(difftime);

    ui->textEdit->append(txt + "\n\n\n");

    DEBUG(DBG_DEBUG, txt.toStdString() << "\n\n\n");

    if (i != vectIndex.size())
    {
        ui->tableWidget->item( vectIndex[i], column)->setBackground(Qt::darkRed);

        if (this->_failCheckInfo)
        {
            ui->tableWidget->item( vectIndex[i], column)->setBackground(Qt::darkRed);
        }
    }

    ui->label_34->setText(txt.remove("Test Done. "));

    _test_running = false;
    this->setWidgetProperties(!_test_running);

    ui->pushButtonStartTest->setText(tr("Start Test"));
    startSeqTestConf->setText(tr("Start Test (Space)"));
}

void MainWindow::customMenuRequested(QPoint pos)
{
    if (!_pConfig || !_pConfig->isConfigured())
    {
        return;
    }

    int indexSeqPres = ui->comboBoxSeqPreset->currentIndex();

    if (indexSeqPres < 0)
    {
        return;
    }

    QModelIndex index = ui->tableWidget->indexAt(pos);

    if (ui->tableWidget->rowCount() < index.row() || index.row() < 0)
    {
        return;
    }

    QMenu *menu = new QMenu(this);

    QAction *setItem = menu->addAction("Set Item");
    QAction *delItem = menu->addAction("Del Item");
    QAction *sel = menu->exec(ui->tableWidget->viewport()->mapToGlobal(pos));

    if (sel == setItem)
    {
        DialogSetTestItem dialogTestItem;

        if(!_pConfig )
        {
            return;
        }

        if(!_pConfig->isConfigured())
        {
            return;
        }

        if (!dialogTestItem.setCurrentPreset(this->_pConfig,
                                             &this->_common_conf,
                                             indexSeqPres,
                                             index.row()))
        {
            return;
        }

        int position = ui->tableWidget->verticalScrollBar()->sliderPosition();

        dialogTestItem.show();

        int dialogCode = dialogTestItem.exec();

        if (dialogCode == QDialog::Accepted)
        {
            this->savePresetsToConf();
        }

        ui->tableWidget->verticalScrollBar()->setSliderPosition(position);
        ui->tableWidget->selectRow(index.row());
    }
    if (sel == delItem)
    {
        std::string msg_text;

        msg_text = "Delete Item #";
        msg_text += std::to_string(index.row());
        msg_text += " '";
        msg_text += _common_conf.presetSequenceV[indexSeqPres].seqTest[index.row()].commandDesc.c_str();
        msg_text += "'?";

        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Warning", msg_text.c_str(),
                                      QMessageBox::Yes|QMessageBox::No);

        if (reply == QMessageBox::Yes)
        {
            ui->tableWidget->removeRow(index.row());
            _pConfig->removeSeqPresetItem(this->_common_conf, indexSeqPres, index.row());
            this->savePresetsToConf();
        }
    }
}

void MainWindow::tableItemClicked(int row, int column)
{
    Q_UNUSED(column)

    if (!_pConfig || !_pConfig->isConfigured())
    {
        return;
    }

    int indexSeqPres = ui->comboBoxSeqPreset->currentIndex();

    if (row < 0 || indexSeqPres < 0)
    {
        return;
    }

    DialogSetTestItem dialogTestItem;

    if(!_pConfig )
    {
        return;
    }

    if(!_pConfig->isConfigured())
    {
        return;
    }

    if (!dialogTestItem.setCurrentPreset(this->_pConfig,
                                         &this->_common_conf,
                                         indexSeqPres,
                                         row))
    {
        return;
    }

    int position = ui->tableWidget->verticalScrollBar()->sliderPosition();

    dialogTestItem.show();

    int dialogCode = dialogTestItem.exec();

    if (dialogCode == QDialog::Accepted)
    {
        this->savePresetsToConf();
    }

    ui->tableWidget->verticalScrollBar()->setSliderPosition(position);
    ui->tableWidget->selectRow(row);
}

void MainWindow::AddSeqTestItem_clicked()
{
    if (_test_running)
    {
        return;
    }

    DialogNewSeqPreset dial;

    if(!_pConfig )
    {
        return;
    }

    if(!_pConfig->isConfigured())
    {
        return;
    }

    if (!dial.setCurrentConfig(_pConfig, &this->_common_conf))
    {
        return;
    }

    dial.show();
    dial.exec();

    this->savePresetsToConf();
}

void MainWindow::saveSeqConfiguration_triggered()
{
    if (_test_running)
    {
        return;
    }

    if (!_pConfig || !_pConfig->isConfigured())
    {
        return;
    }

    this->savePresetsToConf();
}

void MainWindow::clearSeqConfiguration_triggered()
{
    if (_test_running)
    {
        return;
    }

    if (!_pConfig || !_pConfig->isConfigured())
    {
        return;
    }

    std::string msg_text;

    msg_text = "Clear Test Sequence?";

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Warning", msg_text.c_str(),
                                  QMessageBox::Yes|QMessageBox::No);

    if (reply == QMessageBox::Yes)
    {
        while (ui->tableWidget->rowCount() > 0)
        {
            ui->tableWidget->removeRow(0);
        }

        this->_pConfig->clearSequenceList(this->_common_conf,
                                          ui->comboBoxSeqPreset->currentIndex());

        this->savePresetsToConf();
    }
}

void MainWindow::reloadConfiguration_triggered()
{
    if (_test_running)
    {
        return;
    }

    updateLocalConfig();
}

bool MainWindow::savePresetsToConf()
{
    if (!_pConfig->savePresetsConf(this->_common_conf))
    {
        ui->textEdit->append("Sequece not saved");
        return false;
    }

    updateLocalConfig();

    return true;
}

void MainWindow::printExchageBuffers(const char *cmd, const int cLength,
                                     const char *rpl, const int rLength, unsigned time_ms)
{
    QString text = "C-APDU (";
    text += "Dev ";
    text += "): ";

    for (int i = 0;i < cLength;i++)
    {
        text += QString("%1").arg((0xff & cmd[i]), 2, 16, QChar('0')).toUpper();
        text += " ";
    }

    ui->textEdit->append(text);
    DEBUG(DBG_DEBUG, text.toStdString());

    text = "R-APDU (";
    text += "Dev ";
    text += "): ";

    for (int i = 0;i < rLength;i++)
    {
        text += QString("%1").arg((0xff & rpl[i]), 2, 16, QChar('0')).toUpper();
        text += " ";
    }

    ui->textEdit->append(text);
    DEBUG(DBG_DEBUG, text.toStdString());

    text = "Time exchange: ";
    text += QString::number(time_ms);
    text += " ms";

    ui->textEdit->append(text);
    DEBUG(DBG_DEBUG, text.toStdString());
}

long MainWindow::scardExchange(const char *cmd,
                               int cLength, char *rpl, int *rLength)
{
    long res;

#ifdef DEBUG_EXCHANGE_BUFFERS
    unsigned start_tm = getTimeMs();
#endif // DEBUG_EXCHANGE_BUFFERS

    res = this->_SCard.transmit(cmd,
                                cLength,
                                rpl,
                                rLength);

#ifdef DEBUG_EXCHANGE_BUFFERS
    unsigned stop_tm = getTimeMs();

    this->printExchageBuffers(cmd,
                              cLength, rpl, *rLength,
                              stop_tm - start_tm);
#endif // DEBUG_EXCHANGE_BUFFERS

    qApp->processEvents();

    return res;
}

long MainWindow::scardExchange(const char *cmd,
                               int cLength, char *rpl, int *rLength,
                               unsigned &ex_time)
{
    long res;

    unsigned start_tm = getTimeMs();

    res = this->_SCard.transmit(cmd,
                                cLength,
                                rpl,
                                rLength);

    unsigned stop_tm = getTimeMs();

    ex_time = stop_tm - start_tm;

    this->printExchageBuffers(cmd,
                              cLength, rpl, *rLength,
                              ex_time);

    qApp->processEvents();

    return res;
}

void MainWindow::on_pushButtonCopyToClipboard_clicked()
{
    QClipboard *clipboard = QApplication::clipboard();
    QString logPath = ui->labelFileLog->text();

    logPath.replace("<a href=", "");
    logPath.replace("</a>", "");
    int last_index = logPath.lastIndexOf(">");
    logPath.remove(last_index, logPath.size() - last_index);


    clipboard->setText(logPath);
}

void MainWindow::on_comboBoxSeqPreset_currentIndexChanged(int index)
{
    if (index < 0)
    {
        return;
    }

    while (ui->tableWidget->rowCount() > 0)
    {
        ui->tableWidget->removeRow(0);
    }

    for (unsigned  i = 0;i < _common_conf.presetSequenceV[index].seqTest.size();i++)
    {
        int row = ui->tableWidget->rowCount();

        ui->tableWidget->insertRow(row);

        QString command;
        command += QString("%1").arg(_common_conf.presetSequenceV[index].seqTest[i].cla, 2, 16, QChar('0')).toUpper();
        command += ":";
        command += QString("%1").arg(_common_conf.presetSequenceV[index].seqTest[i].ins, 2, 16, QChar('0')).toUpper();
        command += ":";
        command += QString("%1").arg(_common_conf.presetSequenceV[index].seqTest[i].p1, 2, 16, QChar('0')).toUpper();
        command += ":";
        command += QString("%1").arg(_common_conf.presetSequenceV[index].seqTest[i].p2, 2, 16, QChar('0')).toUpper();
        command += " | ";
        command += QString("%1").arg(_common_conf.presetSequenceV[index].seqTest[i].len, 2, 16, QChar('0')).toUpper();
        command += " | ";

        if (!_common_conf.presetSequenceV[index].seqTest[i].data_field.empty())
        {
            if (_common_conf.presetSequenceV[index].seqTest[i].len > 0)
            {
                for (unsigned j = 0;j < _common_conf.presetSequenceV[index].seqTest[i].data_field.size()-1;j+=2)
                {
                    command += _common_conf.presetSequenceV[index].seqTest[i].data_field.substr(j,2).c_str();

                    if (j != _common_conf.presetSequenceV[index].seqTest[i].data_field.size()-2)
                    {
                        command += ":";
                    }
                }
            }
        }

        ui->tableWidget->setItem(row, TABLE_COL_CMD_NAME, new QTableWidgetItem(_common_conf.presetSequenceV[index].seqTest[i].commandDesc.c_str()));
        ui->tableWidget->setItem(row, TABLE_COL_APDU, new QTableWidgetItem(command));

        this->showCurExecCommand();
    }

    if (ui->tableWidget->rowCount() > 0)
    {
        QString text = tr("Total: ");
        text += QString::number(ui->tableWidget->rowCount());
        ui->label_33->setText(text);
    }

    ui->label_34->setText("---");
}

void MainWindow::on_tableWidget_clicked(const QModelIndex &index)
{
    test_conf_t temp_seq_item;

    temp_seq_item = this->_common_conf.presetSequenceV[ui->comboBoxSeqPreset->currentIndex()].seqTest[index.row()];

    QString txt = "Item configuration\n";

    txt += "Command:\t";
    txt += temp_seq_item.commandDesc.c_str();
    txt += "\n";

    txt += "CLA:\t\t";
    txt += QString("%1").arg(temp_seq_item.cla, 2, 16, QChar('0')).toUpper();
    txt += "\n";

    txt += "INS:\t\t";
    txt += QString("%1").arg(temp_seq_item.ins, 2, 16, QChar('0')).toUpper();
    txt += "\n";

    txt += "P1:\t\t";
    txt += QString("%1").arg(temp_seq_item.p1, 2, 16, QChar('0')).toUpper();
    txt += "\n";

    txt += "P2:\t\t";
    txt += QString("%1").arg(temp_seq_item.p2, 2, 16, QChar('0')).toUpper();
    txt += "\n";

    txt += "LC:\t\t";
    txt += QString("%1").arg(temp_seq_item.len, 2, 16, QChar('0')).toUpper();
    txt += "\n";

    txt += "Data:\t\t";
    if (temp_seq_item.data_field.size() > 11)
    {
        txt += "...";
        txt += temp_seq_item.data_field.substr(temp_seq_item.data_field.size() - 11).c_str();
    }
    else
    {
        txt += temp_seq_item.data_field.c_str();
    }
    txt += "\n";

    txt += "Resp len:\t\t";
    txt += QString::number(temp_seq_item.ansLen - 2); // without SW's
    txt += "\n";

    txt += "Min resp len:\t";
    txt += QString::number(temp_seq_item.ansLenMin - 2); // without SW's
    txt += "\n";

    txt += "Max resp len:\t";
    txt += QString::number(temp_seq_item.ansLenMax - 2); // without SW's
    txt += "\n";

    txt += "SW1:\t\t";
    txt += QString("%1").arg(temp_seq_item.SW1, 2, 16, QChar('0')).toUpper();
    txt += "\n";

    txt += "SW2:\t\t";
    txt += QString("%1").arg(temp_seq_item.SW2, 2, 16, QChar('0')).toUpper();
    txt += "\n";

    txt += "Save2Buf1:\t";
    txt += temp_seq_item.save2Buf1?"TRUE":"FALSE";
    txt += "\n";

    txt += "Save2Buf2:\t";
    txt += temp_seq_item.save2Buf2?"TRUE":"FALSE";
    txt += "\n";

    txt += "LoadFromBuf1:\t";
    txt += temp_seq_item.loadFromBuf1?"TRUE":"FALSE";
    txt += "\n";

    txt += "LoadFromBuf2:\t";
    txt += temp_seq_item.loadFromBuf2?"TRUE":"FALSE";
    txt += "\n";

    txt += "USB Reset:\t";
    txt += temp_seq_item.resetUSB?"TRUE":"FALSE";
    txt += "\n";

    popUp->setPopupText(txt);
    popUp->show();
}

void MainWindow::on_pushButtonShowGraph_clicked()
{
    if(!ui->widgetPlot->graphCount())
    {
        QMessageBox messageBox;
        messageBox.warning(this,"Show graph","No data");
        messageBox.setFixedSize(500,200);
        return;
    }

    DialogShowGraph dialGraph;

    dialGraph.setParentPlot(_customPlot);

    dialGraph.show();
    dialGraph.exec();

}

void MainWindow::deleteSelectedRows_pressed()
{
    QList<QTableWidgetItem *> itemList = ui->tableWidget->selectedItems();
    int indexSeqPreset = ui->comboBoxSeqPreset->currentIndex();

    std::vector<int> vectIndex;

    for (int i = (itemList.size() - 1); i > 0; i -= ui->tableWidget->columnCount())
    {
        vectIndex.push_back(itemList[i]->row());
    }

    // lambdas not working Hz
    std::sort(vectIndex.begin(), vectIndex.end());

    std::string msg_text;

    msg_text = "Delete item(s):";

    for (int i = 0;i < itemList.size();i += ui->tableWidget->columnCount())
    {
        msg_text += "\n";
        msg_text += "#";
        msg_text += std::to_string(itemList[i]->row() + 1);
        msg_text += " '";
        msg_text += (_common_conf.presetSequenceV[indexSeqPreset].seqTest.begin() +
                     itemList[i]->row())->commandDesc;
        msg_text += "'";
    }

    msg_text += " ?";

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this,
                                  "Warning",
                                  msg_text.c_str(),
                                  QMessageBox::Yes|QMessageBox::No);

    if (reply == QMessageBox::Yes)
    {
        for (int i = (int)vectIndex.size() - 1; i >= 0; i--)
        {
            this->_pConfig->removeSeqPresetItem(this->_common_conf,indexSeqPreset,vectIndex[i]);
        }

        this->savePresetsToConf();
    }
}

void MainWindow::copySelectedRows_pressed()
{
    _copyVectIndex.resize(0);
    QList<QTableWidgetItem *> itemList = ui->tableWidget->selectedItems();

    for (int i = (itemList.size() - 1); i > 0; i -= ui->tableWidget->columnCount())
    {
        _copyVectIndex.push_back(itemList[i]->row());
    }

    // lambdas not working Hz
    std::sort(_copyVectIndex.begin(), _copyVectIndex.end());

    this->_copySeqPreset = ui->comboBoxSeqPreset->currentIndex();
}

void MainWindow::pasteSelectedRows_pressed()
{
    QList<QTableWidgetItem *> itemList = ui->tableWidget->selectedItems();
    int indexSeqPreset = ui->comboBoxSeqPreset->currentIndex();

    for (unsigned int i = 0; i < _copyVectIndex.size(); i++)
    {
        _common_conf.presetSequenceV[indexSeqPreset].seqTest.push_back(_common_conf.presetSequenceV[this->_copySeqPreset].seqTest[_copyVectIndex[i]]);
    }

    this->savePresetsToConf();
}

//void MainWindow::closeEvent(QCloseEvent *event)
//{
//#if _WIN32 || _WIN64
//    if (_test_running)
//    {
//        QMessageBox::information(this, tr("test_ex"),
//                                 tr("The program will keep running in the "
//                                    "system tray. To terminate the program, "
//                                    "choose <b>Restore</b> or <b>Maximize</b> in the context menu "
//                                    "of the system tray entry and stop the current test."));
//    }
//#else
//    this->_test_running = false;
//#endif
//}

void MainWindow::on_tableWidget_cellEntered(int row, int column)
{
    Q_UNUSED(column)

    if (ui->checkBoxMultiSelect->isChecked())
    {
        ui->tableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
        return;
    }

    int colCount =ui->tableWidget->columnCount();

    int rowsel;

    if(ui->tableWidget->currentIndex().row() < row)
        rowsel = row - 1; //down
    else if(ui->tableWidget->currentIndex().row() > row)
        rowsel = row + 1; //up
    else
        return;

    test_conf_t temp_seq_conf = this->_common_conf.presetSequenceV[ui->comboBoxSeqPreset->currentIndex()].seqTest[row];

    this->_common_conf.presetSequenceV[ui->comboBoxSeqPreset->currentIndex()].seqTest[row] =
            this->_common_conf.presetSequenceV[ui->comboBoxSeqPreset->currentIndex()].seqTest[rowsel];

    this->_common_conf.presetSequenceV[ui->comboBoxSeqPreset->currentIndex()].seqTest[rowsel] = temp_seq_conf;

    QList<QTableWidgetItem*> rowItems, rowItems1;

    for (int col = 0; col < colCount; ++col)
    {
        rowItems << ui->tableWidget->takeItem(row, col);
        rowItems1 << ui->tableWidget->takeItem(rowsel, col);
    }

    for (int cola = 0; cola < colCount; ++cola)
    {
        ui->tableWidget->setItem(rowsel, cola, rowItems.at(cola));
        ui->tableWidget->setItem(row, cola, rowItems1.at(cola));
    }
}

void MainWindow::on_checkBoxMultiSelect_toggled(bool checked)
{
    if (checked)
    {
        ui->tableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    }
    else
    {
        ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    }
}

void MainWindow::keyPressEvent(QKeyEvent * event)
{
    switch(event->key())
    {
        case Qt::Key_Shift:
            ui->checkBoxMultiSelect->setChecked(false);
            break;
        default:
            QWidget::keyPressEvent(event);
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent * event)
{
    switch(event->key())
    {
        case Qt::Key_Shift:
            ui->checkBoxMultiSelect->setChecked(true);
            this->savePresetsToConf();
            break;
        default:
            QWidget::keyPressEvent(event);
    }
}
