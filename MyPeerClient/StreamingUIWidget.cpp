#include "StreamingUIWidget.h"
#include <QPainter>
#include <QFile>
#include <QBitmap>
const char kConnecting[] = "Connecting... ";
const char kNoVideoStreams[] = "(no video streams either way)";
const char kNoIncomingStream[] = "(no incoming video)";


StreamingUIWidget::StreamingUIWidget(QWidget *parent) :
    QWidget(parent)
{
}

void StreamingUIWidget::StartLocalRenderer(webrtc::VideoTrackInterface *local_video)
{
   local_renderer_.reset(new VideoRenderer(this, 1, 1, local_video));
}

void StreamingUIWidget::StopLocalRenderer()
{
    local_renderer_.reset();
}

void StreamingUIWidget::StartRemoteRenderer(webrtc::VideoTrackInterface *remote_video)
{
    remote_renderer_.reset(new VideoRenderer(this, 1, 1, remote_video));
}

void StreamingUIWidget::StopRemoteRenderer()
{
    remote_renderer_.reset();
}

void StreamingUIWidget::paintEvent(QPaintEvent *event)
{
    //    RECT rc;
    //    ::GetClientRect(handle(), &rc);
    QPainter painter(this);
    QRect tmpRect = this->rect();
    VideoRenderer* local_renderer = local_renderer_.get();
    VideoRenderer* remote_renderer = remote_renderer_.get();
    if (remote_renderer && local_renderer) {
        AutoLock<VideoRenderer> local_lock(local_renderer);
        AutoLock<VideoRenderer> remote_lock(remote_renderer);

        const BITMAPINFO& bmi = remote_renderer->bmi();
        int height = abs(bmi.bmiHeader.biHeight);
        int width = bmi.bmiHeader.biWidth;
        //this->resize(width,height);
        const uint8* image = remote_renderer->image();
        if (image != NULL) {
            QByteArray bmpData;
            //make bmp
            BITMAPFILEHEADER fileHead;
            fileHead.bfType = 0x4D42;//bmp类型
            fileHead.bfSize= sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFO)
                    + bmi.bmiHeader.biSizeImage;
            fileHead.bfOffBits=sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFO);

            bmpData.append((char *)&fileHead,sizeof(BITMAPFILEHEADER));
            bmpData.append((char *)&bmi,sizeof(BITMAPINFO));
            bmpData.append((char *)image,bmi.bmiHeader.biSizeImage);

            QImage  tmpImage = QImage::fromData(bmpData,"BMP");


            painter.drawPixmap(0,0,width,height,QPixmap::fromImage(tmpImage));
        } else {
            QString text(kConnecting);
            if (!local_renderer->image()) {
                text += kNoVideoStreams;
            } else {
                text += kNoIncomingStream;
            }
            painter.drawText(10,10,text);
        }
    }
}


VideoRenderer::VideoRenderer(QWidget * wnd, int width, int height,
                             webrtc::VideoTrackInterface *track_to_render):
    wnd_(wnd), rendered_track_(track_to_render)
{
    ZeroMemory(&bmi_, sizeof(bmi_));
    bmi_.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi_.bmiHeader.biPlanes = 1;
    bmi_.bmiHeader.biBitCount = 32;
    bmi_.bmiHeader.biCompression = BI_RGB;
    bmi_.bmiHeader.biWidth = width;
    bmi_.bmiHeader.biHeight = -height;
    bmi_.bmiHeader.biSizeImage = width * height *
            (bmi_.bmiHeader.biBitCount >> 3);
    rendered_track_->AddRenderer(this);
}

VideoRenderer::~VideoRenderer()
{
    rendered_track_->RemoveRenderer(this);
}

void VideoRenderer::Lock() {
    buffer_lock_.lock();
}

void VideoRenderer::Unlock() {
    buffer_lock_.unlock();
}

void VideoRenderer::SetSize(int width, int height)
{
    AutoLock<VideoRenderer> lock(this);

    bmi_.bmiHeader.biWidth = width;
    bmi_.bmiHeader.biHeight = -height;
    bmi_.bmiHeader.biSizeImage = width * height *
            (bmi_.bmiHeader.biBitCount >> 3);
    image_.reset(new uint8[bmi_.bmiHeader.biSizeImage]);
}

void VideoRenderer::RenderFrame(const cricket::VideoFrame *frame)
{
    if (!frame)
        return;
    {
        AutoLock<VideoRenderer> lock(this);
        ASSERT(image_.get() != NULL);
        frame->ConvertToRgbBuffer(cricket::FOURCC_ARGB,
                                  image_.get(),
                                  bmi_.bmiHeader.biSizeImage,
                                  bmi_.bmiHeader.biWidth *
                                  bmi_.bmiHeader.biBitCount / 8);
    }
    wnd_->update();
}
