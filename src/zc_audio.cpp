/*
    Copyright 2026, Philip Rose, GM3ZZA

    This file is part of ZZACOMMON.

    ZZACOMMON is free software: you can redistribute it and/or modify it under the
    terms of the Lesser GNU General Public License as published by the Free Software
    Foundation, either version 3 of the License, or (at your option) any later version.

    ZZACOMMON is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
    without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
    PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with ZZACOMMON.
    If not, see <https://www.gnu.org/licenses/>.

*/

#include "zc_audio.h"

#include "zc_async_queue.h"

#include "portaudio.h"
#ifdef _WIN32
#include "pa_win_wasapi.h"
#endif

#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <iostream>
#include <list>
#include <map>
#include <queue>
#include <stdexcept>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

extern double DEFAULT_SAMPLE_RATE;
extern int BUFFER_DEPTH;

namespace {
    zc_audio_base::logger_t g_logger = nullptr;

    constexpr auto ST_ERROR = zc_audio_base::log_level_t::Error;
    constexpr auto ST_OK = zc_audio_base::log_level_t::Ok;
    constexpr auto ST_NOTE = zc_audio_base::log_level_t::Note;

    // Printf-style reporter; keeps the status_->misc_status(...) call shape used below
    struct audio_status {
        void misc_status(zc_audio_base::log_level_t level, const char* format, ...) {
            char message[1024];
            va_list args;
            va_start(args, format);
            vsnprintf(message, sizeof(message), format, args);
            va_end(args);
            if (g_logger) {
                g_logger(level, message);
            }
            else {
                fprintf(level == zc_audio_base::log_level_t::Error ? stderr : stdout, "%s\n", message);
            }
        }
    };
    audio_status audio_status_instance;
    audio_status* status_ = &audio_status_instance;
}

void zc_audio_base::set_logger(logger_t logger) {
    g_logger = logger;
}

//! \brief Constructor.
//! \param app_audio generated audio data queue
zc_audio_base::zc_audio_base(
    zc_audio_direction direction,
    int channels,
    double sample_rate,
    PaSampleFormat format
) {
    direction_ = direction;
    channels_ = channels;
    sample_rate_ = sample_rate;
    sample_format_ = format;
    buffer_depth_ = BUFFER_DEPTH;
    idle_ = true;
    reset();
}

//! Start
bool zc_audio_base::enable() {
    if (state_ != STATE_DISCONNECTED) return false;
	state_ = STATE_CONNECTING;
    if (port_ids_.find(port_index_) == port_ids_.end() || !use_port(port_ids_.at(port_index_))) {
        if (status_) {
            status_->misc_status(ST_ERROR, "Failed to start port.");
        }
        state_ = STATE_DISCONNECTED;
        return false;
    }
    state_ = STATE_CONNECTED;
    return true;
}

//! Destructor
zc_audio_base::~zc_audio_base() {
    if (state_ == STATE_CONNECTED) disconnect_port();
}

//! Reset
bool zc_audio_base::reset() {
    if (state_ == STATE_DISCONNECTED) return false;
    if (state_ == STATE_CONNECTED) {
        state_ = STATE_DISCONNECTING;
        while (!idle_) std::this_thread::yield();
        disconnect_port();
    }

    PaError err;

    if (state_ == STATE_RESET) {
        /* Initialize library before making any other calls. */
        err = Pa_Initialize();
        if (err != paNoError) {
            if (status_) {
                status_->misc_status(ST_ERROR, "Unable to initialise portaudio");
            }
            return false;
        }
        state_ = STATE_DISCONNECTED;
    }
    enumerate_ports();
    bool found = false;
    if (port_index_ >= 0) {
        port_id& current_port = port_ids_.at(port_index_);
        status_->misc_status(ST_OK, "Port %s/%s reset OK",
            current_port.audio_host.c_str(), current_port.port_name.c_str());
        return true;
    }
    return false;
}

