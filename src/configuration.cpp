
#include "configuration.h"
#include <QDebug>
#include <algorithm>
#include <QMessageBox>
#include <QXmlStreamReader>
#include <QXmlStreamAttribute>
#include <QDebug>

#include "logger.h"

//using namespace libconfig;
using namespace conf;

Configuration::Configuration()
    :  _configured(false)
{
    conf_file_path = "";
}

Configuration::~Configuration()
{

}

const char * Configuration::getConfFilePath()
{
    return conf_file_path.c_str();
}

void Configuration::setConfFilePath(const char * filePath)
{
    conf_file_path = filePath;
}

bool Configuration::readConfiguration()
{
    if (conf_file_path.empty())
    {
#ifdef DEBUG_LOAD_CONFIG
        DEBUG(DBG_ERROR, "Path is empty");
#endif // DEBUG_LOAD_CONFIG
        _configured = false;
        return false;
    }

    QFile file(conf_file_path.c_str());

    if (!file.exists())
    {
        QMessageBox messageBox;
        messageBox.critical(0,"Error","File not Found");
        messageBox.setFixedSize(500,200);

        return false;
    }

    return true;
}

bool Configuration::savePresetsConf(config_preset_t& common_conf)
{
    if (!_configured)
    {
        return false;
    }

    bool ret = false;

    QFile file(conf_file_path.c_str());

    if (!file.open(QIODevice::WriteOnly))
    {
        QMessageBox messageBox;
        messageBox.critical(0,"Error","File not Found");
        messageBox.setFixedSize(500,200);

        return ret;
    }

    QXmlStreamWriter xmlWriter(&file);
    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument();

    xmlWriter.writeStartElement("configuration");
    xmlWriter.writeStartElement("commands");

    for (int i = 0;i < common_conf.presetCommandsV.size();i++)
    {
        xmlWriter.writeStartElement("preset");
        xmlWriter.writeAttribute("id",common_conf.presetCommandsV[i].preset_name.c_str());

        for (std::vector<conf_item_t>::iterator it = common_conf.presetCommandsV[i].commandConf.begin();
             it != common_conf.presetCommandsV[i].commandConf.end() ; it++)
        {
            xmlWriter.writeStartElement("command");
            xmlWriter.writeAttribute("name",it->commandDesc.c_str());
            xmlWriter.writeAttribute("cla",QString("%1").arg(it->cla, 2, 16, QChar('0')).toUpper());
            xmlWriter.writeAttribute("ins",QString("%1").arg(it->ins, 2, 16, QChar('0')).toUpper());
            xmlWriter.writeAttribute("p1",QString("%1").arg(it->p1, 2, 16, QChar('0')).toUpper());
            xmlWriter.writeAttribute("p2",QString("%1").arg(it->p2, 2, 16, QChar('0')).toUpper());
            xmlWriter.writeAttribute("len",QString("%1").arg(it->len, 2, 16, QChar('0')).toUpper());
            xmlWriter.writeAttribute("data",it->data_field.c_str());
            // command
            xmlWriter.writeEndElement();
        }

        // preset
        xmlWriter.writeEndElement();
    }

    // commands
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("sequences");


    for (auto i = 0;i < common_conf.presetSequenceV.size();i++)
    {
        xmlWriter.writeStartElement("sequence");
        xmlWriter.writeAttribute("id",common_conf.presetSequenceV[i].seqPresetName.c_str());

        for (std::vector<test_conf_t>::iterator it = common_conf.presetSequenceV[i].seqTest.begin();
             it != common_conf.presetSequenceV[i].seqTest.end() ; it++)
        {
            xmlWriter.writeStartElement("command");
            xmlWriter.writeAttribute("name",it->commandDesc.c_str());
            xmlWriter.writeAttribute("cla",QString("%1").arg(it->cla, 2, 16, QChar('0')).toUpper());
            xmlWriter.writeAttribute("ins",QString("%1").arg(it->ins, 2, 16, QChar('0')).toUpper());
            xmlWriter.writeAttribute("p1",QString("%1").arg(it->p1, 2, 16, QChar('0')).toUpper());
            xmlWriter.writeAttribute("p2",QString("%1").arg(it->p2, 2, 16, QChar('0')).toUpper());
            xmlWriter.writeAttribute("len",QString("%1").arg(it->len, 2, 16, QChar('0')).toUpper());
            xmlWriter.writeAttribute("data",it->data_field.c_str());
            xmlWriter.writeAttribute("ansLen",QString::number(it->ansLen));
            xmlWriter.writeAttribute("ansLenMin",QString::number(it->ansLenMin));
            xmlWriter.writeAttribute("ansLenMax",QString::number(it->ansLenMax));
            xmlWriter.writeAttribute("SW1",QString("%1").arg(it->SW1, 2, 16, QChar('0')).toUpper());
            xmlWriter.writeAttribute("SW2",QString("%1").arg(it->SW2, 2, 16, QChar('0')).toUpper());
            xmlWriter.writeAttribute("save2Buf1",(it->save2Buf1?"true":"false"));
            xmlWriter.writeAttribute("save2Buf2",(it->save2Buf2?"true":"false"));
            xmlWriter.writeAttribute("loadFromBuf1",(it->loadFromBuf1?"true":"false"));
            xmlWriter.writeAttribute("loadFromBuf2",(it->loadFromBuf2?"true":"false"));
            xmlWriter.writeAttribute("resetUSB",(it->resetUSB?"true":"false"));

            // command
            xmlWriter.writeEndElement();
        }

        // sequence
        xmlWriter.writeEndElement();
    }

    // sequences
    xmlWriter.writeEndElement();

    // configuration
    xmlWriter.writeEndElement();

    xmlWriter.writeEndDocument();
    file.close();

    ret = true;

    return ret;
}

