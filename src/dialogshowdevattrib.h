#ifndef DIALOGSHOWDEVATTRIB_H
#define DIALOGSHOWDEVATTRIB_H

#include <QDialog>
#include "smartcard.h"

namespace Ui {
class DialogShowDevAttrib;
}

class DialogShowDevAttrib : public QDialog
{
    Q_OBJECT

public:
    explicit DialogShowDevAttrib(QWidget *parent = 0,
                                 smcard::SmartCard *SCard = nullptr);
    ~DialogShowDevAttrib();

    bool updateDeviceAttrib(smcard::SmartCard *SCard);

    bool getSCardAttrib(smcard::SmartCard *SCard);

private:
    Ui::DialogShowDevAttrib *ui;

};

#endif // DIALOGSHOWDEVATTRIB_H
