#include "soundrecorder.h"

SoundRecorder *SoundRecorder::getInstance()
{
    if (instance == nullptr)
        instance = new SoundRecorder();

    return instance;
}

SoundRecorder::SoundRecorder(QObject *parent)
    : QObject{parent}
{
    // 创建目录
    QDir soundRootPath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    soundRootPath.mkdir("sound");

    // 初始化录制模块
    soundFile.setFileName(RECORD_PATH);

    QAudioFormat inputFormat;
    inputFormat.setSampleRate(16000);
    inputFormat.setChannelCount(1);
    inputFormat.setSampleFormat(QAudioFormat::Int16);

    QAudioDevice info = QMediaDevices::defaultAudioInput();
    if (!info.isFormatSupported(inputFormat))
    {
        LOG() << "录制设备, 格式不支持!";
        return;
    }

    audioSource = new QAudioSource(inputFormat, this);
    connect(audioSource, &QAudioSource::stateChanged, this, [=](QtAudio::State state) {
        if (state != QtAudio::StoppedState)
            return;

        if (audioSource->error() != QAudio::NoError)
            LOG() << audioSource->error();
    });

    // 初始化播放模块
    outputDevices = new QMediaDevices(this);
    outputDevice = outputDevices->defaultAudioOutput();
    QAudioFormat outputFormat;
    outputFormat.setSampleRate(16000);
    outputFormat.setChannelCount(1);
    outputFormat.setSampleFormat(QAudioFormat::Int16);
    if (!outputDevice.isFormatSupported(outputFormat))
    {
        LOG() << "播放设备, 格式不支持";
        return;
    }
    audioSink = new QAudioSink(outputDevice, outputFormat);

    connect(audioSink, &QAudioSink::stateChanged, this, [=](QtAudio::State state) {
        if (state == QtAudio::IdleState)
        {
            LOG() << "IdleState";
            stopPlay();
            emit soundPlayDone();
        }
        else if (state == QAudio::ActiveState)
        {
            LOG() << "ActiveState";
        }
        else if (state == QAudio::StoppedState)
        {
            LOG() << "StoppedState";
            if (audioSink->error() != QtAudio::NoError)
                LOG() << audioSink->error();
        }
    });
}

void SoundRecorder::startRecord()
{
    soundFile.open(QIODevice::WriteOnly | QIODevice::Truncate );
    audioSource->start(&soundFile);
}

void SoundRecorder::stopRecord()
{
    audioSource->stop();
    soundFile.close();
    emit soundRecordDone(RECORD_PATH);
}

void SoundRecorder::startPlay(const QByteArray& content)
{
    if (content.isEmpty())
    {
        Toast::showMessage("数据加载中, 请稍后播放");
        return;
    }
    // 数据写入到临时文件
    IM::writeByteArrayToFile(PLAY_PATH, content);

    // 播放语音
    inputFile.setFileName(PLAY_PATH);
    inputFile.open(QIODevice::ReadOnly);
    audioSink->start(&inputFile);
}

void SoundRecorder::stopPlay()
{
    audioSink->stop();
    inputFile.close();
}
