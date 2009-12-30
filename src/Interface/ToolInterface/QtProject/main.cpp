#include <QtGui/QApplication>
#include "PaintToolInterface.h"
#include "FlipToolInterface.h"
#include "InvertToolInterface.h"
#include "PolyLineToolInterface.h"
#include "ThresholdToolInterface.h"
#include "AnisotropicDiffusionFilterInterface.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    AnisotropicDiffusionFilterInterface t;
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
