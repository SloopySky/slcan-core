#include "slcan.hpp"
#include <algorithm>

namespace slcan::core {

void Slcan::processSlRxMsg(const SlMsg& msg) const {
    SlMsg response;

    if (can.state() == CanInterface::State::OPEN) {
        switch (static_cast<SlcanCommand>(msg.command())) {
            case SlcanCommand::CAN_STD:       canStdCmd(msg, response); break;
            case SlcanCommand::CAN_EXT:       canExtCmd(msg, response); break;
            case SlcanCommand::CLOSE:         closeCmd(msg, response); break;
            case SlcanCommand::CR:            crCmd(msg, response); break;
            case SlcanCommand::VERSION:       versionCmd(msg, response); break;
            case SlcanCommand::SERIAL_NUMBER: serialNumberCmd(msg, response); break;
            default:                          unsupportedCmd(msg, response);
        }
    } else {
        switch (static_cast<SlcanCommand>(msg.command())) {
            case SlcanCommand::OPEN:          openCmd(msg, response); break;
            case SlcanCommand::CR:            crCmd(msg, response); break;
            case SlcanCommand::VERSION:       versionCmd(msg, response); break;
            case SlcanCommand::SERIAL_NUMBER: serialNumberCmd(msg, response); break;
            default:                          unsupportedCmd(msg, response);
        }
    }

    serial.transmit(response);
}

inline void Slcan::crCmd(const SlMsg& request, SlMsg& response) const {
    response = SlMsg({SlMsg::CR});
}

inline void Slcan::openCmd(const SlMsg& request, SlMsg& response) const {
    bool result = can.open();
    response = result ? SlMsg({SlMsg::CR}) : SlMsg({SlMsg::BELL});
}

inline void Slcan::closeCmd(const SlMsg& request, SlMsg& response) const {
    bool result = can.close();
    response = result ? SlMsg({SlMsg::CR}) : SlMsg({SlMsg::BELL});
}

inline void Slcan::canStdCmd(const SlMsg& request, SlMsg& response) const {
    unsupportedCmd(request, response);
}

inline void Slcan::canExtCmd(const SlMsg& request, SlMsg& response) const {
    unsupportedCmd(request, response);
}

inline void Slcan::versionCmd(const SlMsg& request, SlMsg& response) const {
    response = SlMsg({
        static_cast<char>(SlcanCommand::VERSION),
        VERSION[0],
        VERSION[1],
        VERSION[2],
        VERSION[3],
        SlMsg::CR
    });
}

inline void Slcan::serialNumberCmd(const SlMsg& request, SlMsg& response) const {
    response = SlMsg({
        static_cast<char>(SlcanCommand::SERIAL_NUMBER),
        SERIAL_NUMBER[0],
        SERIAL_NUMBER[1],
        SERIAL_NUMBER[2],
        SERIAL_NUMBER[3],
        SlMsg::CR
    });
}

inline void Slcan::unsupportedCmd(const SlMsg& request, SlMsg& response) const {
    response = SlMsg({SlMsg::BELL});
}

};

