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
#ifndef _ECHO_XMPPTHREAD_H_
#define _ECHO_XMPPTHREAD_H_

#include "talk/xmpp/xmppthread.h"
#include "talk/xmpp/pingtask.h"
#include "talk/xmpp/presenceouttask.h"
#include "sendtask.h"
#include "receivetask.h"

namespace echo {
  class EchoThread : public buzz::XmppThread
      , public sigslot::has_slots<>
  {
 public:
    EchoThread(){
      //message_queue_.reset(new talk_base::MessageQueue);
      //client()->SignalStateChange.connect(this, &EchoThread::OnMessage);
    };
    ~EchoThread(){};
    buzz::XmppReturnStatus Send(const buzz::Jid& to, const std::string& message);
    virtual void OnStateChange(buzz::XmppEngine::State state);
    void OnMessage(const buzz::Jid& from,
                   const buzz::Jid& to,
                   const std::string& message);
    /* data */
 private:
    talk_base::scoped_ptr<buzz::PingTask> ping_task_; 
    // We send presence information through this object.
    talk_base::scoped_ptr<buzz::PresenceOutTask> presence_out_task_;
    talk_base::scoped_ptr<echo::SendTask> send_task_;
    talk_base::scoped_ptr<echo::ReceiveTask> receive_task_;
    //talk_base::scoped_ptr<talk_base::MessageQueue> message_queue_;
    talk_base::MessageQueue message_queue_;
    DISALLOW_EVIL_CONSTRUCTORS(EchoThread);
  };
}
#endif  // _ECHO_XMPPTHREAD_H_