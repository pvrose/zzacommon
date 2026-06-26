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
#include "zc_status.h"

#include "portaudio.h"

#include <cmath>
#include <cstdio>
#include <iostream>
#include <queue>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

extern double DEFAULT_SAMPLE_RATE;
extern int BUFFER_DEPTH;

//! \brief Constructor.
//! \param app_audio generated audio data queue
zc_audio::zc_audio(
    zc_audio_direction direction,
    int channels,
    double sample_rate,
    zc_async_queue<double>* audio_data,
    zc_async_queue<double>* monitor_data
) {
    direction_ = direction;
    channels_ = channels;
    app_audio_ = audio_data;
    monitor_audio_ = monitor_data;
    sample_rate_ = sample_rate;
    buffer_depth_ = BUFFER_DEPTH;
    idle_ = true;
    reset();
}

//! Start
bool zc_audio::enable() {
    if (state_ != STATE_DISCONNECTED) return false;
	state_ = STATE_CONNECTING;
    if (!use_port(port_number_)) {
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
zc_audio::~zc_audio() {
    if (state_ == STATE_CONNECTED) disconnect_port();
}

//! Reset
bool zc_audio::reset() {
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
    get_ports(sample_rate_);
    bool found = false;
    if (port_name_.length()) {
        size_t index = 0;
        for (size_t ix = 0; ix < port_names_.size(); ix++) {
            if (port_name_ == port_names_[ix]) {
                found = true;
                port_index_ = port_indices_[ix];
                index = ix;
            }
        }
        if (!found) {
            if (status_) {
                status_->misc_status(ST_ERROR, "Port %s not found!\n", port_name_.c_str());
            }
            port_index_ = 0;
        }
        else {
            port_number_ = index;
        }
    }
    return true;
}

//! \brief Callback from PortAudio.
//! \param output buffer to receive audio data
//! \param frameCount Njmber of frames per buffer
//! \param timeInfo ?
//! \param statusFlags ?
//! \param userData pointer to  the zc_audio instance
int zc_audio::cb_pa_stream(const void* input,
    void* output,
    unsigned long frameCount,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData) {
    zc_audio* that = (zc_audio*)userData;
    return that->pa_stream(input, output, frameCount, timeInfo, statusFlags);
}

//! \brief instance specific version of callback
//! \param output buffer to receive audio data
//! \param frame_count Number of frames per buffer
//! \param time_info ?
//! \param status_flags ?
int zc_audio::pa_stream(const void* input,
    void* output,
    unsigned long frame_count,
    const PaStreamCallbackTimeInfo* time_info,
    PaStreamCallbackFlags status_flags) {
    if (direction_ == zc_audio_direction::AUDIO_OUT && output) {
        float* out = (float*)output;
        unsigned long samples_to_send = frame_count;
        while (samples_to_send) {
            // Channel data is interleaved in both app and portaudio.
            for (int i = 0; i < channels_; i++) {
                double sample;
                if (!app_audio_->try_pop(sample)) {
                    sample = 0.0;
                }
                *(out++) = static_cast<float>(sample);
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
        float* in = (float*)input;
        unsigned long samples_to_receive = frame_count;
        while (samples_to_receive) {
            // Channel data is interleaved in bot app and port.
            for (int i = 0; i < channels_; i++) {
                app_audio_->push(static_cast<double>(*(in++)));
            }
            samples_to_receive--;
        }
    }
    else {
        throw std::logic_error("Input/output mismatch");
    }
    return paContinue;
}

//! Initialise portaudio
bool zc_audio::initialise_port() {
    if (state_ != STATE_DISCONNECTED && state_ != STATE_CONNECTING) return false;
    state_ = STATE_CONNECTING;
    PaError err;
    const PaDeviceInfo* info = Pa_GetDeviceInfo(port_index_);

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
            buffer_depth_,        /* frames per buffer */
            paClipOff,
            cb_pa_stream,         // 
            this);

    if (err != paNoError) {
        if (status_) {
            status_->misc_status(ST_ERROR, "Port %d(%s) could not be opened",
                port_index_, port_name_.c_str());
        }
		state_ = STATE_DISCONNECTED;
        return false;
    }


    err = Pa_StartStream(stream_);
    if (err != paNoError) {
        if (status_) {
            status_->misc_status(ST_ERROR, "Port %d(%s) could not be started",
                port_index_, port_name_.c_str());
        }
        return false;
    }
    port_name_ = info->name;
    state_ = STATE_CONNECTED;
    return true;

}

bool zc_audio::idle() const {
    return idle_;
}

bool zc_audio::ready() const {
    return state_ != STATE_RESET;

}

bool zc_audio::enabled() const {
    return state_ == STATE_CONNECTED;
}

bool zc_audio::disconnect_port() {
	if (state_ == STATE_DISCONNECTED) return false;
	state_ = STATE_DISCONNECTING;
    PaError err = paNoError;
    err = Pa_StopStream(stream_);
    if (err == paNoError) {
        err = Pa_CloseStream(stream_);
    }
    if (err == paNoError) {
        if (status_) {
            status_->misc_status(ST_OK, "Port %d disconnected OK", port_index_);
        }
        port_index_ = -1;
        port_name_ = "";
        state_ = STATE_DISCONNECTED;
        return true;
    }
    else {
        if (status_) {
            status_->misc_status(ST_ERROR, "Fail to disconnect port %d (%s)", port_index_, Pa_GetErrorText(err));
        }
        return false;
    }
}

bool zc_audio::close_pa() {
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

std::vector<std::string> zc_audio::get_ports(double sample_rate) {
    // If neither ready nor enabled return an empty list
    if (state_ == STATE_RESET) return {};
    // Now enumerate all the ports
    PaError err;
    PaDeviceIndex num_devices = Pa_GetDeviceCount();
    char t[128];
    const PaDeviceInfo* info;
    // TODO this will haveto change if ever a variable sample-rate is implemented.
    // For now we assume once these have been read in a session, they do not change.
    if (port_indices_.size() == 0) {
        // port_indices_.clear();
        // port_names_.clear();
        if (num_devices < 0) {
            if (status_) {
                status_->misc_status(ST_ERROR, "No audio devices");
            }
            return annotated_names_;
        }
        for (auto ix = 0; ix < num_devices; ix++) {
            info = Pa_GetDeviceInfo(ix);
            if (info) {
#ifdef _DEBUG
                const PaHostApiInfo* apiInfo = Pa_GetHostApiInfo(info->hostApi);
                printf("DEBUG: Checking port %d (API %s(%d)) (%s)", ix, apiInfo->name, apiInfo->type, info->name);
#endif
                // Set output stream parametsr
                PaStreamParameters parameters;
                parameters.device = ix;   // \todo set up from settings
                parameters.channelCount = channels_;       
                parameters.sampleFormat = paFloat32;    // double
                parameters.suggestedLatency =
                    info->defaultLowOutputLatency;
                parameters.hostApiSpecificStreamInfo = nullptr;

                if (direction_ == zc_audio_direction::AUDIO_OUT)
                    err = Pa_IsFormatSupported(nullptr, &parameters, sample_rate);
                else 
                    err = Pa_IsFormatSupported(&parameters, nullptr, sample_rate);

                if (err == paFormatIsSupported) {
                    if (ix == port_index_) port_number_ = port_indices_.size();
                    port_indices_.push_back(ix);
                    port_names_.push_back(std::string(info->name));
                    snprintf(t, sizeof(t), "%0d: %s", ix, info->name);
                    annotated_names_.push_back(std::string(t));
#ifdef _DEBUG
                    printf(" - OK\n");
                }
                else {
                    printf(" - Not OK\n");
#endif
                }
            }
        }
    }
    return annotated_names_;
}

bool zc_audio::use_port(int port_number) {
    // We have no portaudio
    if (state_ == STATE_RESET) return false;
    // We have another port currently connected
    if (state_ == STATE_CONNECTED) {
        if (port_number == port_number_) {
            if (status_) {
                status_->misc_status(ST_NOTE, "Already using audio port %d", port_indices_[port_number]);
            }
            return true;
        }
        disconnect_port();
    }
    if (port_number < 0 || port_number >= port_indices_.size()) {
        if (status_) {
            status_->misc_status(ST_ERROR, "Invalid audio port %d selected", port_number);
        }
        return false;
    }
    port_number_ = port_number;
    port_index_ = port_indices_[port_number];
    // Set new output stream parameters
    parameters_.device = port_index_;   // 
    parameters_.channelCount = channels_;       
    parameters_.sampleFormat = paFloat32;    // double
    parameters_.suggestedLatency =
        Pa_GetDeviceInfo(parameters_.device)->defaultLowOutputLatency;
    parameters_.hostApiSpecificStreamInfo = nullptr;
    // Set new port
    return initialise_port();
}

void zc_audio::buffer_depth(int d) {
    // Only change the buffer depth when not active
    if (state_ != STATE_DISCONNECTED) return;
    buffer_depth_ = d;
}

int zc_audio::buffer_depth() const {
    return buffer_depth_;
}

void zc_audio::volume(double v) {
    volume_ = v;
    vol_xier_ = v2x(v);
}

double zc_audio::volume() const {
    return volume_;
}

int zc_audio::port_number() const {
    return port_number_;
}

double zc_audio::v2x(double v) {
    if (v == MINIMUM_VOLUME) return 0.0;
    else {
        // Voltage ratio.
        double result = pow(10.0, (v * 0.05));
        return result;
    }
}

