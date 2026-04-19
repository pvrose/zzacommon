#pragma once

#include <queue>
#include <string>

//! \brief The structure of audio data passed between the audio generation and the speaker.
//! This can be used to pass metadata about the audio, such as the text that was synthesised.
struct zc_audio_data {
    //! The audio data, as a queue of samples.
    std::queue<float> data = {};
    //! Metadata about the audio, such as the text that was synthesised.
    std::string metadata = "";
};
