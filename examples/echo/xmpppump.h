/*
 * gxmpp
 * Copyright 2013, Henry Lee <henglinli@gmail.com>.
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


#ifndef _ECHO_XMPPPUMP_H_
#define _ECHO_XMPPPUMP_H_

#include "talk/base/taskrunner.h"
#include "talk/base/thread.h"
#include "talk/xmpp/xmppsocket.h"
#include "talk/xmpp/xmppauth.h"
#include "talk/xmpp/xmppclient.h"

namespace echo {

// Simple xmpp pump

class XmppPumpNotify {
public:
  XmppPumpNotify() {}
  virtual ~XmppPumpNotify() {}
  virtual void OnStateChange(buzz::XmppEngine::State state) = 0;
};

class XmppPump : public talk_base::MessageHandler, public talk_base::TaskRunner {
public:
  XmppPump(XmppPumpNotify *notify = NULL);
  virtual ~XmppPump();
  
  buzz::XmppClient *client() { return client_; }

  void DoLogin(const buzz::XmppClientSettings & xcs);

  void DoDisconnect();

  void OnStateChange(buzz::XmppEngine::State state);

  void OnXmppSocketClose(int state);
  
  void WakeTasks();

  int64 CurrentTime();

  void OnMessage(talk_base::Message *pmsg);

  buzz::XmppReturnStatus SendStanza(const buzz::XmlElement *stanza);

private:
  buzz::XmppEngine::State state_;
  XmppPumpNotify *notify_;
  buzz::XmppClient *client_;
  talk_base::CriticalSection disconnect_cs_;
  buzz::XmppSocket *socket_;
  XmppAuth *auth_;
  bool disconnecting_;
};

}  // namespace buzz

#endif // _ECHO_XMPPPUMP_H_
