#pragma once

#include "serial.hpp"
#include "can.hpp"

namespace slcan::core {

constexpr std::array<std::uint8_t, 4> VERSION = { '0', '0', '0', '1' };
constexpr std::array<std::uint8_t, 4> SERIAL_NUMBER = { '-', '-', '-', '-' };

constexpr std::size_t COMMAND_LEN = 1;

enum class SlcanCommand : char {
	// [CR]
	CR = SlMsg::CR,

    // O[CR]
    // Open the CAN channel
    // This command is only active if the CAN channel is closed
    // Returns: CR for OK or BELL for ERROR
    OPEN = 'O',

    // C[CR]
    // Close the CAN channel
    // This command is only active if the CAN channel is open
    // Returns: CR for OK or BELL for ERROR
    CLOSE = 'C',

    // t[CR]
    // Transmit a standard (11bit) CAN frame
    // This command is only active if the CAN channel is open
    // iii - Identifier in hex (000-7FF)
    // l - Data length (0-8)
    // dd - Byte value in hex (00-FF)
    // Example: t10021133[CR] - Sends a CAN frame with ID=0x100, DLC=2, data=0x1133
    // Returns: z[CR] for OK or BELL for ERROR.
    CAN_STD = 't',

    // T[CR]
    // Transmit an extended (29bit) CAN frame
    // This command is only active if the CAN channel is open
    // iiiiiiii - Identifier in hex (00000000-1FFFFFFF)
    // l - Data length (0-8)
    // dd - Byte value in hex (00-FF)
    // Example: T0000010021133[CR] - Sends a CAN frame with ID=0x100, DLC=2, data=0x1133
    // Returns: Z[CR] for OK or BELL for ERROR
    CAN_EXT = 'T',

	// V[CR]
	// Get software version number
	// Returns: V and a 4 bytes BCD value plus CR for OK (e.g. V1013[CR])
	VERSION = 'V',

	// N[CR]
	// Get serial number
	// Returns: N and a 4 bytes value for serial number plus CR for OK (e.g. NA123[CR])
	SERIAL_NUMBER = 'N',
};

enum class SlcanResponse : char {
	CR = SlMsg::CR,
    BELL = SlMsg::BELL,
    CAN_STD = 'z',
    CAN_EXT = 'Z',
};

class Slcan {
public:
    Slcan(SerialInterface& serial, CanInterface& can) : serial(serial), can(can) { }

    void processSlRxMsg(const SlMsg&) const;

private:
    enum class State { OPEN, CLOSED };
    State state = State::CLOSED;

    SerialInterface& serial;
    CanInterface& can;

    void crCmd(const SlMsg& request, SlMsg& response) const;
    void openCmd(const SlMsg& request, SlMsg& response) const;
    void closeCmd(const SlMsg& request, SlMsg& response) const;
    void canStdCmd(const SlMsg& request, SlMsg& response) const;
    void canExtCmd(const SlMsg& request, SlMsg& response) const;
    void versionCmd(const SlMsg& request, SlMsg& response) const;
    void serialNumberCmd(const SlMsg& request, SlMsg& response) const;
    void unsupportedCmd(const SlMsg& request, SlMsg& response) const;
};

};

