#include <QDebug>
#include <QMessageBox>

#include "dialogadditem.h"
#include "ui_dialogadditem.h"

#include "logger.h"

using namespace conf;

DialogAddItem::DialogAddItem(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAddItem),
    _itemAdded(false),
    _common_confP(nullptr),
    _preset_index(-1),
    _cmd_index(-1),
    _curr_item(nullptr)
{
    ui->setupUi(this);
    ui->checkBoxBase->setChecked(true);
    this->setFixedSize(400, 300);
    this->setWindowFlags(Qt::WindowStaysOnTopHint);
    ui->comboBoxExistPreset->hide();
}

DialogAddItem::~DialogAddItem()
{
    _common_confP = nullptr;

    if (_cmd_index < 0)
    {
        delete _curr_item;
    }
    else
    {
        _curr_item = nullptr;
    }

    delete ui;
}

bool DialogAddItem::setCurrentPreset(conf::Configuration *pConf,
                                     config_preset_t *common_conf,
                                     int pres_index,
                                     int cmd_index,
                                     bool fill)
{
    if (!pConf)
    {
        DEBUG(DBG_ERROR, "Bad Data");
        return false;
    }

    _pConf = pConf;

    if (common_conf)
    {
        // preset exist
        _common_confP = common_conf;
        _preset_index = pres_index;
        _cmd_index = cmd_index;
    }
    else
    {
        return false;
    }

    if (_preset_index >= 0)
    {
        ui->lineEditPresetName->setText(_common_confP->presetCommandsV[_preset_index].preset_name.c_str());
    }

    if (fill)
    {
        _curr_item = &_common_confP->presetCommandsV[_preset_index].commandConf[_cmd_index];
        ui->lineEditCMDName->setText(_curr_item->commandDesc.c_str());

        ui->lineEditCLA->setText(QString("%1").arg(_curr_item->cla, 2, 16, QChar('0')).toUpper());
        ui->lineEditINS->setText(QString("%1").arg(_curr_item->ins, 2, 16, QChar('0')).toUpper());
        ui->lineEditP1->setText(QString("%1").arg(_curr_item->p1, 2, 16, QChar('0')).toUpper());
        ui->lineEditP2->setText(QString("%1").arg(_curr_item->p2, 2, 16, QChar('0')).toUpper());
        ui->lineEditLEN->setText(QString("%1").arg(_curr_item->len, 2, 16, QChar('0')).toUpper());

        if (!_curr_item->data_field.empty())
        {
            ui->lineEditDataField->setText(_curr_item->data_field.c_str());
        }
    }
    else
    {
        _curr_item = new conf_item_t;
        setItemToDefault();
    }

    return true;
}

void DialogAddItem::on_pushButtonAdd_clicked()
{
    addNewItemToConf();
}

void DialogAddItem::on_buttonBox_accepted()
{
    addNewItemToConf();
}

