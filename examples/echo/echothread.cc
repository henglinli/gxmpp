#include "talk/base/logging.h"
#include "talk/xmpp/pingtask.h"
#include "receivetask.h"
#include "echothread.h"

namespace echo {
// EchoThread::
EchoThread::EchoThread() {
}

EchoThread::~EchoThread() {
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
}

void EchoThread::OnXmppOpen() {
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
  // presence out
#if 1
  presence_out_task_.reset(new buzz::PresenceOutTask(client()));  
  presence_out_task_->Start();      
  buzz::PresenceStatus presence_status;
  presence_status.set_jid(client()->jid());
  presence_status.set_available(true);
  presence_status.set_show(buzz::PresenceStatus::SHOW_ONLINE);
  presence_out_task_->Send(presence_status);
#endif
  // send
#if 1
  send_task_.reset(new echo::SendTask(client()));
  send_task_->Start();
#endif
  // receive
#if 1
  receive_task_.reset(new echo::ReceiveTask(client()));
  receive_task_->SignalReceived.connect(this, &EchoThread::OnXmppMessage);
  receive_task_->Start();
#endif
  // ping
#if 1
  ping_task_.reset(new buzz::PingTask(client(), 
                                      &message_queue_,
                                      8800,
                                      2400));
  ping_task_->Start();
#endif
}

void EchoThread::OnXmppClosed() {
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
  LOG(LS_SENSITIVE) << "Error " << client()->GetError(NULL);
  LOG(LS_SENSITIVE) << "IsDOne " << receive_task_->IsDone();
  // stop task_;
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
