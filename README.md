# A simple (and somewhat dirty) sockets wrapper for linux

TODO:
Ensure <EOF> parsing can handle null bytes

KNOWN ISSUES:
Race condition if file exists beforehand.
Worst case, a few of the first writes are ignored and it is patched up after
downloading. Still should fix.
