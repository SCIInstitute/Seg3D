#ifndef INVERTTOOLINTERFACE_H
#define INVERTTOOLINTERFACE_H

#include <QtGui/QWidget>

namespace Ui {
    class InvertToolInterface;
}

class InvertToolInterface : public QWidget {

    Q_OBJECT

    signals:
        void activeChanged(int);

        // sends a bool representing wether the user wants to replace the active layer or not
        void invert(bool);

    public:
        InvertToolInterface(QWidget *parent = 0);
        ~InvertToolInterface();

    public slots:
        void setActive(int);
        void addToActive(QStringList&);

    private:
        Ui::InvertToolInterface *ui;

        void makeConnections();

    private slots:
        void senseActiveChanged(int);

        void senseInverted();

};

#endif // INVERTTOOLINTERFACE_H
