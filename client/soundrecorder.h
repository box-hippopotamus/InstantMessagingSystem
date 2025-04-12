#ifndef SOUNDRECORDER_H
#define SOUNDRECORDER_H

#include <QObject>
#include <QStandardPaths>
#include <QFile>
#include <QDir>
#include <QAudioSource>
#include <QAudioSink>
#include <QMediaDevices>

#include "model/data.h"
#include "toast.h"
#include "common.h"

class SoundRecorder : public QObject
{
    Q_OBJECT
public:
    const QString RECORD_PATH = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/sound/tmpRecord.pcm";
    const QString PLAY_PATH = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/sound/tmpPlay.pcm";

public:
    static SoundRecorder* getInstance();

    // 开始录制
    void startRecord();
    // 停止录制
    void stopRecord();

private:
    inline static SoundRecorder* instance = nullptr;
    explicit SoundRecorder(QObject *parent = nullptr);

    QFile soundFile;
    QAudioSource* audioSource;

public:
    // 开始播放
    void startPlay(const QByteArray& content);
    // 停止播放
    void stopPlay();

private:
    QAudioSink *audioSink;
    QMediaDevices *outputDevices;
    QAudioDevice outputDevice;
    QFile inputFile;

signals:
    void soundRecordDone(const QString& path);
    void soundPlayDone();

};

#endif // SOUNDRECORDER_H
