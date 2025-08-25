#ifndef __APT_H__
#define __APT_H__

#include <iostream>

#include <sys/stat.h>

#define APT_ASSIGN "APT:"
#define APT_LISTS "APT:lists/"
#define APT_TEMPDIR "T:apt"
#define APT_TEMPFILES_DIR "T:apt/files/"
#define APT_PACKAGES "APT:packages/"
#define APT_SOURCE_LIST "APT:sources.list"
#define SDK_DIR "SDK:"
#define VERSION "0.2"

#define APT_USAGE "Usage: apt [--update|--search <term>|--install <package>|--remove <package>|--verbose|--ignorepeers|--help]"

class APT {
    public:
        APT(bool verbose = false, bool ignorepeers = false) : _verbose(verbose), _ignorepeers(ignorepeers) {}
        bool createDirs();

        bool verbose() { return _verbose; };
        void verbose(bool verbose) { _verbose = verbose; };
        bool ignorePeers() { return _ignorepeers; };
        void ignorePeers(bool ignorepeers) { _ignorepeers = ignorepeers; };
    private:
        bool _verbose;
        bool _ignorepeers;
};

#endif