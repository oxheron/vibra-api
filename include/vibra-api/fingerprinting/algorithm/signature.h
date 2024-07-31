#pragma once

#include <map>
#include <list>
#include <memory>
#include <sstream>
#include "frequency.h"

// Prevent Structure Padding
#ifdef _MSC_VER
    #pragma pack(push, 1)
#endif
struct RawSignatureHeader {
    uint32_t magic1;
    uint32_t crc32;
    uint32_t size_minus_header;
    uint32_t magic2;
    uint32_t void1[3];
    uint32_t shifted_sample_rate_id;
    uint32_t void2[2];
    uint32_t number_samples_plus_divided_sample_rate;
    uint32_t fixed_value;
} 
#ifdef _MSC_VER
    #pragma pack(pop)
#else
    __attribute__((packed)); 
#endif

class Signature
{
public:
    Signature(std::uint32_t sampleRate, std::uint32_t numberOfSamples);
    ~Signature();
    void Reset(std::uint32_t sampleRate, std::uint32_t numberOfSamples);

    inline void AddNumberOfSamples(std::uint32_t numberOfSamples) { mNumberOfSamples += numberOfSamples; }
    inline std::uint32_t SampleRate() const { return mSampleRate; }
    inline std::uint32_t NumberOfSamples() const { return mNumberOfSamples; }
    inline std::map<FrequancyBand, std::list<FrequancyPeak>>& FrequancyBandToPeaks() { return mFrequancyBandToPeaks; }
    std::uint32_t SumOfPeaksLength() const;
    std::string GetBase64Uri() const;

private:
    template <typename T>
    std::stringstream& writeLittleEndian(std::stringstream& stream, const T&& value, size_t size = sizeof(T)) const
    {
        for (size_t i = 0; i < size; ++i)
        {
            stream << static_cast<char>(value >> (i << 3));
        }
        return stream;
    }

private:
    std::uint32_t mSampleRate;
    std::uint32_t mNumberOfSamples;
    std::map<FrequancyBand, std::list<FrequancyPeak>> mFrequancyBandToPeaks;
};

#ifdef VIBRA_API_IMPL

#include "../utils/crc32.h"
#include "../utils/base64.h"
#include <sstream>
#include <algorithm>

Signature::Signature(std::uint32_t sampleRate, std::uint32_t numberOfSamples)
    : mSampleRate(sampleRate)
    , mNumberOfSamples(numberOfSamples)
{
}

void Signature::Reset(std::uint32_t sampleRate, std::uint32_t numberOfSamples)
{
    mSampleRate = sampleRate;
    mNumberOfSamples = numberOfSamples;
    mFrequancyBandToPeaks.clear();
}

std::uint32_t Signature::SumOfPeaksLength() const
{
    std::uint32_t sum = 0;
    for (const auto& pair : mFrequancyBandToPeaks)
    {
        sum += pair.second.size();
    }
    return sum;
}

std::string Signature::GetBase64Uri() const
{
    RawSignatureHeader header;
    header.magic1 = 0xcafe2580;
    header.magic2 = 0x94119c00;
    header.shifted_sample_rate_id = 3 << 27;
    header.fixed_value = ((15 << 19) + 0x40000);
    header.number_samples_plus_divided_sample_rate = static_cast<std::uint32_t>(mNumberOfSamples + mSampleRate * 0.24);
    std::stringstream contents;
    for (const auto& pair : mFrequancyBandToPeaks)
    {
        const auto& band = pair.first;
        const auto& peaks = pair.second;

        std::stringstream peak_buf;
        std::size_t fft_pass_number = 0;

        for (const auto& peak : peaks)
        {            
            if (peak.GetFFTPassNumber() - fft_pass_number >= 255)
            {
                peak_buf << "\xff";
                writeLittleEndian(peak_buf, peak.GetFFTPassNumber());
                fft_pass_number = peak.GetFFTPassNumber();
            }

            peak_buf << static_cast<char>(peak.GetFFTPassNumber() - fft_pass_number);
            writeLittleEndian(peak_buf, peak.GetPeakMagnitude(), 2); 
            writeLittleEndian(peak_buf, peak.GetCorrectedPeakFrequencyBin(), 2);

            fft_pass_number = peak.GetFFTPassNumber();
        }

        writeLittleEndian(contents, 0x60030040u + static_cast<std::uint32_t>(band));
        writeLittleEndian(contents, static_cast<std::uint32_t>(peak_buf.str().size()));
        contents << peak_buf.str();

        for (std::size_t i = 0; i < (-peak_buf.str().size() % 4); ++i)
            contents << '\0';
    }

    header.size_minus_header = contents.str().size() + 8;
    
    std::stringstream header_buf;
    header_buf.write(reinterpret_cast<const char*>(&header), sizeof(header));

    writeLittleEndian(header_buf, 0x40000000u);
    writeLittleEndian(header_buf, static_cast<std::uint32_t>(contents.str().size()) + 8);

    header_buf << contents.str();

    const auto& header_buf_str = header_buf.str();
    header.crc32 = crc32::crc32(header_buf_str.c_str() + 8, header_buf_str.size() - 8) & 0xffffffff;

    header_buf.seekp(0);
    header_buf.write(reinterpret_cast<const char*>(&header), sizeof(header));

    std::string header_string = header_buf.str();
    
    std::string base64Uri;
    base64Uri += "data:audio/vnd.shazam.sig;base64,";
    base64Uri += base64::encode(header_string.c_str(), header_string.size());
    return base64Uri;
}

Signature::~Signature()
{

}

#endif


