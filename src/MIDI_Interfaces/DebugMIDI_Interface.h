#pragma once

#include "SerialMIDI_Interface.h"

const static char *MIDI_STATUS_TYPE_NAMES[] = { // @todo PROGMEM
    "Note Off\t",       "Note On\t\t",      "Key Pressure\t",
    "Control Change\t", "Program Change\t", "Channel Pressure",
    "Pitch Bend\t"};

/**
 * @brief   A class for MIDI interfaces sending and receiving 
 *          human-readable MIDI messages over a Stream.
 */
class StreamDebugMIDI_Interface : public StreamMIDI_Interface {
  public:
    /**
     * @brief   Construct a debug MIDI interface on the given Stream.
     * 
     * @param   stream
     *          The Stream interface.
     */
    StreamDebugMIDI_Interface(Stream &stream) : StreamMIDI_Interface(stream) {}

  protected:
    void sendImpl(uint8_t m, uint8_t c, uint8_t d1, uint8_t d2) override {
        uint8_t messageType = (m >> 4) - 8;
        if (messageType >= 7)
            return;
        stream.print(MIDI_STATUS_TYPE_NAMES[messageType]);
        stream.print("\tChannel: ");
        stream.print(c + 1);
        stream.print("\tData 1: 0x");
        stream.print(d1, HEX);
        stream.print("\tData 2: 0x");
        stream.print(d2, HEX);
        stream.print("\r\n");
        stream.flush();
    }

    void sendImpl(uint8_t m, uint8_t c, uint8_t d1) override {
        uint8_t messageType = (m >> 4) - 8;
        if (messageType >= 7)
            return;
        stream.print(MIDI_STATUS_TYPE_NAMES[messageType]);
        stream.print("\tChannel: ");
        stream.print(c + 1);
        stream.print("\tData 1: 0x");
        stream.print(d1, HEX);
        stream.print("\r\n");
        stream.flush();
    }

    virtual MIDI_read_t read() override {
        while (stream.available() > 0) {
            char data = stream.read();

            if (isHexChar(toLowerCase(data))) {
                data = toLowerCase(data);
                if (firstChar == '\0') {
                    firstChar = data;
                } else if (secondChar == '\0') {
                    secondChar = data;
                } else {
                    firstChar = secondChar;
                    secondChar = data;
                }
            } else if (isWhiteSpace(data) && firstChar && secondChar) {
                // if we received two hex characters followed by
                // whitespace
                uint8_t midiByte = hexCharToNibble(firstChar) << 4 |
                                   hexCharToNibble(secondChar);
                firstChar = '\0';
                secondChar = '\0';
                MIDI_read_t parseResult = parser.parse(midiByte);
                if (parseResult != NO_MESSAGE)
                    return parseResult;
            } else {
                ; // Ignore any characters other than whitespace and hexadecimal digits
            }
        }
        return NO_MESSAGE;
    }

  private:
    char firstChar = '\0';
    char secondChar = '\0';

    /**
     * @brief   Check if a given character is a lowercase hexadecimal digit
     *          (0-9 or a-f).
     * 
     * @param   hex
     *          The hexadecimal character to check.
     * @return  true
     *          The character is a lowercase hexadecimal digit.
     * @return  false
     *          The character is not a lowercase hexadecimal digit.
     */
    bool isHexChar(char hex) {
        return (hex >= '0' && hex <= '9') || (hex >= 'a' && hex <= 'f');
    }
    /**
     * @brief   Convert a given letter to lower case, no effect if
     *          it's a digit (0 - 9) or if it's already lower case.
     */
    char toLowerCase(char x) { return x | 0b0100000; }
    /**
     * @brief   Convert a hexadecimal character to a 4-bit nibble.
     */
    uint8_t hexCharToNibble(char hex) {
        return hex < 'a' ? hex - '0' : hex - 'a' + 10;
    }
    /**
     * @brief   Check if the given character is whitespace (SP, CR or LF).
     * 
     * @return  true
     *          The given character is either a space,
     *          a carriage return or a line feed (SP, CR or LF).
     */
    inline bool isWhiteSpace(char x) {
        return x == ' ' || x == '\r' || x == '\n';
    }
};

/**
 * @brief   A wrapper class for debug MIDI interfaces sending and receiving 
 *          human-readable MIDI messages over a Serial port of class T.
 * 
 * @note    This is a template class because the class of the Serial object
 *          is completely different on different architectures, and they
 *          do not share a common super-class that has a `begin` method.
 */
template <typename T>
class SerialDebugMIDI_Interface : public StreamDebugMIDI_Interface {
  public:
    /**
     * @brief   Construct a new Debug MIDI Interface on the given Serial interface
     *          with the given baud rate.
     * 
     * @param   serial
     *          The Serial interface.
     * @param   baud
     *          The baud rate for the Serial interface.
     */
    SerialDebugMIDI_Interface(T &serial, unsigned long baud)
        : StreamDebugMIDI_Interface(serial), serial(serial), baud(baud) {}
    /**
     * @brief   Start the Serial interface at the predefined baud rate.
     */
    virtual void begin() override { serial.begin(baud); }

