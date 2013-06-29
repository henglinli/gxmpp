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


#ifndef __gxmpp__pumpthread__
#define __gxmpp__pumpthread__

#include <memory>
#include "talk/base/messagehandler.h"
#include "talk/base/thread.h"
#include "talk/xmpp/xmppclient.h"
#include "talk/xmpp/xmppauth.h"
#include "talk/xmpp/xmppsocket.h"

namespace gxmpp {
#if 1
  struct NonCopyable {
    NonCopyable & operator=(const NonCopyable&) = delete;
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable() = default;
  };
#endif
  class PumpThread
  : NonCopyable
  , public talk_base::MessageHandler
  , public talk_base::Thread
  , public sigslot::has_slots<> {
  public:
    PumpThread();
    virtual ~PumpThread();
    bool Init(const std::string &jid,
              const std::string &password,
              const std::string &server = "");

    void Login();
    void DoLogin();
    void Disconnect();
    void DoDisconnect();
    
    void OnStateChange(buzz::XmppEngine::State state);
    void OnMessage(talk_base::Message *pmsg);
    
    virtual void DoOnXmppStart();
    virtual void DoOnXmppOpening();
    virtual void DoOnXmppOpen();
    virtual void DoOnXmppClosed();

    sigslot::signal0<> SignalXmppOpen;
    sigslot::signal1<buzz::XmppEngine::Error> SignalXmppClosed;
  private:
    class PrivateTaskRunner;
    friend class PrivateTaskRunner;
    std::shared_ptr<PrivateTaskRunner> task_runner_;

    class PrivateRosterModule;
    friend class PrivateRosterModule;
    std::shared_ptr<PrivateRosterModule> roster_module_;
  };
} // gxmpp

#endif /* defined(__gxmpp__pumpthread__) */
