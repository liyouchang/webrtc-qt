#ifndef STREAMINGUIWIDGET_H
#define STREAMINGUIWIDGET_H

#include <QWidget>
#include <QMutex>
#include "talk/app/webrtc/mediastreaminterface.h"
#include "talk/base/win32.h"
#include "peer_connection_client.h"
#include "talk/media/base/mediachannel.h"
#include "talk/media/base/videocommon.h"
#include "talk/media/base/videoframe.h"
#include "talk/media/base/videorenderer.h"

// A little helper class to make sure we always to proper locking and
// unlocking when working with VideoRenderer buffers.
template <typename T>
class AutoLock {
public:
    explicit AutoLock(T* obj) : obj_(obj) { obj_->Lock(); }
    ~AutoLock() { obj_->Unlock(); }
protected:
    T* obj_;
};


class VideoRenderer : public webrtc::VideoRendererInterface {
public:
    VideoRenderer(QWidget * wnd, int width, int height,
                  webrtc::VideoTrackInterface* track_to_render);
    virtual ~VideoRenderer();
    void Lock();
    void Unlock();

    // VideoRendererInterface implementation
    virtual void SetSize(int width, int height);
    virtual void RenderFrame(const cricket::VideoFrame* frame);

    const BITMAPINFO& bmi() const { return bmi_; }
    const uint8* image() const { return image_.get(); }

protected:
    enum {
        SET_SIZE,
        RENDER_FRAME,
    };

    QWidget * wnd_;
    BITMAPINFO bmi_;
    talk_base::scoped_ptr<uint8[]> image_;
    QMutex buffer_lock_;
    talk_base::scoped_refptr<webrtc::VideoTrackInterface> rendered_track_;
};

class StreamingUIWidget : public QWidget
{
    Q_OBJECT
public:
    explicit StreamingUIWidget(QWidget *parent = 0);
    void StartLocalRenderer(webrtc::VideoTrackInterface *local_video);
    void StopLocalRenderer();
    void StartRemoteRenderer(webrtc::VideoTrackInterface *remote_video);
    void StopRemoteRenderer();

signals:

public slots:


    // QWidget interface
protected:
    void paintEvent(QPaintEvent *);
private:
    talk_base::scoped_ptr<VideoRenderer> local_renderer_;
    talk_base::scoped_ptr<VideoRenderer> remote_renderer_;

};

#endif // STREAMINGUIWIDGET_H
