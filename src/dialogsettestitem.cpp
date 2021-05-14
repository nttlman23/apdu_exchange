#include "dialogsettestitem.h"
#include "ui_dialogsettestitem.h"
#include <QDebug>

#include "logger.h"

using namespace conf;

DialogSetTestItem::DialogSetTestItem(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSetTestItem),
    _pConf(nullptr),
    _itemSetted(false),
    _currentSequenceIndex(-1),
    _currCommandIndex(-1),
    _common_conf(nullptr)
{
    ui->setupUi(this);

    this->setFixedSize(this->size());
    this->setWindowFlags(Qt::WindowStaysOnTopHint);

    _itemSetted = false;

    ui->groupBoxCommand->setStyleSheet("QGroupBox  { border: 2px solid gray; border-radius: 3px; }");
    ui->groupBoxAnswer->setStyleSheet("QGroupBox  { border: 2px solid gray; border-radius: 3px; }");

    ui->lineEditPresetName->setReadOnly(true);
}

DialogSetTestItem::~DialogSetTestItem()
{
    _pConf = nullptr;
    _common_conf = nullptr;
    delete ui;
}

bool DialogSetTestItem::setCurrentPreset(conf::Configuration *pConf,
                                         config_preset_t* common_conf,
                                         int seqPresetIndex,
                                         int commandIndex)
{
    if (!pConf)
    {
        return false;
    }

    if (seqPresetIndex < 0 || commandIndex < 0)
    {
        return false;
    }

    _pConf = pConf;

    _currentSequenceIndex = seqPresetIndex;
    _currCommandIndex = commandIndex;
    _common_conf = common_conf;

    ui->lineEditPresetName->setText(_common_conf->presetSequenceV[_currentSequenceIndex].seqPresetName.c_str());
    ui->lineEditCommandName->setText(_common_conf->presetSequenceV[_currentSequenceIndex].seqTest[_currCommandIndex].commandDesc.c_str());
    ui->lineEditCLA->setText(QString("%1").arg(_common_conf->presetSequenceV[_currentSequenceIndex].seqTest[_currCommandIndex].cla, 2, 16, QChar('0')).toUpper());
    ui->lineEditINS->setText(QString("%1").arg(_common_conf->presetSequenceV[_currentSequenceIndex].seqTest[_currCommandIndex].ins, 2, 16, QChar('0')).toUpper());
    ui->lineEditP1->setText(QString("%1").arg(_common_conf->presetSequenceV[_currentSequenceIndex].seqTest[_currCommandIndex].p1, 2, 16, QChar('0')).toUpper());
    ui->lineEditP2->setText(QString("%1").arg(_common_conf->presetSequenceV[_currentSequenceIndex].seqTest[_currCommandIndex].p2, 2, 16, QChar('0')).toUpper());
    ui->lineEditLEN->setText(QString("%1").arg(_common_conf->presetSequenceV[_currentSequenceIndex].seqTest[_currCommandIndex].len, 2, 16, QChar('0')).toUpper());

    if (!_common_conf->presetSequenceV[_currentSequenceIndex].seqTest[_currCommandIndex].data_field.empty())
    {
        ui->lineEditDataField->setText(_common_conf->presetSequenceV[_currentSequenceIndex].seqTest[_currCommandIndex].data_field.c_str());
    }

    ui->lineEditAnsLen->setText(QString::number(_common_conf->presetSequenceV[_currentSequenceIndex].seqTest[_currCommandIndex].ansLen - 2));
    ui->lineEditAnsLenMin->setText(QString::number(_common_conf->presetSequenceV[_currentSequenceIndex].seqTest[_currCommandIndex].ansLenMin - 2));
    ui->lineEditAnsLenMax->setText(QString::number(_common_conf->presetSequenceV[_currentSequenceIndex].seqTest[_currCommandIndex].ansLenMax - 2));
    ui->lineEditSW1->setText(QString("%1").arg(_common_conf->presetSequenceV[_currentSequenceIndex].seqTest[_currCommandIndex].SW1, 2, 16, QChar('0')).toUpper());
    ui->lineEditSW2->setText(QString("%1").arg(_common_conf->presetSequenceV[_currentSequenceIndex].seqTest[_currCommandIndex].SW2, 2, 16, QChar('0')).toUpper());

    if(_common_conf->presetSequenceV[_currentSequenceIndex].seqTest[_currCommandIndex].save2Buf1)
    {
        ui->checkSave2Buf1->setChecked(true);
    }
    else
    {
        ui->checkSave2Buf1->setChecked(false);
    }

    if(_common_conf->presetSequenceV[_currentSequenceIndex].seqTest[_currCommandIndex].save2Buf2)
    {
        ui->checkSave2Buf2->setChecked(true);
    }
    else
    {
        ui->checkSave2Buf2->setChecked(false);
    }

    if(_common_conf->presetSequenceV[_currentSequenceIndex].seqTest[_currCommandIndex].loadFromBuf1)
    {
        ui->checkLoadFromBuf1->setChecked(true);
    }
    else
    {
        ui->checkLoadFromBuf1->setChecked(false);
    }

    if(_common_conf->presetSequenceV[_currentSequenceIndex].seqTest[_currCommandIndex].loadFromBuf2)
    {
        ui->checkLoadFromBuf2->setChecked(true);
    }
    else
    {
        ui->checkLoadFromBuf2->setChecked(false);
    }

    if(_common_conf->presetSequenceV[_currentSequenceIndex].seqTest[_currCommandIndex].resetUSB)
    {
        ui->checkBoxResetUSB->setChecked(true);
    }
    else
    {
        ui->checkBoxResetUSB->setChecked(false);
    }

    return true;
}

