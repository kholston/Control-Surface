#pragma once

#include <Banks/BankableMIDIOutputAddressable.hpp>
#include <Hardware/Button.h>
#include <MIDI_Outputs/AbstractMIDIOutput.hpp>

namespace BankableNS {

/**
 * @brief   An abstract class for momentary push buttons that send MIDI events.
 * 
 * The button is debounced.
 * 
 * @see     Button
 */
template <class Sender>
class MIDIButton : public BankableMIDIOutputAddressable,
                   public AbstractMIDIOutput {
  public:
    /**
     * @brief   Construct a new bankable MIDIButton.
     * 
     * @param   pin
     *          The digital input pin with the button connected.  
     *          The internal pull-up resistor will be enabled.
     */
    MIDIButton(pin_t pin, uint8_t baseAddress, uint8_t baseChannel)
        : button{pin}, baseAddress(baseAddress), baseChannel(baseChannel) {}

    void update() final override {
        Button::State state = button.getState();
        if (state == Button::Falling) {
            lock();
            Sender::sendOn(getChannel(baseChannel), getAddress(baseAddress));
        } else if (state == Button::Rising) {
            Sender::sendOff(getChannel(baseChannel), getAddress(baseAddress));
            unlock();
        }
    }

  private:
    Button button;
    const uint8_t baseAddress;
    const uint8_t baseChannel;
};

} // namespace BankableNS