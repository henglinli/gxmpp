/*
 * libjingle
 * Copyright 2004--2005, Google Inc.
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

#ifndef _HELLO_XMPPPUMP_H_
#define _HELLO_XMPPPUMP_H_

#include "talk/base/messagequeue.h"
#include "talk/base/taskrunner.h"
#include "talk/base/thread.h"
#include "talk/base/timeutils.h"
#include "talk/xmpp/xmppclient.h"
#include "talk/xmpp/xmppengine.h"
#include "talk/xmpp/xmpptask.h"
 #include "xmpptasks.h"

namespace hello {

class XmppPumpNotify {
  public:
    virtual ~XmppPumpNotify() {}
    virtual void OnStateChange(buzz::XmppEngine::State state) = 0;
};

class XmppPump : public talk_base::MessageHandler, public talk_base::TaskRunner {
  public:
    XmppPump(XmppPumpNotify * notify = NULL);

    buzz::XmppClient *client() { return client_; }
    buzz::XmppReturnStatus SendStanza(const buzz::XmlElement *stanza);
    int64 CurrentTime();

    void DoLogin(const buzz::XmppClientSettings & xcs,
                 buzz::AsyncSocket* socket,
                 buzz::PreXmppAuth* auth);
    void DoDisconnect();
    void WakeTasks();

    void OnStateChange(buzz::XmppEngine::State state);
    void OnMessage(talk_base::Message *pmsg);

  private:
    talk_base::scoped_ptr<XmppTaskMessage> task_message_;
    talk_base::scoped_ptr<XmppTaskPresence> task_presence_;
    talk_base::scoped_ptr<XmppTaskIq> task_iq_;
    buzz::XmppClient *client_;
    buzz::XmppEngine::State state_;
    XmppPumpNotify *notify_;
};

}  // namespace hello

#endif  // _HELLO_XMPPPUMP_H_