void DialogSetTestItem::on_buttonBox_accepted()
{
    this->saveTestItemSets();
}

bool DialogSetTestItem::saveTestItemSets()
{
    bool ok = true;

    _common_conf->presetSequenceV[_currentSequenceIndex].seqTest[_currCommandIndex].commandDesc = ui->lineEditCommandName->text().toStdString();
    _common_conf->presetSequenceV[_currentSequenceIndex].seqTest[_currCommandIndex].cla = ui->lineEditCLA->text().toInt(&ok,16);
    _common_conf->presetSequenceV[_currentSequenceIndex].seqTest[_currCommandIndex].ins = ui->lineEditINS->text().toInt(&ok,16);
    _common_conf->presetSequenceV[_currentSequenceIndex].seqTest[_currCommandIndex].p1 = ui->lineEditP1->text().toInt(&ok,16);
    _common_conf->presetSequenceV[_currentSequenceIndex].seqTest[_currCommandIndex].p2 = ui->lineEditP2->text().toInt(&ok,16);
    _common_conf->presetSequenceV[_currentSequenceIndex].seqTest[_currCommandIndex].len = ui->lineEditLEN->text().toInt(&ok,16);

    if (ui->lineEditDataField->text().isEmpty())
    {
        _common_conf->presetSequenceV[_currentSequenceIndex].seqTest[_currCommandIndex].len = 0;
        _common_conf->presetSequenceV[_currentSequenceIndex].seqTest[_currCommandIndex].data_field.clear();
    }
    else
    {
        _common_conf->presetSequenceV[_currentSequenceIndex].seqTest[_currCommandIndex].data_field = ui->lineEditDataField->text().toStdString();

        if (!(ui->lineEditDataField->text().size() % 2))
        {
            _common_conf->presetSequenceV[_currentSequenceIndex].seqTest[_currCommandIndex].len = ui->lineEditDataField->text().size() / 2;
        }
        else
        {
            _common_conf->presetSequenceV[_currentSequenceIndex].seqTest[_currCommandIndex].len = ui->lineEditDataField->text().size() / 2 + 1;
        }
    }

     // remove spaces
     _common_conf->presetSequenceV[_currentSequenceIndex].seqTest[_currCommandIndex].data_field.erase(std::remove(_common_conf->presetSequenceV[_currentSequenceIndex].seqTest[_currCommandIndex].data_field.begin(),
                                                                      _common_conf->presetSequenceV[_currentSequenceIndex].seqTest[_currCommandIndex].data_field.end(),' '),
                                                          _common_conf->presetSequenceV[_currentSequenceIndex].seqTest[_currCommandIndex].data_field.end());

    _common_conf->presetSequenceV[_currentSequenceIndex].seqTest[_currCommandIndex].ansLen = ui->lineEditAnsLen->text().toInt(&ok,10) + 2;
    _common_conf->presetSequenceV[_currentSequenceIndex].seqTest[_currCommandIndex].ansLenMin = ui->lineEditAnsLenMin->text().toInt(&ok,10) + 2;
    _common_conf->presetSequenceV[_currentSequenceIndex].seqTest[_currCommandIndex].ansLenMax = ui->lineEditAnsLenMax->text().toInt(&ok,10) + 2;
    _common_conf->presetSequenceV[_currentSequenceIndex].seqTest[_currCommandIndex].SW1 = ui->lineEditSW1->text().toInt(&ok,16);
    _common_conf->presetSequenceV[_currentSequenceIndex].seqTest[_currCommandIndex].SW2 = ui->lineEditSW2->text().toInt(&ok,16);

    _common_conf->presetSequenceV[_currentSequenceIndex].seqTest[_currCommandIndex].save2Buf1 = ui->checkSave2Buf1->isChecked();
    _common_conf->presetSequenceV[_currentSequenceIndex].seqTest[_currCommandIndex].save2Buf2 = ui->checkSave2Buf2->isChecked();
    _common_conf->presetSequenceV[_currentSequenceIndex].seqTest[_currCommandIndex].loadFromBuf1 = ui->checkLoadFromBuf1->isChecked();
    _common_conf->presetSequenceV[_currentSequenceIndex].seqTest[_currCommandIndex].loadFromBuf2 = ui->checkLoadFromBuf2->isChecked();
    _common_conf->presetSequenceV[_currentSequenceIndex].seqTest[_currCommandIndex].resetUSB = ui->checkBoxResetUSB->isChecked();

    _itemSetted = true;

    return true;
}


void DialogSetTestItem::on_checkLoadFromBuf1_toggled(bool checked)
{
    if (checked)
    {
        ui->checkLoadFromBuf2->setChecked(false);
    }
}

void DialogSetTestItem::on_checkLoadFromBuf2_toggled(bool checked)
{
    if (checked)
    {
        ui->checkLoadFromBuf1->setChecked(false);
    }
}
