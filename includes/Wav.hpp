#ifndef OCT_WAV_H
#define OCT_WAV_H

#include <string>

#define SUCCESSED 0
#define FAILED 1

namespace wav {

typedef uint32_t GUID;

enum class WaveFormatType {
    WAVEFORMAT,
    PCMWAVEFORMAT,
    WAVEFORMATEX,
    WAVEFORMATEXTENSIBLE
};

typedef struct waveformat_tag {
    uint16_t w_format_tag;
    uint16_t n_channels;
    uint32_t n_samples_per_sec;
    uint32_t n_avg_bytes_per_sec;
    uint16_t n_block_align;
} WAVEFORMAT;

typedef struct pcmwaveformat_tag {
    WAVEFORMAT wf;
    uint16_t w_bits_per_sample;
} PCMWAVEFORMAT;

// TODO: support ex and extensible
typedef struct waveformatex_tag {
    uint16_t w_format_tag;
    uint16_t n_channels;
    uint32_t n_samples_per_sec;
    uint32_t n_avg_bytes_per_sec;
    uint16_t n_block_align;
    uint16_t w_bits_per_sample;
    uint16_t cb_size;
} WAVEFORMATEX;

typedef struct waveformatextensible_tag {
    WAVEFORMATEX format;
    union {
        uint16_t w_valid_bits_per_sample;
        uint16_t w_samples_per_block;
        uint16_t w_reserved;
    } samples;
    uint32_t dw_channel_mask;
    GUID sub_format;
} WAVEFORMATEXTENSIBLE;

typedef struct waveinfo_tag {
    WaveFormatType format_type;
    union {
        WAVEFORMAT waveformat;
        PCMWAVEFORMAT pcm_waveformat;
        WAVEFORMATEX waveformat_ex;
        WAVEFORMATEXTENSIBLE waveformat_extensible;
    } info;
    uint32_t data_size;
} WAVEINFO;

int read(std::string filepath, uint8_t** out, WAVEINFO* wi);
}
// namespace wav

#endif
