
#include "window.h"
#include "audio.h"
#include <iostream>
#include <string>


void Audio::set_audio_level(const char* fileName )
{
    int ret;
    audio_stream_index = -1;
}

void Audio::audio_level(const char* fileName )
{
    int ret;
    audio_stream_index = -1;

    logging("Open input file: %s",fileName );

    if ( (ret = open_input_file( fileName )) < 0){
        logging("Could not open input file: %s",fileName );
        exit_prog(ret);
    }

    current_frame = 1;
    perFrame = current_frame*100/all_frame;
    emit set_pD(perFrame);

    logging("Init filter");

    if ((ret = init_filters(filter_descr)) < 0){
        logging("Could not init filter");
        exit_prog(ret);
    }

    logging("Read packets");

    if ((ret = read_all_packets()) != AVERROR_EOF){
        logging("Error read packets");
        exit_prog(ret);
    } else {
        /* signal EOF to the filtergraph */
        if (av_buffersrc_add_frame_flags(buffersrc_ctx, NULL, 0) < 0) {
            logging("Error while closing the filtergraph");
            exit_prog(-1);
        }

        /* pull remaining frames from the filtergraph */
        while (1) {
            ret = av_buffersink_get_frame(buffersink_ctx, filt_frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                break;
            if (ret < 0)
                exit_prog(ret);
            av_frame_unref(filt_frame);
        }
    }

    logging("End read packets");

    av_log_set_callback(get_max_vol);

    avfilter_graph_free(&filter_graph);
    avcodec_free_context(&dec_ctx);
    avformat_close_input(&fmt_ctx);
    av_packet_free(&packet);
    av_frame_free(&frame);
    av_frame_free(&filt_frame);
    av_log_set_callback(av_log_default_callback);

    emit set_pD(100);
    emit send_max_vol(audio_max_vol);
}



/* read all packets */
int Audio::read_all_packets()
{

    int ret=0;
    current_frame=1;
    packet = av_packet_alloc();
    filt_frame = av_frame_alloc();
    frame = av_frame_alloc();

        if (!packet || !filt_frame || !frame) {
            logging("Could not allocate frame or packet or filt_frame");
            exit_prog(1);
        }


    while (1) {

//        fprintf(stderr, "Current frame: %d\r", current_frame);

        fprintf(stderr, "Current frame: %d, %% - %d\r", current_frame, perFrame);
        emit set_pD(perFrame);

        if ((ret = av_read_frame(fmt_ctx, packet)) < 0)
            break;

        if (packet->stream_index == audio_stream_index) {
            ret = avcodec_send_packet(dec_ctx, packet);
            if (ret < 0) {
                logging("Error while sending a packet to the decoder");
                break;
            }

            while (ret >= 0) {
                ret = avcodec_receive_frame(dec_ctx, frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    //                    av_strerror(ret, errstr, sizeof(errstr));
                    //                    fprintf(stderr, "BREAK2: %d, %s\n", ret, errstr);
                    break;
                } else if (ret < 0) {
                    logging("Error while receiving a frame from the decoder");
                    exit_prog(ret);
                }

                if (ret >= 0) {
                    /* push the audio data from decoded frame into the filtergraph */
                    if (av_buffersrc_add_frame_flags(buffersrc_ctx, frame, AV_BUFFERSRC_FLAG_KEEP_REF) < 0) {
                        logging("Error while feeding the audio filtergraph");
                        exit_prog(-1);
                    }

                    /* pull filtered audio from the filtergraph */
                    while (1) {
                        ret = av_buffersink_get_frame(buffersink_ctx, filt_frame);
                        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                            break;
                        if (ret < 0)
                            exit_prog(ret);
                        // AVN                  print_frame(filt_frame);
                        av_frame_unref(filt_frame);
                        current_frame++;
                        perFrame = current_frame*100/all_frame;
                        //                        progress.setValue(perFrame);
                    }
                    av_frame_unref(frame);
                }
            }
        }
        av_packet_unref(packet);
    }
    return ret;
}


int Audio::open_input_file(const char *filename)
{
    const AVCodec *dec;
    AVPacket *pk = av_packet_alloc();

        if (!pk) {
            logging("Could not allocate  packet pk");
            exit_prog(1);
        }

        fmt_ctx = avformat_alloc_context();
        if (!fmt_ctx)
        {
            logging("Could not allocate fmt_ctx");
            exit_prog(1);
        }

    int ret;

// открываем файл для подсчета аудио фреймов.
    if ((ret = avformat_open_input(&fmt_ctx, filename, NULL, NULL)) < 0) {
        logging("Cannot open input file: %s", filename);
        exit_prog(ret);
    }

    if ((ret = avformat_find_stream_info(fmt_ctx, NULL)) < 0) {
        logging("Cannot find stream information");
        exit_prog(ret);
    }

    /* select the audio stream */
    ret = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, &dec, 0);
    if (ret < 0) {
        logging("Cannot find an audio stream in the input file");
        exit_prog(ret);
    }

    audio_stream_index = ret;

    // подсчет аудио фреймов всего

    while (1) {

        if ((ret = av_read_frame(fmt_ctx, pk)) < 0)
            break;
        if (pk->stream_index == audio_stream_index)  current_frame++;
    }

    fprintf(stderr, "All Audio Frame %d in file %s\n", current_frame, filename);
    all_frame = current_frame;

    avformat_close_input(&fmt_ctx);

// параноя, ещё раз открываем файл.
    if ((ret = avformat_open_input(&fmt_ctx, filename, NULL, NULL)) < 0) {
        logging("Cannot open input file: %s", filename);
        exit_prog(ret);
    }

    if ((ret = avformat_find_stream_info(fmt_ctx, NULL)) < 0) {
        logging("Cannot find stream information");
        exit_prog(ret);
    }

    /* select the audio stream */
    ret = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, &dec, 0);
    if (ret < 0) {
        logging("Cannot find an audio stream in the input file");
        exit_prog(ret);
    }
    audio_stream_index = ret;

    /* create decoding context */
    dec_ctx = avcodec_alloc_context3(dec);
    if (!dec_ctx)
        return AVERROR(ENOMEM);
    avcodec_parameters_to_context(dec_ctx, fmt_ctx->streams[audio_stream_index]->codecpar);

    /* init the audio decoder */
    if ((ret = avcodec_open2(dec_ctx, dec, NULL)) < 0) {
        logging("Cannot open audio decoder");
        exit_prog(ret);
    }

    return 0;
}


