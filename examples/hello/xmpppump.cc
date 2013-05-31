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

#include "xmpppump.h"

#include <iostream>

#include "talk/base/logging.h"
#include "talk/xmpp/prexmppauth.h"
#include "xmpptasks.h"

namespace hello {

XmppPump::XmppPump(XmppPumpNotify * notify) {
  state_ = buzz::XmppEngine::STATE_NONE;
  notify_ = notify;
  client_ = new buzz::XmppClient(this);  // NOTE: deleted by TaskRunner
}

void XmppPump::DoLogin(const buzz::XmppClientSettings & xcs,
                       buzz::AsyncSocket* socket,
                       buzz::PreXmppAuth* auth) {
  OnStateChange(buzz::XmppEngine::STATE_START);
  if (!AllChildrenDone()) {
    client_->SignalStateChange.connect(this, &XmppPump::OnStateChange);
    if (client_->Connect(xcs, "", socket, auth) != buzz::XMPP_RETURN_OK) {
      LOG(LS_ERROR) << "Failed to connect.";
    }
    client_->Start();
  }
}

void XmppPump::DoDisconnect() {
  if (!AllChildrenDone())
    client_->Disconnect();
  OnStateChange(buzz::XmppEngine::STATE_CLOSED);
}

void XmppPump::OnStateChange(buzz::XmppEngine::State state) {
  if (this->state_ == state)
    return;
  std::cout << "STATE: " << this->state_ << std::endl;
  switch(state) {
    case buzz::XmppEngine::STATE_OPEN: {
      // task_message, task_precence and task_iq are deleted by client_
      //
      // This accepts <message/> stanzas and prints the sender and message
      // to stdout
      XmppTaskMessage *task_message = new XmppTaskMessage(client_);
      task_message->Start();
      // This accepts <presence/> stanzas and prints whom they're from
      // to stdout
      XmppTaskPresence *task_presence = new XmppTaskPresence(client_);
      task_presence->Start();
      // This sends a privacy list request on Start and handles only its
      // response
      XmppTaskIq *task_iq = new XmppTaskIq(client_);
      task_iq->Start();
      break;
    }
    case buzz::XmppEngine::STATE_START:
    case buzz::XmppEngine::STATE_OPENING:
      break;
  case buzz::XmppEngine::STATE_CLOSED:
      std::cout << "Error: " << client_->GetError(NULL) << std::endl;
      break;
  default :
    break;
  }
  this->state_ = state;
  if (notify_ != NULL)
    notify_->OnStateChange(state);
}

void XmppPump::WakeTasks() {
  talk_base::Thread::Current()->Post(this);
}

int64 XmppPump::CurrentTime() {
  return (int64)talk_base::Time();
}

void XmppPump::OnMessage(talk_base::Message *pmsg) {
  RunTasks();
}

buzz::XmppReturnStatus XmppPump::SendStanza(const buzz::XmlElement *stanza) {
  if (!AllChildrenDone())
    return client_->SendStanza(stanza);
  return buzz::XMPP_RETURN_BADSTATE;
}

}  // namespace hello
