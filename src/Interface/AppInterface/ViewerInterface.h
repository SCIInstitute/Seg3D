#ifndef VIEWERINTERFACE_H
#define VIEWERINTERFACE_H


// QT includes
#include <QtGui>


// STL includes
#include <string>

// Boost includes
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>

namespace Seg3D {

class ViewerInterfacePrivate;
typedef boost::shared_ptr<ViewerInterfacePrivate> ViewerInterfacePrivateHandle;

class ViewerInterface : public QWidget {
    
    Q_OBJECT

  public:
    ViewerInterface(QWidget *parent = 0);
    virtual ~ViewerInterface();
    
  //  void writeSizeSettings();
  //  void readSizeSettings();
    
    void set_layout(const std::string& layout);
    void set_selected_viewer(int);

  private:
    ViewerInterfacePrivateHandle private_;
    
  public:
    typedef QPointer<ViewerInterface> qpointer_type;
    
    // SetViewerLayout: (Thread safe - slot)
    static void SetViewerLayout(qpointer_type qpointer, std::string layout);
};

} // end namespace  
#endif // VIEWERINTERFACE_H