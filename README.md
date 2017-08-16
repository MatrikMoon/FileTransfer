# A simple (and somewhat dirty) sockets wrapper for linux

# EDIT: This socket wrapper turned into a file transfer program. I'll fork from an old point before
# the change and create a new repo for only the wrapper.

TODO:
Ensure <EOF> parsing can handle null bytes

KNOWN ISSUES:
Race condition if file exists beforehand.
Worst case, a few of the first writes are ignored and it is patched up after
downloading. Still should fix.

Race condition with unordered_map "statlist." Causing random segfaults.
Fix this for stability.

If a patch request is split into multiple pieces, multiple "end" segments
will be sent back. This is suuuper slow with large files. Fix it.



Technically, because of the desynchronization between TCP and UDP receivers,
There's likely to be an extra bit of data being sent to patch because TCP delivers
and "end" message before the previous patch messages sent in UDP are completely
processed. And so, these are the sacrifices we must make for fast transfer.
