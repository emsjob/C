#include <QAudioDeviceInfo>
#include <QAudioFormat>
#include <QAudioInput>
#include <QByteArray>
#include <QCustomPlot>
#include <QDebug>
#include <QMainWindow>
#include <QVector>
#include <QVBoxLayout>
#include <QWidget>

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget* parent = nullptr) : QMainWindow(parent) {
    setupAudio();
    setupGraph();

    startRecording();
  }

private:
  QAudioFormat audioFormat;
  QAudioInput* audioInput;
  QIODevice* audioDevice;
  QByteArray audioBuffer;
  QVector<double> audioSamples;

  QCustomPlot* customPlot;

  void setupAudio() {
    // Set up the audio format
    audioFormat.setSampleRate(44100);
    audioFormat.setChannelCount(1);
    audioFormat.setSampleSize(16);
    audioFormat.setCodec("audio/pcm");
    audioFormat.setByteOrder(QAudioFormat::LittleEndian);
    audioFormat.setSampleType(QAudioFormat::SignedInt);

    // Check for available audio input devices
    QAudioDeviceInfo inputDeviceInfo = QAudioDeviceInfo::defaultInputDevice();
    if (!inputDeviceInfo.isFormatSupported(audioFormat)) {
      qWarning() << "Default audio input device does not support the specified format.";
      return;
    }

    // Create the audio input object
    audioInput = new QAudioInput(inputDeviceInfo, audioFormat, this);
    audioBuffer.resize(audioInput->periodSize());

    // Open the audio input device
    audioDevice = audioInput->start();
    connect(audioDevice, &QIODevice::readyRead, this, &MainWindow::readAudioData);
  }

  void readAudioData() {
    // Read the available audio data from the device into the buffer
    qint64 bytesRead = audioDevice->read(audioBuffer.data(), audioBuffer.size());

    // Process the audio data
    // Here, you can apply your desired signal processing techniques to the audioBuffer

    // Example: Store the processed audio samples in the audioSamples vector
    for (int i = 0; i < bytesRead; ++i) {
      qint16 sampleValue = *reinterpret_cast<const qint16*>(audioBuffer.constData() + i);
      double normalizedSample = static_cast<double>(sampleValue) / static_cast<double>(INT16_MAX);
      audioSamples.append(normalizedSample);
    }

    // Update the graph with the new audio samples
    customPlot->graph(0)->setData(QVector<double>::fromStdVector(audioSamples.toStdVector()));
    customPlot->replot();
  }

  void setupGraph() {
    // Create the graphing widget
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(centralWidget);
    customPlot = new QCustomPlot(centralWidget);
    layout->addWidget(customPlot);
    setCentralWidget(centralWidget);

    // Configure the graph
    customPlot->addGraph();
    customPlot->xAxis->setLabel("Time");
    customPlot->yAxis->setLabel("Amplitude");

    // Set the range and properties of the axes
    customPlot->xAxis->setRange(0, 1);  // Modify according to your requirements
    customPlot->yAxis->setRange(-1, 1); // Modify according to your requirements
  }

  void startRecording() {
    audioInput->start(audioDevice);
  }
};

#include "main.moc"

int main(int argc, char** argv) {
  QApplication app(argc, argv);

  MainWindow mainWindow;
  mainWindow.show();

  return app.exec();
}