//! \brief Callback from PortAudio.
//! \param output buffer to receive audio data
//! \param frameCount Njmber of frames per buffer
//! \param timeInfo ?
//! \param statusFlags ?
//! \param userData pointer to  the zc_audio instance
int zc_audio_base::cb_pa_stream(const void* input,
    void* output,
    unsigned long frameCount,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData) {
    zc_audio_base* that = (zc_audio_base*)userData;
    return that->pa_stream(input, output, frameCount, timeInfo, statusFlags);
}

//! Constructor
template <typename T>
zc_audio<T>::zc_audio(
    zc_audio_direction direction,
    int channels,
    double sample_rate,
    zc_async_queue<T>* audio_data,
    zc_async_queue<T>* monitor_data
) : zc_audio_base(direction, channels, sample_rate,
        std::is_same_v<T, int16_t> ? paInt16 : paFloat32) {
    app_audio_ = audio_data;
    monitor_audio_ = monitor_data;
}

//! Destructor - stop the stream before the queues' owner-facing members go away
template <typename T>
zc_audio<T>::~zc_audio() {
    if (state_ == STATE_CONNECTED) disconnect_port();
}

//! \brief instance specific version of callback
//! \param output buffer to receive audio data
//! \param frame_count Number of frames per buffer
//! \param time_info ?
//! \param status_flags ?
template <typename T>
int zc_audio<T>::pa_stream(const void* input,
    void* output,
    unsigned long frame_count,
    const PaStreamCallbackTimeInfo* time_info,
    PaStreamCallbackFlags status_flags) {
    // PortAudio buffer type: int16_t natively, double is carried as float32.
    using pa_t = std::conditional_t<std::is_same_v<T, int16_t>, int16_t, float>;
    if (direction_ == zc_audio_direction::AUDIO_OUT && output) {
        pa_t* out = (pa_t*)output;
        unsigned long samples_to_send = frame_count;
        while (samples_to_send) {
            // Channel data is interleaved in both app and portaudio.
            for (int i = 0; i < channels_; i++) {
                T sample;
                if (!app_audio_->try_pop(sample)) {
                    sample = T(0);
                }
                *(out++) = static_cast<pa_t>(sample);
                if (monitor_audio_) {
                    monitor_audio_->push(sample);
                }
                if (!idle_ && !samples_to_send) {
                    idle_ = true;
                }
            }
            samples_to_send--;
        }
    }
    else if (direction_ == zc_audio_direction::AUDIO_IN && input) {
        const pa_t* in = (const pa_t*)input;
        unsigned long samples_to_receive = frame_count;
        while (samples_to_receive) {
            // Channel data is interleaved in bot app and port.
            for (int i = 0; i < channels_; i++) {
                app_audio_->push(static_cast<T>(*(in++)));
            }
            samples_to_receive--;
        }
    }
    else {
        throw std::logic_error("Input/output mismatch");
    }
    return paContinue;
}

template class zc_audio<double>;
template class zc_audio<int16_t>;

//! Initialise portaudio
bool zc_audio_base::initialise_port() {
    if (state_ != STATE_DISCONNECTED && state_ != STATE_CONNECTING) return false;
    state_ = STATE_CONNECTING;
    PaError err;
    const PaDeviceInfo* info = Pa_GetDeviceInfo(port_index_);
    port_id current_port = port_ids_.at(port_index_);
    // Sanity check that hostApi and name still match
    const PaHostApiInfo* api_info = Pa_GetHostApiInfo(info->hostApi);
    if (info->name != current_port.port_name || api_info->name != current_port.audio_host) {
        status_->misc_status(ST_ERROR, "Portaudio mismatch");
        return false;
    }

    /* Open an audio I/O stream. */
    if (direction_ == zc_audio_direction::AUDIO_OUT) 
        err = Pa_OpenStream(
            &stream_,
            nullptr,   
            &parameters_,
            sample_rate_,
            buffer_depth_,        /* frames per buffer */
            paClipOff,
            cb_pa_stream,         // 
            this);
    else 
        err = Pa_OpenStream(
            &stream_,
            &parameters_,
            nullptr,
            sample_rate_,
            paFramesPerBufferUnspecified,        /* frames per buffer */
            paClipOff,
            cb_pa_stream,         // 
            this);

    if (err != paNoError) {
        if (status_) {
            status_->misc_status(ST_ERROR, "Port %d(%s/%s) could not be opened",
                port_index_, current_port.audio_host.c_str(), current_port.port_name.c_str());
        }
		state_ = STATE_DISCONNECTED;
        return false;
    }


    err = Pa_StartStream(stream_);
    if (err != paNoError) {
        if (status_) {
            status_->misc_status(ST_ERROR, "Port %d(%s/%s) could not be started",
                port_index_, current_port.audio_host.c_str(), current_port.port_name.c_str());
        }
        state_ = STATE_DISCONNECTED;
        return false;
    }
    else {
		if (status_) {
			status_->misc_status(ST_OK, "Port %d(%s/%s) started OK",
				port_index_, current_port.audio_host.c_str(), current_port.port_name.c_str());
		}
    }
    state_ = STATE_CONNECTED;
    return true;

}

