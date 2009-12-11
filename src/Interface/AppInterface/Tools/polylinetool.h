#ifndef POLYLINETOOL_H
#define POLYLINETOOL_H

#include <QWidget>

namespace Ui {
    class PolyLineTool;
}

class PolyLineTool : public QWidget {

    Q_OBJECT

    signals:
        void activeChanged(int);

        void resetPolyLineTool();
        void fillPolyLine();
        void erasePolyLine();

    public:
        PolyLineTool(QWidget *parent = 0);
        ~PolyLineTool();

    public slots:
        void setActive(int);
        void addToActive(QStringList&);


    private:
        Ui::PolyLineTool *ui;
        void makeConnections();

    private slots:
        void senseActiveChanged(int);

        void senseResetPolyLineTool();
        void senseFillPolyLine();
        void senseErasePolyLine();

};

#endif // POLYLINETOOL_H
