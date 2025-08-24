#ifndef APT_COMMAND_H
#define APT_COMMAND_H

#include <vector>
#include <string>

// Supported commands
enum class AptCommand {
    Install,
    Uninstall,
    Search,
    Update,
    Upgrade,
    List,
    Unknown
};

#endif // APT_COMMAND_H