bool zc_audio_base::idle() const {
    return idle_;
}

bool zc_audio_base::ready() const {
    return state_ != STATE_RESET;

}

bool zc_audio_base::enabled() const {
    return state_ == STATE_CONNECTED;
}

bool zc_audio_base::disconnect_port() {
	if (state_ == STATE_DISCONNECTED) return false;
	state_ = STATE_DISCONNECTING;
    port_id current_port = port_ids_.at(port_index_);
    PaError err = paNoError;
    err = Pa_StopStream(stream_);
    if (err == paNoError) {
        err = Pa_CloseStream(stream_);
    }
    if (err == paNoError) {
        if (status_) {
            status_->misc_status(ST_OK, "Port %d(%s/%s) disconnected OK", port_index_,
                current_port.audio_host.c_str(), current_port.port_name.c_str());
        }
        port_index_ = -1;
        state_ = STATE_DISCONNECTED;
        return true;
    }
    else {
        if (status_) {
            status_->misc_status(ST_ERROR, "Fail to disconnect port %d(%s/%s): %s", port_index_,
                current_port.audio_host.c_str(), current_port.port_name.c_str(), Pa_GetErrorText(err));
        }
        return false;
    }
}

bool zc_audio_base::close_pa() {
    PaError err = paNoError;
    err = Pa_Terminate();
    if (err != paNoError) {
        if (status_) {
            status_->misc_status(ST_ERROR, "Error closing portaudio: Code: %d: %s",
                err, Pa_GetErrorText(err));
        }
        return false;
    }
    return true;
}

const std::list<zc_audio_base::port_id> zc_audio_base::get_ports() {
    std::list<port_id> result;
    for (auto& id : port_ids_) {
        result.push_back(id.second);
    }
    return result;
}

void zc_audio_base::enumerate_ports() {
    // If neither ready nor enabled return an empty list
    if (state_ == STATE_RESET) return;
    // Now enumerate all the ports
    PaError err;
    PaDeviceIndex num_devices = Pa_GetDeviceCount();
    char t[128];
    const PaDeviceInfo* info;
    port_ids_.clear();
    if (num_devices < 0) {
        if (status_) {
            status_->misc_status(ST_ERROR, "No audio devices");
        }
        return;
    }
    for (auto ix = 0; ix < num_devices; ix++) {
        info = Pa_GetDeviceInfo(ix);
        if (info) {
            const PaHostApiInfo* api_info = Pa_GetHostApiInfo(info->hostApi);
            printf("DEBUG: Checking port %d (%s/%s)", ix, api_info->name, info->name);
            // Set output stream parametsr
            PaStreamParameters parameters;
            parameters.device = ix;   // \todo set up from settings
            parameters.channelCount = channels_;       
            parameters.sampleFormat = sample_format_;
            parameters.suggestedLatency =
                info->defaultHighOutputLatency;
#ifdef _WIN32
            // Set WASAPI parameters - allow up/down sampling
            if (api_info->type == paWASAPI) {
                PaWasapiStreamInfo wasapi_info;
                wasapi_info.size = sizeof(PaWasapiStreamInfo);
                wasapi_info.hostApiType = paWASAPI;
                wasapi_info.version = 1;
                wasapi_info.flags = paWinWasapiAutoConvert;
                parameters.hostApiSpecificStreamInfo = &wasapi_info;
            }
            else {
                parameters.hostApiSpecificStreamInfo = nullptr;
            }
#else
            parameters.hostApiSpecificStreamInfo = nullptr;
#endif

            if (direction_ == zc_audio_direction::AUDIO_OUT)
                err = Pa_IsFormatSupported(nullptr, &parameters, sample_rate_);
            else 
                err = Pa_IsFormatSupported(&parameters, nullptr, sample_rate_);

            if (err == paFormatIsSupported) {
                port_id id;
                id.audio_host = api_info->name;
                id.port_name = info->name;
                port_ids_[ix] = id;
                printf(" - OK\n");
            }
            else {
                printf(" - Not OK - %s\n", Pa_GetErrorText(err));
            }
        }
    }
}