int Audio::init_filters(const char *filters_descr)
{
    char args[512];
    int ret = 0;

    const AVFilter *abuffersrc  = avfilter_get_by_name("abuffer");
    const AVFilter *abuffersink = avfilter_get_by_name("abuffersink");

    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs  = avfilter_inout_alloc();

    AVRational time_base = fmt_ctx->streams[audio_stream_index]->time_base;

    filter_graph = avfilter_graph_alloc();
    if (!outputs || !inputs || !filter_graph) {
        ret = AVERROR(ENOMEM);
        goto end;
    }

    /* buffer audio source: the decoded frames from the decoder will be inserted here. */
    if (dec_ctx->ch_layout.order == AV_CHANNEL_ORDER_UNSPEC)
        av_channel_layout_default(&dec_ctx->ch_layout, dec_ctx->ch_layout.nb_channels);
    ret = snprintf(args, sizeof(args),
                   "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=",
                   time_base.num, time_base.den, dec_ctx->sample_rate,
                   av_get_sample_fmt_name(dec_ctx->sample_fmt));
    av_channel_layout_describe(&dec_ctx->ch_layout, args + ret, sizeof(args) - ret);
    ret = avfilter_graph_create_filter(&buffersrc_ctx, abuffersrc, "in",
                                       args, NULL, filter_graph);
    if (ret < 0) {
        logging("Cannot create audio buffer source");
        goto end;
    }

    /* buffer audio sink: to terminate the filter chain. */
    buffersink_ctx = avfilter_graph_alloc_filter(filter_graph, abuffersink, "out");
    if (!buffersink_ctx) {
        logging("Cannot create audio buffer sink");
        ret = AVERROR(ENOMEM);
        goto end;
    }

    ret = av_opt_set(buffersink_ctx, "sample_formats", "s16",
                     AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        logging("Cannot set output sample format");
        goto end;
    }

    ret = av_opt_set(buffersink_ctx, "channel_layouts", "mono",
                     AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        logging("Cannot set output channel layout");
        goto end;
    }

    ret = av_opt_set_array(buffersink_ctx, "samplerates", AV_OPT_SEARCH_CHILDREN,
                           0, 1, AV_OPT_TYPE_INT, &out_sample_rate);
    if (ret < 0) {
        logging("Cannot set output sample rate");
        goto end;
    }

    ret = avfilter_init_dict(buffersink_ctx, NULL);
    if (ret < 0) {
        logging("Cannot initialize audio buffer sink");
        goto end;
    }

    /*
     * Set the endpoints for the filter graph. The filter_graph will
     * be linked to the graph described by filters_descr.
     */

    /*
     * The buffer source output must be connected to the input pad of
     * the first filter described by filters_descr; since the first
     * filter input label is not specified, it is set to "in" by
     * default.
     */
    outputs->name       = av_strdup("in");
    outputs->filter_ctx = buffersrc_ctx;
    outputs->pad_idx    = 0;
    outputs->next       = NULL;

    /*
     * The buffer sink input must be connected to the output pad of
     * the last filter described by filters_descr; since the last
     * filter output label is not specified, it is set to "out" by
     * default.
     */
    inputs->name       = av_strdup("out");
    inputs->filter_ctx = buffersink_ctx;
    inputs->pad_idx    = 0;
    inputs->next       = NULL;

    if ((ret = avfilter_graph_parse_ptr(filter_graph, filters_descr,
                                        &inputs, &outputs, NULL)) < 0)
        goto end;

    if ((ret = avfilter_graph_config(filter_graph, NULL)) < 0)
        goto end;

    /* Print summary of the sink buffer
     * Note: args buffer is reused to store channel layout string */
    outlink = buffersink_ctx->inputs[0];
    av_channel_layout_describe(&outlink->ch_layout, args, sizeof(args));

end:
    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);

    return ret;
}


