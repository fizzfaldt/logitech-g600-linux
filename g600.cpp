#include <linux/input.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <assert.h>

static input_event events[64];
static const char kDir[] = "/dev/input/by-id/";
static const char kPrefix[] = "usb-Logitech_Gaming_Mouse_G600_";
static const char kSuffix[] = "-if01-event-kbd";

enum {
    SCANCODE_G7  = 30u,
    SCANCODE_G8  = 31u,
    SCANCODE_G9  = 32u,
    SCANCODE_G10 = 33u,
    SCANCODE_G11 = 34u,
    SCANCODE_G12 = 35u,
    SCANCODE_G13 = 36u,
    SCANCODE_G14 = 37u,
    SCANCODE_G15 = 38u,
    SCANCODE_G16 = 39u,
    SCANCODE_G17 = 45u,
    SCANCODE_G18 = 48u,
    SCANCODE_G19 = 47u,
    SCANCODE_G20 = 46u,

    HIGHEST = 48u,
    NELEM   = HIGHEST + 1u,
};

struct Command {
  const size_t scancode_plus_nelem;
  const char* command_down;  //Executed command when user stops pressing the button
  const char* command_up;    //Executed command when user stops pressing the button
};

#define SCANCODE(Gcode, down, up) [SCANCODE_## Gcode] = { \
    .scancode_plus_nelem = NELEM + SCANCODE_## Gcode,     \
    .command_down = down,                                 \
    .command_up   = up,                                   \
}
// ADD KEY->COMMAND MAPPINGS HERE:
static const Command kCommands[NELEM] = {
    SCANCODE(G7,
             /*Down*/ "xdotool keydown t",
             /*Up  */ "xdotool keyup   t"),
    SCANCODE(G8,
             /*Down*/ "/usr/bin/xte 'keydown Alt_L' 'key F4' 'keyup Alt_L'",
             /*Up  */ ""),
    SCANCODE(G9,
             /*Down*/ "xdotool keydown 1",
             /*Up  */ "xdotool keyup   1"),
    SCANCODE(G10,
             /*Down*/ "xdotool keydown 2",
             /*Up  */ "xdotool keyup   2"),
    SCANCODE(G11,
             /*Down*/ "xdotool keydown 3",
             /*Up  */ "xdotool keyup   3"),
    SCANCODE(G12,
             /*Down*/ "xdotool keydown 4",
             /*Up  */ "xdotool keyup   4"),
    SCANCODE(G13,
             /*Down*/ "xdotool keydown 5",
             /*Up  */ "xdotool keyup   5"),
    SCANCODE(G14,
             /*Down*/ "xdotool keydown 6",
             /*Up  */ "xdotool keyup   6"),
    SCANCODE(G15,
             /*Down*/ "xdotool keydown z",
             /*Up  */ "xdotool keyup   z"),
    SCANCODE(G16,
             /*Down*/ "xdotool keydown x",
             /*Up  */ "xdotool keyup   x"),
    SCANCODE(G17,
             /*Down*/ "xdotool keydown c",
             /*Up  */ "xdotool keyup   c"),
    SCANCODE(G18,
             /*Down*/ "xdotool keydown v",
             /*Up  */ "xdotool keyup   v"),
    SCANCODE(G19,
             /*Down*/ "xdotool keydown b",
             /*Up  */ "xdotool keyup   b"),
    SCANCODE(G20,
             /*Down*/ "xdotool keydown n",
             /*Up  */ "xdotool keyup   n"),
};

static bool
starts_with(const char* haystack, const char* prefix) {
    auto prefix_length = strlen(prefix), haystack_length = strlen(haystack);
    if (haystack_length < prefix_length) return false;
    return strncmp(prefix, haystack, prefix_length) == 0;
}

static bool
ends_with(const char* haystack, const char* suffix) {
  auto suffix_length = strlen(suffix), haystack_length = strlen(haystack);
  if (haystack_length < suffix_length) return false;
  auto haystack_end = haystack + haystack_length - suffix_length;
  return strncmp(suffix, haystack_end, suffix_length) == 0;
}

