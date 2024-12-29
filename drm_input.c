/*
DRM Input for handling Keyboard Event.
*/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <errno.h>
#include <signal.h>
#include <poll.h>
#include <string.h>

#define MAX_KEYBOARDS 10
#define isBitSet(bit, arr) (arr[(bit) / 8] & (1 << ((bit) % 8)))

struct Keyboard {
    int fd;
};

struct Keyboard keyboards[MAX_KEYBOARDS];
int numKeyboards = 0;

void closeKeyboard(struct Keyboard* kb) {
    if (kb->fd >= 0) {
        close(kb->fd);
        kb->fd = -1;
    }
}

const char* translateScanCode(int code) {
    switch (code) {
    case KEY_A: return "A";
    case KEY_B: return "B";
    case KEY_C: return "C";
    case KEY_D: return "D";
    case KEY_E: return "E";
    case KEY_F: return "F";
    case KEY_G: return "G";
    case KEY_H: return "H";
    case KEY_I: return "I";
    case KEY_J: return "J";
    case KEY_K: return "K";
    case KEY_L: return "L";
    case KEY_M: return "M";
    case KEY_N: return "N";
    case KEY_O: return "O";
    case KEY_P: return "P";
    case KEY_Q: return "Q";
    case KEY_R: return "R";
    case KEY_S: return "S";
    case KEY_T: return "T";
    case KEY_U: return "U";
    case KEY_V: return "V";
    case KEY_W: return "W";
    case KEY_X: return "X";
    case KEY_Y: return "Y";
    case KEY_Z: return "Z";
    case KEY_1: return "1";
    case KEY_2: return "2";
    case KEY_3: return "3";
    case KEY_4: return "4";
    case KEY_5: return "5";
    case KEY_6: return "6";
    case KEY_7: return "7";
    case KEY_8: return "8";
    case KEY_9: return "9";
    case KEY_0: return "0";
    case KEY_ENTER: return "Enter";
    case KEY_ESC: return "Escape";
    case KEY_BACKSPACE: return "Backspace";
    case KEY_TAB: return "Tab";
    case KEY_SPACE: return "Space";
    case KEY_MINUS: return "-";
    case KEY_EQUAL: return "=";
    case KEY_LEFTBRACE: return "[";
    case KEY_RIGHTBRACE: return "]";
    case KEY_BACKSLASH: return "\\";
    case KEY_SEMICOLON: return ";";
    case KEY_APOSTROPHE: return "'";
    case KEY_GRAVE: return "`";
    case KEY_COMMA: return ",";
    case KEY_DOT: return ".";
    case KEY_SLASH: return "/";
    case KEY_CAPSLOCK: return "Caps Lock";
    case KEY_F1: return "F1";
    case KEY_F2: return "F2";
    case KEY_F3: return "F3";
    case KEY_F4: return "F4";
    case KEY_F5: return "F5";
    case KEY_F6: return "F6";
    case KEY_F7: return "F7";
    case KEY_F8: return "F8";
    case KEY_F9: return "F9";
    case KEY_F10: return "F10";
    case KEY_F11: return "F11";
    case KEY_F12: return "F12";
        // Add more key codes as needed
    default: return "Unknown";
    }
}

void pollKeyboardEvent(struct Keyboard* kb) {
    struct input_event e;
    int dropped = 0;

    errno = 0;
    if (read(kb->fd, &e, sizeof(e)) < 0) {
        // Reset the keyboard slot if the device was disconnected
        if (errno == ENODEV)
            closeKeyboard(kb);

        return;
    }

    printf("Event type=%u code=%u value=%u\n", e.type, e.code, e.value);

    if (e.type == EV_SYN) {
        if (e.code == SYN_DROPPED)
            dropped = 1;
        else if (e.code == SYN_REPORT) {
            dropped = 0;
            // pollAbsState(js);
        }
    }

    if (dropped)
        return;

    // if (e.type == EV_KEY) {
//     const char* keyName = translateScanCode(e.code);
//     if (e.value == 1)
//         printf("Key %s[%d] pressed\n", keyName, e.code);
//     else if (e.value == 0)
//         printf("Key %s[%d] released\n", keyName, e.code);
// } else if (e.type == EV_REL) {
//     printf("Relative axis %d moved by %d\n", e.code, e.value);
// } else if (e.type == EV_ABS) {
//     printf("Absolute axis %d moved to %d\n", e.code, e.value);
// }


// if (e.type == EV_SYN) {
//     if (e.code == SYN_MT_REPORT)
//         printf("++++++++++++++ %s ++++++++++++\n", "codename(type, code)");
//     else if (e.code == SYN_DROPPED)
//         printf(">>>>>>>>>>>>>> %s <<<<<<<<<<<<\n", "codename(type, code)");
//     else
//         printf("-------------- %s ------------\n", "codename(type, code)");
// } else {
//     printf("type %d (%s), code %d (%s), ",
//         e.type, "typename(type)",
//         e.code, "codename(type, code)");
//     if (e.type == EV_MSC && (e.code == MSC_RAW || e.code == MSC_SCAN))
//         printf("value %02x\n", e.value);
//     else
//         printf("value %d\n", e.value);
// }
}

void handleSignal(int signal) {
    (void) signal;
    // Close all keyboard devices
    for (int i = 0; i < numKeyboards; i++) {
        closeKeyboard(&keyboards[i]);
    }
    printf("Exiting...\n");
    exit(EXIT_SUCCESS);
}

