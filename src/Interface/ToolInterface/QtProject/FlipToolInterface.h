#ifndef FLIPTOOLINTERFACE_H
#define FLIPTOOLINTERFACE_H

#include <QWidget>

namespace Ui
{
class FlipToolInterface;
}

class FlipToolInterface : public QWidget
{

Q_OBJECT

public:
  FlipToolInterface( QWidget *parent = 0 );
  ~FlipToolInterface();

private:
  Ui::FlipToolInterface *ui;

};

#endif // FlipToolInterface_H
