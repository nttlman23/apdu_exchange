#ifndef DIALOGNEWSEQPRESET_H
#define DIALOGNEWSEQPRESET_H

#include <QDialog>
#include "configuration.h"

namespace Ui {
class DialogNewSeqPreset;
}

class DialogNewSeqPreset : public QDialog
{
    Q_OBJECT

public:
    explicit DialogNewSeqPreset(QWidget *parent = 0);
    ~DialogNewSeqPreset();

    bool setCurrentConfig(conf::Configuration *pConf,
                          config_preset_t *common_conf);

private slots:
    void on_buttonBox_accepted();

private:
    Ui::DialogNewSeqPreset *ui;

    conf::Configuration *_pConf;
    config_preset_t *_common_confP;
    std::string _seqPresetName;
};

#endif // DIALOGNEWSEQPRESET_H
