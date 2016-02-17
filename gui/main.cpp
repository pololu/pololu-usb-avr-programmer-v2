#include <QApplication>
#include "main_model.h"
#include "main_controller.h"
#include "main_view.h"

int main(int argc, char ** argv)
{
    QApplication app(argc, argv);
    MainModel model;
    MainController controller;
    MainView view;
    view.init(&model, &controller);
    controller.init(&model, &view);
    view.showWindow();
    return app.exec();
}
