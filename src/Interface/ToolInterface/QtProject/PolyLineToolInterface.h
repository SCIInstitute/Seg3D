#ifndef POLYLINETOOLINTERFACE_H
#define POLYLINETOOLINTERFACE_H

#include <QWidget>

namespace Ui {
    class PolyLineToolInterface;
}

class PolyLineToolInterface : public QWidget {

    Q_OBJECT

    signals:
        void activeChanged(int);
        void resetPolyLineTool();
        void fillPolyLine();
        void erasePolyLine();

    public:
        PolyLineToolInterface(QWidget *parent = 0);
        ~PolyLineToolInterface();

    public slots:
        void setActive(int);
        void addToActive(QStringList&);


    private:
        Ui::PolyLineToolInterface *ui;
        void makeConnections();

    private slots:
        void senseActiveChanged(int);

        void senseResetPolyLineTool();
        void senseFillPolyLine();
        void senseErasePolyLine();

};

#endif // PolyLineToolInterface_H
