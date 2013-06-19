#include "talk/xmpp/xmppauth.h"
#include "talk/base/logging.h"
#include "talk/xmpp/pingtask.h"
#include "receivetask.h"
#include "echothread.h"

namespace echo {
namespace {

const uint32 MSG_LOGIN = 1;
const uint32 MSG_DISCONNECT = 2;
const uint32 MSG_XMPPOPEN = 3;
const uint32 MSG_XMPPMESSAGE = 4;
const uint32 MSG_XMPPCLOSED = 5;

struct LoginData: public talk_base::MessageData {
  LoginData(const buzz::XmppClientSettings& s) : xcs(s) {}
  virtual ~LoginData() {}

  buzz::XmppClientSettings xcs;
};

} // namespace

// EchoThread::
EchoThread::EchoThread()
    : ping_task_(NULL)
    , presence_out_task_(NULL)
    , send_task_(NULL)
    , receive_task_(NULL)
    , message_queue_()
    , xmpp_handler_(NULL)
    , socket_(NULL)
    , socket_closed_(false) {
  //nil
}

EchoThread::~EchoThread() {
  RemoveXmppHandler();
}

void EchoThread::ProcessMessages(int cms) {
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
  talk_base::Thread::ProcessMessages(cms);
}

void EchoThread::Login(const buzz::XmppClientSettings& xcs) {
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
  Post(this, MSG_LOGIN, new LoginData(xcs));
}

void EchoThread::Disconnect() {
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
  Post(this, MSG_DISCONNECT);
}

void EchoThread::OnMessage(talk_base::Message* pmsg) {
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
  switch(pmsg->message_id) {
    case MSG_LOGIN : {
      LOG(LS_SENSITIVE) << "MSG_LOGIN";
      xmpp_pump_.reset(new buzz::XmppPump(this));
      ASSERT(pmsg->pdata != NULL);
      LoginData* data = reinterpret_cast<LoginData*>(pmsg->pdata);
      socket_ = new buzz::XmppSocket(buzz::TLS_DISABLED);
      xmpp_pump_->DoLogin(data->xcs,
                          socket_,
                          new XmppAuth);
      //socket_->SignalCloseEvent.connect(this, &EchoThread::OnXmppSocketClose);
      delete data;
      break;
    }
    case MSG_DISCONNECT : {
      LOG(LS_SENSITIVE) << "MSG_DISCONNECT";
      xmpp_pump_->DoDisconnect();
      socket_ = NULL;
      break;
    }
    case MSG_XMPPOPEN : {
      break;
    }
    case MSG_XMPPMESSAGE : {
      break;
    }
    case MSG_XMPPCLOSED : {
      break;
    }
    default : {
      ASSERT(false);
    }
  }
}

void EchoThread::RegisterXmppHandler(XmppHandler *xmpp_handler) {
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
  xmpp_handler_ = xmpp_handler;
  SignalXmppMessage.connect(xmpp_handler_, &XmppHandler::DoOnXmppMessage);
  SignalXmppOpen.connect(xmpp_handler_, &XmppHandler::DoOnXmppOpen);
  SignalXmppClosed.connect(xmpp_handler_, &XmppHandler::DoOnXmppClosed);
}

void EchoThread::RemoveXmppHandler()
{
  if (xmpp_handler_)
  {
    SignalXmppMessage.disconnect(xmpp_handler_);
    SignalXmppOpen.disconnect(xmpp_handler_);
    SignalXmppClosed.disconnect(xmpp_handler_);
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
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;                         
  if(xmpp_handler_) {
    std::string response;
    xmpp_handler_->DoOnXmppMessage(from, to, message, &response);
    //SignalXmppMessage(from, to, message, &response);
    if(xmpp_handler_->Response()) {
      if (client()->jid() == to) {
        send_task_->Send(from, response);
      }
    }
  }
}

void EchoThread::OnXmppOpen() {
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
  // presence out
#define PRESENCEOUT
#ifdef PRESENCEOUT
  presence_out_task_ = new buzz::PresenceOutTask(client());
  presence_out_task_->Start();      
  buzz::PresenceStatus presence_status;
  presence_status.set_jid(client()->jid());
  presence_status.set_available(true);
  presence_status.set_show(buzz::PresenceStatus::SHOW_ONLINE);
  presence_out_task_->Send(presence_status);
#endif //PRESENCEOUT
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
#define RECEIVE
#ifdef RECEIVE
  ping_task_ = new buzz::PingTask(client(), 
                                  talk_base::Thread::Current(),
                                  16000,
                                  5000);
  ping_task_->SignalTimeout.connect(this, &EchoThread::OnPingTimeout);
  ping_task_->Start();
#endif // RECEIVE
  if(xmpp_handler_) {
    //xmpp_handler_->OnXmppOpen();
    SignalXmppOpen();
  }
}

void EchoThread::OnXmppSocketClose(int state) {
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
  //Extra clean up for a socket close that wasn't originated by a logout.
  buzz::XmppEngine::State current = xmpp_pump_->client()->GetState();
  if (!socket_closed_ && current != buzz::XmppEngine::STATE_CLOSED) {
    Post(this, MSG_DISCONNECT);
  }
}

void EchoThread::OnXmppClosed() {
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
  socket_closed_ = true;
  //receive_task_->SignalReceived.disconnect(this);
  //ping_task_->SignalTimeout.disconnect(this);
  //socket_->SignalCloseEvent.disconnect(this);
  if(xmpp_handler_) {
    //xmpp_handler_->OnXmppClosed(client()->GetError(NULL));
    SignalXmppClosed(client()->GetError(NULL));
  }
}

void EchoThread::OnPingTimeout() {
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
  Post(this, MSG_DISCONNECT);
}

void EchoThread::OnStateChange(buzz::XmppEngine::State state) { 
  switch(state) {
    case buzz::XmppEngine::STATE_START: {
      LOG(LS_SENSITIVE) << "STATE_START";      
      break;
    }
    case buzz::XmppEngine::STATE_OPENING: {
      LOG(LS_SENSITIVE) << "STATE_OPENING";
      break;
    }
    case buzz::XmppEngine::STATE_OPEN: {
      LOG(LS_SENSITIVE) << "STATE_OPEN";
      OnXmppOpen();
      break;
    }
    case buzz::XmppEngine::STATE_CLOSED: {
      LOG(LS_SENSITIVE) << "STATE_CLOSED";
      OnXmppClosed();
      break;
    }
    default: {
      ASSERT(false);
    }
  }
}
}
