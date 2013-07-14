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

#include <vector>
#include <map>
#include "talk/base/logging.h"
#include "talk/xmpp/xmppauth.h"
#include "talk/xmpp/xmppsocket.h"
#include "talk/xmpp/pingtask.h"
#include "talk/xmpp/rostermoduleimpl.h"
#include "rosterhandler.h"
#include "receivetask.h"
#include "echothread.h"

namespace echo {
namespace {

const uint32 MSG_LOGIN = 1;
const uint32 MSG_DISCONNECT = 2;
const uint32 MSG_XMPPSTART = 3;
const uint32 MSG_XMPPOPENING = 4;
const uint32 MSG_XMPPOPEN = 5;
const uint32 MSG_XMPPMESSAGE = 6;
const uint32 MSG_XMPPCLOSED = 7;

buzz::XmppClientSettings xcs_;
buzz::XmppEngine::Error error_;
buzz::Jid from_;
buzz::Jid to_;
std::string message_;
} // namespace
// EchoThread::Module
class EchoThread::Module
{
 public:
  Module() {
    roster_module_.reset(buzz::XmppRosterModule::Create());
    roster_module_->set_roster_handler(&roster_handler_);    
  }
  virtual ~Module() {
  }
  void Start(buzz::XmppEngine *engine) {
    roster_module_->RegisterEngine(engine);
    roster_module_->BroadcastPresence();
    roster_module_->RequestRosterUpdate();
  }
 private:
  RosterHandler roster_handler_;
  talk_base::scoped_ptr<buzz::XmppRosterModule> roster_module_;
};
// EchoThread::Task
class EchoThread::Task
{
 public:
  Task(){}
  virtual ~Task(){}
 private:
  //buzz::PingTask *ping_task_;
  //echo::SendTask *send_task_;
  //echo::ReceiveTask *receive_task_;
};
// EchoThread::
EchoThread::EchoThread()
    : module_(NULL)
    , ping_task_(NULL)
    , presence_out_task_(NULL)
    , presence_receive_task_(NULL)
    , send_task_(NULL)
    , receive_task_(NULL)
    , xmpp_handler_(NULL) {
  // nil
}
EchoThread::~EchoThread() {
  // nil
  if (xmpp_handler_) {
    SignalXmppOpen.disconnect(xmpp_handler_);
    SignalXmppClosed.disconnect(xmpp_handler_);
  }
  xmpp_handler_ = NULL;
}
void EchoThread::ProcessMessages(int cms) {
  talk_base::Thread::ProcessMessages(cms);
}

void EchoThread::Login(const buzz::XmppClientSettings& xcs) {
  module_.reset(new Module);
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
  xcs_ = xcs;
  Post(this, MSG_LOGIN);
}

void EchoThread::Disconnect() {
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
  module_.reset();
  Post(this, MSG_DISCONNECT);
}

void EchoThread::OnMessage(talk_base::Message* pmsg) {
  switch(pmsg->message_id) {
    case MSG_LOGIN : {
#ifndef SELF_XMPP_PUMP
      xmpp_pump_.reset(new buzz::XmppPump(this));
#else
      xmpp_pump_.reset(new XmppPump(this));
#endif      
      xmpp_pump_->DoLogin(xcs_, new buzz::XmppSocket(xcs_.use_tls()), new XmppAuth);
      break;
    }
    case MSG_DISCONNECT : {
      xmpp_pump_->DoDisconnect();
      break;
    }
    case MSG_XMPPSTART: {
      OnXmppStart();
      break;
    }
    case MSG_XMPPOPENING: {
      OnXmppOpening();
      break;
    } 
    case MSG_XMPPOPEN : {
      OnXmppOpen();
      break;
    }
    case MSG_XMPPMESSAGE : {
      OnXmppMessage();
      break;
    }
    case MSG_XMPPCLOSED : {
      OnXmppClosed();
      break;
    }
    default : {
      ASSERT(false);
    }
  }
}

void EchoThread::RegisterXmppHandler(XmppHandler *xmpp_handler) {
  xmpp_handler_ = xmpp_handler;
  if (xmpp_handler_) {
    SignalXmppOpen.connect(xmpp_handler_, &XmppHandler::DoOnXmppOpen);
    SignalXmppClosed.connect(xmpp_handler_, &XmppHandler::DoOnXmppClosed);
  }
}

buzz::XmppReturnStatus EchoThread::SendXmppMessage(const buzz::Jid& to,
                                                   const std::string& message) {
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
  // Make sure we are actually connected.
  if (client()->GetState() != buzz::XmppEngine::STATE_OPEN) {
    return buzz::XMPP_RETURN_BADSTATE;
  }
  return send_task_->Send(to, message);
}

void EchoThread::OnXmppMessage(const buzz::Jid& from,
                               const buzz::Jid& to,
                               const std::string& message) {
  from_.CopyFrom(from);
  to_.CopyFrom(to);
  message_ = message;

  Post(this, MSG_XMPPMESSAGE);
}

void EchoThread::OnXmppMessage()
{
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
  static std::string response;
  if(xmpp_handler_) {
    xmpp_handler_->DoOnXmppMessage(from_, to_, message_, &response);
    if(xmpp_handler_->Response()) {
      if (client()->jid() != from_) {
        send_task_->Send(from_, response);
      }
    }
  }
}

void EchoThread::OnXmppStart() {
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
  //module_.reset(new Module(client()->engine()));
}

void EchoThread::OnXmppOpening() {
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
  //module_.reset(new Module(client()->engine()));
}

void EchoThread::OnXmppOpen() {
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
#define ROSTER
#ifdef ROSTER
  module_->Start(client()->engine());
#if 0
  roster_module_ = buzz::XmppRosterModule::Create();
  roster_module_->set_roster_handler(&roster_handler_);
  roster_module_->RegisterEngine(client()->engine());
  roster_module_->BroadcastPresence();
  roster_module_->RequestRosterUpdate();
#endif
#endif
  // presence out
  //#define PRESENCEOUT
#ifdef PRESENCEOUT
  presence_out_task_ = new buzz::PresenceOutTask(client());
  presence_out_task_->Start();      
  buzz::PresenceStatus presence_status;
  presence_status.set_jid(client()->jid());
  presence_status.set_available(true);
  presence_status.set_show(buzz::PresenceStatus::SHOW_ONLINE);
  presence_out_task_->Send(presence_status);
#endif //PRESENCEOUT
  //#define PRESENCERECV
#ifdef PRESENCERECV
  presence_receive_task_ = new buzz::PresenceReceiveTask(client());
#endif
  // send
#define SEND
#ifdef SEND
  send_task_ = new echo::SendTask(client());
  send_task_->Start();
#endif //SEND
  // receive
#define RECEIVE
#ifdef RECEIVE
  receive_task_ = new echo::ReceiveTask(client());
  receive_task_->SignalReceived.connect(this, &EchoThread::OnXmppMessage);
  receive_task_->Start();
#endif // RECEIVE
  // ping
#define PING
#ifdef PING
  ping_task_ = new buzz::PingTask(client(), 
                                  talk_base::Thread::Current(),
                                  16000,
                                  5000);
  ping_task_->SignalTimeout.connect(this, &EchoThread::OnPingTimeout);
  ping_task_->Start();
#endif // PING
  if (xmpp_handler_) {
    //xmpp_handler_->DoOnXmppOpen();
    SignalXmppOpen();
  }
}

void EchoThread::OnXmppClosed() {
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
  if(xmpp_handler_) {
    SignalXmppClosed(error_);
    //xmpp_handler_->DoOnXmppClosed(client()->GetError(NULL));
  }
}

void EchoThread::OnPingTimeout() {
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
  Post(this, MSG_DISCONNECT);
}

void EchoThread::OnStateChange(buzz::XmppEngine::State state) { 
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
      error_ = client()->GetError(NULL);
      Post(this, MSG_XMPPCLOSED);
      break;
    }
    default: {
      ASSERT(false);
    }
  }
}
// NewEchoThread;
void NewEchoThread::RegisterXmppHandler(XmppHandler *xmpp_handler) {
  xmpp_handler_ = xmpp_handler;
  if (xmpp_handler_) {
    SignalXmppOpen.connect(xmpp_handler_, &XmppHandler::DoOnXmppOpen);
    SignalXmppClosed.connect(xmpp_handler_, &XmppHandler::DoOnXmppClosed);
  }
}
buzz::XmppReturnStatus NewEchoThread::SendXmppMessage(const buzz::Jid& to,
                                                      const std::string& message) {
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
  // Make sure we are actually connected.
  if (state() != buzz::XmppEngine::STATE_OPEN) {
    return buzz::XMPP_RETURN_BADSTATE;
  }
  return send_task_->Send(to, message);
}
void NewEchoThread::DoOnPingTimeout() {
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
  Post(this, MSG_DISCONNECT);
}

