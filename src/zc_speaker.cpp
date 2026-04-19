#include "zc_speaker.h"

#include "zc_settings.h"
#include "zc_audio_data.h"
#include "zc_status.h"

#include "portaudio.h"

#include <cmath>
#include <cstdio>
#include <iostream>
#include <queue>
#include <string>
#include <thread>
#include <vector>

// Default audio configuration constants
// These can be overridden by the application using zc_speaker
constexpr float DEFAULT_SAMPLE_RATE = 22050.0F;  //!< Default audio sample rate
const int BUFFER_DEPTH = 1024;  //!< Default buffer size

//! \brief Constructor.
//! \param speech synthesised audio data
zc_speaker::zc_speaker(std::queue<zc_audio_data>* speech) {
    speech_ = speech;
    idle_ = true;
    enabled_ = false;
    ready_ = false;
    next_buffer_ = new zc_audio_data;
    samples_sent_ = 0;
    load_settings();
    reset();
}

//! Start
bool zc_speaker::enable() {
    if (!ready_) return false;
    if (!use_port(port_number_)) {
        if (status_) {
            status_->misc_status(ST_ERROR, "Failed to start port.");
        }
        ready_ = true;
        return false;
    }
    enabled_ = true;
    return true;
}

//! Destructor
zc_speaker::~zc_speaker() {
    if (enabled_) disconnect_port();
    save_settings();
}

//! Reset
bool zc_speaker::reset() {
    if (ready_) return false;
    if (enabled_) {
        enabled_ = false;
        while (!idle_) std::this_thread::yield();
        disconnect_port();
    }
    sample_rate_ = DEFAULT_SAMPLE_RATE;

    PaError err;

    if (!pa_initialised_) {
        /* Initialize library before making any other calls. */
        err = Pa_Initialize();
        if (err != paNoError) {
            if (status_) {
                status_->misc_status(ST_ERROR, "Unable to initialise portaudio");
            }
            return false;
        }
        pa_initialised_ = true;
    }
    ready_ = true;
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
        } else {
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
//! \param userData pointer to  the zc_speaker instance
int zc_speaker::cb_pa_stream(const void* input,
    void* output,
    unsigned long frameCount,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData) {
    zc_speaker* that = (zc_speaker*)userData;
    return that->pa_stream(input, output, frameCount, timeInfo, statusFlags);
}


//! LOad the settings 
void zc_speaker::load_settings() {
    zc_settings top_settings;
    top_settings.get("Output Buffer Depth", buffer_depth_, BUFFER_DEPTH);
    top_settings.get("Output Volume", volume_, 0.0F);
    vol_xier_ = v2x(volume_);
    top_settings.get("Output Port Name", port_name_, std::string(""));
}

// Save the settings
void zc_speaker::save_settings() {
    zc_settings top_settings;
    top_settings.set("Output Buffer Depth", buffer_depth_);
    top_settings.set("Output Volume", volume_);
    top_settings.set("Output Port Name", port_name_);
}

//! \brief instance specific version of callback
//! \param output buffer to receive audio data
//! \param frame_count Njmber of frames per buffer
//! \param time_info ?
//! \param status_flags ?
int zc_speaker::pa_stream(const void* input,
    void* output,
    unsigned long frame_count,
    const PaStreamCallbackTimeInfo* time_info,
    PaStreamCallbackFlags status_flags) {
    float* out = (float*)output;
    unsigned long samples_to_send = frame_count;
    while (samples_to_send) {
        if (!next_buffer_->data.empty()) {
            // First copy any left over samples from next_buffer
            idle_ = false;
            float sample = next_buffer_->data.front() * vol_xier_;
            *(out++) = sample;
            if (audio_callback_) {
                audio_callback_(sample);
            }
            next_buffer_->data.pop();
            samples_sent_++;
            samples_to_send--;
        } 
        else if (speech_->empty()) {
            // If no data in input queue send silence
            samples_sent_ = 0;
            *(out++) = 0.0;
            if (audio_callback_) {
                audio_callback_(0.0F);
            }
            samples_to_send--;
            if (!idle_ && !samples_to_send) {
                idle_ = true;
            }
        }
        else {
            // Otherwise get next audio chunk from the input queue and check again.
            samples_sent_ = 0;
            idle_ = false;
            *next_buffer_ = speech_->front();
            // Add the text to the output display window and wake up the GUI.
            if (text_callback_) {
                text_callback_(next_buffer_->metadata);
            }
            Fl::awake();
            speech_->pop();
        }
     }
    return paContinue;    
}

//! Initialise portaudio
bool zc_speaker::initialise_port() {
    if (enabled_ || !ready_) return false;
    ready_ = false;
    PaError err;
    const PaDeviceInfo* info = Pa_GetDeviceInfo(port_index_);
    /* Open an audio I/O stream. */
	err = Pa_OpenStream(
        &stream_,
		nullptr,          /* no input channels */
		&parameters_,       
		sample_rate_,
		buffer_depth_,        /* frames per buffer */
        paClipOff,
		cb_pa_stream,         // 
		this);      
    if (err != paNoError) {
        if (status_) {
            status_->misc_status(ST_ERROR,"Port %d(%s) could not be opened",
                port_index_, port_name_.c_str());
        }
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
    enabled_ = true;
	return true;

}

bool zc_speaker::idle() const {
    return idle_;
}

bool zc_speaker::ready() const {
    return ready_;
}

bool zc_speaker::enabled() const {
    return enabled_;
}

bool zc_speaker::disconnect_port() {
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
        return true;
    }
    else {
        if (status_) {
            status_->misc_status(ST_ERROR, "Fail to disconnect port %d (%s)", port_index_, Pa_GetErrorText(err));
        }
        return false;
    }
}

bool zc_speaker::close_pa() {
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

std::vector<std::string> zc_speaker::get_ports(float sample_rate) {
    // If neither ready nor enabled return an empty list
    if (!ready_  && !enabled_) return {};
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
                printf("DEBUG: Checking port %d (%s)", ix, info->name);
                // Set output stream parametsr
                PaStreamParameters parameters;
                parameters.device = ix;   // \todo set up from settings
                parameters.channelCount = 1;         // Mono
                parameters.sampleFormat = paFloat32;    // float
                parameters.suggestedLatency =
                    info->defaultLowOutputLatency;
                parameters.hostApiSpecificStreamInfo = nullptr;
                err = Pa_IsFormatSupported(nullptr, &parameters, sample_rate);
                if (err == paFormatIsSupported) {
                    if (ix == port_index_) port_number_ = port_indices_.size();
                    port_indices_.push_back(ix);
                    port_names_.push_back(std::string(info->name));
                    snprintf(t, sizeof(t), "%0d: %s", ix, info->name);
                    annotated_names_.push_back(std::string(t));
                    printf(" - OK\n");
                }
                else {
                    printf(" - Not OK\n");
                }
            }
        }
    }
    return annotated_names_;
}

