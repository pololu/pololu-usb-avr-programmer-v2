#pragma once

#include "main_window.h"

class MainModel;
class MainController;

/** The MainView class knows a lot about what we want to display on the screen
 * and when it needs to be updated, but it knows nothing about the underlying
 * GUI framework, so that it can hopefully be reused if we port to a different
 * GUI framework.
 *
 * It uses classes like MainWindow which know about the GUI framework. */
class MainView
{
public:
    void init(const MainModel *, MainController *);

    /** This is a shortcut that allows MainWindow to report user input
    events directly to the controller.  The MainView class normally doesn't need
    to process any of those events, and it is annoying to have a ton of
    one-line functions that just forward all their arguments to the controller. */
    MainController * userInputReceiver() { return controller; }

    /** This function tells the GUI framework that we want to eventually display
     * the window, but it might not actually draw the window properly before
     * returning.  Window drawing will happen later after the GUI framework's
     * event loop is started.  **/
    void showWindow();

    /** This causes the view to call the controller's update() function
     * periodically, on the same thread as everything else.
     *
     * intervalMs is the amount of time between updates, in milliseconds.
     */
    void startUpdateTimer(uint32_t intervalMs);

    void showErrorMessage(const std::string & message);
    void showWarningMessage(const std::string & message);
    void showInfoMessage(const std::string & message);

    /** Show an OK/Cancel dialog, return true if the user selects OK. */
    bool confirm(const std::string & question);

    /** This is called whenever something in the model has changed that might
     * require the view to be updated.  It includes no details about what
     * exactly changed. */
    void handleModelChanged();

private:
    /** This is called whenever it is possible that we have connected to a
     * different device. */
    void handleDeviceChanged();

public:
    void handleVariablesChanged();

    void handleSettingsChanged();

private:
    const MainModel * model;
    MainController * controller;

    MainWindow window;
};
