#include "talk/base/logging.h"
#include "talk/xmpp/pingtask.h"
#include "receivetask.h"
#include "echothread.h"

namespace echo {
// Echo::
void Echo::OnMessage(const buzz::Jid& from,
                     const buzz::Jid& to,
                     const std::string& message) {
  LOG(LS_SENSITIVE) << message << " From " << from.Str() << " To " << to.Str();
}

void Echo::OnOpen() {
  LOG(LS_SENSITIVE) << "xmpp open done!";
  Send(buzz::Jid("li@xmpp.ikanke.cn"), "hello");
}

int Echo::OnClosed() {
  LOG(LS_SENSITIVE) << "xmpp Closed!";
  //return client()->GetError(NULL);  
}
// EchoThread::
buzz::XmppReturnStatus EchoThread::Send(const buzz::Jid& to, const std::string& message) {
  // Make sure we are actually connected.
  if (client()->GetState() != buzz::XmppEngine::STATE_OPEN) {
    return buzz::XMPP_RETURN_BADSTATE;
  }
  return send_task_->Send(to, message);
}

void EchoThread::OnXmppOpen() {     
      // presence out
      presence_out_task_.reset(new buzz::PresenceOutTask(client()));
      presence_out_task_->Start();      
      buzz::PresenceStatus presence_status;
      presence_status.set_jid(client()->jid());
      presence_status.set_available(true);
      presence_status.set_show(buzz::PresenceStatus::SHOW_ONLINE);
      presence_out_task_->Send(presence_status);
      // send
      send_task_.reset(new echo::SendTask(client()));
      send_task_->Start();
      // receive
      receive_task_.reset(new echo::ReceiveTask(client()));
      receive_task_->SignalReceived.connect(this, &EchoThread::OnMessage);
      receive_task_->Start();
      // ping
      ping_task_.reset(new buzz::PingTask(client(), 
                                          &message_queue_,
                                          8800,
                                          2400));
      ping_task_->Start();
}

void EchoThread::OnXmppClosed() {     
      // presence out
      buzz::PresenceStatus presence_status;
      presence_status.set_jid(client()->jid());
      presence_status.set_available(false);
      presence_status.set_show(buzz::PresenceStatus::SHOW_AWAY);
      presence_out_task_->Send(presence_status);
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
      OnOpen();
      break;
    }
    case buzz::XmppEngine::STATE_CLOSED: {
      LOG(LS_SENSITIVE) << "STATE_CLOSED";
      OnXmppClosed();
      OnClosed();
      break;
    }
    default: {
      break;
    }
  }
}

}