bool Configuration::isConfigured()
{
    return _configured;
}

void Configuration::setDefaultTestConf(test_conf_t *test_conf)
{
    if (!test_conf)
    {
        return;
    }

    test_conf->cla = 0x00;
    test_conf->commandDesc = "";
    test_conf->data_field = "";
    test_conf->ins = 0x00;
    test_conf->len = 0x00;
    test_conf->p1 = 0x00;
    test_conf->p2 = 0x00;
    test_conf->ansLen = 2;
    test_conf->ansLenMin = 2;
    test_conf->ansLenMax = 2;
    test_conf->SW1 = 0x90;
    test_conf->SW2 = 0x00;
    test_conf->save2Buf1 = false;
    test_conf->save2Buf2 = false;
    test_conf->loadFromBuf1 = false;
    test_conf->loadFromBuf2 = false;
    test_conf->resetUSB = false;
}

bool Configuration::fillPresetsConf(config_preset_t& common_conf)
{
    common_conf.presetCommandsV.clear();
    common_conf.presetSequenceV.clear();

    QFile file(conf_file_path.c_str());

    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox messageBox;
        messageBox.critical(0,"Error","File not Found");
        messageBox.setFixedSize(500,200);

        return false;
    }

    QXmlStreamReader xmlReader;
    xmlReader.setDevice(&file);
    xmlReader.readNextStartElement();

    // <configuration>
    //      <commands>
    //          <preset id="JC-Flash2">
    //              <command name="Update SD" cla="80" ins="72" .........../>
    //          </preset>
    //      </commands>
    //      <sequences>
    //          <sequence id="Common">
    //              <command name="Select KT 2" cla="00" ins="A4" ........./>
    //          </sequence>
    //      </sequences>
    // </configuration>

    while(!xmlReader.atEnd())
    {
        if(xmlReader.isStartElement()) // configuration
        {
            if(xmlReader.name() == "configuration")
            {
                xmlReader.readNextStartElement();
                if (xmlReader.isEndElement())
                {
                    xmlReader.readNextStartElement();
                }

                if (xmlReader.name() == "commands")
                {
                    xmlReader.readNextStartElement();
                    if (xmlReader.isEndElement())
                    {
                        xmlReader.readNextStartElement();
                    }

                    while (xmlReader.name() == "preset")
                    {
                        if (xmlReader.isEndElement())
                        {
                            xmlReader.readNextStartElement();   // </preset>
                            continue;
                        }

                        preset_conf_t temp_preset;

                        QXmlStreamAttributes command_name = xmlReader.attributes();

                        temp_preset.preset_name = command_name.begin()->value().toString().toStdString();

                        xmlReader.readNextStartElement();
                        if (xmlReader.isEndElement())
                        {
                            xmlReader.readNextStartElement();
                        }

                        while (xmlReader.name() == "command")
                        {
                            conf_item_t temp_conf;

                            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes())
                            {
                                if (attr.name() == "name")
                                {
                                    temp_conf.commandDesc = attr.value().toString().toStdString();
                                }
                                else if (attr.name() == "cla")
                                {
                                    temp_conf.cla = attr.value().toInt(nullptr,16);
                                }
                                else if (attr.name() == "ins")
                                {
                                    temp_conf.ins = attr.value().toInt(nullptr,16);
                                }
                                else if (attr.name() == "p1")
                                {
                                    temp_conf.p1 = attr.value().toInt(nullptr,16);
                                }
                                else if (attr.name() == "p2")
                                {
                                    temp_conf.p2 = attr.value().toInt(nullptr,16);
                                }
                                else if (attr.name() == "len")
                                {
                                    temp_conf.len = attr.value().toInt(nullptr,16);
                                }
                                else if (attr.name() == "data")
                                {
                                    temp_conf.data_field = attr.value().toString().toStdString();
                                }
                            }


                            xmlReader.readNextStartElement();
                            if (xmlReader.isEndElement())
                            {
                                xmlReader.readNextStartElement();
                            }

                            temp_preset.commandConf.push_back(temp_conf);
                        }

                        common_conf.presetCommandsV.push_back(temp_preset);
                    }
                }

                xmlReader.readNextStartElement();

                if (xmlReader.name() == "sequences")
                {
                    xmlReader.readNextStartElement();
                    if (xmlReader.isEndElement())
                    {
                        xmlReader.readNextStartElement();
                    }

                    while (xmlReader.name() == "sequence")
                    {
                        if (xmlReader.isEndElement())
                        {
                            xmlReader.readNextStartElement();   // </preset>
                            continue;
                        }

                        seq_preset_t temp_preset_seq;

                        QXmlStreamAttributes command_name = xmlReader.attributes();

                        temp_preset_seq.seqPresetName = command_name.begin()->value().toString().toStdString();

                        xmlReader.readNextStartElement();
                        if (xmlReader.isEndElement())
                        {
                            xmlReader.readNextStartElement();
                        }

                        while (xmlReader.name() == "command")
                        {
                            test_conf_t temp_conf_seq;

                            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes())
                            {
                                if (attr.name() == "name")
                                {
                                    temp_conf_seq.commandDesc = attr.value().toString().toStdString();
                                }
                                else if (attr.name() == "cla")
                                {
                                    temp_conf_seq.cla = attr.value().toInt(nullptr,16);
                                }
                                else if (attr.name() == "ins")
                                {
                                    temp_conf_seq.ins = attr.value().toInt(nullptr,16);
                                }
                                else if (attr.name() == "p1")
                                {
                                    temp_conf_seq.p1 = attr.value().toInt(nullptr,16);
                                }
                                else if (attr.name() == "p2")
                                {
                                    temp_conf_seq.p2 = attr.value().toInt(nullptr,16);
                                }
                                else if (attr.name() == "len")
                                {
                                    temp_conf_seq.len = attr.value().toInt(nullptr,16);
                                }
                                else if (attr.name() == "data")
                                {
                                    temp_conf_seq.data_field = attr.value().toString().toStdString();
                                }
                                else if (attr.name() == "ansLen")
                                {
                                    temp_conf_seq.ansLen = attr.value().toInt(nullptr,10);
                                }
                                else if (attr.name() == "ansLenMin")
                                {
                                    temp_conf_seq.ansLenMin = attr.value().toInt(nullptr,10);
                                }
                                else if (attr.name() == "ansLenMax")
                                {
                                    temp_conf_seq.ansLenMax = attr.value().toInt(nullptr,10);
                                }
                                else if (attr.name() == "SW1")
                                {
                                    temp_conf_seq.SW1 = attr.value().toInt(nullptr,16);
                                }
                                else if (attr.name() == "SW2")
                                {
                                    temp_conf_seq.SW2 = attr.value().toInt(nullptr,16);
                                }
                                else if (attr.name() == "save2Buf1")
                                {
                                    temp_conf_seq.save2Buf1 = (attr.value() == "true")? true:false;
                                }
                                else if (attr.name() == "save2Buf2")
                                {
                                    temp_conf_seq.save2Buf2 = (attr.value() == "true")? true:false;
                                }
                                else if (attr.name() == "loadFromBuf1")
                                {
                                    temp_conf_seq.loadFromBuf1 = (attr.value() == "true")? true:false;
                                }
                                else if (attr.name() == "loadFromBuf2")
                                {
                                    temp_conf_seq.loadFromBuf2 = (attr.value() == "true")? true:false;
                                }
                                else if (attr.name() == "resetUSB")
                                {
                                    temp_conf_seq.resetUSB = (attr.value() == "true")? true:false;
                                }
                            }

                            xmlReader.readNextStartElement();
                            if (xmlReader.isEndElement())
                            {
                                xmlReader.readNextStartElement();
                            }

                            temp_preset_seq.seqTest.push_back(temp_conf_seq);
                        }

                        common_conf.presetSequenceV.push_back(temp_preset_seq);
                    }
                }
            }
        }

        xmlReader.readNextStartElement();
    }

    file.close();

    _configured = true;

    return true;
}


bool Configuration::removePreset(config_preset_t& common_conf, int presetIndex)
{
    common_conf.presetCommandsV.erase(common_conf.presetCommandsV.begin() +
                                       presetIndex);
    return true;
}

bool Configuration::removeSequence(config_preset_t& common_conf, int sequenceIndex)
{
    common_conf.presetSequenceV.erase(common_conf.presetSequenceV.begin() +
                                       sequenceIndex);
    return true;
}

bool Configuration::removePresetItem(config_preset_t& common_conf, int presetIndex, int commandIndex)
{
    common_conf.presetSequenceV[presetIndex].seqTest.erase(common_conf.presetSequenceV[presetIndex].seqTest.begin() + commandIndex);

    return true;
}

bool Configuration::removeSeqPresetItem(config_preset_t& common_conf, int seqPresetIndex, int commandIndex)
{
    common_conf.presetSequenceV[seqPresetIndex].seqTest.erase(common_conf.presetSequenceV[seqPresetIndex].seqTest.begin() + commandIndex);

    return true;
}

bool Configuration::clearSequenceList(config_preset_t& common_conf, int seqPresetIndex)
{
    common_conf.presetSequenceV[seqPresetIndex].seqTest.resize(0);
    return true;
}
