
#include "../JuceLibraryCode/JuceHeader.h"

class YellowNote
	:
	public Component,
	TextEditor::Listener,
	Timer
{
public:
	class LafTextEditor : public LookAndFeel_V3
	{
	public:
		/* We don't want an outline on the texte editor so this is a mini-look-and-feel 
		implementation to disable it. */
		void drawTextEditorOutline(Graphics&, int width, int height, TextEditor&) override {}
	};

	static const char kNoteContents[];
	
	LafTextEditor lafTextEditor;

	YellowNote()
	{
		/* Disable the background colour */
		editor.setColour(TextEditor::ColourIds::backgroundColourId, Colours::transparentWhite); 

		editor.setMultiLine(true, true); 

		editor.setLookAndFeel(&lafTextEditor); 
		editor.addListener(this); 

		initializePropertiesFile();

		editor.setText(propertiesFile->getValue(kNoteContents)); 

		addAndMakeVisible(editor); 

		startTimer(3000);
	}

private:
	void initializePropertiesFile()
	{
		PropertiesFile::Options options;
		options.applicationName = "Yellow Note"; 
		options.filenameSuffix = ".settings"; 
		options.osxLibrarySubFolder = "Application Support"; 
		propertiesFile = new PropertiesFile(options);
	}

	void resized() override
	{
		editor.setBounds(getLocalBounds().reduced(10));
	}

	void mouseDown(const MouseEvent & e) override
	{
		dragger.startDraggingComponent(getParentComponent(), e); 
	}

	void mouseDrag(const MouseEvent & e) override
	{
		dragger.dragComponent(getParentComponent(), e, nullptr); 
	}

	void paint(Graphics & g) override
	{
		auto stickyNoteYellow = Colours::yellow.withSaturation(0.5f); 
		g.setColour(stickyNoteYellow); 
		g.fillRoundedRectangle(getLocalBounds().toFloat(), 10.0f); 
	}

	void textEditorTextChanged(TextEditor&) override
	{
		propertiesFile->setValue(kNoteContents, editor.getText()); 
	}

	void timerCallback() override
	{
		propertiesFile->saveIfNeeded();
	}


	ComponentDragger dragger; 
	TextEditor editor;
	ScopedPointer<PropertiesFile> propertiesFile;
};

const char YellowNote::kNoteContents[] = "noteContents1";  /* We could support multiple notes in the future. */

class MainWindow 
	:
	public TopLevelWindow
{
public:
	MainWindow()
		:
		TopLevelWindow("Yellow Note", true)
	{
		Component::setVisible(true); 
		centreWithSize(200, 200); 
		addAndMakeVisible(yellowNote);
		setOpaque(false);
		setAlwaysOnTop(true);
	}

	void resized() override
	{
		TopLevelWindow::resized();
		yellowNote.setBounds(getLocalBounds()); 
	}

	void userTriedToCloseWindow() override
	{
		JUCEApplication::getInstance()->quit();
	}

	YellowNote yellowNote;
};
//==============================================================================
class YellowNoteApplication  : public JUCEApplication
{
public:
    //==============================================================================
    YellowNoteApplication() {}

    const String getApplicationName() override       { return ProjectInfo::projectName; }
    const String getApplicationVersion() override    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override       { return true; }

    //==============================================================================
    void initialise (const String& commandLine) override
    {
		mainWindow = new MainWindow(); 
    }

    void shutdown() override
    {
    }

    void systemRequestedQuit() override
    {
        quit();
    }

    void anotherInstanceStarted (const String& commandLine) override
    {

    }
	ScopedPointer<MainWindow> mainWindow; 
};


START_JUCE_APPLICATION (YellowNoteApplication)
