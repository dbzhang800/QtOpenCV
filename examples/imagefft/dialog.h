#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

namespace Ui {
class Dialog;
}

namespace cv {
class Mat;
}

class Dialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

private slots:
    void onOpenButtonClicked();

private:
    Ui::Dialog *ui;
};

#endif // DIALOG_H