  private:
    T &serial;
    const unsigned long baud;
};

/**
 * @brief   A class for debug MIDI interfaces sending and receiving 
 *          human-readable MIDI messages over a HardwareSerial port.
 */
class HardwareSerialDebugMIDI_Interface
    : public SerialDebugMIDI_Interface<HardwareSerial> {
  public:
    /**
     * @brief   Construct a new Debug MIDI Interface on the given HardwareSerial 
     *          interface with the given baud rate.
     * 
     * @param   serial
     *          The HardwareSerial interface.
     * @param   baud
     *          The baud rate for the serial interface.
     */
    HardwareSerialDebugMIDI_Interface(HardwareSerial &serial,
                                      unsigned long baud)
        : SerialDebugMIDI_Interface(serial, baud) {}
};

// The Serial USB connection
// (Why do I have to do this, why don't they all inherit from one single class?)

// Boards without a USB connection (UNO, MEGA, Nano ...)
#if !(defined(USBCON) || defined(CORE_TEENSY))
/**
 * @brief   A class for debug MIDI interfaces sending and receiving 
 *          human-readable MIDI messages over the USB CDC connection.
 * 
 *          Boards without a native USB connection (UNO, MEGA, Nano ...)
 *          use HardwareSerial0 for USB communcication.
 */
class USBDebugMIDI_Interface : public HardwareSerialDebugMIDI_Interface {
  public:
    /**
     * @brief   Construct a USBDebugMIDI_Interface with the given baud rate.
     * 
     * @param   baud
     *          The baud rate to start the USB Serial connection with.
     */
    USBDebugMIDI_Interface(unsigned long baud)
        : HardwareSerialDebugMIDI_Interface(Serial, baud) {}
};

// Teensies
#elif defined(CORE_TEENSY)
/**
 * @brief   A class for debug MIDI interfaces sending and receiving 
 *          human-readable MIDI messages over the USB Serial CDC connection.
 * 
 *          The USB Serial connection `Serial` on Teensies is an instance of
 *          the `usb_serial_class`.
 */
class USBDebugMIDI_Interface
    : public SerialDebugMIDI_Interface<usb_serial_class> {
  public:
    /**
     * @brief   Construct a USBDebugMIDI_Interface with the given baud rate.
     * 
     * @param   baud
     *          The baud rate to start the USB Serial connection with.
     */
    USBDebugMIDI_Interface(unsigned long baud)
        : SerialDebugMIDI_Interface(Serial, baud) {}
};

// Arduino DUE
#elif defined(ARDUINO_ARCH_SAM)
/**
 * @brief   A class for debug MIDI interfaces sending and receiving 
 *          human-readable MIDI messages over the USB Serial CDC connection.
 * 
 *          The USB Serial connection `Serial` on the Arduino DUE is an 
 *          instance of the `UARTClass`.
 */
class USBDebugMIDI_Interface : public SerialDebugMIDI_Interface<UARTClass> {
  public:
    /**
     * @brief   Construct a USBDebugMIDI_Interface with the given baud rate.
     * 
     * @param   baud
     *          The baud rate to start the USB Serial connection with.
     */
    USBDebugMIDI_Interface(unsigned long baud)
        : SerialDebugMIDI_Interface(Serial, baud) {}
};

// Others (Leonardo, Micro ... )
#else
/**
 * @brief   A class for debug MIDI interfaces sending and receiving 
 *          human-readable MIDI messages over the USB Serial CDC connection.
 * 
 *          The USB Serial connection `Serial` on the Arduino Leonardo, Micro,
 *          etc. is an instance of the `Serial_` class.
 */
class USBDebugMIDI_Interface : public SerialDebugMIDI_Interface<Serial_> {
  public:
    /**
     * @brief   Construct a USBDebugMIDI_Interface with the given baud rate.
     * 
     * @param   baud
     *          The baud rate to start the USB Serial connection with.
     */
    USBDebugMIDI_Interface(unsigned long baud)
        : SerialDebugMIDI_Interface(Serial, baud) {}
};
#endif

#if defined(__AVR__) || defined(CORE_TEENSY)
#include <SoftwareSerial.h>
/**
 * @brief   A class for debug MIDI interfaces sending and receiving 
 *          human-readable MIDI messages over a SoftwareSerial interface.
 */
class SoftwareSerialDebugMIDI_Interface
    : public SerialDebugMIDI_Interface<SoftwareSerial> {
  public:
    /**
     * @brief   Construct a SoftwareSerialDebugMIDI_Interface on the given 
     *          SoftwareSerial interface with the given baud rate.
     * 
     * @param   serial
     *          The SoftwareSerial interface.
     * @param   baud
     *          The baud rate for the serial interface.
     */
    SoftwareSerialDebugMIDI_Interface(SoftwareSerial &serial,
                                      unsigned long baud)
        : SerialDebugMIDI_Interface(serial, baud) {}
};
#endif
