#ifndef DIALOGADDITEM_H
#define DIALOGADDITEM_H

#include <QDialog>

#include "configuration.h"

namespace Ui {
class DialogAddItem;
}

class DialogAddItem : public QDialog
{
    Q_OBJECT

public:
    explicit DialogAddItem(QWidget *parent = 0);
    ~DialogAddItem();
    bool setCurrentPreset(conf::Configuration *pConf,
                          config_preset_t *common_conf,
                          int pres_index,
                          int cmd_index,
                          bool fill);

private slots:
    void on_pushButtonAdd_clicked();
    void on_buttonBox_accepted();

    void on_checkBoxSelectExist_toggled(bool checked);

private:
    Ui::DialogAddItem *ui;
    conf::Configuration *_pConf;
    bool _itemAdded;
    config_preset_t *_common_confP;
    int _preset_index;
    int _cmd_index;
    conf_item_t *_curr_item;

private:
    bool addNewItemToConf();
    void setItemToDefault();
};

#endif // DIALOGADDITEM_H
