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

#include "talk/base/logging.h"
#include "talk/base/timeutils.h"
#include "talk/base/taskrunner.h"
#include "talk/xmpp/xmppengine.h"
#include "talk/xmpp/rostermodule.h"
#include "talk/xmpp/xmppclientsettings.h"
#include "pumpthread.h"

namespace gxmpp {
namespace {
const int kDefaultXmppPort = 5222;

const uint32 MSG_LOGIN = 1;
const uint32 MSG_DISCONNECT = 2;
const uint32 MSG_XMPPSTART = 3;
const uint32 MSG_XMPPOPENING = 4;
const uint32 MSG_XMPPOPEN = 5;
const uint32 MSG_XMPPCLOSED = 6;
} // namspace
  
// PrivateTaskRunner
class PumpThread::PrivateTaskRunner
    : NonCopyable
    , public talk_base::TaskRunner
    , public talk_base::MessageHandler
{
 public:
  PrivateTaskRunner() {
  };
  virtual ~PrivateTaskRunner() {
  };

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
  
// PrivateRosterModule
class PumpThread::PrivateRosterModule
// : NonCopyable
    : public buzz::XmppRosterHandler
{
 public:
  PrivateRosterModule() {
    roster_module_.reset(buzz::XmppRosterModule::Create());
    roster_module_->set_roster_handler(this);
  }
  virtual ~PrivateRosterModule() {
    // nil
  }
  void Start(buzz::XmppEngine *engine) {
    roster_module_->RegisterEngine(engine);
    roster_module_->BroadcastPresence();
    roster_module_->RequestRosterUpdate();
  }
  //! A request for a subscription has come in.
  //! Typically, the UI will ask the user if it is okay to let the requester
  //! get presence notifications for the user.  The response is send back
  //! by calling ApproveSubscriber or CancelSubscriber.
  virtual void SubscriptionRequest(buzz::XmppRosterModule* roster,
                                   const buzz::Jid& requesting_jid,
                                   buzz::XmppSubscriptionRequestType type,
                                   const buzz::XmlElement* raw_xml) {
    LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
    roster->ApproveSubscriber(requesting_jid);
  }
  //! Some type of presence error has occured
  virtual void SubscriptionError(buzz::XmppRosterModule* roster,
                                 const buzz::Jid& from,
                                 const buzz::XmlElement* raw_xml) {
    LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
  }

  virtual void RosterError(buzz::XmppRosterModule* roster,
                           const buzz::XmlElement* raw_xml) {
    LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
  }

  //! New presence information has come in
  //! The user is notified with the presence object directly.  This info is also
  //! added to the store accessable from the engine.
  virtual void IncomingPresenceChanged(buzz::XmppRosterModule* roster,
                                       const buzz::XmppPresence* presence) {
    LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
  }

  //! A contact has changed
  //! This indicates that the data for a contact may have changed.  No
  //! contacts have been added or removed.
  virtual void ContactChanged(buzz::XmppRosterModule* roster,
                              const buzz::XmppRosterContact* old_contact,
                              size_t index) {
    LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
  }

  //! A set of contacts have been added
  //! These contacts may have been added in response to the original roster
  //! request or due to a "roster push" from the server.
  virtual void ContactsAdded(buzz::XmppRosterModule* roster,
                             size_t index, size_t number) {
    LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
  }

  //! A contact has been removed
  //! This contact has been removed form the list.
  virtual void ContactRemoved(buzz::XmppRosterModule* roster,
                              const buzz::XmppRosterContact* removed_contact,
                              size_t index) {
    LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
  }
 private:
  std::shared_ptr<buzz::XmppRosterModule> roster_module_;
};

//PumpThread
PumpThread::PumpThread() 
    : error_(buzz::XmppEngine::ERROR_NONE)
    , state_(buzz::XmppEngine::STATE_NONE) {
  // nil
}

PumpThread::~PumpThread() {
  Stop();
}

bool PumpThread::Init(const std::string &jid,
                      const std::string &password,
                      const std::string &server)
{
  buzz::Jid tmp_jid(jid);
  if(!tmp_jid.IsValid()) {
    return false;
  }
  talk_base::InsecureCryptStringImpl insecure;
  insecure.password() = password;

  xcs_.set_user(tmp_jid.node());
  xcs_.set_pass(talk_base::CryptString(insecure));
  xcs_.set_host(tmp_jid.domain());
  xcs_.set_resource("gxmpp_bot");
  xcs_.set_use_tls(buzz::TLS_DISABLED);
  if("" != server) {
    xcs_.set_server(talk_base::SocketAddress(server, kDefaultXmppPort));
  } else {
    xcs_.set_server(talk_base::SocketAddress(tmp_jid.domain(), kDefaultXmppPort));
  }
  task_runner_.reset(new PrivateTaskRunner);    
  //talk_base::LogMessage::LogToDebug(talk_base::LS_SENSITIVE);
  Start();
  return true;
}

buzz::XmppClient* PumpThread::client() {
  if(buzz::XmppEngine::STATE_OPEN != state_) {
    return nullptr;
  }
  return client_;
}

buzz::XmppEngine::State PumpThread::state() {
  return state_;
}

buzz::XmppEngine::Error PumpThread::error() {
  return error_;
}

void PumpThread::Login() {
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
  Post(this, MSG_LOGIN);
}

void PumpThread::Disconnect() {
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
  Post(this, MSG_DISCONNECT);
}

void PumpThread::OnMessage(talk_base::Message* pmsg) {
  switch(pmsg->message_id) {
    case MSG_LOGIN : {
      DoLogin();
      break;
    }
    case MSG_DISCONNECT : {
      DoDisconnect();
      break;
    }
    case MSG_XMPPSTART: {
      DoOnXmppStart();
      break;
    }
    case MSG_XMPPOPENING: {
      DoOnXmppOpening();
      break;
    }
    case MSG_XMPPOPEN : {
      DoOnXmppOpen();
      break;
    }
    case MSG_XMPPCLOSED : {
      DoOnXmppClosed();
      break;
    }
    default : {
      ASSERT(false);
    }
  }
}

void PumpThread::DoLogin() {
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
#ifdef REUSE
  client_.reset(new buzz::XmppClient(task_runner_.get()));
  socket_.reset(new buzz::XmppSocket(xcs_.use_tls()));
  auth_.reset(new XmppAuth);
#else
  client_ = new buzz::XmppClient(task_runner_.get());
  socket_ = new buzz::XmppSocket(xcs_.use_tls());
  auth_ = new XmppAuth;
#endif
  if (!task_runner_->AllChildrenDone()) {
    OnStateChange(buzz::XmppEngine::STATE_START);
    client_->SignalStateChange.connect(this, &PumpThread::OnStateChange);
#ifdef REUSE
    client_->Connect(xcs_, "en", socket_.get(), auth_.get());
#else
    client_->Connect(xcs_, "en", socket_, auth_);
#endif
    client_->Start();
  }
  roster_module_.reset(new PrivateRosterModule);
  roster_module_->Start(client_->engine());
}

void PumpThread::DoDisconnect() {
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
  if (!task_runner_->AllChildrenDone()) {
    client_->Disconnect();
  }
  client_->SignalStateChange.disconnect(this);
}

void PumpThread::OnStateChange(buzz::XmppEngine::State state) {
  state_ = state;
  switch(state) {
    case buzz::XmppEngine::STATE_START: {
      //Post(this, MSG_XMPPSTART);
      break;
    }
    case buzz::XmppEngine::STATE_OPENING: {
      //Post(this, MSG_XMPPOPENING);
      break;
    }
    case buzz::XmppEngine::STATE_OPEN: {     
      Post(this, MSG_XMPPOPEN);
      break;
    }
    case buzz::XmppEngine::STATE_CLOSED: {
      error_ = client_->GetError(nullptr);      
      Post(this, MSG_XMPPCLOSED);
      break;
    }
    default: {
      ASSERT(false);
    }
  }
}

void PumpThread::DoOnXmppStart() {
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
}

void PumpThread::DoOnXmppOpening() {
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
}

void PumpThread::DoOnXmppOpen() {
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
  SignalXmppOpen();
}

void PumpThread::DoOnXmppClosed() {
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
  SignalXmppClosed(error_);
}

} // namepace gxmpp
