gxmpp
=====

_gxmpp_ is a C++ XMPP library created from the non-[XEP-0166][xep0166] portions
of [libjingle][libjingle] without ssl support and for andorid only.

It like [txmpp][txmpp] but up to date with [libjingle][libjingle] and less tweak.
### Info

port from libjinle svn r325.

### Requirements

 * [Expat][expat] >= 2.1.0
 * [Android NDK][ndk] (build)

### Build and Install

    make _or_
    move to folder jni and ndk_build 

### Getting Started

See the `examples` directory.

### License

This work is licensed under the New BSD License (see the LICENSE file).

[xep0166]: http://xmpp.org/extensions/xep-0166.html
[libjingle]: http://code.google.com/p/libjingle/
[gloox]: http://camaya.net/gloox/
[expat]: http://expat.sourceforge.net/
[ndk]: https://developer.android.com/tools/sdk/ndk/index.html
[txmpp]: https://github.com/silas/txmpp