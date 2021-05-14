#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <string>
//#include <libconfig.h++>
#include <vector>
#include <QFile>

#define DEBUG_LOAD_CONFIG
#undef DEBUG_LOAD_CONFIG

#define CONFIG_SECTION_PRESET       "config"
#define CONFIG_SECTION_SEQUENCE     "sequence"

typedef struct conf_item_s
{
    std::string commandDesc;
    int cla;
    int ins;
    int p1;
    int p2;
    int len;
    std::string data_field;
} conf_item_t;

typedef struct test_conf_s
{
    std::string commandDesc;
    int cla;
    int ins;
    int p1;
    int p2;
    int len;
    std::string data_field;
    int ansLen;
    int ansLenMin;
    int ansLenMax;
    int SW1;
    int SW2;
    bool save2Buf1;
    bool save2Buf2;
    bool loadFromBuf1;
    bool loadFromBuf2;
    bool resetUSB;
} test_conf_t;

typedef struct preset_conf_s
{
    std::string preset_name;
    std::vector<conf_item_t> commandConf;
} preset_conf_t;

typedef struct seq_preset_s
{
    std::string seqPresetName;
    std::vector<test_conf_t> seqTest;
} seq_preset_t;

typedef struct answerBuffer_s
{
    unsigned char buffer[1024];
    int len;
} answerBuffer_t;

typedef struct config_preset_s {
    std::vector<preset_conf_t> presetCommandsV;
    std::vector<seq_preset_t> presetSequenceV;
} config_preset_t;

namespace conf {
class Configuration
{
public:
    Configuration();
    virtual ~Configuration();
    const char *getConfFilePath();
    void setConfFilePath(const char * filePath);
    bool isConfigured();

private:
    std::string conf_file_path;
    bool _configured;

public:
    bool readConfiguration();
    void setDefaultTestConf(test_conf_t *test_conf);
    bool fillPresetsConf(config_preset_t& common_conf);
    bool savePresetsConf(config_preset_t& common_conf);
    bool removePreset(config_preset_t& common_conf, int presetIndex);
    bool removeSequence(config_preset_t& common_conf, int sequenceIndex);
    bool removePresetItem(config_preset_t& common_conf, int presetIndex, int commandIndex);
    bool removeSeqPresetItem(config_preset_t& common_conf, int seqPresetIndex, int commandIndex);
    bool clearSequenceList(config_preset_t& common_conf, int seqPresetIndex);
};
}

bool inline sortConfigByName(const conf_item_t &lhs, const conf_item_t &rhs)
{
    return lhs.commandDesc < rhs.commandDesc;
}

#endif // CONFIGURATION_H
