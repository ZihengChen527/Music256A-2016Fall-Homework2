// Music 256a / CS 476a | fall 2016
// CCRMA, Stanford University
// Homework 2
//
// Author: Ziheng Chen (zihengcATccrmaDOTstanfordDOTedu)
// Description: A sound painter which can compose music by painting


#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "Sine.h"

class MainContentComponent :
    public AudioAppComponent,
    private Slider::Listener,
    private Button::Listener
{
public:
    MainContentComponent() : gain (0.0), onOff (0), samplingRate(0.0), frequency(100.0), partial(1), isSquareOn(0), isTriangleOn(0), isSawtoothOn(0)
    {      
        // configuring buttons and adding it to the main window
        addAndMakeVisible (&squareButton);
        squareButton.setButtonText ("Square Wave");
        squareButton.setColour(squareButton.buttonOnColourId, Colour(0xdee3680f));
        squareButton.setClickingTogglesState (true);
        squareButton.setToggleState (false, dontSendNotification);
        squareButton.addListener (this);
            
        // configuring buttons and adding it to the main window
        addAndMakeVisible (&triangleButton);
        triangleButton.setButtonText ("Triangle Wave");
        triangleButton.setColour(triangleButton.buttonOnColourId, Colour(0xdee3680f));
        triangleButton.setClickingTogglesState (true);
        triangleButton.setToggleState (false, dontSendNotification);
        triangleButton.addListener (this);
        
        // configuring buttons and adding it to the main window
        addAndMakeVisible (&sawtoothButton);
        sawtoothButton.setButtonText ("Sawtooth Wave");
        sawtoothButton.setColour(sawtoothButton.buttonOnColourId, Colour(0xdee3680f));
        sawtoothButton.setClickingTogglesState (true);
        sawtoothButton.setToggleState (false, dontSendNotification);
        sawtoothButton.addListener (this);
        
        // configuring gain slider and adding it to the main window
        addAndMakeVisible (gainSlider);
        gainSlider.setSliderStyle(Slider::LinearBar);
        gainSlider.setRange (0.0, 1.0);
        gainSlider.setValue(0.5); // will alsi set the default gain of the sine osc
        gainSlider.addListener (this);
        
        // configuring gain label and adding it to the main window
        addAndMakeVisible(gainLabel);
        gainLabel.setText ("Gain", dontSendNotification);
        gainLabel.attachToComponent (&gainSlider, true);        
        
        // configuring on/off button and adding it to the main window
        addAndMakeVisible(onOffButton);
        onOffButton.addListener(this);
        
        // configuring on/off label and adding it to the main window
        addAndMakeVisible(onOffLabel);
        onOffLabel.setText ("On/Off", dontSendNotification);
        onOffLabel.attachToComponent (&onOffButton, true);
              
        setSize (700, 700);
        nChans = 1;
        setAudioChannels (0, nChans); // no inputs, one output
    }
    
    ~MainContentComponent()
    {
        shutdownAudio();
    }
    
    void resized() override
    {       
        // buttons
        squareButton.changeWidthToFitText(30);
        squareButton.setCentrePosition(150, 560);        
              
        triangleButton.changeWidthToFitText(30);
        triangleButton.setCentrePosition(350, 560);        
              
        sawtoothButton.changeWidthToFitText(30);
        sawtoothButton.setCentrePosition(550, 560);                
        
        // gain slider and on/off button
        const int sliderLeft = 120;
        gainSlider.setBounds (sliderLeft, 600, 500, 20);
        onOffButton.setBounds (sliderLeft, 630, 20, 20);
    }
    
    void paint (Graphics& g) override
    {
        // rectangular area
        float recDistToTop = 30;
        float recDistToLeft = 50;
        float recWidth = 600;
        float recHeight = 500;
        
        g.setColour (Colours::cyan);
        g.fillRect (recDistToLeft, recDistToTop, recWidth, recHeight);
        
        g.setColour (Colours::lightgrey);
        g.setFont (14.0f);
        //g.drawFittedText (x,
        //                  getLocalBounds().reduced (30), Justification::centred, 4);

        for (int i = 0; i < trails.size(); ++i)
            drawTrail (*trails.getUnchecked(i), g);
    }
    
    void mouseDrag (const MouseEvent& e) override
    {
        Trail* t = getTrail (e.source);
        if (t == nullptr)
        {
            t = new Trail (e.source);
            t->path.startNewSubPath (e.position);
            trails.add (t);
        }

        t->pushPoint (e.position, e.mods, e.pressure);
        repaint(50, 30, 600, 500);
        
        int x = e.x;
        int y = e.y;
        
        // partial number, x axis
        if (x > 50 && x <= 90) 
            partial = 1;
        else if (x > 90 && x <= 130)
            partial = 2;
        else if (x > 130 && x <= 170)
            partial = 3;
        else if (x > 170 && x <= 210)
            partial = 4;
        else if (x > 210 && x <= 250)
            partial = 5;
        else if (x > 250 && x <= 290)
            partial = 6;
        else if (x > 290 && x <= 330)
            partial = 7;
        else if (x > 330 && x <= 370)
            partial = 8;
        else if (x > 370 && x <= 410)
            partial = 9;
        else if (x > 410 && x <= 450)
            partial = 10;
        else if (x > 450 && x <= 490)
            partial = 11;
        else if (x > 490 && x <= 530)
            partial = 12;
        else if (x > 530 && x <= 570)
            partial = 13;
        else if (x > 570 && x <= 610)
            partial = 14;
        else if (x > 610 && x <= 650)
            partial = 15;
            
        // frequency, y axis
        frequency = 100 + 530 - y;
        for (int i = 0; i < maxPartialNum; i++) {
           sineForSquare[i].setFrequency(frequency * (2*i+1));
           sineForTriangle[i].setFrequency(frequency * (2*i+1));
           sineForSawtooth[i].setFrequency(frequency * (i+1));
        }
        
    }
    
    void mouseUp (const MouseEvent& e) override
    {
        trails.removeObject (getTrail (e.source));
        //repaint();
    }
    
    struct Trail
    {
        Trail (const MouseInputSource& ms)
            : source (ms), colour (getRandomBrightColour().withAlpha (0.6f))
        {}

        void pushPoint (Point<float> newPoint, ModifierKeys newMods, float pressure)
        {
            currentPosition = newPoint;
            modifierKeys = newMods;

            if (lastPoint.getDistanceFrom (newPoint) > 5.0f)
            {
                if (lastPoint != Point<float>())
                {
                    Path newSegment;
                    newSegment.startNewSubPath (lastPoint);
                    newSegment.lineTo (newPoint);

                    float diameter = 10.0f * (pressure > 0 && pressure < 1.0f ? pressure : 1.0f);

                    PathStrokeType (diameter, PathStrokeType::curved, PathStrokeType::rounded).createStrokedPath (newSegment, newSegment);
                    path.addPath (newSegment);
                }

                lastPoint = newPoint;
            }
        }

        MouseInputSource source;
        Path path;
        Colour colour;
        Point<float> lastPoint, currentPosition;
        ModifierKeys modifierKeys;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Trail)
    };
    
    void drawTrail (Trail& trail, Graphics& g)
    {
        g.setColour (trail.colour);
        g.fillPath (trail.path);

        const float radius = 40.0f;

        g.setColour (Colours::black);
        g.drawEllipse (trail.currentPosition.x - radius,
                       trail.currentPosition.y - radius,
                       radius * 2.0f, radius * 2.0f, 2.0f);

        g.setFont (14.0f);

        String desc ("Mouse #");
        desc << trail.source.getIndex();

        float pressure = trail.source.getCurrentPressure();

        if (pressure > 0.0f && pressure < 1.0f)
            desc << "  (pressure: " << (int) (pressure * 100.0f) << "%)";

        if (trail.modifierKeys.isCommandDown()) desc << " (CMD)";
        if (trail.modifierKeys.isShiftDown())   desc << " (SHIFT)";
        if (trail.modifierKeys.isCtrlDown())    desc << " (CTRL)";
        if (trail.modifierKeys.isAltDown())     desc << " (ALT)";

        /*
        g.drawText (desc,
                    Rectangle<int> ((int) trail.currentPosition.x - 200,
                                    (int) trail.currentPosition.y - 60,
                                    400, 20),
                    Justification::centredTop, false);
         */
    }

    Trail* getTrail (const MouseInputSource& source)
    {
        for (int i = 0; i < trails.size(); ++i)
        {
            Trail* t = trails.getUnchecked(i);

            if (t->source == source)
                return t;
        }

        return nullptr;
    }
    
    static Colour getRandomColour (float brightness)
    {
    return Colour::fromHSV (Random::getSystemRandom().nextFloat(), 0.5f, brightness, 1.0f);
    }

    static Colour getRandomBrightColour()   { return getRandomColour (0.8f); }
    static Colour getRandomDarkColour()     { return getRandomColour (0.3f); }
      
    
    void sliderValueChanged (Slider* slider) override
    {
        gain = gainSlider.getValue();
    }
    
    void buttonClicked (Button* button) override
    {
        if(button == &squareButton && squareButton.getToggleState()){
            isSquareOn = 1;
        }
        else if(button == &squareButton && !squareButton.getToggleState()) {
            isSquareOn = 0;
        }
        
        if(button == &triangleButton && triangleButton.getToggleState()){
            isTriangleOn = 1;
        }
        else if(button == &triangleButton && !triangleButton.getToggleState()){
            isTriangleOn = 0;
        }
        
        if(button == &sawtoothButton && sawtoothButton.getToggleState()){
            isSawtoothOn = 1;
        }
        else if(button == &sawtoothButton && !sawtoothButton.getToggleState()){
            isSawtoothOn = 0;
        }
        
        
        
        /*
        if(squareButton.isDown()){
            isSquareOn = 1;
        }
        else {
            isSquareOn = 0;
        }
        
        if(triangleButton.isDown()){
            isTriangleOn = 1;
        }
        else {
            isTriangleOn = 0;
        }
        
        if(sawtoothButton.isDown()){
            isSawtoothOn = 1;
        }
        else {
            isSawtoothOn = 0;
        }*/
        
        
        // turns audio on or off
        if(button == &onOffButton && onOffButton.getToggleState()){
            onOff = 1;
        }
        else if(button == &onOffButton && !onOffButton.getToggleState()){
            onOff = 0;
        }
    }
   
    /*
    void buttonStateChanged(Button* button) override
    {
    
        if(button == &squareButton && squareButton.getToggleState()){
            isSquareOn = 1;
        }
        else {
            isSquareOn = 0;
        }
        
        if(button == &triangleButton && triangleButton.getToggleState()){
            isTriangleOn = 1;
        }
        else {
            isTriangleOn = 0;
        }
        
        if(button == &sawtoothButton && sawtoothButton.getToggleState()){
            isSawtoothOn = 1;
        }
        else {
            isSawtoothOn = 0;
        }
        
    }
    */
    
    
    void prepareToPlay (int /*samplesPerBlockExpected*/, double sampleRate) override
    {
        samplingRate = sampleRate;
        for (int i = 0; i < maxPartialNum; i++) {
            sineForSquare[i].setSamplingRate(sampleRate);
            sineForTriangle[i].setSamplingRate(sampleRate);
            sineForSawtooth[i].setSamplingRate(sampleRate);
        }
    }
    
    void releaseResources() override
    {
    }
    
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        // getting the audio output buffer to be filled
        float* const buffer = bufferToFill.buffer->getWritePointer (0, bufferToFill.startSample);
        
        // computing one block
        for (int sample = 0; sample < bufferToFill.numSamples; ++sample)
        {
            float squareSum = 0, triangleSum = 0, sawtoothSum = 0;
            if(onOff == 1) {
                // compute each sample using additive synthesis, scale to make sure the amplitude is below 1
                for (int i = 0; i < partial; i++) {     
                    squareSum += sineForSquare[i].tick() * 4/PI * 1/(2*i+1);
                    triangleSum += sineForTriangle[i].tick() * 8/(PI*PI) * 1/pow(2*i+1, 2) * pow(-1, i);
                    sawtoothSum += sineForSawtooth[i].tick() * 2/PI * 1/(i+1);
                }
                // add the three waves together to compose the output
                buffer[sample] = (squareSum * isSquareOn + triangleSum * isTriangleOn + sawtoothSum * isSawtoothOn) * gain;
            }
            else buffer[sample] = 0.0;
        }
    }
    
    
private:
    // UI Elements
    ComboBox lafBox;
    Slider gainSlider;
    TextButton squareButton, triangleButton, sawtoothButton;
    ToggleButton onOffButton;
    Label gainLabel, onOffLabel;
    
    OwnedArray<Trail> trails;
    
    // Global Variables
    float gain, frequency;
    int samplingRate, nChans, partial;
    int onOff, isSquareOn, isTriangleOn, isSawtoothOn;

    const static int maxPartialNum = 15;
    
    Sine sineForSquare[maxPartialNum];    // the sine waves which composed a square wave
    Sine sineForTriangle[maxPartialNum];  // the sine waves which composed a triangle wave
    Sine sineForSawtooth[maxPartialNum];  // the sine waves which composed a sawtooth wave
    
    const double PI = 3.1415926535897931;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};

Component* createMainContentComponent()     { return new MainContentComponent(); }


#endif  // MAINCOMPONENT_H_INCLUDED
