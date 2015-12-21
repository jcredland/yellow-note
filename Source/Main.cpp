
#include "../JuceLibraryCode/JuceHeader.h"
#include "FontAwesome.h"

class FontAwesomeButton
:
public Button
{
public:
    FontAwesomeButton(int iconCode_) : Button("button"), iconCode(iconCode_) {}
    void paintButton(Graphics & g, bool, bool)
    {
        g.setColour(getToggleState() ? Colours::black : Colours::darkgrey);
        FontAwesomeIcons::drawIcon(g, iconCode, getLocalBounds().toFloat());
    }
private:
    int iconCode;
};

class TitleBar
:
public Component,
public Button::Listener
{
public:
    TitleBar(TopLevelWindow & owner_)
    :
    owner(owner_),
    close(FontAwesomeIcons::faClose),
    pinOnTop(FontAwesomeIcons::faThumbTack)
    {
        setInterceptsMouseClicks(false, true);

        addAndMakeVisible(close);
        close.addListener(this);

        addAndMakeVisible(pinOnTop);
        pinOnTop.addListener(this);
        pinOnTop.setClickingTogglesState(true);
        pinOnTop.setToggleState(true, dontSendNotification);

        addMouseListener(this, true);
    }
    void resized() override
    {
        auto h = getHeight();
        auto area = getLocalBounds().withWidth(h);
        close.setBounds(area);
        pinOnTop.setBounds(area.withX(getLocalBounds().getWidth() - h));
        dim();
    }
    void mouseEnter(const MouseEvent & e) override {
        Desktop::getInstance().getAnimator().animateComponent(this, getBounds(), 1.0f, 500, false, 0.1, 0.1);
    }
    void mouseExit(const MouseEvent & e) override {
        dim();
    }
    void buttonClicked(Button * b) override {
        if (b == &close)
            JUCEApplication::quit();

        if (b == &pinOnTop)
            updatePinOnTop();
    }
private:
    void updatePinOnTop() {
        owner.setAlwaysOnTop(pinOnTop.getToggleState());
    }
    void dim() {
        Desktop::getInstance().getAnimator().animateComponent(this, getBounds(), 0.1f, 500, false, 0.1, 0.1);
    }
    TopLevelWindow & owner;
    FontAwesomeButton close;
    FontAwesomeButton pinOnTop;
};

class HiddenResizingCorner
	:
	public ResizableCornerComponent
{
public:
	HiddenResizingCorner(Component * componentToResize) :
	ResizableCornerComponent(componentToResize, &boundsConstrainer)
	{
        boundsConstrainer.setMinimumSize(50, 20);
    }
	void paint(Graphics & g) override {}
private:
    ComponentBoundsConstrainer boundsConstrainer;
};

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
		editor.setReturnKeyStartsNewLine(true); 

		editor.setLookAndFeel(&lafTextEditor); 
		editor.addListener(this); 

		initializePropertiesFile();

		editor.setText(propertiesFile->getValue(kNoteContents)); 

		addAndMakeVisible(editor); 

		startTimer(3000);
	}

	~YellowNote()
	{
		/* Better save on quit as well as every 3 seconds */
		propertiesFile->saveIfNeeded();
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
		editor.setBounds(getLocalBounds().reduced(10).withTrimmedTop(5));
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
		TopLevelWindow("Yellow Note", true),
        titleBar(*this),
		resizingCorner(this)
	{
        setInterceptsMouseClicks(false, true);

		setOpaque(false);
		Component::setVisible(true); 
		centreWithSize(200, 200);
		addAndMakeVisible(yellowNote);
        addAndMakeVisible(titleBar);
		addAndMakeVisible(resizingCorner); 
		setAlwaysOnTop(true);
	}

	void resized() override
	{
		TopLevelWindow::resized();
        titleBar.setBounds(getLocalBounds().reduced(4, 5).withHeight(15));
		yellowNote.setBounds(getLocalBounds());
		resizingCorner.setBounds(getWidth() - 15, getHeight() - 15, 15, 15);
	}

	void userTriedToCloseWindow() override
	{
		JUCEApplication::getInstance()->quit();
	}

    TitleBar titleBar;
	YellowNote yellowNote;
	HiddenResizingCorner resizingCorner;
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
