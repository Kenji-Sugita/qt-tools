// AudioLevelMonitor の Objective-C++ 実装です。ScreenCaptureKit でシステム音声を
// 取得し、左右チャンネルの RMS を dBFS に変換して Qt の signal へ橋渡しします。
// 画面収録権限の確認や macOS 13 系の API 差分も、このファイル内で扱います。

#include "audiolevelmonitor.h"

#include <QMetaObject>
#include <QString>

#import <AppKit/AppKit.h>
#import <CoreGraphics/CoreGraphics.h>
#import <CoreMedia/CoreMedia.h>
#import <ScreenCaptureKit/ScreenCaptureKit.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <vector>

namespace {

constexpr double kMinDbfs = -60.0;
constexpr double kMaxDbfs = 0.0;

double clampDbfs(double value)
{
    return std::max(kMinDbfs, std::min(value, kMaxDbfs));
}

double smoothedDbfs(double previousDbfs, double currentDbfs)
{
    constexpr double kRise = 0.30;
    constexpr double kFall = 0.08;
    const double factor = currentDbfs > previousDbfs ? kRise : kFall;
    return previousDbfs + (currentDbfs - previousDbfs) * factor;
}

double powerToDbfs(double sumSquares, std::uint64_t sampleCount)
{
    if (sampleCount == 0 || sumSquares <= 0.0) {
        return kMinDbfs;
    }

    const double rms = std::sqrt(sumSquares / static_cast<double>(sampleCount));
    if (rms <= 0.0) {
        return kMinDbfs;
    }

    return clampDbfs(20.0 * std::log10(rms));
}

template<typename Sample>
double normalizedSample(Sample sample)
{
    return static_cast<double>(sample);
}

template<>
double normalizedSample<std::int16_t>(std::int16_t sample)
{
    return static_cast<double>(sample) / 32768.0;
}

template<>
double normalizedSample<std::int32_t>(std::int32_t sample)
{
    return static_cast<double>(sample) / 2147483648.0;
}

template<typename Sample>
void accumulateInterleaved(const AudioBuffer &buffer,
                           UInt32 channels,
                           double &leftSumSquares,
                           double &rightSumSquares,
                           std::uint64_t &leftCount,
                           std::uint64_t &rightCount)
{
    if (!buffer.mData || channels == 0) {
        return;
    }

    const auto *samples = static_cast<const Sample *>(buffer.mData);
    const UInt32 frameCount = buffer.mDataByteSize / (sizeof(Sample) * channels);
    for (UInt32 frame = 0; frame < frameCount; ++frame) {
        const double left = normalizedSample(samples[frame * channels]);
        const double right = normalizedSample(samples[frame * channels + (channels > 1 ? 1 : 0)]);

        leftSumSquares += left * left;
        rightSumSquares += right * right;
        ++leftCount;
        ++rightCount;
    }
}

template<typename Sample>
void accumulatePlanar(const AudioBufferList *audioBufferList,
                      double &leftSumSquares,
                      double &rightSumSquares,
                      std::uint64_t &leftCount,
                      std::uint64_t &rightCount)
{
    if (!audioBufferList || audioBufferList->mNumberBuffers == 0) {
        return;
    }

    const AudioBuffer &leftBuffer = audioBufferList->mBuffers[0];
    if (leftBuffer.mData) {
        const auto *samples = static_cast<const Sample *>(leftBuffer.mData);
        const UInt32 sampleCount = leftBuffer.mDataByteSize / sizeof(Sample);
        for (UInt32 i = 0; i < sampleCount; ++i) {
            const double value = normalizedSample(samples[i]);
            leftSumSquares += value * value;
            ++leftCount;
        }
    }

    const AudioBuffer &rightBuffer = audioBufferList->mBuffers[audioBufferList->mNumberBuffers > 1 ? 1 : 0];
    if (rightBuffer.mData) {
        const auto *samples = static_cast<const Sample *>(rightBuffer.mData);
        const UInt32 sampleCount = rightBuffer.mDataByteSize / sizeof(Sample);
        for (UInt32 i = 0; i < sampleCount; ++i) {
            const double value = normalizedSample(samples[i]);
            rightSumSquares += value * value;
            ++rightCount;
        }
    }
}

} // namespace

@interface AudioLevelStreamOutput : NSObject <SCStreamOutput, SCStreamDelegate>
- (instancetype)initWithOwner:(AudioLevelMonitor *)owner;
- (void)emitError:(NSString *)message;
- (void)emitQtError:(const QString &)message;
@end

class AudioLevelMonitor::Private {
public:
    explicit Private(AudioLevelMonitor *owner)
        : owner(owner)
    {
    }

    ~Private()
    {
        stop();
    }

