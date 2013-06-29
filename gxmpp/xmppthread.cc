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
#include "talk/xmpp/xmppengine.h"
#include "talk/xmpp/rostermodule.h"
#include "xmpppump.h"
#include "xmppthread.h"

namespace gxmpp {
namespace {
const int kDefaultXmppPort = 5222;

const uint32 MSG_LOGIN = 1;
const uint32 MSG_DISCONNECT = 2;
const uint32 MSG_XMPPSTART = 3;
const uint32 MSG_XMPPOPENING = 4;
const uint32 MSG_XMPPOPEN = 5;
const uint32 MSG_XMPPMESSAGE = 6;
const uint32 MSG_XMPPCLOSED = 7;

buzz::XmppEngine::Error error_ = buzz::XmppEngine::ERROR_NONE;
buzz::Jid from_;
buzz::Jid to_;
std::string message_;
buzz::XmppClientSettings xcs_;
} // namspace
// XmppThread::PrivateModule
class XmppThread::PrivateRosterModule
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
// XmppThread
XmppThread::XmppThread() {
    // nil
}

XmppThread::~XmppThread() {
  xmpp_pump_->SignalXmppState.disconnect(this);
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
  // nil
}
bool XmppThread::Init(const std::string &jid,
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

  xmpp_pump_.reset(new XmppPump);
  xmpp_pump_->SignalXmppState.connect(this, &XmppThread::OnStateChange);
  roster_module_.reset(new PrivateRosterModule);
    //roster_module_ = new PrivateRosterModule;
  talk_base::LogMessage::LogToDebug(talk_base::LS_SENSITIVE);
  Start();
  return true;
}

void XmppThread::Login() {
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
  Post(this, MSG_LOGIN);
}

void XmppThread::Disconnect() {
  Post(this, MSG_DISCONNECT);
}

void XmppThread::OnStateChange(buzz::XmppEngine::State state) {
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
        //error_ = client()->GetError(nullptr);
      error_ = xmpp_pump_->error();
      Post(this, MSG_XMPPCLOSED);
      break;
    }
    default: {
      ASSERT(false);
    }
  }
}

void XmppThread::OnMessage(talk_base::Message* pmsg) {
  switch(pmsg->message_id) {
    case MSG_LOGIN : {
      xmpp_pump_->DoLogin(xcs_);
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
void XmppThread::OnXmppMessage(const buzz::Jid& from,
                               const buzz::Jid& to,
                               const std::string& message) {
  from_.CopyFrom(from);
  to_.CopyFrom(to);
  message_ = message;

  Post(this, MSG_XMPPMESSAGE);
}

void XmppThread::OnXmppMessage()
{
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
}

void XmppThread::OnXmppStart() {
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
}

void XmppThread::OnXmppOpening() {
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
}

void XmppThread::OnXmppOpen() {
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
  roster_module_->Start(client()->engine());
}

void XmppThread::OnXmppClosed() {
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
}

void XmppThread::OnPingTimeout() {
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
  Post(this, MSG_DISCONNECT);
}

} // namespace gxmpp