void NewEchoThread::DoOnXmppOpen() {
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
  // send
#define SEND
#ifdef SEND
  send_task_ = new echo::SendTask(client());
  send_task_->Start();
#endif //SEND
  // receive
#define RECEIVE
#ifdef RECEIVE
  receive_task_ = new echo::ReceiveTask(client());
  receive_task_->SignalReceived.connect(this, &NewEchoThread::DoOnXmppMessage);
  receive_task_->Start();
#endif // RECEIVE
  // ping
#define PING
#ifdef PING
  ping_task_ = new buzz::PingTask(client(), 
                                  talk_base::Thread::Current(),
                                  16000,
                                  5000);
  ping_task_->SignalTimeout.connect(this, &NewEchoThread::DoOnPingTimeout);
  ping_task_->Start();
#endif // PING
  if (xmpp_handler_) {
    //xmpp_handler_->DoOnXmppOpen();
    SignalXmppOpen();
  }
}
void NewEchoThread::DoOnXmppClosed() {
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
  if (xmpp_handler_) {
    //xmpp_handler_->DoOnXmppClosed(error_);
    SignalXmppClosed(error());
  }
}
void NewEchoThread::DoOnXmppMessage(const buzz::Jid& from,
                                    const buzz::Jid& to,
                                    const std::string& message) {
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
  static std::string response;
  if(xmpp_handler_) {
    xmpp_handler_->DoOnXmppMessage(from, to, message, &response);
    if(xmpp_handler_->Response()) {
      if (client()->jid() != from) {
        send_task_->Send(from, response);
      }
    }
  }
}
} // namespace echo