    bool start()
    {
        if (running.exchange(true)) {
            return true;
        }

        if (!CGPreflightScreenCaptureAccess()) {
            running.store(false);
            [NSApp activateIgnoringOtherApps:YES];
            CGRequestScreenCaptureAccess();
            emitError(AudioLevelMonitor::tr("Allow screen recording, then quit and restart VuStereo."));
            return false;
        }

        if (@available(macOS 13.0, *)) {
            output = [[AudioLevelStreamOutput alloc] initWithOwner:owner];
            sampleQueue = dispatch_queue_create("VuStereo.audio-levels", DISPATCH_QUEUE_SERIAL);

            [SCShareableContent getShareableContentExcludingDesktopWindows:YES
                                                       onScreenWindowsOnly:YES
                                                         completionHandler:^(SCShareableContent *shareableContent, NSError *error) {
                if (!running.load()) {
                    return;
                }

                if (error) {
                    [output emitError:error.localizedDescription];
                    running.store(false);
                    return;
                }

                SCDisplay *display = shareableContent.displays.firstObject;
                if (!display) {
                    [output emitQtError:AudioLevelMonitor::tr("No display is available to ScreenCaptureKit.")];
                    running.store(false);
                    return;
                }

                SCContentFilter *filter = [[SCContentFilter alloc] initWithDisplay:display excludingWindows:@[]];

                SCStreamConfiguration *config = [[SCStreamConfiguration alloc] init];
                config.width = 2;
                config.height = 2;
                config.minimumFrameInterval = CMTimeMake(1, 2);
                config.queueDepth = 3;
                config.showsCursor = NO;
                config.capturesAudio = YES;
                config.excludesCurrentProcessAudio = YES;
                config.sampleRate = 48000;
                config.channelCount = 2;

                stream = [[SCStream alloc] initWithFilter:filter configuration:config delegate:output];

                NSError *addScreenOutputError = nil;
                if (![stream addStreamOutput:output type:SCStreamOutputTypeScreen sampleHandlerQueue:sampleQueue error:&addScreenOutputError]) {
                    if (addScreenOutputError.localizedDescription) {
                        [output emitError:addScreenOutputError.localizedDescription];
                    } else {
                        [output emitQtError:AudioLevelMonitor::tr("Could not add ScreenCaptureKit screen output.")];
                    }
                    running.store(false);
                    return;
                }

                NSError *addOutputError = nil;
                if (![stream addStreamOutput:output type:SCStreamOutputTypeAudio sampleHandlerQueue:sampleQueue error:&addOutputError]) {
                    if (addOutputError.localizedDescription) {
                        [output emitError:addOutputError.localizedDescription];
                    } else {
                        [output emitQtError:AudioLevelMonitor::tr("Could not add ScreenCaptureKit audio output.")];
                    }
                    running.store(false);
                    return;
                }

                [stream startCaptureWithCompletionHandler:^(NSError *startError) {
                    if (startError) {
                        [output emitError:startError.localizedDescription];
                        running.store(false);
                        return;
                    }

                    emitError(AudioLevelMonitor::tr("Audio monitor is running"));
                }];
            }];

            return true;
        }

        running.store(false);
        emitError(AudioLevelMonitor::tr("ScreenCaptureKit audio capture requires macOS 13 or later."));
        return false;
    }

    void stop()
    {
        if (!running.exchange(false)) {
            return;
        }

        if (@available(macOS 13.0, *)) {
            SCStream *streamToStop = stream;
            AudioLevelStreamOutput *outputToRemove = output;
            stream = nil;
            output = nil;
            sampleQueue = nil;

            if (streamToStop) {
                NSError *removeError = nil;
                [streamToStop removeStreamOutput:outputToRemove type:SCStreamOutputTypeAudio error:&removeError];
                [streamToStop removeStreamOutput:outputToRemove type:SCStreamOutputTypeScreen error:&removeError];
                [streamToStop stopCaptureWithCompletionHandler:nil];
            }
        }
    }

    void emitLevels(double leftDbfs, double rightDbfs)
    {
        smoothedLeftDbfs = smoothedDbfs(smoothedLeftDbfs, leftDbfs);
        smoothedRightDbfs = smoothedDbfs(smoothedRightDbfs, rightDbfs);

        const double left = smoothedLeftDbfs;
        const double right = smoothedRightDbfs;
        QMetaObject::invokeMethod(owner, [owner = owner, left, right] {
            emit owner->levelsChanged(left, right);
        }, Qt::QueuedConnection);
    }

    void emitError(const QString &message)
    {
        QMetaObject::invokeMethod(owner, [owner = owner, message] {
            emit owner->errorOccurred(message);
        }, Qt::QueuedConnection);
    }

    AudioLevelMonitor *owner = nullptr;
    std::atomic_bool running = false;
    double smoothedLeftDbfs = kMinDbfs;
    double smoothedRightDbfs = kMinDbfs;

    SCStream *stream = nil;
    AudioLevelStreamOutput *output = nil;
    dispatch_queue_t sampleQueue = nil;
};

@implementation AudioLevelStreamOutput {
    AudioLevelMonitor *_owner;
}

- (instancetype)initWithOwner:(AudioLevelMonitor *)owner
{
    self = [super init];
    if (self) {
        _owner = owner;
    }
    return self;
}

- (void)emitError:(NSString *)message
{
    if (!_owner) {
        return;
    }
    _owner->emitNativeError(QString::fromNSString(message ?: @"Unknown audio capture error."));
}

