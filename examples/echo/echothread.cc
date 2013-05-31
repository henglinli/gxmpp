#include <stdio.h>
#include "talk/xmpp/pingtask.h"

#include "receivetask.h"
#include "echothread.h"

namespace echo {
buzz::XmppReturnStatus EchoThread::Send(const buzz::Jid& to, const std::string& message) {
 return send_task_->Send(to, message);
}

void EchoThread::OnMessage(const buzz::Jid& from,
                           const buzz::Jid& to,
                           const std::string& message) {
  printf("%s from %s to %s\n", message.c_str(), from.Str().c_str(), to.Str().c_str());
}

void EchoThread::OnStateChange(buzz::XmppEngine::State state) { 
  //printf("STATE: %d\n", state);
  switch(state) {
    case buzz::XmppEngine::STATE_START: {
      printf("STATE_START\n");
      break;
    }
    case buzz::XmppEngine::STATE_OPENING: {
      printf("STATE_OPENING\n");
      break;
    }
    case buzz::XmppEngine::STATE_OPEN: {
      printf("STATE_OPEN\n");
#if 1
      ping_task_.reset(new buzz::PingTask(client(), 
                                          &message_queue_,
                                          88000,
                                          24000));
      ping_task_->Start();
#endif

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
      
      send_task_->Send(buzz::Jid("li@xmpp.ikanke.cn"), "hello");
      // receive      
      receive_task_.reset(new echo::ReceiveTask(client()));
      receive_task_->SignalReceived.connect(this, &EchoThread::OnMessage);
      receive_task_->Start();
      break;
    }
    case buzz::XmppEngine::STATE_CLOSED: {
      printf("STATE_CLOSED\nError: %u\n", this->client()->GetError(NULL));
      break;
    }
    default:
      break;
  }
}
}