int Audio::exit_prog(int err)
{

    char errstr[1024];
    avfilter_graph_free(&filter_graph);
    avcodec_free_context(&dec_ctx);
    avformat_close_input(&fmt_ctx);
    av_packet_free(&packet);
    av_frame_free(&frame);
    av_frame_free(&filt_frame);

    if (err != 0 && err != AVERROR_EOF) {
        if ( err<0 ) {
            av_strerror(err, errstr, sizeof(errstr));
            fprintf(stderr, "Error occurred: %s\n", errstr);
        }
        fprintf(stderr, "Exit Error:%d\n", err);
        exit(1);
    }
    fprintf(stderr, "Exit:%d\n", err);
    exit(0);
}

void Audio::logging(const char *fmt, ...)
{
    va_list args;
    fprintf( stderr, "LOG: " );
    va_start( args, fmt );
    vfprintf( stderr, fmt, args );
    va_end( args );
    fprintf( stderr, "\n" );
}

//---------------------------------------------------------------------------------------
Audio::Audio(QObject *parent)
    : QObject{parent}
{
        setObjectName("Audio");
}

//---------------------------------------------------------------------------------------
Audio::~Audio()
{
//    stop();
}

void get_max_vol(void *ptr, int, const char *fmt, va_list vl)
{
    char str[100];
    std::string s1, s2;

    vsprintf(str, fmt, vl);
//    fprintf(stdout,"Строка:%s",str);
    s1=str;

    s2=s1.substr(0, s1.find(":") );

    if ( s2 == "max_volume" )
    {
        s2=s1.substr(s1.find(":")+1, s1.find("dB")-(s1.find(":")+1) );
        std::cout << "MAX VALUE max_volume#"<< s2 << std::endl;
        audio_max_vol = s2;
    }
}
