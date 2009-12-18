#include <QtGui/QApplication>
#include "PaintToolInterface.h"
#include "FlipToolInterface.h"
#include "InvertToolInterface.h"
#include "PolyLineToolInterface.h"
#include "ThresholdToolInterface.h"
#include "anisotropicdiffusionfilter.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    AnisotropicDiffusionFilter t;
    t.show();

    PaintToolInterface v;
    v.show();

    PolyLineToolInterface w;
    w.show();

    FlipToolInterface x;
    x.show();

    InvertToolInterface z;
    z.show();

    ThresholdToolInterface u;
    u.show();

    return a.exec();
}