int isKeyboardDevice(const char* devicePath, char* deviceName, size_t nameSize) {
    int fd = open(devicePath, O_RDONLY);
    if (fd < 0) {
        return 0;
    }

    struct input_id deviceInfo;
    if (ioctl(fd, EVIOCGID, &deviceInfo) < 0) {
        close(fd);
        return 0;
    }

    if (ioctl(fd, EVIOCGNAME(nameSize), deviceName) < 0) {
        strncpy(deviceName, "Unknown", nameSize - 1);
        deviceName[nameSize - 1] = '\0';
    }

    unsigned long evbit[EV_MAX / sizeof(unsigned long)];
    if (ioctl(fd, EVIOCGBIT(0, sizeof(evbit)), evbit) < 0) {
        close(fd);
        return 0;
    }

    unsigned long keybit[KEY_MAX / sizeof(unsigned long)];
    if (ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(keybit)), keybit) < 0) {
        close(fd);
        return 0;
    }

    close(fd);

    // Check if the device supports EV_KEY events and has keys typically found on keyboards
    return (evbit[EV_KEY / (8 * sizeof(unsigned long))] & (1 << (EV_KEY % (8 * sizeof(unsigned long))))) &&
        (keybit[KEY_A / (8 * sizeof(unsigned long))] & (1 << (KEY_A % (8 * sizeof(unsigned long)))));
}

int main() {
    // Set up signal handler for SIGINT
    signal(SIGINT, handleSignal);

    // Open multiple keyboard device files
    for (int i = 0; i < MAX_KEYBOARDS; i++) {
        char devicePath[32];
        char deviceName[256];
        snprintf(devicePath, sizeof(devicePath), "/dev/input/event%d", i);
        if (isKeyboardDevice(devicePath, deviceName, sizeof(deviceName))) {
            keyboards[numKeyboards].fd = open(devicePath, O_RDONLY);
            if (keyboards[numKeyboards].fd >= 0) {
                printf("Found keyboard: %s\n", deviceName);

                char evBits[(EV_CNT + 7) / 8] = { 0 };
                char keyBits[(KEY_CNT + 7) / 8] = { 0 };
                char absBits[(ABS_CNT + 7) / 8] = { 0 };
                char guid[33] = "";
                struct input_id id;

                if (ioctl(keyboards[numKeyboards].fd, EVIOCGBIT(0, sizeof(evBits)), evBits) < 0 ||
                    ioctl(keyboards[numKeyboards].fd, EVIOCGBIT(EV_KEY, sizeof(keyBits)), keyBits) < 0 ||
                    ioctl(keyboards[numKeyboards].fd, EVIOCGBIT(EV_ABS, sizeof(absBits)), absBits) < 0 ||
                    ioctl(keyboards[numKeyboards].fd, EVIOCGID, &id) < 0) {
                    printf("Linux: Failed to query input device: %s", strerror(errno));
                    // close(keyboards[numKeyboards].fd);
                    // return GLFW_FALSE;
                } else {
                    // Generate a keyboard GUID that matches the SDL 2.0.5+ one
                    if (id.vendor && id.product && id.version) {
                        sprintf(guid, "%02x%02x0000%02x%02x0000%02x%02x0000%02x%02x0000",
                            id.bustype & 0xff, id.bustype >> 8,
                            id.vendor & 0xff, id.vendor >> 8,
                            id.product & 0xff, id.product >> 8,
                            id.version & 0xff, id.version >> 8);
                    } else {
                        sprintf(guid, "%02x%02x0000%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x00",
                            id.bustype & 0xff, id.bustype >> 8,
                            deviceName[0], deviceName[1], deviceName[2], deviceName[3],
                            deviceName[4], deviceName[5], deviceName[6], deviceName[7],
                            deviceName[8], deviceName[9], deviceName[10]);
                    }
                    printf("input device GUID: %s\n", guid);

                    // Count its buttons and axes
                    int axisCount = 0, buttonCount = 0, hatCount = 0;
                    for (int code = BTN_MISC; code < KEY_CNT; code++) {
                        if (!isBitSet(code, keyBits))
                            continue;

                        buttonCount++;
                    }
                    for (int code = 0; code < ABS_CNT; code++) {
                        if (!isBitSet(code, absBits))
                            continue;

                        if (code >= ABS_HAT0X && code <= ABS_HAT3Y) {
                            hatCount++;
                            // Skip the Y axis
                            code++;
                        } else {
                            struct input_absinfo absInfo;
                            if (ioctl(keyboards[numKeyboards].fd, EVIOCGABS(code), &absInfo) < 0)
                                continue;

                            axisCount++;
                        }
                    }
                    printf("Found %d buttons, %d axes, %d hats\n", buttonCount, axisCount, hatCount);
                }
                numKeyboards++;
            }
        }
    }

    if (numKeyboards == 0) {
        fprintf(stderr, "No keyboard devices found\n");
        return EXIT_FAILURE;
    } else {
        printf("Found %d keyboard devices\n", numKeyboards);
    }

    struct pollfd fds[MAX_KEYBOARDS];
    for (int i = 0; i < numKeyboards; i++) {
        fds[i].fd = keyboards[i].fd;
        fds[i].events = POLLIN;
    }

    // Poll events from all keyboard devices
    while (1) {
        int ret = poll(fds, numKeyboards, -1);
        if (ret < 0) {
            perror("poll");
            break;
        }

        for (int i = 0; i < numKeyboards; i++) {
            if (fds[i].revents & POLLIN) {
                pollKeyboardEvent(&keyboards[i]);
            }
        }
    }

    return EXIT_SUCCESS;
}
