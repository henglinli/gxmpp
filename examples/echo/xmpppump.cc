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
#include "talk/xmpp/xmppauth.h"
#include "talk/xmpp/xmppsocket.h"
#include "xmpppump.h"
namespace echo {

XmppPump::XmppPump(XmppPumpNotify * notify) {
  state_ = buzz::XmppEngine::STATE_NONE;       
  notify_ = notify;
  client_ = NULL;
  socket_ = NULL;
  auth_ = NULL;
  disconnecting_ = false; 
}

XmppPump::~XmppPump () {
}

void XmppPump::DoLogin(const buzz::XmppClientSettings & xcs) {
  if (socket_ == NULL) {
    socket_ = new buzz::XmppSocket(xcs.use_tls());  // NOTE: deleted by TaskRunner
  }
  if (auth_ == NULL) {
    auth_ = new XmppAuth();  // NOTE: deleted by TaskRunner
  }
  if (client_ == NULL) {
    client_ = new buzz::XmppClient(this);  // NOTE: deleted by TaskRunner
  }

  if (client_ && !AllChildrenDone()) {
    OnStateChange(buzz::XmppEngine::STATE_START);
    client_->SignalStateChange.connect(this, &XmppPump::OnStateChange);
    socket_->SignalCloseEvent.connect(this, &XmppPump::OnXmppSocketClose);
    client_->Connect(xcs, "", socket_, auth_);
    client_->Start();
  }
}
void XmppPump::DoDisconnect() {
  talk_base::CritScope lock(&disconnect_cs_);
  if (!disconnecting_  && !AllChildrenDone()) {
    disconnecting_ = true;
    client_->Disconnect();
    client_ = NULL;
  }
  OnStateChange(buzz::XmppEngine::STATE_CLOSED);
}

void XmppPump::OnStateChange(buzz::XmppEngine::State state) {
  if (state_ == state) {
    return;
  }
  state_ = state;
  if (notify_ != NULL)
    notify_->OnStateChange(state);
}

void XmppPump::OnXmppSocketClose(int state) {
  if (notify_ != NULL) {
    notify_->OnXmppSocketClose(state);
  }
  //Extra clean up for a socket close that wasn't originated by a logout.
  if (!disconnecting_ && state_ != buzz::XmppEngine::STATE_CLOSED) {
    DoDisconnect();
  }
}

void XmppPump::WakeTasks() {
  talk_base::Thread::Current()->Post(this);
}

int64 XmppPump::CurrentTime() {
  return static_cast<int64>(talk_base::Time());
}

void XmppPump::OnMessage(talk_base::Message *pmsg) {
  RunTasks();
}

buzz::XmppReturnStatus XmppPump::SendStanza(const buzz::XmlElement *stanza) {
  if (!AllChildrenDone()) {
    return client_->SendStanza(stanza);
  }
  return buzz::XMPP_RETURN_BADSTATE;
}

}  // namespace echo
