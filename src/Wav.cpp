#include "Wav.hpp"
#include "OctBinary.hpp"

#include <stdio.h>
#include <stdio.h>

#define FMT_CK_SIZE_NORMAL      14
#define FMT_CK_SIZE_PCM         16
#define FMT_CK_SIZE_EX          18
#define FMT_CK_SIZE_EXTENSIBLE  28
//#define FMT_CK_SIZE_EXTENSIBLE  40

#define STORE_TO(x) (uint8_t*)&x, sizeof(x)

constexpr uint32_t getASCII(const char s[5]) {
    uint32_t v = 0;
    for(int i = 0; i < 4; i++) {
        v <<= 8;
        v |= static_cast<int>(s[i]);
    }
    return v;
}

int readFmt(OctBinary* data, uint32_t fmt_ck_size, wav::WAVEINFO* wi) {
    if(fmt_ck_size == FMT_CK_SIZE_NORMAL) {
        wi->format_type = wav::WaveFormatType::WAVEFORMAT;
        data->readBytes(2, STORE_TO(wi->info.waveformat.w_format_tag), false, false, false);
        data->readBytes(2, STORE_TO(wi->info.waveformat.n_channels), false, false, false);
        data->readBytes(4, STORE_TO(wi->info.waveformat.n_samples_per_sec), false, false, false);
        data->readBytes(4, STORE_TO(wi->info.waveformat.n_avg_bytes_per_sec), false, false, false);
        data->readBytes(2, STORE_TO(wi->info.waveformat.n_block_align), false, false, false);
    } else if(fmt_ck_size == FMT_CK_SIZE_PCM) {
        wi->format_type = wav::WaveFormatType::PCMWAVEFORMAT;
        data->readBytes(2, STORE_TO(wi->info.pcm_waveformat.wf.w_format_tag), false, false, false);
        data->readBytes(2, STORE_TO(wi->info.pcm_waveformat.wf.n_channels), false, false, false);
        data->readBytes(4, STORE_TO(wi->info.pcm_waveformat.wf.n_samples_per_sec), false, false, false);
        data->readBytes(4, STORE_TO(wi->info.pcm_waveformat.wf.n_avg_bytes_per_sec), false, false, false);
        data->readBytes(2, STORE_TO(wi->info.pcm_waveformat.wf.n_block_align), false, false, false);
        data->readBytes(2, STORE_TO(wi->info.pcm_waveformat.w_bits_per_sample), false, false, false);
    } else if(fmt_ck_size == FMT_CK_SIZE_EX) {
        wi->format_type = wav::WaveFormatType::WAVEFORMATEX;
        data->readBytes(2, STORE_TO(wi->info.waveformat_ex.w_format_tag), false, false, false);
        data->readBytes(2, STORE_TO(wi->info.waveformat_ex.n_channels), false, false, false);
        data->readBytes(4, STORE_TO(wi->info.waveformat_ex.n_samples_per_sec), false, false, false);
        data->readBytes(4, STORE_TO(wi->info.waveformat_ex.n_avg_bytes_per_sec), false, false, false);
        data->readBytes(2, STORE_TO(wi->info.waveformat_ex.n_block_align), false, false, false);
        data->readBytes(2, STORE_TO(wi->info.waveformat_ex.w_bits_per_sample), false, false, false);
        data->readBytes(2, STORE_TO(wi->info.waveformat_ex.cb_size), false, false, false);
    } else if(fmt_ck_size == FMT_CK_SIZE_EXTENSIBLE) {
        wi->format_type = wav::WaveFormatType::WAVEFORMATEXTENSIBLE;
        data->readBytes(2, STORE_TO(wi->info.waveformat_extensible.format.w_format_tag), false, false, false);
        data->readBytes(2, STORE_TO(wi->info.waveformat_extensible.format.n_channels), false, false, false);
        data->readBytes(4, STORE_TO(wi->info.waveformat_extensible.format.n_samples_per_sec), false, false, false);
        data->readBytes(4, STORE_TO(wi->info.waveformat_extensible.format.n_avg_bytes_per_sec), false, false, false);
        data->readBytes(2, STORE_TO(wi->info.waveformat_extensible.format.n_block_align), false, false, false);
        data->readBytes(2, STORE_TO(wi->info.waveformat_extensible.format.w_bits_per_sample), false, false, false);
        data->readBytes(2, STORE_TO(wi->info.waveformat_extensible.format.cb_size), false, false, false);
        data->readBytes(2, STORE_TO(wi->info.waveformat_extensible.samples.w_valid_bits_per_sample), false, false, false);
        data->readBytes(4, STORE_TO(wi->info.waveformat_extensible.dw_channel_mask), false, false, false);
        data->readBytes(4, STORE_TO(wi->info.waveformat_extensible.sub_format), false, false, false);
    } else {
        printf("fmt chunk: invalid ck_size %d\n", fmt_ck_size);
        return FAILED;
    }

    return SUCCESSED;
}

