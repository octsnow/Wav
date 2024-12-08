#include "Wav.hpp"

#include <al.h>
#include <alc.h>
#include <stdint.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <limits.h>

#pragma comment(lib, "OpenAL32.lib")
#pragma comment(lib, "oct_wav.lib")
#pragma comment(lib, "oct_binary.lib")


void ShowWaveInfo(wav::WAVEINFO wi) {
    printf("data_size: %d\n", wi.data_size);

    switch(wi.format_type) {
        case wav::WaveFormatType::WAVEFORMAT:
            printf("format_type: waveformat\n");
            printf("w_format_tag: %d\n", wi.info.waveformat.w_format_tag);
            printf("n_channels: %d\n", wi.info.waveformat.n_channels);
            printf("n_samples_per_sec: %d\n", wi.info.waveformat.n_samples_per_sec);
            printf("n_samples_per_sec: %d\n", wi.info.waveformat.n_avg_bytes_per_sec);
            printf("n_avg_bytes_per_sec: %d\n", wi.info.waveformat.n_block_align);
            break;
        case wav::WaveFormatType::PCMWAVEFORMAT:
            printf("format_type: pcm_waveformat\n");
            printf("w_format_tag: %d\n", wi.info.pcm_waveformat.wf.w_format_tag);
            printf("n_channels: %d\n", wi.info.pcm_waveformat.wf.n_channels);
            printf("n_samples_per_sec: %d\n", wi.info.pcm_waveformat.wf.n_samples_per_sec);
            printf("n_samples_per_sec: %d\n", wi.info.pcm_waveformat.wf.n_avg_bytes_per_sec);
            printf("n_avg_bytes_per_sec: %d\n", wi.info.pcm_waveformat.wf.n_block_align);
            printf("w_bits_per_sample: %d\n", wi.info.pcm_waveformat.w_bits_per_sample);
            break;
        case wav::WaveFormatType::WAVEFORMATEX:
            printf("format_type: waveformat_ex\n");
            printf("w_format_tag: %d\n", wi.info.waveformat_ex.w_format_tag);
            printf("n_channels: %d\n", wi.info.waveformat_ex.n_channels);
            printf("n_samples_per_sec: %d\n", wi.info.waveformat_ex.n_samples_per_sec);
            printf("n_samples_per_sec: %d\n", wi.info.waveformat_ex.n_avg_bytes_per_sec);
            printf("n_avg_bytes_per_sec: %d\n", wi.info.waveformat_ex.n_block_align);
            printf("w_bits_per_sample: %d\n", wi.info.waveformat_ex.w_bits_per_sample);
            printf("cb_size: %d\n", wi.info.waveformat_ex.cb_size);
            break;
        case wav::WaveFormatType::WAVEFORMATEXTENSIBLE:
            printf("format_type: waveformat_extensible\n");
            printf("w_format_tag: %d\n", wi.info.waveformat_extensible.format.w_format_tag);
            printf("n_channels: %d\n", wi.info.waveformat_extensible.format.n_channels);
            printf("n_samples_per_sec: %d\n", wi.info.waveformat_extensible.format.n_samples_per_sec);
            printf("n_samples_per_sec: %d\n", wi.info.waveformat_extensible.format.n_avg_bytes_per_sec);
            printf("n_avg_bytes_per_sec: %d\n", wi.info.waveformat_extensible.format.n_block_align);
            printf("w_bits_per_sample: %d\n", wi.info.waveformat_extensible.format.w_bits_per_sample);
            printf("w_valid_bits_per_sample: %d\n", wi.info.waveformat_extensible.samples.w_valid_bits_per_sample);
            printf("sub_format: %d", wi.info.waveformat_extensible.sub_format);
            break;
    }
}

uint8_t* ConvertTo16bit(wav::WAVEINFO* wi, uint8_t* data) {
    uint16_t sample_size = wi->info.pcm_waveformat.w_bits_per_sample / 8;
    uint32_t sample_num = wi->data_size / sample_size;
    uint8_t* new_data = (uint8_t*)malloc(sample_num * 2);

    for(int i = 0; i < sample_num; i++) {
        uint64_t buffer = 0;
        for(int j = 0; j < sample_size; j++) {
            buffer = (buffer << 8) | data[i * sample_size + (sample_size - j - 1)];
        }

        buffer ^= 1 << (sample_size * 8 - 1);
        buffer = (double)buffer / ((1 << (sample_size * 8)) - 1) * 65535 - 32768;
        new_data[i * 2    ] = (buffer     ) & 0xFF;
        new_data[i * 2 + 1] = (buffer >> 8) & 0xFF;
    }

    wi->data_size = sample_num * 2;

    return new_data;
}

void Play(ALuint source) {
	alSourcePlay(source);
}


