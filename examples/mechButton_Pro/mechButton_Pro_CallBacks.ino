#include <mechButton.h>
#include <blinker.h>    // The base LC_baseTools blinker class.
#include <idlers.h>

#define BUTTON_PIN  2     // Pin we'll hook the button to. The other side hooks to ground.
#define LED_PIN      13    // Usual pin number for built in LED.

#define DEF_HOLD_MS 10000     // How many ms we'll call a "long" button press.
#define __DEF_HOLD_MS__ 5000 // How many ms we'll call a "long" button press.
#define NUM_CHECKS 4 // Number of conflicting checks in a row that tells us to change.


// ***********************************************************
//                proButton class definition
//
//    Inherit from mechButton and create an class that does
//    your custom bidding.
// ***********************************************************


class ProButton : public mechButton,
                  public timeObj
{
public:
    ProButton(uint8_t inPin);
    virtual ~ProButton(void);
    virtual void takeaction(void);
    virtual void trueFalse(void);
    void setLongCallback(funct_callback_t funct);
    void setProCallback(funct_callback_t funct);
    virtual void idle(void);

private:
    timeObj _proTimer; // The timer for the long button press.
    void setTimer(void);
    funct_callback_t _longCallback;
    funct_callback_t _proCallback;
};



// ***********************************************************
//                proButton class code
// ***********************************************************



ProButton::ProButton(uint8_t inPin) : mechButton(inPin),
                                      timeObj(DEF_HOLD_MS, false),
                                      _longCallback(nullptr),
                                      _proCallback(nullptr) {}

ProButton::~ProButton(void) {}

void ProButton::setTimer(void)
{
    _proTimer.setTime(__DEF_HOLD_MS__);
}

void ProButton::setLongCallback(funct_callback_t funct)
{
    _longCallback = funct;
    hookup();
}

void ProButton::setProCallback(funct_callback_t funct)
{
    setTimer();
    _proCallback = funct;
    hookup();
}

void ProButton::idle(void)
{
  this->trueFalse();
}

bool ProButton::trueFalse(void)
{
    mechButton::trueFalse();
    if (!beenInitialized)
    {                                  // If not ready to run..
        pinMode(pinNum, INPUT_PULLUP); // Set up our pin.
        setAs = digitalRead(pinNum);   // The first thing we see is our initial state.
        beenInitialized = true;        // Note we are open for business.
        mTimer.start();                // And don't bug us again for this much time.
        _proTimer.start(); // We start our timer.
        start();
    }
    else
    { // Else, we are already up and running..
        if (mTimer.ding())
        { // If the timer has expired..
            if (setAs == digitalRead(pinNum))
            {                 // If we get another reading like the saved state..
                checkNum = 0; // Reset to zero conflicting readings.
            }
            else
            {               // Else, its a conflicting reading..
                checkNum++; // Bump up checkNum.
                if (checkNum >= NUM_CHECKS)
                {                   // If we have enough conflicting readings..
                    setAs = !setAs; // Flip our state.
                    checkNum = 0;   // Reset to zero conflicting readings.
                  
                    if (_longCallback)
                    {                    // We run out of time AND have a callback..
                        _longCallback(); // Call the callback.
                        reset();         // Turn the timer off.
                    }

                    if (_proCallback)
                    {                              // We run out of time AND have a callback..
                        _proCallback();    // Call the callback.
                        _proTimer.reset(); // Turn the timer off.
                    }
                    else
                    {                 // Else, no callback..
                        takeAction(); // Do whatever action is required.
                    }
                }
            }
            mTimer.start(); // Don't bug us, restarting the timer..
            _proTimer.start(); // We start our timer.
            start();
        }
    }
    return setAs; // In all cases, return how we are set.
}


// takeAction, this is what's called when there is no callback set.
void ProButton::takeAction(void) {

	if (setAs) {
		blank(36);
		Serial.println(F("Live long and prosper."));
	} else {
		line(4);
		blank(10);Serial.println(F("                                     ---------------------------==="));
		blank(10);Serial.println(F("            __                      ( |                          =="));
		blank(10);Serial.println(F("         /------\\                    ---------------------------="));
		blank(10);Serial.println(F("------------------------------           |  |"));
		blank(10);Serial.println(F("\\____________________________/]          |  |"));
		blank(10);Serial.println(F("         --------       \\     \\          |  |"));
		blank(10);Serial.println(F("                         \\     \\         |  |"));
		blank(10);Serial.println(F("                          \\ --------_____|  |__"));
		blank(10);Serial.println(F("                         | |              --  /"));
		blank(10);Serial.println(F("                        -||                  ||"));
		blank(10);Serial.println(F("                         | |__________/------== "));
		line(2);
	}
}

void ProButton::begin()
{
    this->setCallback([this](void) -> void
                                    { quickCallback(); }); // When the button gets clicked. Call this function.
    this->setLongCallback([this](void) -> void
                                        { holdCallback(); }); // If it's been held down for a "long" time call this.
    this->setCalibrationCallback([this](void) -> void
                                            { longholdCallback(); }); // If it's been held down for a "long" time call this.
}

//***************************************************
//*                                                 *
//*              END OF ProButton                   *
//*                                                 *
//***************************************************


// ***********************************************************
//                   Sketch starts here..
// ***********************************************************

proButton button1(BUTTON_PIN);                             // Set button1 to pin 2.

// Your standard sketch setup()
void setup() {

  Serial.begin(57600);                                     // Fire up our serial monitor thing.
  pinMode(LED_PIN, OUTPUT);                                // Set up the LED pin for output.
  button1.begin();                                         // Fire up the button. (Calls hookup() for idling.)
}

// Your standard sketch loop()
void loop() {

	bool buttonState;

	idle();                                                 // Let all the idlers have time to do their thing.
	buttonState = button1.trueFalse();                      // Have a look at what the current button state is.
	digitalWrite(LED_PIN, !buttonState);                    // Since the button grounds when pushed, invert logic with !
}
