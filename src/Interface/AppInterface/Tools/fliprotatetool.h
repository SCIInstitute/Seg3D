#ifndef FLIPROTATETOOL_H
#define FLIPROTATETOOL_H

#include <QWidget>

namespace Ui {
    class FlipRotateTool;
}

class FlipRotateTool : public QWidget {

    Q_OBJECT

    signals:
        void activeChanged(int);

        void flipAxial();
        void flipCoronal();
        void flipSagittal();

        void rotateAxialCoronal();
        void rotateAxialSagittal();
        void rotateSagittalAxial();


    public:
        FlipRotateTool(QWidget *parent = 0);
        ~FlipRotateTool();

    public slots:
        void setActive(int);
        void addToActive(QStringList&);

    private:
        Ui::FlipRotateTool *ui;

        void makeConnections();

    private slots:
        void senseActiveChanged(int);

        void senseFlipAxial();
        void senseFlipCoronal();
        void senseFlipSagittal();

        void senseRotateAxialCoronal();
        void senseRotateAxialSagital();
        void senseRotateSagittalAxial();
};

#endif // FLIPROTATETOOL_H
