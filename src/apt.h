#ifndef __APT_H__
#define __APT_H__

#define APT_ASSIGN "APT:"
#define APT_LISTS "APT:lists/"
#define APT_PACKAGES "APT:packages/"
#define APT_USAGE "Usage: apt [--update|--search <term>|--install <package>|--remove <package>|--verbose|--ignorepeers]"

class APT {
    public:
        bool verbose() { return _verbose; };
        void verbose(bool verbose) { _verbose = verbose; };
        bool ignorePeers() { return _ignorepeers; };
        void ignorePeers(bool ignorepeers) { _ignorepeers = ignorepeers; };
    private:
        bool _verbose;
        bool _ignorepeers;
};

#endif