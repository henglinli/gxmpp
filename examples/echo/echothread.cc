#include "talk/base/logging.h"
#include "talk/xmpp/pingtask.h"
#include "receivetask.h"
#include "echothread.h"

namespace echo {
// EchoThread::
EchoThread::EchoThread()
    : ping_task_(NULL)
    , presence_out_task_(NULL)
    , send_task_(NULL)
    , receive_task_(NULL)
    , message_queue_()
    , xmpp_handler_(NULL)
{
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
}

EchoThread::~EchoThread() {
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
}

void EchoThread::RegisterXmppHandler(XmppHandler *xmpp_handler) {
  xmpp_handler_ = xmpp_handler;
}

buzz::XmppReturnStatus EchoThread::Send(const buzz::Jid& to, const std::string& message) {
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
  LOG(LS_SENSITIVE) << message << " From " << from.Str() << " To " << to.Str();
  if(xmpp_handler_) {
    if(xmpp_handler_->Response()) {
      std::string response = xmpp_handler_->OnXmppMessage(from, to, message);
      Send(from, response);
    } else {
      xmpp_handler_->OnXmppMessage(from, to, message);
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
                                  &message_queue_,
                                  8800,
                                  2400);
  ping_task_->Start();
#endif // RECEIVE
  if(xmpp_handler_) {
    xmpp_handler_->OnXmppOpen();
  }
}

void EchoThread::OnXmppClosed() {
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
  LOG(LS_SENSITIVE) << "Error " << client()->GetError(NULL);
  // stop task_;
  if(xmpp_handler_) {
    xmpp_handler_->OnXmppClosed(client()->GetError(NULL));
  }
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
      break;
    }
  }
}
}
