#ifndef CURSES_H
#define CURSES_H

// Required for int16_6, int32_t, int64_t
#include <stdint.h>

// The port numbers for the server
#define CURSES_PORT0            1313
#define CURSES_PORT1            4444
#define CURSES_PORT2            1344
#define CURSES_PORT3            4413

// The protocol id for this version of the Curses Transfer Protocol
#define CURSES_PROTOCOL_ID      13

// An enumeration of the different client operations
typedef enum {
    // PING the server
    PING = 0,
    // Get a single curse.
    GET_SINGLE,

    // Get a random number of n curses. The argument is an integer number
    GET_MULTI,
} Operation;

// The request is followed by some variable number of integer arguments
// in network byte order. The request header itself is transmitted with each
// field in network byte order.
typedef struct req {
    int32_t protocol_id;
    Operation op;
    int16_t numargs;
} RequestHeader;


// An enumeration of the different server response statuses
typedef enum {
    // Success
    OK = 0,

    // Bad protocol
    INVALID_PROTOCOL,

    // The request was malformed somehow
    BAD_REQUEST,

    // There were not enough arguments
    INSUFFICIENT_ARGS,
} Status;

// The request is folowed by length bytes of char text in JSON format, if any.
// The request header itself is transmitted with each field in network byte
// order.
typedef struct res {
    int32_t protocol_id;
    Status status;
    int32_t length;
} ResponseHeader;

#endif // CURSES_H
