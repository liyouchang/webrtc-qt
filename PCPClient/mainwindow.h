#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>
#include "main_wnd.h"


class MainWindow : public QMainWindow,public MainWindowInterface
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();


signals:
    void signalUIThreadCallback(int msgID,void * data);
private slots:
    void OnConnectServer();
    void OnItemDoubleClicked(QTreeWidgetItem * item,int column);
    void OnUIThreadCallback(int msgID,void * data);
private:
    QWidget * connectUI_;
    QTreeWidget * peerListUI_;
    //StreamingUIWidget * streamUI_;
    // MainWindowInterface interface
public:
    void RegisterObserver(MainWndCallback *callback);
    bool IsWindow();
    void ShowMessageBox(const char *caption, const char *text, bool is_error);
    UI current_ui();
    void SwitchToConnectUI();
    void SwitchToPeerList(const Peers &peers);
    void SwitchToStreamingUI();
//    void StartLocalRenderer(webrtc::VideoTrackInterface *local_video);
//    void StopLocalRenderer();
//    void StartRemoteRenderer(webrtc::VideoTrackInterface *remote_video);
//    void StopRemoteRenderer();
    void QueueUIThreadCallback(int msg_id, void *data);
private:
    UI ui_;
    DWORD ui_thread_id_;
    bool destroyed_;
    void* nested_msg_;
    MainWndCallback* callback_;
    static ATOM wnd_class_;

    // QWidget interface
protected:
    void keyPressEvent(QKeyEvent *);

    // QWidget interface
protected:
    void closeEvent(QCloseEvent *);
};

#endif // MAINWINDOW_H
