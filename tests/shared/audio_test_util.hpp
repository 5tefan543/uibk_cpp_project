#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <vector>

namespace test {

// Writes a minimal valid mono 16-bit 44100 Hz silent PCM WAV file to the
// given path. Useful for exercising audio code paths without shipping binary
// assets.
inline void writeMinimalWav(const std::filesystem::path &path)
{
    constexpr uint32_t sampleRate = 44100;
    constexpr uint16_t numChannels = 1;
    constexpr uint16_t bitsPerSample = 16;
    constexpr uint32_t numSamples = 4;
    constexpr uint32_t dataSize = numSamples * numChannels * (bitsPerSample / 8U);
    constexpr uint32_t riffSize = 36 + dataSize;
    constexpr uint32_t byteRate = sampleRate * numChannels * (bitsPerSample / 8U);
    constexpr uint16_t blockAlign = numChannels * (bitsPerSample / 8U);
    constexpr uint32_t fmtChunkSize = 16;
    constexpr uint16_t pcmFormat = 1;

    std::ofstream out(path, std::ios::binary);
    out.write("RIFF", 4);
    out.write(reinterpret_cast<const char *>(&riffSize), sizeof(riffSize));
    out.write("WAVE", 4);
    out.write("fmt ", 4);
    out.write(reinterpret_cast<const char *>(&fmtChunkSize), sizeof(fmtChunkSize));
    out.write(reinterpret_cast<const char *>(&pcmFormat), sizeof(pcmFormat));
    out.write(reinterpret_cast<const char *>(&numChannels), sizeof(numChannels));
    out.write(reinterpret_cast<const char *>(&sampleRate), sizeof(sampleRate));
    out.write(reinterpret_cast<const char *>(&byteRate), sizeof(byteRate));
    out.write(reinterpret_cast<const char *>(&blockAlign), sizeof(blockAlign));
    out.write(reinterpret_cast<const char *>(&bitsPerSample), sizeof(bitsPerSample));
    out.write("data", 4);
    out.write(reinterpret_cast<const char *>(&dataSize), sizeof(dataSize));
    const std::vector<uint8_t> silence(dataSize, 0);
    out.write(reinterpret_cast<const char *>(silence.data()), static_cast<std::streamsize>(dataSize));
}

} // namespace test