// Returns non-0 on error.
static int
find_g600(std::string* path) {
    *path = kDir;
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(kDir)) == nullptr) {
        return 1;
    }
    while ((ent = readdir(dir))) {
        if (starts_with(ent->d_name, kPrefix) && ends_with(ent->d_name, kSuffix)) {
            *path += ent->d_name;
            closedir(dir);
            return 0;
        }
    }
    closedir(dir);
    return 2;
}

int main() {
    // Validate table
    for (size_t i = 0; i < sizeof(kCommands) / sizeof(kCommands[0]); ++i) {
        const Command* cmd = &kCommands[i];
        if (cmd->scancode_plus_nelem == 0) {
            assert(cmd->command_up   == nullptr);
            assert(cmd->command_down == nullptr);
        } else {
            assert(i + NELEM == cmd->scancode_plus_nelem);
            assert(cmd->command_up   != nullptr);
            assert(cmd->command_down != nullptr);
        }
    }

    printf("Starting G600 Linux controller.\n\n");
    printf("It's a good idea to configure G600 with Logitech Gaming Software before running this program:\n");
    printf(" - assign left, right, middle mouse button and vertical mouse wheel to their normal functions\n");
    printf(" - assign the G-Shift button to \"G-Shift\"\n");
    printf(" - assign all other keys (including horizontal mouse wheel) to arbitrary (unique) keyboard keys\n");
    printf("\n");
    std::string path;
    int find_error = find_g600(&path);
    if (find_error) {
        printf("Error: Couldn't find G600 input device.\n");
        switch(find_error) {
            case 1:
                printf("Suggestion: Maybe the expected directory (%s) is wrong. Check whether this directory exists and fix it by editing \"g600.cpp\".\n", kDir);
                break;
            case 2:
                printf("Suggestion: Maybe the expected device prefix (%s) is wrong. Check whether a device with this prefix exists in %s and fix it by editing \"g600.cpp\".\n", kPrefix, kDir);
                break;
        }
        printf("Suggestion: Maybe a permission is missing. Try running this program with with sudo.\n");
        return 1;
    }
    int fd = open(path.c_str(), O_RDONLY);
    if (fd < 0) {
        printf("Error: Couldn't open \"%s\" for reading.\n", path.c_str());
        printf("Reason: %s.\n", strerror(errno));
        printf("Suggestion: Maybe a permission is missing. Try running this program with with sudo.\n");
        return 1;
    }

    ioctl(fd, EVIOCGRAB, 1);
    printf("G600 controller started successfully.\n\n");
    while (1) {
        {
            const ssize_t sn = read(fd, events, sizeof(events));
            if (sn < 0) {
                if (errno == EINTR) {
                    continue;
                }
            }
            // FIXME: [yrobot 2018-12-05] May want to retry here
            if (sn == 0) return 2;
            const size_t n = static_cast<size_t>(sn);

            // FIXME: [yrobot 2018-12-05] May want to retry here
            //                            but it's a short read and need to CONTINUE! Harder!
            if (n < sizeof(input_event) * 2) {
                continue;
            }
        }
        if (0
            || events[0].type != 4
            || events[0].code != 4
            || events[1].type != 1)
        {
            continue;
        }
        const bool pressed = events[1].value;
        const unsigned scancode = static_cast<unsigned>(events[0].value & ~0x70000);

        // Bounds Check
        assert(scancode >= 0);
        assert(scancode < NELEM);
        const Command* cmd = &kCommands[scancode];
        // Verify it's initialized
        assert(scancode + NELEM  == cmd->scancode_plus_nelem);
        assert(cmd->command_up   != nullptr);
        assert(cmd->command_down != nullptr);

        if (!*cmd->command_down && !*cmd->command_down) {
            printf("Warning: Pressed a key (%d) without a mapping.\n", scancode);
            printf("Suggestion: Add a mapping by editing \"g600.cpp\".\n");
            printf("\n");
            continue;
        }
        const char* cmdToRun = (pressed) ? cmd->command_down : cmd->command_up;
        const char* actionStr = (pressed) ? "Pressed" : "Released";
        printf("%s scancode %d. Mapped command: \"%s\"\n",actionStr, scancode, cmdToRun);
        if (!*cmdToRun) {
            continue;
        }
        system(cmdToRun);
        printf("Subprocess finished.\n\n");
    }

    // No need to close(fd) cause we never quit
}
