#ifndef FLIPTOOLINTERFACE_H
#define FLIPTOOLINTERFACE_H

#include <QWidget>

namespace Ui {
    class FlipToolInterface;
}

class FlipToolInterface : public QWidget {

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
        FlipToolInterface(QWidget *parent = 0);
        ~FlipToolInterface();

    public slots:
        void setActive(int);
        void addToActive(QStringList&);

    private:
        Ui::FlipToolInterface *ui;

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

#endif // FlipToolInterface_H
