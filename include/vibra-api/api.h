#pragma once

#include "fingerprinting/algorithm/frequency.h"
#include "fingerprinting/algorithm/signature_generator.h"
#include "fingerprinting/algorithm/signature.h"
#include "fingerprinting/audio/wav.h"
#include "communication/shazam.h" 

inline Signature getSignatureFromPcm(const Raw16bitPCM& pcm)
{
    SignatureGenerator generator;
    generator.FeedInput(pcm);
    generator.SetMaxTimeSeconds(12);
    auto duaration = pcm.size() / LOW_QUALITY_SAMPLE_RATE;
    if (duaration > 12 * 3)
        generator.AddSampleProcessed(LOW_QUALITY_SAMPLE_RATE * ((int)duaration / 2) - 6);

    return generator.GetNextSignature();
}

inline Raw16bitPCM getPcmFromBuffer(std::vector<char> bytes, int chunk_seconds, int sample_rate, int channels, int bits_per_sample)
{
    Wav wav(bytes.data(), bytes.size(), sample_rate, bits_per_sample, channels);
    Raw16bitPCM pcm;
    wav.GetLowQualityPCM(&pcm);
    return pcm;
}
