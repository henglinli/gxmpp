#include <vector>
#include <map>
#include "talk/xmpp/rostermoduleimpl.h"
#include "talk/xmpp/xmppauth.h"
#include "talk/xmpp/xmppsocket.h"
#include "talk/base/logging.h"
#include "talk/xmpp/pingtask.h"
#include "rosterhandler.h"
#include "receivetask.h"
#include "echothread.h"

namespace echo {
namespace {

const uint32 MSG_LOGIN = 1;
const uint32 MSG_DISCONNECT = 2;
const uint32 MSG_XMPPOPEN = 3;
const uint32 MSG_XMPPMESSAGE = 4;
const uint32 MSG_XMPPCLOSED = 5;

buzz::XmppClientSettings xcs_;
buzz::XmppEngine::Error error_;
buzz::Jid from_;
buzz::Jid to_;
std::string message_;
} // namespace

class EchoThread::Private
{
 public:
  Private(EchoThread *echothread) 
      : echothread_(echothread)
      , roster_module_(NULL)
      , roster_handler_(NULL) {
    roster_handler_ = new RosterHandler;
  };
  virtual ~Private(){
    delete roster_handler_;
    roster_handler_= NULL;
  };
  bool Init(buzz::XmppEngine *engine) {
    LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
    if (NULL != echothread_) {
      buzz::XmppClient *client = echothread_->xmpp_pump_->client();
      roster_module_ = buzz::XmppRosterModuleImpl::Create();
      roster_module_->RegisterEngine(client->engine());
      roster_module_->set_roster_handler(roster_handler_);
      roster_module_->BroadcastPresence();
      roster_module_->RequestRosterUpdate();
      return true;
    } else {
      if (engine){
        roster_module_ = buzz::XmppRosterModuleImpl::Create();
        roster_module_->RegisterEngine(engine);
        roster_module_->set_roster_handler(roster_handler_);
        roster_module_->BroadcastPresence();
        roster_module_->RequestRosterUpdate();
        return true;
      }
      return false;
    }
  }
 private:
  // the owner
  EchoThread const *echothread_;
  buzz::XmppRosterModule *roster_module_;
  buzz::XmppRosterHandler *roster_handler_;
};
// EchoThread::
EchoThread::EchoThread()
    : ping_task_(NULL)
    , presence_out_task_(NULL)
    , presence_receive_task_(NULL)
    , send_task_(NULL)
    , receive_task_(NULL)
    , message_queue_()
    , xmpp_handler_(NULL) {
  //nil
}

EchoThread::~EchoThread() {
  // nil
}

void EchoThread::ProcessMessages(int cms) {
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
  talk_base::Thread::ProcessMessages(cms);
}

void EchoThread::Login(const buzz::XmppClientSettings& xcs) {
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
  xcs_ = xcs;
  Post(this, MSG_LOGIN);
}

void EchoThread::Disconnect() {
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
  Post(this, MSG_DISCONNECT);
}

void EchoThread::OnMessage(talk_base::Message* pmsg) {
  switch(pmsg->message_id) {
    case MSG_LOGIN : {
      LOG(LS_SENSITIVE) << "MSG_LOGIN";
     
#ifndef SELF_XMPP_PUMP
      xmpp_pump_.reset(new buzz::XmppPump(this));

      xmpp_pump_->DoLogin(xcs_, new buzz::XmppSocket(xcs_.use_tls()), new XmppAuth);
#else
      xmpp_pump_.reset(new XmppPump(this));
      xmpp_pump_->DoLogin(xcs_);
#endif
      break;
    }
    case MSG_DISCONNECT : {
      LOG(LS_SENSITIVE) << "MSG_DISCONNECT";
      xmpp_pump_->DoDisconnect();
      break;
    }
    case MSG_XMPPOPEN : {
      LOG(LS_SENSITIVE) << "MSG_XMPPOPEN";
      OnXmppOpen();
      break;
    }
    case MSG_XMPPMESSAGE : {
      LOG(LS_SENSITIVE) << "MSG_XMPPMESSAGE";
      OnXmppMessage();
      break;
    }
    case MSG_XMPPCLOSED : {
      LOG(LS_SENSITIVE) << "MSG_XMPPCLOSED";
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

buzz::XmppReturnStatus EchoThread::Send(const buzz::Jid& to, const std::string& message) {
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

void EchoThread::OnXmppOpen() {
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
  // presence out
#define ROSTER
#ifdef ROSTER
  private_.reset(new Private(NULL));
  private_->Init(client()->engine());
#endif
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
#define PRESENCERECV
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
      break;
    }
    case buzz::XmppEngine::STATE_OPENING: {
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
}