bool zc_audio_base::use_port(const zc_audio_base::port_id& id) {
    // We have no portaudio
    if (state_ == STATE_RESET) return false;
    // We have another port currently connected
    if (state_ == STATE_CONNECTED) {
        port_id& current_port = port_ids_.at(port_index_);
        if (id.audio_host == current_port.audio_host &&
            id.port_name == current_port.port_name) {
            if (status_) {
                status_->misc_status(ST_NOTE, "Already using audio port %s/%s", 
                    id.audio_host.c_str(), id.port_name.c_str());
            }
            return true;
        }
        disconnect_port();
    }
    port_index_ = get_index(id);
    if (port_index_ == -1) {
        if (status_) {
            status_->misc_status(ST_ERROR, "Invalid audio port %s/%s selected", 
                id.audio_host.c_str(), id.port_name.c_str());
        }
        return false;
    }
    // Set new output stream parameters
    const PaDeviceInfo* info = Pa_GetDeviceInfo(port_index_);
    const PaHostApiInfo* api_info = Pa_GetHostApiInfo(info->hostApi);
    parameters_.device = port_index_;   // 
    parameters_.channelCount = channels_;       
    parameters_.sampleFormat = sample_format_;
    parameters_.suggestedLatency =
        info->defaultHighOutputLatency;
#ifdef _WIN32
    // Set WASAPI parameters - allow up/down samplings
    if (api_info->type == paWASAPI && id.audio_host == "Windows WASAPI") {
        PaWasapiStreamInfo wasapi_info;
        wasapi_info.size = sizeof(PaWasapiStreamInfo);
        wasapi_info.hostApiType = paWASAPI;
        wasapi_info.version = 1;
        wasapi_info.flags = paWinWasapiAutoConvert;
        parameters_.hostApiSpecificStreamInfo = &wasapi_info;
    }
    else {
        parameters_.hostApiSpecificStreamInfo = nullptr;
    }
#else
    parameters_.hostApiSpecificStreamInfo = nullptr;
#endif
    // Set new port
    return initialise_port();
}

void zc_audio_base::buffer_depth(int d) {
    // Only change the buffer depth when not active
    if (state_ != STATE_DISCONNECTED) return;
    buffer_depth_ = d;
}

int zc_audio_base::buffer_depth() const {
    return buffer_depth_;
}

void zc_audio_base::volume(double v) {
    volume_ = v;
    vol_xier_ = v2x(v);
}

double zc_audio_base::volume() const {
    return volume_;
}

double zc_audio_base::v2x(double v) {
    if (v == MINIMUM_VOLUME) return 0.0;
    else {
        // Voltage ratio.
        double result = pow(10.0, (v * 0.05));
        return result;
    }
}

// Get the index of the defined id
PaDeviceIndex zc_audio_base::get_index(zc_audio_base::port_id id) {
    for (auto& idr : port_ids_) {
        if (id.audio_host == idr.second.audio_host &&
            id.port_name == idr.second.port_name) {
            return idr.first;
        }
    }
    return -1;
}

