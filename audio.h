#ifndef AUDIO_H
#define AUDIO_H

#include <QObject>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavcodec/avcodec.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavformat/avformat.h>
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
#include <libavutil/mem.h>
#include <libavutil/opt.h>
#include <libavutil/error.h>
#include <libavutil/dict.h>
#include <libavutil/log.h>
}

static int audio_stream_index = -1;
static const char *filter_descr = "volumedetect";
static const char *filter_volume = "volume";
static std::string audio_max_vol;

class Audio : public QObject
{
    Q_OBJECT    
public:

    explicit Audio(QObject *parent = nullptr);
    virtual ~Audio();

    void audio_level(const char* fileName );
    void set_audio_level(const char* fileName );

    int exit_prog(int);
    void logging(const char *fmt, ...);

    int current_frame;
    int all_frame;
    int perFrame;
    int audio_stream_index;

signals:
    void set_pD(int);
    void send_max_vol(std::string);

protected:
        AVCodecContext *dec_ctx{nullptr};
        AVFrame *frame{nullptr};
        AVFrame *filt_frame{nullptr};
        AVPacket *packet{nullptr};
        const AVFilterLink *outlink{nullptr};
        AVFilterContext *buffersink_ctx{nullptr};
        AVFilterContext *buffersrc_ctx{nullptr};
        AVFilterGraph *filter_graph{nullptr};

private:     

    AVFormatContext *fmt_ctx{nullptr};

    const int out_sample_rate = 8000;

    int open_input_file(const char *filename);
    int init_filters(const char *filters_descr);
    int read_all_packets();

};

void get_max_vol(void *ptr, int, const char *fmt, va_list vl);


#endif // AUDIO_H
