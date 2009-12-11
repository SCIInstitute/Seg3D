#ifndef INVERTTOOL_H
#define INVERTTOOL_H

#include <QtGui/QWidget>

namespace Ui {
    class InvertTool;
}

class InvertTool : public QWidget {

    Q_OBJECT

    signals:
        void activeChanged(int);

        // sends a bool representing wether the user wants to replace the active layer or not
        void invert(bool);

    public:
        InvertTool(QWidget *parent = 0);
        ~InvertTool();

    public slots:
        void setActive(int);
        void addToActive(QStringList&);

    private:
        Ui::InvertTool *ui;

        void makeConnections();

    private slots:
        void senseActiveChanged(int);

        void senseInverted();

};

#endif // INVERTTOOL_H
