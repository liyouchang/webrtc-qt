
#include "avcodec.h"
#include "dsputil.h"
#include "mpegvideo.h"
#include "h264.h"

#include <stdarg.h>

void *av_malloc(unsigned int size)
{
    void *ptr;

    ptr = malloc(size);

    return ptr;
}

void *av_realloc(void *ptr, unsigned int size)
{
    return realloc(ptr, size);
}

void av_free(void *ptr)
{
    if (ptr)
	{
        free(ptr);
	}
}

void *av_mallocz(unsigned int size)
{
    void *ptr;

    ptr = av_malloc(size);
    if (!ptr)
        return NULL;
    memset(ptr, 0, size);
    return ptr;
}

char *av_strdup(const char *s)
{
    char *ptr;
    int len;
    len = strlen(s) + 1;
    ptr = av_malloc(len);
    if (!ptr)
        return NULL;
    memcpy(ptr, s, len);
    return ptr;
}

void *av_fast_realloc(void *ptr, int *size, int min_size)
{
    if(min_size < *size)
        return ptr;

    *size= 17*min_size/16 + 32;

    return av_realloc(ptr, *size);
}

void av_freep(void *arg)
{
    void **ptr= (void**)arg;
    av_free(*ptr);
    *ptr = NULL;
}

typedef struct InternalBuffer{
    int last_pic_num;
    uint8_t *base[4];
    uint8_t *data[4];
    int linesize[4];
}InternalBuffer;

#define INTERNAL_BUFFER_SIZE 32

#define ALIGN(x, a) (((x)+(a)-1)&~((a)-1))

void avcodec_align_dimensions(AVCodecContext *s, int *width, int *height)
{
    int w_align= 16;
    int h_align= 16;

    *width = ALIGN(*width , w_align);
    *height= ALIGN(*height, h_align);
}

int avcodec_default_get_buffer(AVCodecContext *s, AVFrame *pic)
{
    int i;
    int w= s->width;
    int h= s->height;
    InternalBuffer *buf;
    int *picture_number;

    if(s->internal_buffer==NULL)
	{
        s->internal_buffer= av_mallocz(INTERNAL_BUFFER_SIZE*sizeof(InternalBuffer));
    }

    buf= &((InternalBuffer*)s->internal_buffer)[s->internal_buffer_count];
    picture_number= &(((InternalBuffer*)s->internal_buffer)[INTERNAL_BUFFER_SIZE-1]).last_pic_num; //FIXME ugly hack
    (*picture_number)++;

    if(buf->base[0])
	{
        buf->last_pic_num= *picture_number;
    }
	else
	{
        int h_chroma_shift=1, v_chroma_shift=1;
        int s_align, pixel_size=1;

        avcodec_align_dimensions(s, &w, &h);

        s_align= 8;

        w+= EDGE_WIDTH*2;
        h+= EDGE_WIDTH*2;

        buf->last_pic_num= -256*256*256*64;

        for(i=0; i<3; i++)
		{
            const int h_shift= i==0 ? 0 : h_chroma_shift;
            const int v_shift= i==0 ? 0 : v_chroma_shift;

            buf->linesize[i]= ALIGN(pixel_size*w>>h_shift, s_align<<(h_chroma_shift-h_shift));

            buf->base[i]= av_mallocz((buf->linesize[i]*h>>v_shift)+16); //FIXME 16
            if(buf->base[i]==NULL) return -1;
            memset(buf->base[i], 128, buf->linesize[i]*h>>v_shift);

            buf->data[i] = buf->base[i] + ALIGN((buf->linesize[i]*EDGE_WIDTH>>v_shift) + (EDGE_WIDTH>>h_shift), s_align);
        }
    }

    for(i=0; i<4; i++)
	{
        pic->base[i]= buf->base[i];
        pic->data[i]= buf->data[i];
        pic->linesize[i]= buf->linesize[i];
    }
    s->internal_buffer_count++;

    return 0;
}

void avcodec_default_release_buffer(AVCodecContext *s, AVFrame *pic)
{
    int i;
    InternalBuffer *buf, *last, temp;

    buf = NULL; /* avoids warning */
    for(i=0; i<s->internal_buffer_count; i++)
	{
        buf= &((InternalBuffer*)s->internal_buffer)[i];
        if(buf->data[0] == pic->data[0])
            break;
    }

    s->internal_buffer_count--;
    last = &((InternalBuffer*)s->internal_buffer)[s->internal_buffer_count];

    temp= *buf;
    *buf= *last;
    *last= temp;

    for(i=0; i<3; i++)
        pic->data[i]=NULL;
}

void avcodec_default_free_buffers(AVCodecContext *s)
{
    int i, j;

    if(s->internal_buffer==NULL) return;

    for(i=0; i<INTERNAL_BUFFER_SIZE; i++)
	{
        InternalBuffer *buf= &((InternalBuffer*)s->internal_buffer)[i];
        for(j=0; j<4; j++)
		{
            av_freep(&buf->base[j]);
            buf->data[j]= NULL;
        }
    }
    av_freep(&s->internal_buffer);

    s->internal_buffer_count=0;
}

AVCodecContext *avcodec_alloc_context(void)
{
    AVCodecContext *avctx= av_malloc(sizeof(AVCodecContext));
    if(avctx==NULL) return NULL;

    memset(avctx, 0, sizeof(AVCodecContext));

    return avctx;
}

AVFrame *avcodec_alloc_frame(void)
{
    AVFrame *pic= av_malloc(sizeof(AVFrame));
    if(pic==NULL) return NULL;

    memset(pic, 0, sizeof(AVFrame));

    return pic;
}

int avcodec_open(AVCodecContext *avctx)
{
    int ret;

    avctx->priv_data = av_mallocz(sizeof(H264Context));
    if (!avctx->priv_data)
        return -1;

    ret = decode_init(avctx);
    if (ret < 0)
	{
        av_freep(&avctx->priv_data);
        return ret;
    }
    return 0;
}

int avcodec_close(AVCodecContext *avctx)
{
    decode_end(avctx);
    av_freep(&avctx->priv_data);
    return 0;
}

