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

//! \file zc_audio.h
//! \brief Audio input/output using PortAudio

// PortAudio
#include "portaudio.h"

#include "zc_async_queue.h"

#include <atomic>
#include <cstdint>
#include <functional>
#include <list>
#include <map>
#include <queue>
#include <string>
#include <thread>
#include <vector>

//! Minimum speech volume (dB relative to full-scale) 
//! data is made zero at this value.
constexpr double MINIMUM_VOLUME = -40.0F;
extern double DEFAULT_SAMPLE_RATE;

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
    //! \param sample_rate Number of audio samples per second.
    //! \param audio_data Audio data stream
    //! \param monitor_data Monitored data stream (Output only)
    zc_audio(
        zc_audio_direction direction,
        int channels,
        double sample_rate,
        zc_async_queue<double>* audio_data,
        zc_async_queue<double>* monitor_data = nullptr
    );

    //! \brief Legacy constructor (without sample rate)
    //! \param direction Input or output
    //! \param channels Number of audio channels (Mono = 1, Stereo = 2 etc.)
    //! \param audio_data Audio data stream
    //! \param monitor_data Monitored data stream (Output only)
    zc_audio(
        zc_audio_direction direction,
        int channels,
        zc_async_queue<double>* audio_data,
        zc_async_queue<double>* monitor_data = nullptr
    ) : zc_audio(direction, channels, DEFAULT_SAMPLE_RATE, audio_data, monitor_data) {
    };

    //! Destructor
    ~zc_audio();

    //! Internal state
    enum state_t : uint8_t {
		STATE_RESET,         //!< Reset state, not initialised
		STATE_DISCONNECTED , //!< Portaudio initialised, but not connected
		STATE_CONNECTING,    //!< Portaudio initialised, port is being connected.
		STATE_CONNECTED,     //!< Portaudio initialised and connected
		STATE_DISCONNECTING, //!< Portaudio initialised, port is being disconnected.
    };

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

    //! Identification for an audio port.
    struct port_id {
        std::string audio_host;   //!< System API name: eg "WASAPI" or "pulse"
        std::string port_name;    //!< Specific port name

    };

    //! Set the port 
    //! \param id Port identifier
    bool use_port(const port_id& id);

    //! Get the identifiers of the available ports at the current sample rate and direction
    const std::list<port_id> get_ports();

    //! Return the identifier of the current active port.
    const port_id get_port();

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

    //! \brief Disconnect current port
    bool disconnect_port();

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

    //! \brief Convert volume control value (-20dB to 0) to multiplier
    double v2x(double v);

    //! \brief Supplied port identifier is valid
    bool valid_port(port_id id) {
        if (id.audio_host.empty() || id.port_name.empty()) {
            return false;
        }
        else {
            return true;
        }
    }

    //! \brief Enumerate ports
    void enumerate_ports();

    //! \brief Get port index
    //! \return -1 if fail
    PaDeviceIndex get_index(port_id id);

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

    //! Audio host API index
    PaHostApiIndex api_index_ = -1;

    //! Audio port index
    PaDeviceIndex port_index_ = -1;

    //! List of port identifiers - indexed by PaDeviceIndex
    std::map<PaDeviceIndex, port_id> port_ids_;

    //! Idle
    bool idle_ = false;

    //! Internal state
    state_t state_ = STATE_RESET;

    //! Direction 
    zc_audio_direction direction_ = zc_audio_direction::AUDIO_OUT;

    //! Number of channels
    int channels_ = 0;

};
