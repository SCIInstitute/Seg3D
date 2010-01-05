#include <QtGui/QApplication>
#include "PaintToolInterface.h"
#include "FlipToolInterface.h"
#include "InvertToolInterface.h"
#include "PolyLineToolInterface.h"
#include "ThresholdToolInterface.h"
#include "AnisotropicDiffusionFilterInterface.h"
#include "ArithmeticFilterInterface.h"
#include "BinaryDialateErodeFilterInterface.h"
#include "BooleanFilterInterface.h"
#include "CannyEdgeDetectionFilterInterface.h"
#include "ConfidenceConnectedFilterInterface.h"
#include "ConnectedComponentFilterInterface.h"
#include "DiscreteGaussianFilterInterface.h"
#include "FillHolesFilterInterface.h"
#include "GradientMagnitudeFilterInterface.h"
#include "HistogramEqualizationFilterInterface.h"
#include "IntensityCorrectionFilterInterface.h"
#include "MaskDataFilterInterface.h"
#include "MedianFilterInterface.h"
#include "NeighborhoodConnectedFilterInterface.h"
#include "OtsuThresholdFilterInterface.h"
#include "ThresholdSegmentationLSFilterInterface.h"

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

    AnisotropicDiffusionFilterInterface b;
    b.show();

    ArithmeticFilterInterface c;
    c.show();

    BinaryDialateErodeFilterInterface d;
    d.show();

    BooleanFilterInterface e;
    e.show();

    CannyEdgeDetectionFilterInterface f;
    f.show();

    ConfidenceConnectedFilterInterface g;
    g.show();

    ConnectedComponentFilterInterface h;
    h.show();

    DiscreteGaussianFilterInterface i;
    i.show();

    FillHolesFilterInterface j;
    j.show();

    GradientMagnitudeFilterInterface k;
    k.show();

    HistogramEqualizationFilterInterface l;
    l.show();

    IntensityCorrectionFilterInterface m;
    m.show();

    MaskDataFilterInterface n;
    n.show();

    MedianFilterInterface o;
    o.show();

    NeighborhoodConnectedFilterInterface p;
    p.show();

    OtsuThresholdFilterInterface q;
    q.show();

    ThresholdSegmentationLSFilterInterface r;
    r.show();




    return a.exec();
}
