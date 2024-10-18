#include "MainComponent.h"
#include "stdio.h"


//==============================================================================
MainComponent::MainComponent() : state(Stopped)
{
    addAndMakeVisible(&openButton);
    openButton.setButtonText("Open...");
    openButton.onClick = [this] {openButtonClicked(); };

    addAndMakeVisible(&playButton);
    playButton.setButtonText("Play");
    playButton.onClick = [this] { playButtonClicked(); };
    playButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
    playButton.setEnabled(false);

    addAndMakeVisible(&stopButton);
    stopButton.setButtonText("Stop");
    stopButton.onClick = [this] { stopButtonClicked(); };
    stopButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
    stopButton.setEnabled(false);

	addAndMakeVisible(&FFTButton);
	FFTButton.setButtonText("FFT");
	FFTButton.onClick = [this] { FFTButtonClicked(); };
	FFTButton.setColour(juce::TextButton::buttonColourId, juce::Colours::blue);
	FFTButton.setEnabled(true);

	addAndMakeVisible(&trackTitle);
	trackTitle.setText("Track Title", juce::dontSendNotification);
	trackTitle.setJustificationType(juce::Justification::centred);

	setPositionLabel(positionData);
	addAndMakeVisible(&positionLabel);
	positionLabel.setJustificationType(juce::Justification::centred);

    formatManager.registerBasicFormats();
    formatManager.registerFormat(new juce::MP3AudioFormat(), true);
    
    transportSource.addChangeListener(this);
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

//==============================================================================

void MainComponent::resized()
{
    // This is called when the MainComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
    openButton.setBounds(10, 10, getWidth() - 20, 20);
    playButton.setBounds(10, 40, getWidth() - 20, 20);
    stopButton.setBounds(10, 70, getWidth() - 20, 20);
	FFTButton.setBounds(10, 160, getWidth() - 20, 20);
	trackTitle.setBounds(10, 100, getWidth() - 20, 20);
	positionLabel.setBounds(10, 130, getWidth() - 20, 20);
}

void MainComponent::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &transportSource)
    {
        if (transportSource.isPlaying())
            changeState(Playing);
        else
            changeState(Stopped);
    }
}

//This function will set the positionData variable so that it contains the current position and total length
//in minutes and seconds
void MainComponent::setPositionLabel(double positionData[])
{
		positionData[0] = readerSource ? transportSource.getCurrentPosition() : 0;
		positionData[1] = readerSource ? transportSource.getLengthInSeconds() : 0;
		sprintf(positionString, "%.2f / %.2f", positionData[0], positionData[1]);
    	positionLabel.setText(positionString, juce::dontSendNotification);
}

void MainComponent::changeState(TransportState newState)
{
    if (state != newState)
    {
        state = newState;

        switch (state)
        {
            //Set by AudioTransportSource (change reported)
            case Stopped:
                stopButton.setEnabled(false);
                playButton.setEnabled(true);
                transportSource.setPosition(0.0);
                break;
            
            //Set by Play button
            case Starting:
                playButton.setEnabled(false);
				setPositionLabel(positionData);
                transportSource.start();
                break;

            //Set by AudioTransportSource (change reported)
            case Playing:
                stopButton.setEnabled(true);
                setPositionLabel(positionData);
                break;

            //Triggered by Stop button
            case Stopping:
                transportSource.stop();
                break;
        }
    }
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (readerSource.get() == nullptr)
    {
        bufferToFill.clearActiveBufferRegion();
        return;
    }

    transportSource.getNextAudioBlock(bufferToFill);
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::releaseResources()
{
    transportSource.releaseResources();
}

void MainComponent::openButtonClicked()
{
    chooser = std::make_unique<juce::FileChooser>("Select an audio file to play...",
        juce::File{}, "*.wav;*.aif;*.aiff;*.mp3");

    auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

    chooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)
        {
            auto file = fc.getResult();

            //if user does not cancel, enter this if statement (juce::File{} is an invalid file)
            if (file != juce::File{})
            {
                auto* reader = formatManager.createReaderFor(file);

                //if createReaderFor does not fail, enter this if statement
                if (reader != nullptr)
                {
                    auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
                    transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
                    playButton.setEnabled(true);
					trackTitle.setText(file.getFileNameWithoutExtension(), juce::dontSendNotification);
                    readerSource.reset(newSource.release());
					setPositionLabel(positionData);

                }
                else {
					juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
						"Failed to load file",
						"Error while loading file: " + file.getFileName());
                }
            }
        });
}

void MainComponent::playButtonClicked()
{
    changeState(Starting);
}

void MainComponent::stopButtonClicked()
{
    changeState(Stopping);
}

void MainComponent::FFTButtonClicked()
{
	//FFTComponent fftComponent;
	//fftComponent.setVisible(true);
}

