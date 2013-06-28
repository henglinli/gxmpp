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

#ifndef _GXMPP_XMPPPUMP_H_
#define _GXMPP_XMPPPUMP_H_

#include <memory>
#include "talk/xmpp/xmppclient.h"

namespace gxmpp {
  
struct NonCopyable {
  NonCopyable & operator=(const NonCopyable&) = delete;
  NonCopyable(const NonCopyable&) = delete;
  NonCopyable() = default;
};

// Simple xmpp pump thread
class XmppPumpNotify : public NonCopyable {
public:
  XmppPumpNotify() {}
  virtual ~XmppPumpNotify() {}
  virtual void OnStateChange(buzz::XmppEngine::State state) = 0;
};

class XmppPump
    : public sigslot::has_slots<>
    , public NonCopyable
{
public:
  explicit XmppPump(XmppPumpNotify *notify = NULL);
  virtual ~XmppPump();
  
  void DoLogin(const buzz::XmppClientSettings & xcs);
  void DoDisconnect();

  buzz::XmppClient *client();

  void OnStateChange(buzz::XmppEngine::State state);
 
  buzz::XmppReturnStatus SendStanza(const buzz::XmlElement *stanza);

  sigslot::signal1<buzz::XmppEngine::State> SignalXmppState;
private:
  XmppPumpNotify *notify_;
  buzz::XmppEngine::State state_;
  std::shared_ptr<buzz::XmppClient> client_;
  std::shared_ptr<buzz::AsyncSocket> socket_;
  std::shared_ptr<buzz::PreXmppAuth> auth_;
  class PrivateTaskRunner;
  friend class PrivateTaskRunner;
  std::shared_ptr<PrivateTaskRunner> task_runner_;
};

}  // namespace gxmpp

#endif // _GXMPP_XMPPPUMP_H_
