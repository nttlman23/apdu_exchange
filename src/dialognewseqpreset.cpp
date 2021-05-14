#include <QDebug>
#include <QMessageBox>

#include "dialognewseqpreset.h"
#include "ui_dialognewseqpreset.h"
#include "logger.h"

DialogNewSeqPreset::DialogNewSeqPreset(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogNewSeqPreset)
{
    ui->setupUi(this);

    this->setWindowFlags(Qt::WindowStaysOnTopHint);
}

DialogNewSeqPreset::~DialogNewSeqPreset()
{
    this->_pConf = nullptr;
    this->_common_confP = nullptr;
    delete ui;
}

bool DialogNewSeqPreset::setCurrentConfig(conf::Configuration *pConf,
                      config_preset_t *common_conf)
{
    if (!pConf && !common_conf)
    {
        DEBUG(DBG_ERROR, "Bad Data");
        return false;
    }

    this->_pConf = pConf;
    this->_common_confP = common_conf;

    return true;
}

void DialogNewSeqPreset::on_buttonBox_accepted()
{
    if (ui->lineEditPresetName->text().isEmpty())
    {
        QMessageBox messageBox;
        messageBox.critical(0,"Warning","Seq Presetn name is missing");
        messageBox.setFixedSize(500,200);
        return;
    }

    _seqPresetName = ui->lineEditPresetName->text().toStdString();

//    // replace spaces!!!!
//    for(std::string::iterator it = _seqPresetName.begin(); it != _seqPresetName.end(); ++it)
//    {
//        if(*it == ' ' ||
//                *it == '\'' ||
//                *it == '(' ||
//                *it == ')' ||
//                *it == '[' ||
//                *it == ']' ||
//                *it == '{' ||
//                *it == '}' ||
//                *it == '.' ||
//                *it == ',')
//        {
//            *it = '_';
//        }
//    }
    seq_preset_t temp_seq_pres;
    temp_seq_pres.seqPresetName = this->_seqPresetName;

    this->_common_confP->presetSequenceV.push_back(temp_seq_pres);
}
