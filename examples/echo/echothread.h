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
#include "talk/xmpp/pingtask.h"
#include "talk/xmpp/presenceouttask.h"
#include "talk/xmpp/presencereceivetask.h"
//#define SELF_XMPP_PUMP
#ifndef SELF_XMPP_PUMP
#include "talk/xmpp/xmpppump.h"
#else
#include "xmpppump.h"
#endif
#include "sendtask.h"
#include "receivetask.h"
#include "rosterhandler.h"

namespace echo {
  class XmppHandler : public sigslot::has_slots<> {
 public:
 XmppHandler():response_(true) {};
    ~XmppHandler() {};   
    virtual void DoOnXmppMessage(const buzz::Jid& from,
                                 const buzz::Jid& to,
                                 const std::string& message,
                                 std::string* response) = 0;
    virtual void DoOnXmppOpen() = 0;
    virtual void DoOnXmppClosed(int error) = 0;
    
    inline void SetResponse(bool yes) {
      response_ = yes;
    }

    inline bool Response() {
      return response_;
    }
 private:
    bool response_;
    DISALLOW_EVIL_CONSTRUCTORS(XmppHandler);
  };
  
  class EchoThread
      : public talk_base::Thread
      , public talk_base::MessageHandler
#ifndef SELF_XMPP_PUMP
      , public buzz::XmppPumpNotify
#else 
      , public XmppPumpNotify
#endif
      , public sigslot::has_slots<>
  {
 public:
    // Slot for chat message callbacks
    sigslot::signal4<const buzz::Jid&,
        const buzz::Jid&,
        const std::string&,
        std::string*> SignalXmppMessage;
    sigslot::signal0<> SignalXmppOpen;
    sigslot::signal1<int> SignalXmppClosed;
 public:
    EchoThread();
    ~EchoThread();
   
    inline buzz::XmppClient* client() { return xmpp_pump_->client(); }

    void ProcessMessages(int cms);
    void RegisterXmppHandler(XmppHandler *xmpp_handler);
    void Login(const buzz::XmppClientSettings & xcs);
    buzz::XmppReturnStatus SendXmppMessage(const buzz::Jid& to, const std::string& message);
    void Disconnect();
 private:
    virtual void OnStateChange(buzz::XmppEngine::State state);
    virtual void OnXmppMessage(const buzz::Jid& from,
                               const buzz::Jid& to,
                               const std::string& message);
    virtual void OnXmppMessage();
    virtual void OnXmppStart();
    virtual void OnXmppOpening();
    virtual void OnXmppOpen();
    virtual void OnXmppClosed();
    virtual void OnPingTimeout();
    virtual void OnMessage(talk_base::Message* pmsg);
 private:
    class Module;
    friend class Module;
    talk_base::scoped_ptr<Module> module_;
    class Task;
    friend class Task;
    talk_base::scoped_ptr<Task> task_;
#ifndef SELF_XMPP_PUMP
    talk_base::scoped_ptr<buzz::XmppPump> xmpp_pump_;
#else
    talk_base::scoped_ptr<XmppPump> xmpp_pump_;
#endif
    buzz::PingTask *ping_task_;
    buzz::PresenceOutTask *presence_out_task_;
    buzz::PresenceReceiveTask *presence_receive_task_;
    echo::SendTask *send_task_;
    echo::ReceiveTask *receive_task_;
    XmppHandler *xmpp_handler_;
    DISALLOW_EVIL_CONSTRUCTORS(EchoThread);
  };
}
#endif  // _ECHO_XMPPTHREAD_H_