bool zc_speaker::use_port(int port_number) {
    // We have no portaudio
    if (!ready_ && !enabled_) return false;
    // We have another port currently connected
    if (enabled_) {
        enabled_ = false;
        PaError err = paNoError;
        err = Pa_StopStream(stream_);
        if (err == paNoError) {
            err = Pa_CloseStream(stream_);
        }
        if (err != paNoError) {
            if (status_) {
                status_->misc_status(ST_ERROR, "Unable to stop current audio device");
            }
            return false;
        }
        ready_ = true;
    }
    if (port_number < 0 || port_number >= port_indices_.size()) {
        if (status_) {
            status_->misc_status(ST_ERROR, "Invalid audio port selected");
        }
        return false;
    }
    port_number_ = port_number;
    port_index_ = port_indices_[port_number];
    // Set new output stream parameters
    parameters_.device = port_index_;   // 
    parameters_.channelCount = 1;         // Mono
    parameters_.sampleFormat = paFloat32;    // float
    parameters_.suggestedLatency =
        Pa_GetDeviceInfo(parameters_.device)->defaultLowOutputLatency;
    parameters_.hostApiSpecificStreamInfo = nullptr;
    // Set new port
    return initialise_port();
}

void zc_speaker::buffer_depth(int d) {
    // Only change the buffer depth when not active
    if (!ready_) return;
    buffer_depth_ = d;
}

int zc_speaker::buffer_depth() const {
    return buffer_depth_;
}

void zc_speaker::volume(float v) {
    volume_ = v;
    vol_xier_ = v2x(v);
}

float zc_speaker::volume() const {
    return volume_;
}

int zc_speaker::port_number() const {
    return port_number_;
}

float zc_speaker::v2x(float v) {
    if (v == MINIMUM_VOLUME) return 0.0;
    else {
        // Voltage ratio.
        float result = pow(10.0, (v * 0.05));
        return result;
    }
}

void zc_speaker::set_audio_callback(std::function<void(float)> callback) {
    audio_callback_ = callback;
}

void zc_speaker::set_text_callback(std::function<void(const std::string&)> callback) {
    text_callback_ = callback;
}
