#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QComboBox>

class QAction;
class QActionGroup;
class QMenu;

#include "configuration.h"
#include "dialogadditem.h"
#include <qcustomplot/qcustomplot.h>

#if __GNUC__
#include <sys/time.h>
#endif

#include "smartcard.h"
#include "commands.h"
#include "popup.h"

#define WIN_TEST
#undef WIN_TEST

#define TABLE_COL_CMD_NAME      0
#define TABLE_COL_DEVICE        1
#define TABLE_COL_APDU          2

#define DEBUG_EXCHANGE_BUFFERS
//#undef DEBUG_EXCHANGE_BUFFERS

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    void updateInfo();
    void findDevices_signal();

private slots:
    void on_pushButtonFindDevs_clicked();
    void LoadCfg_clicked();
    void EditCfg_clicked();
    void AddItem_clicked();
    void on_comboBoxPreset_currentIndexChanged(int index);
    void on_comboBoxAPDUSelect_currentIndexChanged(int index);
    void DelPreset_clicked();
    void DelSeqPreset_clicked();
    void DelItem_clicked();
    void about();
    void on_comboBoxDevice_currentIndexChanged(int index);
    void on_pushButtonConnect_clicked();
    void on_pushButtonSendAPDU_clicked();
    void on_pushButtonClearLog_clicked();
    void on_lineEditLEN_editingFinished();
    void showDeviceAttrib_triggered();
    void on_pushButtonAddTotable_clicked();
    void on_pushButtonStartTest_clicked();
    void StartTest_Selected_activated();
    void customMenuRequested(QPoint pos);
    void tableItemClicked(int row, int column);
    void AddSeqTestItem_clicked();
    void saveSeqConfiguration_triggered();
    void clearSeqConfiguration_triggered();
    void reloadConfiguration_triggered();
    void on_pushButtonCopyToClipboard_clicked();
    void on_comboBoxSeqPreset_currentIndexChanged(int index);
    void on_tableWidget_clicked(const QModelIndex &index);
    void on_pushButtonShowGraph_clicked();
    void deleteSelectedRows_pressed();
    void copySelectedRows_pressed();
    void pasteSelectedRows_pressed();
    void on_tableWidget_cellEntered(int row, int column);
    void on_checkBoxMultiSelect_toggled(bool checked);
    void keyPressEvent(QKeyEvent * event);
    void keyReleaseEvent(QKeyEvent * event);

private:
    Ui::MainWindow *ui;

    // Config
    QMenu *configMenu;
    QAction *loadPresetConf;
    QAction *editPresetConf;
    QAction *addPresetConf;
    QAction *delItemConf;
    QAction *delPresetConf;
    QAction *reloadPresetConf;

    // Test
    QMenu *seqTestMenu;
    QAction *startSeqTestConf;
    QAction *addSeqTestPreset;
    QAction *delSeqTestPreset;
    QAction *addCmd2SeqTestConf;
    QAction *saveSeqTestConf;
    QAction *clearSeqTestConf;

    // Spec Test
    QMenu *specTestMenu;
    QAction *partitionTest;

    // View
    QMenu *viewMenu;
    QAction *showDeviceInfoAct;
    QAction *showDeviceAttributes;

    // Help
    QMenu *helpMenu;
    QAction *aboutAct;
    QAction *aboutQtAct;

    conf::Configuration *_pConfig;
    config_preset_t _common_conf;

    bool _deviceInfoShow;
    bool _test_running;

    smcard::SmartCard _SCard;

    answerBuffer_t ansBuffer;

    int indexSeqPresUpdateInfo;
    int indexSeqCMDUpdateInfo;

    bool _needReselect;
    unsigned char _lastApp[13];

    // window
    int _curr_win_width;
    int _curr_win_height;

    // plot
    QCustomPlot *_customPlot;

    std::vector<int> _copyVectIndex;
    int _copySeqPreset;

    bool _failCheckInfo;
    bool _exclusiveConnect;

    PopUp *popUp;

private:
    bool updateLocalConfig();
    bool findDevices();
    bool addItemToTableByIndex(int index);
    void confSendUpdateInfo(int index_seq_pres, int index_seq_command);
    void showCurExecCommand();
    void setWidgetProperties(bool flag);

    bool savePresetsToConf();
    void printExchageBuffers(const char *cmd, const int cLength,
                             const char *rpl, const int rLength, unsigned time_ms);

    long scardExchange(const char *cmd,
                       int cLength, char *rpl, int *rLength);

    long scardExchange(const char *cmd,
                       int cLength, char *rpl, int *rLength,
                       unsigned &ex_time);

    template <class I> std::string n2hexstr(I w, size_t hex_len)
    {
        static const char* digits = "0123456789ABCDEF";
        std::string rc(hex_len,'0');
        for (size_t i=0, j=(hex_len-1)*4 ; i<hex_len; ++i,j-=4)
            rc[i] = digits[(w>>j) & 0x0f];
        return rc;
    }

protected:
    //void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
};

#endif // MAINWINDOW_H
