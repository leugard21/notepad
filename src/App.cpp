#include "App.h"
#include <QCoreApplication>

App::App(int &argc, char **argv) : QApplication(argc, argv) {
  QCoreApplication::setOrganizationName("Luna");
  QCoreApplication::setOrganizationDomain("luna.app");
  QCoreApplication::setApplicationName("Notepad");
}