bool DialogAddItem::addNewItemToConf()
{
    if (!_itemAdded)
    {
        DEBUG(DBG_DEBUG, "add new item");

        bool ok;
        int base = 16;

        if (!ui->checkBoxBase->isChecked())
        {
            base = 10;
        }

        if (!ui->checkBoxSelectExist->isChecked() &&
                ui->lineEditPresetName->text().isEmpty())
        {
            QMessageBox messageBox;
            messageBox.critical(0,"Warning","Device name is missing");
            messageBox.setFixedSize(500,200);
            return false;
        }
        else if (ui->lineEditCMDName->text().isEmpty())
        {
            QMessageBox messageBox;
            messageBox.critical(0,"Warning","APDU name is missing");
            messageBox.setFixedSize(500,200);
            return false;
        }
        else if(ui->lineEditCLA->text().isEmpty())
        {
            QMessageBox messageBox;
            messageBox.critical(0,"Warning","CLA is missing");
            messageBox.setFixedSize(500,200);
            return false;
        }
        else if(ui->lineEditINS->text().isEmpty())
        {
            QMessageBox messageBox;
            messageBox.critical(0,"Warning","INS is missing");
            messageBox.setFixedSize(500,200);
            return false;
        }
        else if(ui->lineEditP1->text().isEmpty())
        {
            QMessageBox messageBox;
            messageBox.critical(0,"Warning","P1 is missing");
            messageBox.setFixedSize(500,200);
            return false;
        }
        else if(ui->lineEditP2->text().isEmpty())
        {
            QMessageBox messageBox;
            messageBox.critical(0,"Warning","P2 is missing");
            messageBox.setFixedSize(500,200);
            return false;
        }

        _curr_item->commandDesc = ui->lineEditCMDName->text().toStdString();
        _curr_item->cla = ui->lineEditCLA->text().toInt(&ok,base);
        _curr_item->ins = ui->lineEditINS->text().toInt(&ok,base);
        _curr_item->p1 = ui->lineEditP1->text().toInt(&ok,base);
        _curr_item->p2 = ui->lineEditP2->text().toInt(&ok,base);

        if(ui->lineEditLEN->text().isEmpty())
        {
            _curr_item->len = 0x00;
        }
        else
        {
            _curr_item->len = ui->lineEditLEN->text().toInt(&ok,base);
        }

        if (!ui->lineEditDataField->text().isEmpty())
        {
            _curr_item->data_field = ui->lineEditDataField->text().toStdString();
            // remove spaces
            _curr_item->data_field.erase(std::remove(_curr_item->data_field.begin(),
                                                       _curr_item->data_field.end(),' '),
                                           _curr_item->data_field.end());
        }

        if (ui->checkBoxSelectExist->isChecked())
        {
            if (_preset_index != ui->comboBoxExistPreset->currentIndex())
            {
                _preset_index = ui->comboBoxExistPreset->currentIndex();
                _common_confP->presetCommandsV[_preset_index].commandConf.push_back(*_curr_item);
            }
            else
            {
                if (_cmd_index < 0)
                {
                    _common_confP->presetCommandsV[_preset_index].commandConf.push_back(*_curr_item);
                }
                else
                {
                    _common_confP->presetCommandsV[_preset_index].commandConf[_cmd_index].commandDesc = _curr_item->commandDesc;
                    _common_confP->presetCommandsV[_preset_index].commandConf[_cmd_index].cla = _curr_item->cla;
                    _common_confP->presetCommandsV[_preset_index].commandConf[_cmd_index].ins = _curr_item->ins;
                    _common_confP->presetCommandsV[_preset_index].commandConf[_cmd_index].p1 = _curr_item->p1;
                    _common_confP->presetCommandsV[_preset_index].commandConf[_cmd_index].p2 = _curr_item->p2;
                    _common_confP->presetCommandsV[_preset_index].commandConf[_cmd_index].len = _curr_item->len;
                    _common_confP->presetCommandsV[_preset_index].commandConf[_cmd_index].data_field = _curr_item->data_field;
                }
            }
        }

        _itemAdded = true;
    }
    else
    {
        DEBUG(DBG_WARN, "Item already added");
    }

    return true;
}

void DialogAddItem::setItemToDefault()
{
    if (_curr_item)
    {
        _curr_item->cla = 0x00;
        _curr_item->commandDesc = "";
        _curr_item->data_field = "";
        _curr_item->ins = 0x00;
        _curr_item->len = 0x00;
        _curr_item->p1 = 0x00;
        _curr_item->p2 = 0x00;
    }
}

void DialogAddItem::on_checkBoxSelectExist_toggled(bool checked)
{
    if (checked)
    {
        size_t preset_cnt = _common_confP->presetCommandsV.size();

        if (preset_cnt > 0)
        {
            int linEd_X = ui->lineEditPresetName->x();
            int linEd_Y = ui->lineEditPresetName->y();

            int comBox_X = ui->comboBoxExistPreset->x();
            int comBox_Y = ui->comboBoxExistPreset->y();

            ui->lineEditPresetName->hide();
            ui->comboBoxExistPreset->show();

            ui->comboBoxExistPreset->move(linEd_X, linEd_Y);
            ui->lineEditPresetName->move(comBox_X, comBox_Y);

            for (int i = 0;i < preset_cnt;i++)
            {
                ui->comboBoxExistPreset->addItem(_common_confP->presetCommandsV[i].preset_name.c_str());
            }

            ui->comboBoxExistPreset->setCurrentIndex(_preset_index);
        }
    }
    else
    {
        ui->comboBoxExistPreset->hide();

        int linEd_X = ui->lineEditPresetName->x();
        int linEd_Y = ui->lineEditPresetName->y();

        int comBox_X = ui->comboBoxExistPreset->x();
        int comBox_Y = ui->comboBoxExistPreset->y();

        ui->comboBoxExistPreset->move(linEd_X, linEd_Y);
        ui->lineEditPresetName->move(comBox_X, comBox_Y);

        ui->lineEditPresetName->show();
        ui->comboBoxExistPreset->clear();
    }
}
