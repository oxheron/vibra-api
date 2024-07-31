#pragma once

#include <cstdint>

enum class FrequancyBand
{
    _0_150 = -1,
    _250_520,
    _520_1450,
    _1450_3500,
    _3500_5500,
};

class FrequancyPeak
{
public:
    FrequancyPeak(std::uint32_t fft_pass_number, std::uint32_t peak_magnitude, std::uint32_t corrected_peak_frequency_bin, std::uint32_t sample_rate);
    ~FrequancyPeak();

    inline std::uint32_t GetFFTPassNumber() const { return mFFTPassNumber; }
    inline std::uint32_t GetPeakMagnitude() const { return mPeakMagnitude; }
    inline std::uint32_t GetCorrectedPeakFrequencyBin() const { return mCorrectedPeakFrequencyBin; }
    double GetFrequencyHz() const;
    double GetAmplitudePCM() const;
    double GetSeconds() const;

private:
    std::uint32_t mFFTPassNumber;
    std::uint32_t mPeakMagnitude;
    std::uint32_t mCorrectedPeakFrequencyBin;
    std::uint32_t mSampleRate;
};

#ifdef VIBRA_API_IMPL

#include <cmath>

FrequancyPeak::FrequancyPeak(std::uint32_t fft_pass_number, std::uint32_t peak_magnitude, std::uint32_t corrected_peak_frequency_bin, std::uint32_t sample_rate)
    : mFFTPassNumber(fft_pass_number)
    , mPeakMagnitude(peak_magnitude)
    , mCorrectedPeakFrequencyBin(corrected_peak_frequency_bin)
    , mSampleRate(sample_rate)
{
}

FrequancyPeak::~FrequancyPeak()
{
}

double FrequancyPeak::GetFrequencyHz() const
{
    return mCorrectedPeakFrequencyBin * ((double)mSampleRate / 2. / 1024. / 64.);
}
    
double FrequancyPeak::GetAmplitudePCM() const
{
    return std::sqrt(std::exp((mPeakMagnitude - 6144) / 1477.3) * (1 << 17) / 2.) / 1024.;
}

double FrequancyPeak::GetSeconds() const
{
    return (double)mFFTPassNumber * 128. / (double)mSampleRate;
}

#endif
