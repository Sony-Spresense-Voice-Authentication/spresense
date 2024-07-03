# spresense
1.设置录音机模式：

使用 theAudio->setRecorderMode(input_device, input_gain, BUFFER_SIZE, is_digital) 函数设置录音机模式。这里使用模拟麦克风（input_device 设为 AS_SETRECDR_STS_INPUTDEVICE_MIC），增益设为 10 dB（input_gain 设为 100），缓冲区大小设为 BUFFER_SIZE，且 is_digital 设为 false。

2.初始化录音机：

使用 theAudio->initRecorder(AS_CODECTYPE_WAV, "/mnt/sd0/BIN", AS_SAMPLINGRATE_16000, AS_CHANNEL_MONO) 函数初始化录音机，指定编解码器类型为WAV，路径为 /mnt/sd0/BIN，采样率为16000 Hz，通道为单声道。

3.创建WAV文件：

使用 SD.open("Sound.wav", FILE_WRITE) 创建WAV文件。
写入WAV头的占位符。

4.开始录音：

使用 theAudio->startRecorder() 开始录音。
在录音循环中使用 theAudio->readFrames(audioFile) 读取音频数据并写入文件。
停止录音并写入WAV头：

5.停止录音。
使用 theAudio->stopRecorder() 
使用 writeWAVHeader(audioFile, totalDataLen) 写入实际的WAV头