- (void)emitQtError:(const QString &)message
{
    if (!_owner) {
        return;
    }
    _owner->emitNativeError(message);
}

- (void)stream:(SCStream *)stream didStopWithError:(NSError *)error
{
    Q_UNUSED(stream);
    [self emitError:error.localizedDescription ?: @"ScreenCaptureKit audio stream stopped."];
}

- (void)stream:(SCStream *)stream didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer ofType:(SCStreamOutputType)type
{
    Q_UNUSED(stream);

    if (!_owner || !CMSampleBufferIsValid(sampleBuffer)) {
        return;
    }

    if (type != SCStreamOutputTypeAudio) {
        return;
    }

    CMFormatDescriptionRef formatDescription = CMSampleBufferGetFormatDescription(sampleBuffer);
    const AudioStreamBasicDescription *description = CMAudioFormatDescriptionGetStreamBasicDescription(formatDescription);
    if (!description) {
        return;
    }

    size_t audioBufferListSize = 0;
    OSStatus status = CMSampleBufferGetAudioBufferListWithRetainedBlockBuffer(sampleBuffer,
                                                                             &audioBufferListSize,
                                                                             nullptr,
                                                                             0,
                                                                             nullptr,
                                                                             nullptr,
                                                                             kCMSampleBufferFlag_AudioBufferList_Assure16ByteAlignment,
                                                                             nullptr);
    if (status != noErr || audioBufferListSize == 0) {
        return;
    }

    std::vector<std::uint8_t> storage(audioBufferListSize);
    auto *audioBufferList = reinterpret_cast<AudioBufferList *>(storage.data());
    CMBlockBufferRef blockBuffer = nullptr;
    status = CMSampleBufferGetAudioBufferListWithRetainedBlockBuffer(sampleBuffer,
                                                                     nullptr,
                                                                     audioBufferList,
                                                                     audioBufferListSize,
                                                                     nullptr,
                                                                     nullptr,
                                                                     kCMSampleBufferFlag_AudioBufferList_Assure16ByteAlignment,
                                                                     &blockBuffer);
    if (status != noErr) {
        if (blockBuffer) {
            CFRelease(blockBuffer);
        }
        return;
    }

    double leftSumSquares = 0.0;
    double rightSumSquares = 0.0;
    std::uint64_t leftCount = 0;
    std::uint64_t rightCount = 0;

    const bool isFloat = (description->mFormatFlags & kAudioFormatFlagIsFloat) != 0;
    const bool isNonInterleaved = (description->mFormatFlags & kAudioFormatFlagIsNonInterleaved) != 0;
    const UInt32 bitsPerChannel = description->mBitsPerChannel;
    const UInt32 channels = std::max<UInt32>(1, description->mChannelsPerFrame);

    if (isNonInterleaved) {
        if (isFloat && (bitsPerChannel == 32 || bitsPerChannel == 0)) {
            accumulatePlanar<float>(audioBufferList, leftSumSquares, rightSumSquares, leftCount, rightCount);
        } else if (!isFloat && bitsPerChannel == 16) {
            accumulatePlanar<std::int16_t>(audioBufferList, leftSumSquares, rightSumSquares, leftCount, rightCount);
        } else if (!isFloat && bitsPerChannel == 32) {
            accumulatePlanar<std::int32_t>(audioBufferList, leftSumSquares, rightSumSquares, leftCount, rightCount);
        }
    } else if (audioBufferList->mNumberBuffers > 0) {
        const AudioBuffer &buffer = audioBufferList->mBuffers[0];
        if (isFloat && (bitsPerChannel == 32 || bitsPerChannel == 0)) {
            accumulateInterleaved<float>(buffer, channels, leftSumSquares, rightSumSquares, leftCount, rightCount);
        } else if (!isFloat && bitsPerChannel == 16) {
            accumulateInterleaved<std::int16_t>(buffer, channels, leftSumSquares, rightSumSquares, leftCount, rightCount);
        } else if (!isFloat && bitsPerChannel == 32) {
            accumulateInterleaved<std::int32_t>(buffer, channels, leftSumSquares, rightSumSquares, leftCount, rightCount);
        }
    }

    if (blockBuffer) {
        CFRelease(blockBuffer);
    }

    const double leftDbfs = powerToDbfs(leftSumSquares, leftCount);
    const double rightDbfs = powerToDbfs(rightSumSquares, rightCount);
    _owner->emitNativeLevels(leftDbfs, rightDbfs);
}

@end

AudioLevelMonitor::AudioLevelMonitor(QObject *parent)
    : QObject(parent),
      d(new Private(this))
{
}

AudioLevelMonitor::~AudioLevelMonitor()
{
    delete d;
}

bool AudioLevelMonitor::start()
{
    return d->start();
}

void AudioLevelMonitor::stop()
{
    d->stop();
}

void AudioLevelMonitor::emitNativeLevels(double leftDbfs, double rightDbfs)
{
    d->emitLevels(leftDbfs, rightDbfs);
}

void AudioLevelMonitor::emitNativeError(const QString &message)
{
    d->emitError(message);
}