int readList(OctBinary* data, uint32_t ck_size, uint8_t** out, wav::WAVEINFO* wi) {
    // skip list chunk
    uint8_t buffer = 0;
    for(int i = 0; i < ck_size; i++) {
        data->readByte(STORE_TO(buffer), false, false, false);
    }

    return SUCCESSED;
}

int readData(OctBinary* data, uint8_t** out, wav::WAVEINFO* wi) {
    uint32_t ck_id = 0;
    uint32_t ck_size = 0;

    data->readBytes(4, STORE_TO(ck_id), false, false, true);
    data->readBytes(4, STORE_TO(ck_size), false, false, false);

    // skip some chunks (e.g. LIST)
    while(ck_id != getASCII("data") && !data->eod()) {
        uint8_t buffer = 0;
        for(int i = 0; i < ck_size; i++) {
            data->readByte(STORE_TO(buffer), false, false, false);
        }

        ck_id = 0;
        ck_size = 0;
        data->readBytes(4, STORE_TO(ck_id), false, false, true);
        data->readBytes(4, STORE_TO(ck_size), false, false, false);
    }

    if(ck_id != getASCII("data")) {
        printf("data chunk: invalid ck_id\n");
        return FAILED;
    }

    *out = (uint8_t*)malloc(sizeof(uint8_t) * ck_size);

    uint8_t buffer = 0;
    for(int i = 0; i < ck_size; i++) {
        if(data->eod()) break;
        data->readByte(STORE_TO(buffer), false, false, false);
        (*out)[i] = buffer;
    }

    if(ck_size & 1) {
        data->readByte(STORE_TO(buffer), false, false, false);
    }

    wi->data_size = ck_size;

    return SUCCESSED;
}

int readRiff(OctBinary* data, uint32_t riff_ck_size, uint8_t** out, wav::WAVEINFO* wi) {
    uint32_t wave_id = 0;
    uint32_t data_size = riff_ck_size;
    int result;

    data->readBytes(4, STORE_TO(wave_id), false, false, true);
    if(wave_id != getASCII("WAVE")) {
        printf("riff chunk: invalid id\n");
        return FAILED;
    }

    uint32_t ck_id = 0;
    uint32_t ck_size = 0;

    data->readBytes(4, STORE_TO(ck_id), false, false, true);
    data->readBytes(4, STORE_TO(ck_size), false, false, false);

    while(ck_id == getASCII("JUNK")) {
        uint8_t buffer = 0;
        for(int i = 0; i < ck_size; i++) {
            data->readByte(STORE_TO(buffer), false, false, false);
        }

        data->readBytes(4, STORE_TO(ck_id), false, false, true);
        data->readBytes(4, STORE_TO(ck_size), false, false, false);
    }

    if(ck_id != getASCII("fmt ")) {
        return FAILED;
    }

    result = readFmt(data, ck_size, wi);
    if(result == FAILED) {
        return result;
    }

    {
        uint16_t ck_size;
        if(wi->format_type == wav::WaveFormatType::WAVEFORMAT) {
            ck_size = FMT_CK_SIZE_NORMAL + 8;
        } else if(wi->format_type == wav::WaveFormatType::PCMWAVEFORMAT) {
            ck_size = FMT_CK_SIZE_PCM + 8;
        }
        if(data_size <= ck_size) {
            printf("riff chunk: invalid parameter\n");
            return FAILED;
        }
        data_size -= ck_size;
    }

    result = readData(data, out, wi);

    return result;
}

int readFact(OctBinary* data) {
    return SUCCESSED;
}

int readInfo(OctBinary* data) {
    return SUCCESSED;
}

int readChunk(OctBinary* data, uint8_t** out, wav::WAVEINFO* wi) {
    uint32_t ck_id;
    uint32_t ck_size;

    while(!data->eod()) {
        ck_id = 0;
        ck_size = 0;

        data->readBytes(4, STORE_TO(ck_id), false, false, true);
        data->readBytes(4, STORE_TO(ck_size), false, false, false);

        switch(ck_id) {
            case getASCII("RIFF"):
                {
                    int result = readRiff(data, ck_size, out, wi);
                    if(result == FAILED) {
                        return result;
                    }
                    break;
                }
            case getASCII("fact"):
                {
                    readFact(data);
                    return FAILED;
                    break;
                }
            case getASCII("INFO"):
                {
                    readInfo(data);
                    return FAILED;
                    break;
                }
            case getASCII("LIST"):
                {
                    readList(data, ck_size, out, wi);
                    break;
                }
            default:
                {
                    printf("invalid chunk: ");
                    printf("%c", (ck_id >> 24) & 0xFF);
                    printf("%c", (ck_id >> 16) & 0xFF);
                    printf("%c", (ck_id >>  8) & 0xFF);
                    printf("%c", (ck_id      ) & 0xFF);
                    printf("\n");
                    return FAILED;
                }
        }
    }

    return SUCCESSED;
}

int wav::read(std::string filepath, uint8_t** out, wav::WAVEINFO* wi) {
    OctBinary data;
    data.loadFile(filepath);

    return readChunk(&data, out, wi);
}
