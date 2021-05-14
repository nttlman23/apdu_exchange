#ifndef DIALOGSETTESTITEM_H
#define DIALOGSETTESTITEM_H

#include <QDialog>
#include <configuration.h>

namespace Ui {
class DialogSetTestItem;
}

class DialogSetTestItem : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSetTestItem(QWidget *parent = 0);
    ~DialogSetTestItem();

public:
    bool setCurrentPreset(conf::Configuration *pConf,
                          config_preset_t* common_conf,
                          int seqPresetIndex,
                          int commandIndex);

private slots:
    void on_buttonBox_accepted();
    void on_checkLoadFromBuf1_toggled(bool checked);
    void on_checkLoadFromBuf2_toggled(bool checked);

private:
    Ui::DialogSetTestItem *ui;

private:
    conf::Configuration *_pConf;
    bool _itemSetted;
    int _currentSequenceIndex;
    int _currCommandIndex;
    config_preset_t* _common_conf;

public:
    bool saveTestItemSets();
};

#endif // DIALOGSETTESTITEM_H
