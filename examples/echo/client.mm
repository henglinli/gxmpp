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

#include "echothread.h"
#import "client.h"
class ClientHandler : public echo::XmppHandler {
public:
  void DoOnXmppMessage(const buzz::Jid& from,
                       const buzz::Jid& to,
                       const std::string& message,
                       std::string* response) {
    if([delegate conformsToProtocol:@protocol(GxmppClientDelegate)]
       && [delegate respondsToSelector:@selector(didOnXmppMessage)]) {
      [delegate performSelector:@selector(didOnXmppMessage)];
    }
  }
  void DoOnXmppOpen() {
    if([delegate conformsToProtocol:@protocol(GxmppClientDelegate)]
       && [delegate respondsToSelector:@selector(didOnXmppOpen)]) {
      [delegate performSelector:@selector(didOnXmppOpen)];
    }
  }
  void DoOnXmppClosed(buzz::XmppEngine::Error error) {
    if([delegate conformsToProtocol:@protocol(GxmppClientDelegate)]
       && [delegate respondsToSelector:@selector(didOnXmppClosed)]) {
      [delegate performSelector:@selector(didOnXmppClosed)];
    }
  }
  id<GxmppClientDelegate> delegate;
};

@implementation GxmppClient {
@private
  NSString *_jid;
  NSString *_password;
  NSString *_server;
  echo::NewEchoThread thread_;
  ClientHandler handler_;
}

@synthesize jid = _jid;
@synthesize password = _password;
@synthesize server = _server;
  //dynamic delegate
@dynamic delegate;
-(id<GxmppClientDelegate>) delegate {
  return handler_.delegate;
}
-(void)setDelegate: (id<GxmppClientDelegate>) delegate {
  handler_.delegate = delegate;
}
  //
-(id)init {
  return [self initWithJid: @"" Password:@"" Server:@""];
}

-(id)initWithJid:(NSString *)jid Password:(NSString *)password Server:(NSString *)server {
  if ((self = [super init])) {
    _jid = jid;
    _password = password;
    _server = server;
    thread_.RegisterXmppHandler(&handler_);
    thread_.Init([_jid UTF8String], [_password UTF8String], [_server UTF8String]);
  }
  return self;
}

-(void) setResponse: (BOOL) yes {
  handler_.SetResponse(yes);
}

-(void) login {
  thread_.Login();
}
-(int)sendTo:(NSString *)to WithMessage:(NSString *)message {

  return 0;
}
-(void) logout {
  thread_.Disconnect();
}
@end
