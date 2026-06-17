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
#pragma once

//! \file zc_speaker.h
//! \brief Audio input/output using PortAudio

// PortAudio
#include "portaudio.h"

#include "zc_async_queue.h"

#include <atomic>
#include <cstdint>
#include <functional>
#include <queue>
#include <string>
#include <thread>
#include <vector>

//! Minimum speech volume (dB relative to full-scale) 
//! data is made zero at this value.
constexpr double MINIMUM_VOLUME = -40.0F;

enum zc_audio_direction : uint8_t {
    AUDIO_IN,
    AUDIO_OUT
};

//! \brief This class provides a wrapper for the Portaudio interface.
//! It supports either output (speaker) or input (microphone).
class zc_audio {

public:
    //! \brief Constructor.
    //! \param direction Input or output
    //! \param channels Number of audio channels (Mono = 1, Stereo = 2 etc.)
    //! \param audio_data Audio data stream
    //! \param monitor_data Monitored data stream (Output only)
    zc_audio(
        zc_audio_direction direction,
        int channels,
        zc_async_queue<double>* audio_data,
        zc_async_queue<double>* monitor_data = nullptr
    );

    //! Destructor
    ~zc_audio();

    //! \brief Enable audio out.
    //! \return true if successfully enabled.
    bool enable();

    //! \brief Reset back to disconnected, but connectable state.
    //! \return true if successful
    bool reset();

    //! Returns true if audio out is initialised.
    bool ready() const;

    //! Returns true of audio out is enabled.
    bool enabled() const;

    //! Returns true if audio not being output.
    bool idle() const;

    //! Set the port 
    //! \param port_number local index of the port to be used.
    bool use_port(int port_number);

    //! Get the available ports that support \param sample_rate
    std::vector<std::string> get_ports(double sample_rate);

    //! Set buffer depth
    void buffer_depth(int d);
    //! Get buffer depth
    int buffer_depth() const;

    //! Set volume
    //! \param v Volume (dB relative to full output)
    void volume(double v);
    //! Get volume
    //! \return Volume (dB releative to full output)
    double volume() const;

    //! Set Sample rate (samples per second)
    void sample_rate(double s) {
        sample_rate_ = s;
    }
    //! Get sample rate (samples per second)
    double sample_rate() const {
        return sample_rate_;
    }

    //! Get port list index
    int port_number() const;

    //! \brief Callback from PortAudio.
    //! \param input buffer containing received audio data.
    //! \param output buffer to receive audio data
    //! \param frameCount Number of samples per buffer
    //! \param timeInfo Timing information
    //! \param statusFlags Stream callback flags
    //! \param userData pointer to the zc_speaker instance
    static int cb_pa_stream(const void* input,
        void* output,
        unsigned long frameCount,
        const PaStreamCallbackTimeInfo* timeInfo,
        PaStreamCallbackFlags statusFlags,
        void* userData);

protected:

    //! \brief instance specific version of callback
    //! \param input Unused input buffer
    //! \param output buffer to receive audio data
    //! \param frameCount Number of frames per buffer
    //! \param timeInfo Timing information
    //! \param statusFlags Stream callback flags
    int pa_stream(const void* input,
        void* output,
        unsigned long frameCount,
        const PaStreamCallbackTimeInfo* timeInfo,
        PaStreamCallbackFlags statusFlags);

    //! \brief Initialise specific port
    bool initialise_port();

    //! \brief Close down portaudio
    bool close_pa();

    //! \brief Disconnect current port
    bool disconnect_port();

    //! \brief Convert volume control value (-20dB to 0) to multiplier
    double v2x(double v);

    //! Audio stream to/from user.
    zc_async_queue<double>* app_audio_ = nullptr;
    //! Montored audio to user
    zc_async_queue<double>* monitor_audio_ = nullptr;

    //! Audio output stream
    PaStream* stream_ = nullptr;

    //! Stream parameters
    PaStreamParameters parameters_;

    //! \todo add audio port configuration data

    //! Sample rate (samples per second)
    double sample_rate_ = 0.0;

    //! Depth of buffer
    int buffer_depth_ = 0;

    //! Volume (between -20 dB and 0 dB)
    double volume_ = 0.0;

    //! Volume multiplier (= 10^(V/10))
    double vol_xier_ = 1.0;

    //! Audio port index
    PaDeviceIndex port_index_;

    //! Local port index
    size_t port_number_ = 0;

    //! Audio port name
    std::string port_name_ = "";

    //! Idle
    bool idle_ = false;

    //! The audio is in operational state.
    bool enabled_ = false;

    //! Portaudio initialised
    bool pa_initialised_ = false;

    //! Portaudio has been initialised and appears to support the required audio.
    //! If neither \a enabled_ nor \a ready_ are true, the audio is in transition between
    //! the two valid states, yet to be initialised, or does not support the
    //! required sample rate.
    bool ready_ = false;

    //! Direction 
    zc_audio_direction direction_;

    //! Number of channels
    int channels_;

    //! List of port indices, itself indexed locally
    std::vector<PaDeviceIndex> port_indices_ = {};
    //! List of port names, indexed by the samae
    std::vector<std::string> port_names_ = {};
    //! List of port names prefixed with index.
    std::vector<std::string> annotated_names_ = {};

};
