/*
 * gxmpp
 * Copyright 2013, Henry Lee <henglinli@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef _GXMPP_XMPPPUMP_H_
#define _GXMPP_XMPPPUMP_H_

#include "talk/base/asyncsocket.h"
#include "talk/base/base64.h"
#include "talk/base/basicdefs.h"
#include "talk/base/basictypes.h"
#include "talk/base/bytebuffer.h"
#include "talk/base/byteorder.h"
#include "talk/base/common.h"
#include "talk/base/constructormagic.h"
#include "talk/base/criticalsection.h"
#include "talk/base/cryptstring.h"
#include "talk/base/ipaddress.h"
#include "talk/base/linked_ptr.h"
#include "talk/base/logging.h"
#include "talk/base/messagehandler.h"
#include "talk/base/messagequeue.h"
#include "talk/base/proxyinfo.h"
#include "talk/base/scoped_ptr.h"
#include "talk/base/scoped_ref_ptr.h"
#include "talk/base/sigslot.h"
#include "talk/base/sigslotrepeater.h"
#include "talk/base/socket.h"
#include "talk/base/socketaddress.h"
#include "talk/base/socketfactory.h"
#include "talk/base/socketserver.h"
#include "talk/base/stringencode.h"
#include "talk/base/stringutils.h"
#include "talk/base/task.h"
#include "talk/base/taskparent.h"
#include "talk/base/taskrunner.h"
#include "talk/base/thread.h"
#include "talk/base/timeutils.h"
#include "talk/xmpp/asyncsocket.h"
#include "talk/xmpp/chatroommodule.h"
#include "talk/xmpp/constants.h"
#include "talk/xmpp/discoitemsquerytask.h"
#include "talk/xmpp/hangoutpubsubclient.h"
#include "talk/xmpp/iqtask.h"
#include "talk/xmpp/jid.h"
#include "talk/xmpp/module.h"
#include "talk/xmpp/moduleimpl.h"
#include "talk/xmpp/mucroomconfigtask.h"
#include "talk/xmpp/mucroomdiscoverytask.h"
#include "talk/xmpp/mucroomlookuptask.h"
#include "talk/xmpp/mucroomuniquehangoutidtask.h"
#include "talk/xmpp/plainsaslhandler.h"
#include "talk/xmpp/presenceouttask.h"
#include "talk/xmpp/presencereceivetask.h"
#include "talk/xmpp/presencestatus.h"
#include "talk/xmpp/prexmppauth.h"
#include "talk/xmpp/pubsubclient.h"
#include "talk/xmpp/pubsubtasks.h"
#include "talk/xmpp/receivetask.h"
#include "talk/xmpp/rostermodule.h"
#include "talk/xmpp/rostermoduleimpl.h"
#include "talk/xmpp/saslcookiemechanism.h"
#include "talk/xmpp/saslhandler.h"
#include "talk/xmpp/saslmechanism.h"
#include "talk/xmpp/saslplainmechanism.h"
#include "talk/xmpp/xmppauth.h"
#include "talk/xmpp/xmppclient.h"
#include "talk/xmpp/xmppclientsettings.h"
#include "talk/xmpp/xmppengine.h"
#include "talk/xmpp/xmppengineimpl.h"
#include "talk/xmpp/xmpplogintask.h"
#include "talk/xmpp/xmpppump.h"
#include "talk/xmpp/xmppsocket.h"
#include "talk/xmpp/xmppstanzaparser.h"
#include "talk/xmpp/xmpptask.h"

namespace gxmpp = ::talk_base;
namespace gxmpp = ::buzz;
#endif  // _GXMPP_XMPPPUMP_H_