int main(int argc, char** argv) {
    uint8_t* data = NULL;
    wav::WAVEINFO wi;
    ALsizei sampling_rate = 0;
    ALenum bit_per_sample;

    if(argc < 2) {
        printf("too few arguments!");
        return 1;
    }

    if(wav::read(argv[1], &data, &wi) == FAILED) {
        printf("reading wav is failed");
        if(data != NULL) {
            free(data);
        }

        return 1;
    }

    ShowWaveInfo(wi);

    if(wi.format_type == wav::WaveFormatType::WAVEFORMAT) {
        sampling_rate = wi.info.waveformat.n_samples_per_sec;
        bit_per_sample = AL_FORMAT_MONO8;
    } else if(wi.format_type == wav::WaveFormatType::PCMWAVEFORMAT) {
        sampling_rate = wi.info.pcm_waveformat.wf.n_samples_per_sec;
        if(wi.info.pcm_waveformat.w_bits_per_sample == 8) {
            if(wi.info.pcm_waveformat.wf.n_channels == 2) {
                bit_per_sample = AL_FORMAT_STEREO8;
            } else {
                bit_per_sample = AL_FORMAT_MONO8;
            }
        } else if(wi.info.pcm_waveformat.w_bits_per_sample == 16) {
            if(wi.info.pcm_waveformat.wf.n_channels == 2) {
                bit_per_sample = AL_FORMAT_STEREO16;
            } else {
                bit_per_sample = AL_FORMAT_MONO16;
            }
        } else {
            uint8_t* new_data = ConvertTo16bit(&wi, data);
            free(data);
            data = new_data;

            if(wi.info.pcm_waveformat.wf.n_channels == 2) {
                bit_per_sample = AL_FORMAT_STEREO16;
            } else {
                bit_per_sample = AL_FORMAT_MONO16;
            }
        }
    } else if(wi.format_type == wav::WaveFormatType::WAVEFORMATEX) {
        sampling_rate = wi.info.waveformat_ex.n_samples_per_sec;
        if(wi.info.waveformat_ex.w_bits_per_sample == 8) {
            if(wi.info.waveformat_ex.n_channels == 2) {
                bit_per_sample = AL_FORMAT_STEREO8;
            } else {
                bit_per_sample = AL_FORMAT_MONO8;
            }
        } else if(wi.info.waveformat_ex.w_bits_per_sample == 16) {
            if(wi.info.waveformat_ex.n_channels == 2) {
                bit_per_sample = AL_FORMAT_STEREO16;
            } else {
                bit_per_sample = AL_FORMAT_MONO16;
            }
        } else {
            uint8_t* new_data = ConvertTo16bit(&wi, data);
            free(data);
            data = new_data;

            if(wi.info.pcm_waveformat.wf.n_channels == 2) {
                bit_per_sample = AL_FORMAT_STEREO16;
            } else {
                bit_per_sample = AL_FORMAT_MONO16;
            }
        }
    } else if(wi.format_type == wav::WaveFormatType::WAVEFORMATEXTENSIBLE) {
        sampling_rate = wi.info.waveformat_extensible.format.n_samples_per_sec;
        if(wi.info.waveformat_extensible.format.w_bits_per_sample == 8) {
            if(wi.info.waveformat_extensible.format.n_channels == 2) {
                bit_per_sample = AL_FORMAT_STEREO8;
            } else {
                bit_per_sample = AL_FORMAT_MONO8;
            }
        } else if(wi.info.waveformat_extensible.format.w_bits_per_sample == 16) {
            if(wi.info.waveformat_extensible.format.n_channels == 2) {
                bit_per_sample = AL_FORMAT_STEREO16;
            } else {
                bit_per_sample = AL_FORMAT_MONO16;
            }
        } else {
            uint8_t* new_data = ConvertTo16bit(&wi, data);
            free(data);
            data = new_data;

            if(wi.info.pcm_waveformat.wf.n_channels == 2) {
                bit_per_sample = AL_FORMAT_STEREO16;
            } else {
                bit_per_sample = AL_FORMAT_MONO16;
            }
        }
    }

	ALCdevice *device = alcOpenDevice(NULL);
	ALCcontext *context = alcCreateContext(device, NULL);
	alcMakeContextCurrent(context);
	
	ALuint buffer;
	ALuint source;
	alGenBuffers(1, &buffer);
	alGenSources(1, &source);

	alBufferData(buffer, bit_per_sample, data, wi.data_size, sampling_rate);

    if(data != NULL) {
        free(data);
    }

	alSourcei(source, AL_BUFFER, buffer);
	alSourcei(source, AL_LOOPING, AL_TRUE);
    alSourcef(source, AL_GAIN, 0.01);

    Play(source);

	system("PAUSE");

	alDeleteBuffers(1, &buffer);
	alDeleteSources(1, &source);
	
	alcMakeContextCurrent(nullptr);
	alcDestroyContext(context);
	alcCloseDevice(device);

    return 0;
}
