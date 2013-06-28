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

#ifdef LOGGING
#include "talk/base/logging.h"
#endif
#include "talk/base/timeutils.h"
#include "talk/base/taskrunner.h"
#include "talk/base/messagehandler.h"
#include "talk/base/thread.h"
#include "talk/xmpp/xmppauth.h"
#include "talk/xmpp/xmppsocket.h"
#include "talk/xmpp/xmppclientsettings.h"
#include "xmpppump.h"

namespace gxmpp {

class XmppPump::PrivateTaskRunner
    : public talk_base::TaskRunner
    , public talk_base::MessageHandler
{
 public:
  PrivateTaskRunner() {};
  virtual ~PrivateTaskRunner() {};
  
  virtual void WakeTasks() final {
    talk_base::Thread::Current()->Post(this);
  }
  
  virtual int64 CurrentTime() final {
    return static_cast<int64>(talk_base::Time());
  }
  
  virtual void OnMessage(talk_base::Message* msg) final {
    RunTasks();
  }
};
  
XmppPump::XmppPump(XmppPumpNotify * notify)
    : notify_(notify) 
    , state_(buzz::XmppEngine::STATE_NONE) {
  // nil
  auth_.reset(new XmppAuth);
  task_runner_.reset(new PrivateTaskRunner);
}
    
XmppPump::~XmppPump () {
  // nil
}
        
buzz::XmppClient *XmppPump::client() {
  if (buzz::XmppEngine::STATE_OPEN != state_) {
    return nullptr;
  }
  return client_.get();
}

void XmppPump::DoLogin(const buzz::XmppClientSettings & xcs) {
  client_.reset(new buzz::XmppClient(task_runner_.get()));
  socket_.reset(new buzz::XmppSocket(xcs.use_tls()));
  if (!task_runner_->AllChildrenDone()) {
    OnStateChange(buzz::XmppEngine::STATE_START);
    client_->SignalStateChange.connect(this, &XmppPump::OnStateChange);
    client_->Connect(xcs, "en", socket_.get(), auth_.get());
    client_->Start();
  }
}

void XmppPump::DoDisconnect() {
  if (!task_runner_->AllChildrenDone()) {
    client_->Disconnect();
  }
  //OnStateChange(buzz::XmppEngine::STATE_CLOSED);
  client_->SignalStateChange.disconnect(this);
}

void XmppPump::OnStateChange(buzz::XmppEngine::State state) {
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__ << " state: " << state;
  if (state_ == state) {
    return;
  }
  state_ = state;
  if (notify_ != NULL) {
    notify_->OnStateChange(state);
  }
}

buzz::XmppReturnStatus XmppPump::SendStanza(const buzz::XmlElement *stanza) {
  if (!task_runner_->AllChildrenDone()) {
    return client_->SendStanza(stanza);
  }
  return buzz::XMPP_RETURN_BADSTATE;
}
} // namespace gxmpp
